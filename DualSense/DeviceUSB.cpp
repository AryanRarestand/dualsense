#include "DeviceUSB.h"


namespace DeviceUSB
{
    VOID
    DualSenseEvtUsbInterruptPipeReadComplete(
            WDFUSBPIPE  Pipe,
            WDFMEMORY   Buffer,
            size_t      NumBytesTransferred,
            WDFCONTEXT  Context
    )
    {
        PDEVICE_CONTEXT     deviceContext = (PDEVICE_CONTEXT)Context;
        NTSTATUS            status;
        WDFREQUEST          request;
        UNREFERENCED_PARAMETER(Pipe);

        if (NumBytesTransferred == 0)
            return;

        PUCHAR usbData = (PUCHAR)WdfMemoryGetBuffer(Buffer, NULL);
        // =======================================================
        // 1. POP REQUEST 1: Send the raw Gamepad Report (0x01)
        // =======================================================
        status = WdfIoQueueRetrieveNextRequest(deviceContext->InterruptMsgQueue, &request);
        if (NT_SUCCESS(status)) {
            status = RequestCopyFromBuffer(request, usbData, NumBytesTransferred);
            WdfRequestComplete(request, status);
        }
        // =======================================================
        // 2. POP REQUEST 2: Parse touchpad and send Virtual Mouse (0x41)
        // =======================================================
        if (NumBytesTransferred >= 41) { // Safety check to ensure touchpad bytes exist
            status = WdfIoQueueRetrieveNextRequest(deviceContext->InterruptMsgQueue, &request);
            if (NT_SUCCESS(status)) {
                VIRTUAL_MOUSE_REPORT mouseReport = { 0 };
                mouseReport.ReportId = 0x41; // MATCHES NEW DESCRIPTOR
                PDS_TOUCH_POINT points = (PDS_TOUCH_POINT)(usbData + 33);
                BOOLEAN f1Active = (points[0].Contact & 0x80) == 0;
                SHORT f1X = ((points[0].X_Hi_Y_Lo & 0x0F) << 8) | points[0].X_Lo;
                SHORT f1Y = (points[0].Y_Hi << 4) | ((points[0].X_Hi_Y_Lo & 0xF0) >> 4);
                BOOLEAN f2Active = (points[1].Contact & 0x80) == 0;
                SHORT f2X = ((points[1].X_Hi_Y_Lo & 0x0F) << 8) | points[1].X_Lo;
                SHORT f2Y = (points[1].Y_Hi << 4) | ((points[1].X_Hi_Y_Lo & 0xF0) >> 4);
                int dx = 0;
                int dy = 0;
                if (f1Active && deviceContext->Finger1Active) {
                    dx = (f1X - deviceContext->Finger1PrevX) / 2;
                    dy = (f1Y - deviceContext->Finger1PrevY) / 2;
                }
                else if (f2Active && deviceContext->Finger2Active) {
                    dx = (f2X - deviceContext->Finger2PrevX) / 2;
                    dy = (f2Y - deviceContext->Finger2PrevY) / 2;
                }
                // Clamp to safe 8-bit limits so Windows mouhid.sys doesn't freak out
                if (dx > 127) dx = 127;
                if (dx < -127) dx = -127;
                if (dy > 127) dy = 127;
                if (dy < -127) dy = -127;
                mouseReport.DeltaX = (CHAR)dx;
                mouseReport.DeltaY = (CHAR)dy;
                BOOLEAN isPadClicked = (usbData[10] & 0x02) != 0;
                if (isPadClicked) {
                    if (f1Active && f2Active) {
                        mouseReport.Buttons |= 0x02; // Right Click (2 fingers + click)
                    }
                    else {
                        mouseReport.Buttons |= 0x01; // Left Click (1 finger + click)
                    }
                }
                deviceContext->Finger1Active = f1Active;
                deviceContext->Finger1PrevX = f1X;
                deviceContext->Finger1PrevY = f1Y;
                deviceContext->Finger2Active = f2Active;
                deviceContext->Finger2PrevX = f2X;
                deviceContext->Finger2PrevY = f2Y;
                status = RequestCopyFromBuffer(request, &mouseReport, sizeof(mouseReport));
                WdfRequestComplete(request, status);
            }
        }
    }

