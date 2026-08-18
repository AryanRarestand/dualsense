#include "Driver.h"

namespace DeviceUSB {
    // Define the descriptors here to avoid LNK2005 multiply defined symbols
    const unsigned char DualSenseUSBReportDescriptor[] = {
            // =========================================================================
            // TOP LEVEL COLLECTION 1: GAMEPAD (Original DualSense Descriptor)
            // =========================================================================
            0x05,
            0x01,        // Usage Page (Generic Desktop Ctrls) - This tells the OS we are defining a generic desktop device
            0x09, 0x05,        // Usage (Game Pad) - Specifically, this collection is a Gamepad
            0xA1, 0x01,        // Collection (Application) - Start of the Gamepad collection
            0x85, 0x01,        //   Report ID (1) - All input reports for this Gamepad must start with byte 0x01

            // --- Joysticks & Triggers (6 axes) ---
            0x09, 0x30,        //   Usage (X) - Left Stick X
            0x09, 0x31,        //   Usage (Y) - Left Stick Y
            0x09, 0x32,        //   Usage (Z) - Right Stick X (Usually mapped to Z)
            0x09, 0x35,        //   Usage (Rz) - Right Stick Y
            0x09, 0x33,        //   Usage (Rx) - Left Trigger (L2)
            0x09, 0x34,        //   Usage (Ry) - Right Trigger (R2)
            0x15, 0x00,        //   Logical Minimum (0) - Minimum value of the axes
            0x26, 0xFF, 0x00,  //   Logical Maximum (255) - Maximum value of the axes
            0x75, 0x08,        //   Report Size (8) - Each axis is 8 bits (1 byte)
            0x95, 0x06,        //   Report Count (6) - There are 6 axes in a row
            0x81, 0x02,        //   Input (Data,Var,Abs) - Declare these 6 bytes as absolute variables

            // --- Sequence Number / State (Vendor Defined) ---
            0x06, 0x00, 0xFF,  //   Usage Page (Vendor Defined 0xFF00)
            0x09, 0x20,        //   Usage (0x20)
            0x95, 0x01,        //   Report Count (1) - 1 byte
            0x81, 0x02,        //   Input (Data,Var,Abs)

            // --- Hat Switch (D-Pad) ---
            0x05, 0x01,        //   Usage Page (Generic Desktop Ctrls)
            0x09, 0x39,        //   Usage (Hat switch)
            0x15, 0x00,        //   Logical Minimum (0)
            0x25, 0x07,        //   Logical Maximum (7) - 8 directions (0-7)
            0x35, 0x00,        //   Physical Minimum (0)
            0x46, 0x3B, 0x01,  //   Physical Maximum (315) - 315 degrees
            0x65, 0x14,        //   Unit (System: English Rotation, Length: Centimeter)
            0x75, 0x04,        //   Report Size (4) - 4 bits (half a byte)
            0x95, 0x01,        //   Report Count (1) - 1 hat switch
            0x81, 0x42,        //   Input (Data,Var,Abs,Null State)

            // --- Buttons ---
            0x65, 0x00,        //   Unit (None)
            0x05, 0x09,        //   Usage Page (Button)
            0x19, 0x01,        //   Usage Minimum (Button 1)
            0x29, 0x0F,        //   Usage Maximum (Button 15)
            0x15, 0x00,        //   Logical Minimum (0) - Unpressed
            0x25, 0x01,        //   Logical Maximum (1) - Pressed
            0x75, 0x01,        //   Report Size (1) - 1 bit per button
            0x95, 0x0F,        //   Report Count (15) - 15 buttons (takes up 15 bits)
            0x81, 0x02,        //   Input (Data,Var,Abs)

            // --- Padding/Vendor specific bits (to align to full bytes) ---
            0x06, 0x00, 0xFF,  //   Usage Page (Vendor Defined 0xFF00)
            0x09, 0x21,        //   Usage (0x21)
            0x95, 0x0D,        //   Report Count (13) - 13 bits (so Hat+Buttons+This = 32 bits / 4 bytes)
            0x81, 0x02,        //   Input (Data,Var,Abs)

            // --- The massive 52-byte Vendor chunk (Contains Touchpad, Gyro, Accel) ---
            0x06, 0x00, 0xFF,  //   Usage Page (Vendor Defined 0xFF00)
            0x09, 0x22,        //   Usage (0x22)
            0x15, 0x00,        //   Logical Minimum (0)
            0x26, 0xFF, 0x00,  //   Logical Maximum (255)
            0x75, 0x08,        //   Report Size (8) - 8 bits (1 byte)
            0x95, 0x34,        //   Report Count (52) - 52 bytes total
            0x81, 0x02,        //   Input (Data,Var,Abs)
            // ... (Your original output feature reports remain unchanged) ...
            0x85, 0x02, 0x09, 0x23, 0x95, 0x2F, 0x91, 0x02,
            0x85, 0x05, 0x09, 0x33, 0x95, 0x28, 0xB1, 0x02,
            0x85, 0x08, 0x09, 0x34, 0x95, 0x2F, 0xB1, 0x02,
            0x85, 0x09, 0x09, 0x24, 0x95, 0x13, 0xB1, 0x02,
            0x85, 0x0A, 0x09, 0x25, 0x95, 0x1A, 0xB1, 0x02,
            0x85, 0x20, 0x09, 0x26, 0x95, 0x3F, 0xB1, 0x02,
            0x85, 0x21, 0x09, 0x27, 0x95, 0x04, 0xB1, 0x02,
            0x85, 0x22, 0x09, 0x40, 0x95, 0x3F, 0xB1, 0x02,
            0x85, 0x80, 0x09, 0x28, 0x95, 0x3F, 0xB1, 0x02,
            0x85, 0x81, 0x09, 0x29, 0x95, 0x3F, 0xB1, 0x02,
            0x85, 0x82, 0x09, 0x2A, 0x95, 0x09, 0xB1, 0x02,
            0x85, 0x83, 0x09, 0x2B, 0x95, 0x3F, 0xB1, 0x02,
            0x85, 0x84, 0x09, 0x2C, 0x95, 0x3F, 0xB1, 0x02,
            0x85, 0x85, 0x09, 0x2D, 0x95, 0x02, 0xB1, 0x02,
            0x85, 0xA0, 0x09, 0x2E, 0x95, 0x01, 0xB1, 0x02,
            0x85, 0xE0, 0x09, 0x2F, 0x95, 0x3F, 0xB1, 0x02,
            0x85, 0xF0, 0x09, 0x30, 0x95, 0x3F, 0xB1, 0x02,
            0x85, 0xF1, 0x09, 0x31, 0x95, 0x3F, 0xB1, 0x02,
            0x85, 0xF2, 0x09, 0x32, 0x95, 0x0F, 0xB1, 0x02,
            0x85, 0xF4, 0x09, 0x35, 0x95, 0x3F, 0xB1, 0x02,
            0x85, 0xF5, 0x09, 0x36, 0x95, 0x03, 0xB1, 0x02,
            0xC0,              // End Collection (Ends the Gamepad)
            // =========================================================================
            // TOP LEVEL COLLECTION 2: PRECISION TOUCHPAD
            // =========================================================================
            0x05, 0x0D,        // Usage Page (Digitizers)
            0x09, 0x05,        // Usage (Touch Pad)
            0xA1, 0x01,        // Collection (Application)
            0x85, 0x41,        //   Report ID (65 / 0x41) - Input Report

            // --- Finger 1 ---
            0x05, 0x0D,        //   Usage Page (Digitizers)
            0x09, 0x22,        //   Usage (Finger)
            0xA1, 0x02,        //   Collection (Logical)
            0x09, 0x42,        //     Usage (Tip Switch)
            0x15, 0x00,        //     Logical Minimum (0)
            0x25, 0x01,        //     Logical Maximum (1)
            0x75, 0x01,        //     Report Size (1)
            0x95, 0x01,        //     Report Count (1)
            0x81, 0x02,        //     Input (Data,Var,Abs)
            0x09, 0x47,        //     Usage (Confidence)
            0x81, 0x02,        //     Input (Data,Var,Abs)
            0x09, 0x51,        //     Usage (Contact Identifier)
            0x25, 0x0F,        //     Logical Maximum (15)
            0x75, 0x06,        //     Report Size (6)
            0x95, 0x01,        //     Report Count (1)
            0x81, 0x02,        //     Input (Data,Var,Abs)
            0x05, 0x01,        //     Usage Page (Generic Desktop Ctrls)
            0x09, 0x30,        //     Usage (X)
            0x26, 0x80, 0x07,  //     Logical Maximum (1920)
            0x75, 0x10,        //     Report Size (16)
            0x95, 0x01,        //     Report Count (1)
            0x81, 0x02,        //     Input (Data,Var,Abs)
            0x09, 0x31,        //     Usage (Y)
            0x26, 0x38, 0x04,  //     Logical Maximum (1080)
            0x81, 0x02,        //     Input (Data,Var,Abs)
            0xC0,              //   End Collection

            // --- Finger 2 ---
            0x05, 0x0D,        //   Usage Page (Digitizers)
            0x09, 0x22,        //   Usage (Finger)
            0xA1, 0x02,        //   Collection (Logical)
            0x09, 0x42,        //     Usage (Tip Switch)
            0x15, 0x00,        //     Logical Minimum (0)
            0x25, 0x01,        //     Logical Maximum (1)
            0x75, 0x01,        //     Report Size (1)
            0x95, 0x01,        //     Report Count (1)
            0x81, 0x02,        //     Input (Data,Var,Abs)
            0x09, 0x47,        //     Usage (Confidence)
            0x81, 0x02,        //     Input (Data,Var,Abs)
            0x09, 0x51,        //     Usage (Contact Identifier)
            0x25, 0x0F,        //     Logical Maximum (15)
            0x75, 0x06,        //     Report Size (6)
            0x95, 0x01,        //     Report Count (1)
            0x81, 0x02,        //     Input (Data,Var,Abs)
            0x05, 0x01,        //     Usage Page (Generic Desktop Ctrls)
            0x09, 0x30,        //     Usage (X)
            0x26, 0x80, 0x07,  //     Logical Maximum (1920)
            0x75, 0x10,        //     Report Size (16)
            0x95, 0x01,        //     Report Count (1)
            0x81, 0x02,        //     Input (Data,Var,Abs)
            0x09, 0x31,        //     Usage (Y)
            0x26, 0x38, 0x04,  //     Logical Maximum (1080)
            0x81, 0x02,        //     Input (Data,Var,Abs)
            0xC0,              //   End Collection

            // --- Scan Time ---
            0x05, 0x0D,        //   Usage Page (Digitizers)
            0x09, 0x56,        //   Usage (Scan Time)
            0x27, 0xFF, 0xFF, 0x00, 0x00, // Logical Maximum (65535)
            0x75, 0x10,        //   Report Size (16)
            0x95, 0x01,        //   Report Count (1)
            0x81, 0x02,        //   Input (Data,Var,Abs)

            // --- Contact Count ---
            0x05, 0x0D,        //   Usage Page (Digitizers)
            0x09, 0x54,        //   Usage (Contact Count)
            0x25, 0x02,        //   Logical Maximum (2)
            0x75, 0x08,        //   Report Size (8)
            0x95, 0x01,        //   Report Count (1)
            0x81, 0x02,        //   Input (Data,Var,Abs)

            // --- Button ---
            0x05, 0x09,        //   Usage Page (Button)
            0x09, 0x01,        //   Usage Minimum (Button 1)
            0x25, 0x01,        //   Logical Maximum (1)
            0x75, 0x01,        //   Report Size (1)
            0x95, 0x01,        //   Report Count (1)
            0x81, 0x02,        //   Input (Data,Var,Abs)

            // --- Padding (7 bits) ---
            0x75, 0x07,        //   Report Size (7)
            0x95, 0x01,        //   Report Count (1)
            0x81, 0x03,        //   Input (Const,Var,Abs)

            // =========================================================================
            // FEATURE REPORTS FOR PTP CONFIGURATION & CERTIFICATION
            // =========================================================================
            0x85, 0x42,        //   Report ID (66 / 0x42) - PTP Configuration
            0x05, 0x0D,        //   Usage Page (Digitizers)
            0x09, 0x55,        //   Usage (Contact Count Maximum)
            0x25, 0x02,        //   Logical Maximum (2)
            0x75, 0x08,        //   Report Size (8)
            0x95, 0x01,        //   Report Count (1)
            0xB1, 0x02,        //   Feature (Data,Var,Abs)
            0x09, 0x59,        //   Usage (Pad Type)
            0x15, 0x00,        //   Logical Minimum (0)
            0x25, 0x01,        //   Logical Maximum (1)
            0x75, 0x08,        //   Report Size (8)
            0x95, 0x01,        //   Report Count (1)
            0xB1, 0x02,        //   Feature (Data,Var,Abs)

            0x85, 0x43,        //   Report ID (67 / 0x43) - PTP Certification Status
            0x06, 0x00, 0xFF,  //   Usage Page (Vendor Defined 0xFF00)
            0x09, 0xC5,        //   Usage (0xC5)
            0x15, 0x00,        //   Logical Minimum (0)
            0x26, 0xFF, 0x00,  //   Logical Maximum (255)
            0x75, 0x08,        //   Report Size (8)
            0x96, 0x00, 0x01,  //   Report Count (256)
            0xB1, 0x02,        //   Feature (Data,Var,Abs)

            0xC0               // End Collection (Ends the Touch Pad)
    };
    HID_DESCRIPTOR DualSenseDeviceDescriptor = {
            0x09,       // bLength (9 bytes)
            0x21,       // bDescriptorType (HID descriptor)
            0x0111,     // bcdHID (USHORT, Version 1.11)
            0x00,       // bCountryCode
            0x01,       // bNumDescriptors
            {           // DescriptorList array
                    {       // DescriptorList[0]
                            0x22, // bReportType
                            sizeof(DualSenseUSBReportDescriptor) // wReportLength
                    }
            }
    };
#pragma pack(push, 1)
    typedef struct _DS_TOUCH_POINT {
        UCHAR Contact;
        UCHAR X_Lo;
        UCHAR X_Hi_Y_Lo;
        UCHAR Y_Hi;
    } DS_TOUCH_POINT, * PDS_TOUCH_POINT;
    typedef struct _PTP_CONTACT {
        UCHAR TipSwitch : 1;
        UCHAR Confidence : 1;
        UCHAR ContactID : 6;
        USHORT X;
        USHORT Y;
    } PTP_CONTACT, * PPTP_CONTACT;

