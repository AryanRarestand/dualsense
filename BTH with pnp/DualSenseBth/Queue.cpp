#include "driver.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text (PAGE, DualSenseBthQueueInitialize)
#endif

NTSTATUS
DualSenseBthQueueInitialize(
    _In_ WDFDEVICE Device
    )
{
    WDFQUEUE queue;
    NTSTATUS status;
	PDEVICE_CONTEXT deviceContext;
    WDF_IO_QUEUE_CONFIG queueConfig;

    PAGED_CODE();

    print_kd("[DualSenseBth] Queue Initialize Entry\n");

    deviceContext = DeviceGetContext(Device);

	// Create a default queue for handling IOCTLs

    WDF_IO_QUEUE_CONFIG_INIT_DEFAULT_QUEUE(&queueConfig, WdfIoQueueDispatchParallel);

    queueConfig.EvtIoInternalDeviceControl = DualSenseBthEvtIoInternalDeviceControl;

    status = WdfIoQueueCreate(
                 Device,
                 &queueConfig,
                 WDF_NO_OBJECT_ATTRIBUTES,
                 &queue
                 );

    if(!NT_SUCCESS(status)) {
        print_kd("[DualSenseBth] WdfIoQueueCreate failed 0x%x\n", status);
        return status;
    }

	// Create a manual queue for IOCTL_HID_READ_REPORT
    WDF_IO_QUEUE_CONFIG_INIT(&queueConfig, WdfIoQueueDispatchManual);
    queueConfig.PowerManaged = WdfFalse;

    status = WdfIoQueueCreate(Device,
        &queueConfig,
        WDF_NO_OBJECT_ATTRIBUTES,
        &deviceContext->ManualQueue
    );

    if (!NT_SUCCESS(status)) {
        print_kd("[DualSense] WdfIoQueueCreate on the manual queue failed 0x%x\n", status);
        return status;
    }


    print_kd("[DualSenseBth] Queue Initialize Exit\n");

    return status;
}