    NTSTATUS
    DualSenseConfigContReaderForInterruptEndPoint(
            PDEVICE_CONTEXT DeviceContext
    )
    /*++

    Routine Description:

        This routine configures a continuous reader on the
        interrupt endpoint. It's called from the PrepareHarware event.

    Arguments:

        DeviceContext - Pointer to device context structure

    Return Value:

        NT status value

    --*/
    {
        WDF_USB_CONTINUOUS_READER_CONFIG contReaderConfig;
        NTSTATUS status = STATUS_SUCCESS;

        print_kd("[DualSense] DualSenseConfigContReaderForInterruptEndPoint Entry\n");

        PAGED_CODE();

        WDF_USB_CONTINUOUS_READER_CONFIG_INIT(&contReaderConfig,
                                              DualSenseEvtUsbInterruptPipeReadComplete,
                                              DeviceContext,    // Context
                                              64);   // TransferLength
        //
        // Reader requests are not posted to the target automatically.
        // Driver must explictly call WdfIoTargetStart to kick start the
        // reader.  In this sample, it's done in D0Entry.
        // By defaut, framework queues two requests to the target
        // endpoint. Driver can configure up to 10 requests with CONFIG macro.
        //
        status = WdfUsbTargetPipeConfigContinuousReader(DeviceContext->HidInterruptPipe,
                                                        &contReaderConfig);

        if (!NT_SUCCESS(status)) {
            print_kd("[DualSense] DualSenseConfigContReaderForInterruptEndPoint failed %x\n", status);
            return status;
        }

        print_kd("[DualSense] DualSenseConfigContReaderForInterruptEndPoint Exit, status:0x%x\n", status);

        return status;
    }