    typedef struct _PTP_REPORT {
        UCHAR ReportId;
        PTP_CONTACT Contacts[2];
        USHORT ScanTime;
        UCHAR ContactCount;
        UCHAR Button;
    } PTP_REPORT, * PPTP_REPORT;
#pragma pack(pop)


    VOID DualSenseEvtUsbInterruptPipeReadComplete(
            WDFUSBPIPE Pipe,
            WDFMEMORY Buffer,
            size_t NumBytesTransferred,
            WDFCONTEXT Context
    ) {
        UNREFERENCED_PARAMETER(Pipe);

        PDEVICE_CONTEXT deviceContext;
        PUCHAR usbData;

        if (NumBytesTransferred == 0)
            return;

        deviceContext = (PDEVICE_CONTEXT) Context;
        usbData = (PUCHAR) WdfMemoryGetBuffer(Buffer, NULL);

        // 1. POP REQUEST 1: Send the raw Gamepad Report (0x01)
        DualSenseProcessGamepadReport(deviceContext, usbData, NumBytesTransferred);

        // 2. POP REQUEST 2: Parse touchpad and send PTP Report (0x41)
        DualSenseProcessPtpReport(deviceContext, usbData, NumBytesTransferred);
    }

    //HelperFunction: DualSenseEvtUsbInterruptPipeReadComplete
    VOID DualSenseProcessGamepadReport(
            _In_ PDEVICE_CONTEXT DeviceContext,
            _In_ PUCHAR UsbData,
            _In_ size_t NumBytesTransferred
    ) {
        NTSTATUS status;
        WDFREQUEST request;

        status = WdfIoQueueRetrieveNextRequest(DeviceContext->InterruptMsgQueue, &request);
        if (NT_SUCCESS(status)) {
            status = RequestCopyFromBuffer(request, UsbData, NumBytesTransferred);
            WdfRequestComplete(request, status);
        }
    }