VOID
DualSenseBthEvtIoInternalDeviceControl(
    _In_ WDFQUEUE Queue,
    _In_ WDFREQUEST Request,
    _In_ size_t OutputBufferLength,
    _In_ size_t InputBufferLength,
    _In_ ULONG IoControlCode
    )
{

    NTSTATUS            status = STATUS_SUCCESS;
    WDFDEVICE           device;
    PDEVICE_CONTEXT     deviceContext;

    device = WdfIoQueueGetDevice(Queue);
    deviceContext = DeviceGetContext(device);

    UNREFERENCED_PARAMETER(InputBufferLength);
    UNREFERENCED_PARAMETER(OutputBufferLength);

    switch (IoControlCode) {
    case IOCTL_HID_GET_DEVICE_DESCRIPTOR: {
        status = RequestCopyFromBuffer(Request,
            &DualSenseBTHDeviceDescriptor,
            DualSenseBTHDeviceDescriptor.bLength);

        break;
    }
    case IOCTL_HID_GET_DEVICE_ATTRIBUTES:
    {
        status = RequestCopyFromBuffer(Request,
            &DualSenseBTHDeviceAttributes,
            sizeof(HID_DEVICE_ATTRIBUTES));
        break;
    }

    case IOCTL_HID_GET_REPORT_DESCRIPTOR:
    {
        status = RequestCopyFromBuffer(Request,
            (PVOID) &DualSenseBTHReportDescriptor,
            DualSenseBTHDeviceDescriptor.DescriptorList[0].wReportLength);

        break;
    }

    case IOCTL_HID_READ_REPORT:
    {
        WDF_REQUEST_SEND_OPTIONS options;
        WDF_REQUEST_SEND_OPTIONS_INIT(&options, WDF_REQUEST_SEND_OPTION_SEND_AND_FORGET);
        
        WdfRequestFormatRequestUsingCurrentType(Request);
        
        if (WdfRequestSend(Request, WdfDeviceGetIoTarget(device), &options) == FALSE) {
            status = WdfRequestGetStatus(Request);
            print_kd("[DualSense] WdfRequestSend failed with status: 0x%x\n", status);
            WdfRequestComplete(Request, status);
        }
        return;
    }

    case IOCTL_HID_WRITE_REPORT:            // METHOD_NEITHER TODO:
        //
        // Transmits a class driver-supplied report to the device.
        //
        //status = WriteReport(queueContext, Request);
        break;

    case IOCTL_HID_GET_FEATURE:             // METHOD_OUT_DIRECT
    {
        WDFMEMORY memory;
        size_t outputBufferLength;
        PUCHAR outputBuffer;

        status = WdfRequestRetrieveOutputMemory(Request, &memory);
        if (NT_SUCCESS(status)) {
            outputBuffer = (PUCHAR)WdfMemoryGetBuffer(memory, &outputBufferLength);
            if (outputBufferLength >= 1) {
                UCHAR reportId = outputBuffer[0];

                if (reportId == 0x42 && outputBufferLength >= 3) {
                    // PTP Configuration Feature Report
                    outputBuffer[0] = 0x42; // Report ID
                    outputBuffer[1] = 0x02; // Maximum Contacts (2)
                    outputBuffer[2] = 0x00; // Pad Type (0 = Touchpad)
                    WdfRequestSetInformation(Request, 3);
                    status = STATUS_SUCCESS;
                }
                else if (reportId == 0x43 && outputBufferLength >= 256) {
                    // PTP Certification Status Feature Report
                    RtlZeroMemory(outputBuffer, outputBufferLength);
                    outputBuffer[0] = 0x43; // Report ID

                    // Windows requires a 256-byte blob for PTPHQA. 
                    // Any valid 256 byte response allows basic PTP features.
                    outputBuffer[1] = 0xFC;
                    outputBuffer[2] = 0x28;

                    WdfRequestSetInformation(Request, 256);
                    status = STATUS_SUCCESS;
                }
                else {
                    status = STATUS_NOT_SUPPORTED;
                }
            }
            else {
                status = STATUS_INVALID_BUFFER_SIZE;
            }
        }
        break;
    }

    case IOCTL_HID_SET_FEATURE:             // METHOD_IN_DIRECT TODO:

        //status = SetFeature(queueContext, Request);
        break;

    case IOCTL_HID_GET_INPUT_REPORT:        // METHOD_OUT_DIRECT TODO:

        //status = GetInputReport(queueContext, Request);
        break;

    case IOCTL_HID_SET_OUTPUT_REPORT:       // METHOD_IN_DIRECT TODO:

        //status = SetOutputReport(queueContext, Request);
        break;

    case IOCTL_HID_GET_STRING:                      // METHOD_NEITHER TODO:

        //status = GetString(Request);
        break;

    case IOCTL_HID_GET_INDEXED_STRING:              // METHOD_OUT_DIRECT TODO:

        //status = GetIndexedString(Request);
        break;

    case IOCTL_HID_SEND_IDLE_NOTIFICATION_REQUEST:  // METHOD_NEITHER TODO:
        //
        // This has the USBSS Idle notification callback. If the lower driver
        // can handle it (e.g. USB stack can handle it) then pass it down
        // otherwise complete it here as not inplemented. For a virtual
        // device, idling is not needed.
        //
        // Not implemented. fall through...
        //
    case IOCTL_HID_ACTIVATE_DEVICE:                 // METHOD_NEITHER
    case IOCTL_HID_DEACTIVATE_DEVICE:               // METHOD_NEITHER
    case IOCTL_GET_PHYSICAL_DESCRIPTOR:             // METHOD_OUT_DIRECT
        //
        // We don't do anything for these IOCTLs but some minidrivers might.
        //
        // Not implemented. fall through...
        //
    default:
        status = STATUS_NOT_IMPLEMENTED;
        break;

    }

    WdfRequestComplete(Request, status);

}

NTSTATUS
RequestCopyFromBuffer(
    _In_  WDFREQUEST        Request,
    _In_  PVOID             SourceBuffer,
    _When_(NumBytesToCopyFrom == 0, __drv_reportError(NumBytesToCopyFrom cannot be zero))
    _In_  size_t            NumBytesToCopyFrom
)
/*++

Routine Description:

    A helper function to copy specified bytes to the request's output memory

Arguments:

    Request - A handle to a framework request object.

    SourceBuffer - The buffer to copy data from.

    NumBytesToCopyFrom - The length, in bytes, of data to be copied.

Return Value:

    NTSTATUS

--*/
{
    NTSTATUS                status;
    WDFMEMORY               memory;
    size_t                  outputBufferLength;

    status = WdfRequestRetrieveOutputMemory(Request, &memory);
    if (!NT_SUCCESS(status)) {
        print_kd("[DualSense] WdfRequestRetrieveOutputMemory failed 0x%x\n", status);
        return status;
    }

    WdfMemoryGetBuffer(memory, &outputBufferLength);
    if (outputBufferLength < NumBytesToCopyFrom) {
        status = STATUS_INVALID_BUFFER_SIZE;
        print_kd("[DualSense] RequestCopyFromBuffer: buffer too small. Size %d, expect %d\n",
            (int)outputBufferLength, (int)NumBytesToCopyFrom);
        return status;
    }

    status = WdfMemoryCopyFromBuffer(memory,
        0,
        SourceBuffer,
        NumBytesToCopyFrom);

    if (!NT_SUCCESS(status)) {
        print_kd("[DualSense] WdfMemoryCopyFromBuffer failed 0x%x\n", status);
        return status;
    }

    WdfRequestSetInformation(Request, NumBytesToCopyFrom);
    return status;
}