    NTSTATUS
    DualSenseEvtDevicePrepareHardware(
            _In_ WDFDEVICE Device,
            _In_ WDFCMRESLIST ResourcesRaw,
            _In_ WDFCMRESLIST ResourcesTranslated
    ) {
        UNREFERENCED_PARAMETER(ResourcesTranslated);
        UNREFERENCED_PARAMETER(ResourcesRaw);

        NTSTATUS                            status = STATUS_SUCCESS;
        PDEVICE_CONTEXT                     deviceContext = NULL;
        WDF_USB_DEVICE_SELECT_CONFIG_PARAMS configParams;
        WDF_USB_CONTROL_SETUP_PACKET        controlSetupPacket;
        WDF_OBJECT_ATTRIBUTES               attributes;
        PUSB_DEVICE_DESCRIPTOR              pdsDeviceDescriptor = NULL;

        PAGED_CODE();

        print_kd("[DualSense] HidFx2EvtDevicePrepareHardware Enter\n");

        deviceContext = GetDeviceContext(Device);

        if (deviceContext->DsUsbDevice== NULL) {
            status = WdfUsbTargetDeviceCreate(Device,
                                              WDF_NO_OBJECT_ATTRIBUTES,
                                              &deviceContext->DsUsbDevice);

            if (!NT_SUCCESS(status)) {
                print_kd("[DualSense] WdfUsbTargetDeviceCreate failed 0x%x\n", status);
                return status;
            }

            //
            // TODO: If you are fetching configuration descriptor from device for
            // selecting a configuration or to parse other descriptors, call
            // HidFx2ValidateConfigurationDescriptor
            // to do basic validation on the descriptors before you access them.
            //
        }

        WDF_USB_DEVICE_SELECT_CONFIG_PARAMS_INIT_MULTIPLE_INTERFACES(&configParams, 0, NULL);

        status = WdfUsbTargetDeviceSelectConfig(deviceContext->DsUsbDevice,
                                                WDF_NO_OBJECT_ATTRIBUTES,
                                                &configParams);
        if (!NT_SUCCESS(status)) {
            print_kd("[DualSense] WdfUsbTargetDeviceSelectConfig failed %!STATUS!\n", status);
            return status;
        }

        deviceContext->NumDsUsbInterfaces = WdfUsbTargetDeviceGetNumInterfaces(deviceContext->DsUsbDevice);
        if (deviceContext->DsUsbInterfaces != NULL) {
            ExFreePoolWithTag(deviceContext->DsUsbInterfaces, 'SIKT');
        }
        deviceContext->DsUsbInterfaces = (WDFUSBINTERFACE*)ExAllocatePool2(POOL_FLAG_NON_PAGED, deviceContext->NumDsUsbInterfaces * sizeof(WDFUSBINTERFACE), 'SIKT');

        print_kd("[DualSense] Number of dsUsbInterfaces: %hhu\n", deviceContext->NumDsUsbInterfaces);

        if (!deviceContext->DsUsbInterfaces) {
            print_kd("[DualSense] Cannot allocate DsUsbInterfaces.\n");
            return STATUS_INSUFFICIENT_RESOURCES;
        }



        WDF_OBJECT_ATTRIBUTES_INIT(&attributes);
        attributes.ParentObject = Device;

        status = WdfMemoryCreate(
                &attributes,
                NonPagedPoolNx,
                0,
                sizeof(USB_DEVICE_DESCRIPTOR),
                &deviceContext->DsDeviceDescriptorHandle,
                (PVOID*) &pdsDeviceDescriptor
        );

        if (!NT_SUCCESS(status)) {
            print_kd("[DualSense] Cannot allocate DsDeviceDescriptor.\n");
            return status;
        }

        WdfUsbTargetDeviceGetDeviceDescriptor(
                deviceContext->DsUsbDevice,
                pdsDeviceDescriptor
        );

        UCHAR hidInterfaceNumber = 0;

        for (UCHAR i = 0; i < deviceContext->NumDsUsbInterfaces; i++) {
            deviceContext->DsUsbInterfaces[i] = WdfUsbTargetDeviceGetInterface(deviceContext->DsUsbDevice, i);

            // Get the descriptor to check what kind of interface this is
            USB_INTERFACE_DESCRIPTOR interfaceDescriptor;
            WdfUsbInterfaceGetDescriptor(deviceContext->DsUsbInterfaces[i], 0, &interfaceDescriptor);

            print_kd("[DualSense] Interface %d: Class 0x%02X, SubClass 0x%02X\n",
                     i, interfaceDescriptor.bInterfaceClass, interfaceDescriptor.bInterfaceSubClass);

            // 0x03 is USB_DEVICE_CLASS_HUMAN_INTERFACE
            if (interfaceDescriptor.bInterfaceClass == 0x03) {
                print_kd("[DualSense] Found HID Interface at index %d!\n", i);
                deviceContext->DsHidUsbInterface = deviceContext->DsUsbInterfaces[i];
                hidInterfaceNumber = interfaceDescriptor.bInterfaceNumber;
            }

        }

        WDF_USB_CONTROL_SETUP_PACKET_INIT(
                &controlSetupPacket,
                BmRequestDeviceToHost,
                BmRequestToInterface, // WDF assumes BmRequestStandard for this macro
                0x06, // USB_REQUEST_GET_DESCRIPTOR
                (0x21 << 8) | 0, // Descriptor type (0x21 for HID), Index 0
                hidInterfaceNumber // wIndex is the target interface number
        );
        WDF_MEMORY_DESCRIPTOR memDesc;
        WDF_MEMORY_DESCRIPTOR_INIT_BUFFER(&memDesc, &deviceContext->DsHidDescriptor, sizeof(HID_DESCRIPTOR));
        ULONG bytesTransferred = 0;
        status = WdfUsbTargetDeviceSendControlTransferSynchronously(
                deviceContext->DsUsbDevice,
                WDF_NO_HANDLE, // Optional WDFREQUEST
                NULL,          // PWDF_REQUEST_SEND_OPTIONS
                &controlSetupPacket,
                &memDesc,
                &bytesTransferred
        );

        if (!NT_SUCCESS(status)) {
            print_kd("[DualSense] Failed to fetch HID Descriptor: 0x%x\n", status);
            return status;
        }

        print_kd("[DualSense] HID Descriptor Bytes:\n");
        PUCHAR hidDescBytes = (PUCHAR)&deviceContext->DsHidDescriptor;

        for (ULONG i = 0; i < deviceContext->DsHidDescriptor.bLength; i++) {
            print_kd("%02X ", hidDescBytes[i]);
        }
        print_kd("\n");

        // 3. Look at the wReportLength inside the fetched DsHidDescriptor
        //    and allocate memory for deviceContext->DsReportDescriptorHandle.
        USHORT reportDescLength = deviceContext->DsHidDescriptor.DescriptorList[0].wReportLength;

        WDF_OBJECT_ATTRIBUTES_INIT(&attributes);
        attributes.ParentObject = Device;
        status = WdfMemoryCreate(
                &attributes,
                NonPagedPoolNx,
                0, // pool tag
                reportDescLength,
                &deviceContext->DsReportDescriptorHandle,
                NULL // We don't need the raw pointer immediately
        );
        if (!NT_SUCCESS(status)) {
            print_kd("[DualSense] Failed to allocate memory for Report Descriptor: 0x%x\n", status);
            return status;
        }

        // 4. Setup a second Control Transfer for the Report Descriptor (Type 0x22)
        WDF_USB_CONTROL_SETUP_PACKET_INIT(
                &controlSetupPacket,
                BmRequestDeviceToHost,
                BmRequestToInterface,
                0x06, // USB_REQUEST_GET_DESCRIPTOR
                (0x22 << 8) | 0, // Descriptor type (0x22 for Report), Index 0
                hidInterfaceNumber
        );

        // Get the raw pointer from our newly allocated memory object
        PVOID reportDescBuffer = WdfMemoryGetBuffer(deviceContext->DsReportDescriptorHandle, NULL);
        WDF_MEMORY_DESCRIPTOR_INIT_BUFFER(&memDesc, reportDescBuffer, reportDescLength);
        // 5. Send the transfer to fetch the Report Descriptor
        status = WdfUsbTargetDeviceSendControlTransferSynchronously(
                deviceContext->DsUsbDevice,
                WDF_NO_HANDLE,
                NULL,
                &controlSetupPacket,
                &memDesc,
                &bytesTransferred
        );
        if (!NT_SUCCESS(status)) {
            print_kd("[DualSense] Failed to fetch Report Descriptor: 0x%x\n", status);
            return status;
        }

        print_kd("[DualSense] Successfully fetched HID and Report Descriptors!\n");

        for (ULONG i = 0; i < bytesTransferred; i++)
            print_kd("%02X ", ((PUCHAR)reportDescBuffer)[i]);

        print_kd("\n");


        UCHAR numPipes = WdfUsbInterfaceGetNumConfiguredPipes(deviceContext->DsHidUsbInterface);

        for (UCHAR i = 0; i < numPipes; i++) {
            WDF_USB_PIPE_INFORMATION pipeInfo;
            WDF_USB_PIPE_INFORMATION_INIT(&pipeInfo);

            WDFUSBPIPE pipe = WdfUsbInterfaceGetConfiguredPipe(deviceContext->DsHidUsbInterface, i, &pipeInfo);

            // We want an Interrupt pipe, and it MUST be an IN endpoint (Device to Host)
            if (pipeInfo.PipeType == WdfUsbPipeTypeInterrupt && WdfUsbTargetPipeIsInEndpoint(pipe)) {
                deviceContext->HidInterruptPipe = pipe;
                print_kd("[DualSense] Found Interrupt IN pipe at index %d!\n", i);
                break;
            }
        }

        if (!deviceContext->HidInterruptPipe) {
            print_kd("[DualSense] Failed to get Interrupt Pipe on DsHidUsbInterface.\n");
            return STATUS_INVALID_DEVICE_STATE;
        }

        //
        // Tell the framework that it's okay to read less than
        // MaximumPacketSize
        //
        WdfUsbTargetPipeSetNoMaximumPacketSizeCheck(deviceContext->HidInterruptPipe);

        //
        //configure continuous reader
        //
        status = DualSenseConfigContReaderForInterruptEndPoint(deviceContext);

        print_kd("[DualSense] DualSenseEvtDevicePrepareHardware Exit.\n");

        return status;
    }
}