    //HelperFunction: DualSenseProcessPtpReport
    // Small inline helper to unpack the DualSense 12-bit coordinate format
    FORCEINLINE VOID DualSenseUnpackTouchData(
            _In_ PDS_TOUCH_POINT Point,
            _Out_ PBOOLEAN IsActive,
            _Out_ PSHORT X,
            _Out_ PSHORT Y,
            _Out_ PUCHAR ContactId
    ) {
        *IsActive = (Point->Contact & 0x80) == 0;
        *ContactId = Point->Contact & 0x7F;
        *X = ((Point->X_Hi_Y_Lo & 0x0F) << 8) | Point->X_Lo;
        *Y = (Point->Y_Hi << 4) | ((Point->X_Hi_Y_Lo & 0xF0) >> 4);
    }

    //HelperFunction: DualSenseEvtUsbInterruptPipeReadComplete
    VOID DualSenseProcessPtpReport(
            _In_ PDEVICE_CONTEXT DeviceContext,
            _In_ PUCHAR UsbData,
            _In_ size_t NumBytesTransferred
    ) {

        // Safety check to ensure touchpad bytes exist
        if (NumBytesTransferred < DS_REPORT_PTP_MIN_SIZE) {
            return;
        }

        NTSTATUS status;
        WDFREQUEST request;

        status = WdfIoQueueRetrieveNextRequest(DeviceContext->InterruptMsgQueue, &request);
        if (NT_SUCCESS(status)) {
            PTP_REPORT ptpReport = {0};
            ptpReport.ReportId = 0x41; // MATCHES NEW DESCRIPTOR
            PDS_TOUCH_POINT points = (PDS_TOUCH_POINT)(UsbData + DS_TOUCH_DATA_OFFSET);

            BOOLEAN f1Active, f2Active;
            SHORT f1X, f1Y, f2X, f2Y;
            UCHAR f1Id, f2Id;

            DualSenseUnpackTouchData(&points[0], &f1Active, &f1X, &f1Y, &f1Id);
            DualSenseUnpackTouchData(&points[1], &f2Active, &f2X, &f2Y, &f2Id);

            UCHAR contactCount = 0;

            if (f1Active) {
                ptpReport.Contacts[0].TipSwitch = 1;
                ptpReport.Contacts[0].Confidence = 1;
                ptpReport.Contacts[0].ContactID = points[0].Contact & DS_TOUCH_CONTACT_ID_MASK; // Use raw contact ID from DualSense
                ptpReport.Contacts[0].X = f1X;
                ptpReport.Contacts[0].Y = f1Y;
                contactCount++;
            }

            if (f2Active) {
                ptpReport.Contacts[1].TipSwitch = 1;
                ptpReport.Contacts[1].Confidence = 1;
                ptpReport.Contacts[1].ContactID = points[1].Contact & DS_TOUCH_CONTACT_ID_MASK; // Use raw contact ID from DualSense
                ptpReport.Contacts[1].X = f2X;
                ptpReport.Contacts[1].Y = f2Y;
                contactCount++;
            }

            ptpReport.ContactCount = contactCount;

            // Track time
            DeviceContext->currentScanTime += 100; // Increment roughly every 10ms report (100 * 100us)
            ptpReport.ScanTime = DeviceContext->currentScanTime;

            BOOLEAN isPadClicked = (UsbData[DS_BUTTON_DATA_OFFSET] & DS_BUTTON_PAD_CLICK_MASK) != 0;
            if (isPadClicked) {
                ptpReport.Button = 1; // 1 bit for left click (PTP only requires single click state)
            }

            DeviceContext->Finger1Active = f1Active;
            DeviceContext->Finger1PrevX = f1X;
            DeviceContext->Finger1PrevY = f1Y;
            DeviceContext->Finger2Active = f2Active;
            DeviceContext->Finger2PrevX = f2X;
            DeviceContext->Finger2PrevY = f2Y;

            status = RequestCopyFromBuffer(request, &ptpReport, sizeof(ptpReport));
            WdfRequestComplete(request, status);
        }
    }

    NTSTATUS DualSenseConfigContReaderForInterruptEndPoint(PDEVICE_CONTEXT DeviceContext) {
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

    NTSTATUS DualSenseEvtDevicePrepareHardware(
            _In_ WDFDEVICE Device,
            _In_ WDFCMRESLIST ResourcesRaw,
            _In_ WDFCMRESLIST ResourcesTranslated
    ) {

        UNREFERENCED_PARAMETER(ResourcesRaw);
        UNREFERENCED_PARAMETER(ResourcesTranslated);

        NTSTATUS status = STATUS_SUCCESS;
        PDEVICE_CONTEXT deviceContext = NULL;
        UCHAR hidInterfaceNumber = 0;

        PAGED_CODE();

        print_kd("[DualSense] Entering PrepareHardware\n");

        deviceContext = GetDeviceContext(Device);

        status = DualSenseInitUsbTarget(Device, deviceContext);
        if (!NT_SUCCESS(status)) {
            print_kd("[DualSense] Failed to initialize USB Target Device. Status: 0x%08X\n", status);
            return status;
        }
        
        status = DualSenseInitializeHidInterface(Device, deviceContext, &hidInterfaceNumber);
        if (!NT_SUCCESS(status)) {
            print_kd("[DualSense] Failed to find HID Interface. Status: 0x%08X\n", status);
            return status;
        }

        status = DualSenseFetchHidAndReportDescriptors(Device, deviceContext, hidInterfaceNumber);
        if (!NT_SUCCESS(status)) {
            print_kd("[DualSense] Failed to fetch HID/Report Descriptors. Status: 0x%08X\n", status);
            return status;
        }

        status = DualSenseInitInterruptPipe(deviceContext);
        if (!NT_SUCCESS(status)) {
            print_kd("[DualSense] Failed to setup Interrupt Pipe. Status: 0x%08X\n", status);
            return status;
        }

        print_kd("[DualSense] PrepareHardware completed successfully\n");
        return STATUS_SUCCESS;
    }

    // Helper PrepareHardware: USB Target Init
    NTSTATUS DualSenseInitUsbTarget(_In_ WDFDEVICE Device, _In_ PDEVICE_CONTEXT DeviceContext) {
        NTSTATUS status = STATUS_SUCCESS;
        WDF_USB_DEVICE_SELECT_CONFIG_PARAMS configParams;

        PAGED_CODE();

        if (DeviceContext->UsbDevice == NULL) {
            status = WdfUsbTargetDeviceCreate(Device, WDF_NO_OBJECT_ATTRIBUTES, &DeviceContext->UsbDevice);
            if (!NT_SUCCESS(status)) {
                print_kd("[DualSense] WdfUsbTargetDeviceCreate failed: 0x%08X\n", status);
                return status;
            }
        }

        WDF_USB_DEVICE_SELECT_CONFIG_PARAMS_INIT_MULTIPLE_INTERFACES(&configParams, 0, NULL);
        status = WdfUsbTargetDeviceSelectConfig(DeviceContext->UsbDevice, WDF_NO_OBJECT_ATTRIBUTES, &configParams);
        if (!NT_SUCCESS(status)) {
            print_kd("WdfUsbTargetDeviceSelectConfig failed: 0x%08X\n", status);
        }

        return status;
    }

    // Helper PrepareHardware: Find HID Interface & Retrieve Device Descriptor
    NTSTATUS DualSenseInitializeHidInterface(_In_ WDFDEVICE Device, _In_ PDEVICE_CONTEXT DeviceContext, _Out_ PUCHAR
                                       HidInterfaceNumber) {
        NTSTATUS status = STATUS_SUCCESS;
        WDF_OBJECT_ATTRIBUTES attributes;
        BOOLEAN hidFound = FALSE;

        PAGED_CODE();

        *HidInterfaceNumber = 0;
        DeviceContext->HidInterface = NULL;
        DeviceContext->UsbInterfaceCount = WdfUsbTargetDeviceGetNumInterfaces(DeviceContext->UsbDevice);

        // 1. GUARD: Prevent STATUS_INVALID_PARAMETER if device reports 0 interfaces
        if (DeviceContext->UsbInterfaceCount == 0) {
            print_kd("[DualSense] Error: No USB Interfaces detected on the device.\n");
            return STATUS_DEVICE_CONFIGURATION_ERROR;
        }

        if (DeviceContext->UsbInterfacesMemoryHandle != NULL) {
            WdfObjectDelete(DeviceContext->UsbInterfacesMemoryHandle);
            DeviceContext->UsbInterfacesMemoryHandle = NULL;
        }

        WDF_OBJECT_ATTRIBUTES_INIT(&attributes);
        attributes.ParentObject = Device;

        size_t allocationSize = DeviceContext->UsbInterfaceCount * sizeof(WDFUSBINTERFACE);
        WDFUSBINTERFACE* localUsbInterfaces = NULL;

        status = WdfMemoryCreate(
                &attributes,
                NonPagedPoolNx,
                'SIKT',
                allocationSize,
                &DeviceContext->UsbInterfacesMemoryHandle,
                (PVOID*)&localUsbInterfaces
        );

        if (!NT_SUCCESS(status)) {
            print_kd("[DualSense] Memory allocation failed for UsbInterfaces array. Status: 0x%X\n", status);
            return status;
        }

        RtlZeroMemory(localUsbInterfaces, allocationSize);

        print_kd("[DualSense] Total USB Interfaces detected: %hhu\n", DeviceContext->UsbInterfaceCount);

        // iterating interfaces
        for (UCHAR i = 0; i < DeviceContext->UsbInterfaceCount; i++) {
            USB_INTERFACE_DESCRIPTOR interfaceDescriptor;

            localUsbInterfaces[i] = WdfUsbTargetDeviceGetInterface(DeviceContext->UsbDevice, i);

            WdfUsbInterfaceGetDescriptor(localUsbInterfaces[i], 0, &interfaceDescriptor);

            print_kd("[DualSense] Interface %u -> Class: 0x%02X, SubClass: 0x%02X\n",
                     i, interfaceDescriptor.bInterfaceClass, interfaceDescriptor.bInterfaceSubClass);

            if (interfaceDescriptor.bInterfaceClass == 0x03) {
                print_kd("[DualSense] HID Interface identified at index %u (Interface Number %u)\n", i,
                         interfaceDescriptor.bInterfaceNumber);

                DeviceContext->HidInterface = localUsbInterfaces[i];
                *HidInterfaceNumber = interfaceDescriptor.bInterfaceNumber;
                hidFound = TRUE;
            }
        }

        return hidFound ? STATUS_SUCCESS : STATUS_NOT_FOUND;
    }

    // Helper PrepareHardware: Fetch Descriptors via Control Transfer
    NTSTATUS DualSenseFetchHidAndReportDescriptors(_In_ WDFDEVICE Device, _In_ PDEVICE_CONTEXT DeviceContext, _In_ UCHAR
                                                   HidInterfaceNumber) {
        NTSTATUS status = STATUS_SUCCESS;
        WDF_USB_CONTROL_SETUP_PACKET controlSetupPacket;
        WDF_MEMORY_DESCRIPTOR memDesc;
        WDF_OBJECT_ATTRIBUTES attributes;
        ULONG bytesTransferred = 0;

        PAGED_CODE();

        //Getting HID Descriptor (Type 0x21)
        WDF_USB_CONTROL_SETUP_PACKET_INIT(
                &controlSetupPacket,
                BmRequestDeviceToHost,
                BmRequestToInterface,
                0x06, // USB_REQUEST_GET_DESCRIPTOR
                (0x21 << 8) | 0,
                HidInterfaceNumber
        );

        WDF_MEMORY_DESCRIPTOR_INIT_BUFFER(&memDesc, &DeviceContext->DsHidDescriptor, sizeof(HID_DESCRIPTOR));

        status = WdfUsbTargetDeviceSendControlTransferSynchronously(
                DeviceContext->UsbDevice,
                WDF_NO_HANDLE,
                NULL,
                &controlSetupPacket,
                &memDesc,
                &bytesTransferred
        );

        if (!NT_SUCCESS(status)) {
            print_kd("[DualSense] Control Transfer for HID Descriptor failed: 0x%08X\n", status);
            return status;
        }

        USHORT reportDescLength = DeviceContext->DsHidDescriptor.DescriptorList[0].wReportLength;

        if (reportDescLength == 0) {
            print_kd("[DualSense] Invalid Report Descriptor length (0) reported by device\n");
            return STATUS_DEVICE_DATA_ERROR;

        }

        print_kd("[DualSense] HID Descriptor fetched successfully. Report Descriptor Length: %u bytes\n", reportDescLength);

        if (DeviceContext->DsReportDescriptorHandle != NULL) {
            WdfObjectDelete(DeviceContext->DsReportDescriptorHandle);
            DeviceContext->DsReportDescriptorHandle = NULL;
        }

        //Report Descriptor
        WDF_OBJECT_ATTRIBUTES_INIT(&attributes);
        attributes.ParentObject = Device;
        status = WdfMemoryCreate(
                &attributes,
                NonPagedPoolNx,
                'SIKT',
                reportDescLength,
                &DeviceContext->DsReportDescriptorHandle,
                NULL
        );

        if (!NT_SUCCESS(status)) {
            print_kd("[DualSense] Allocation failed for Report Descriptor memory: 0x%08X\n", status);
            return status;
        }

        //Getting Report Descriptor (Type 0x22)
        WDF_USB_CONTROL_SETUP_PACKET_INIT(
                &controlSetupPacket,
                BmRequestDeviceToHost,
                BmRequestToInterface,
                0x06,
                (0x22 << 8) | 0,
                HidInterfaceNumber
        );

        PVOID reportDescBuffer = WdfMemoryGetBuffer(DeviceContext->DsReportDescriptorHandle, NULL);
        WDF_MEMORY_DESCRIPTOR_INIT_BUFFER(&memDesc, reportDescBuffer, reportDescLength);

        status = WdfUsbTargetDeviceSendControlTransferSynchronously(
                DeviceContext->UsbDevice,
                WDF_NO_HANDLE,
                NULL,
                &controlSetupPacket,
                &memDesc,
                &bytesTransferred
        );

        if (!NT_SUCCESS(status)) {
            print_kd("[DualSense] Control Transfer for Report Descriptor failed: 0x%08X\n", status);
            return status;
        }

        if (bytesTransferred != reportDescLength) {
            print_kd("[DualSense] Report Descriptor mismatch: Expected %u, Got %lu\n", reportDescLength, bytesTransferred);
            return STATUS_DEVICE_DATA_ERROR;
        }

        print_kd("[DualSense] Successfully fetched Report Descriptor (%lu bytes transferred)\n", bytesTransferred);
        return STATUS_SUCCESS;
    }

    // Helper PrepareHardware: Pipe & Continuous Reader Setup
    NTSTATUS DualSenseInitInterruptPipe(_In_ PDEVICE_CONTEXT DeviceContext) {
        UCHAR numPipes;
        PAGED_CODE();

        DeviceContext->HidInterruptPipe = NULL;
        numPipes = WdfUsbInterfaceGetNumConfiguredPipes(DeviceContext->HidInterface);

        for (UCHAR i = 0; i < numPipes; i++) {
            WDF_USB_PIPE_INFORMATION pipeInfo;
            WDF_USB_PIPE_INFORMATION_INIT(&pipeInfo);

            WDFUSBPIPE pipe = WdfUsbInterfaceGetConfiguredPipe(DeviceContext->HidInterface, i, &pipeInfo);

            if (pipeInfo.PipeType == WdfUsbPipeTypeInterrupt && WdfUsbTargetPipeIsInEndpoint(pipe)) {
                DeviceContext->HidInterruptPipe = pipe;
                print_kd("[DualSense] Interrupt IN Pipe configured at index %u\n", i);
                break;
            }
        }

        if (!DeviceContext->HidInterruptPipe) {
            print_kd("[DualSense] Interrupt IN Pipe not found on HID Interface\n");
            return STATUS_INVALID_DEVICE_STATE;
        }

        WdfUsbTargetPipeSetNoMaximumPacketSizeCheck(DeviceContext->HidInterruptPipe);

        return DualSenseConfigContReaderForInterruptEndPoint(DeviceContext);
    }
}