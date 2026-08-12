/*++

Module Name:

    device.c - Device handling events for example driver.

Abstract:

   This file contains the device entry points and callbacks.
    
Environment:

    Kernel-mode Driver Framework

--*/

#include "driver.h"

// Define the descriptors here to avoid LNK2005 multiply defined symbols
const unsigned char DualSenseReportDescriptor[] = {
    // =========================================================================
    // TOP LEVEL COLLECTION 1: GAMEPAD (Original DualSense Descriptor)
    // =========================================================================
    0x05, 0x01,        // Usage Page (Generic Desktop Ctrls) - This tells the OS we are defining a generic desktop device
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
    // TOP LEVEL COLLECTION 2: VIRTUAL MOUSE (Added by us!)
    // =========================================================================
    0x05, 0x01,        // Usage Page (Generic Desktop Ctrls)
    0x09, 0x02,        // Usage (Mouse) - Tells Windows this collection is a Mouse
    0xA1, 0x01,        // Collection (Application) - Start of the Mouse collection

    0x85, 0x41,        //   Report ID (65 / 0x41) - CHANGED TO BUST WINDOWS CACHE!

    0x09, 0x01,        //   Usage (Pointer) - The mouse controls a pointer
    0xA1, 0x00,        //   Collection (Physical) - Start of the physical pointer data

    // --- Mouse Buttons (Left, Right, Middle) ---
    0x05, 0x09,        //     Usage Page (Button)
    0x19, 0x01,        //     Usage Minimum (Button 1 - Left Click)
    0x29, 0x03,        //     Usage Maximum (Button 3 - Middle Click)
    0x15, 0x00,        //     Logical Minimum (0) - Unpressed
    0x25, 0x01,        //     Logical Maximum (1) - Pressed
    0x95, 0x03,        //     Report Count (3) - We have 3 buttons
    0x75, 0x01,        //     Report Size (1) - Each is 1 bit
    0x81, 0x02,        //     Input (Data,Var,Abs) - Declare the 3 button bits

    // --- Mouse Button Padding ---
    0x95, 0x05,        //     Report Count (1)
    0x81, 0x03,        //     Input (Const,Var,Abs) - (3 bits + 5 bits = 1 full byte for buttons)

    // --- Mouse X and Y Movement ---
    0x05, 0x01,        //     Usage Page (Generic Desktop Ctrls)
    0x09, 0x30,        //     Usage (X)
    0x09, 0x31,        //     Usage (Y)
    0x15, 0x81,        //     Logical Minimum (-127)  <-- 8-bit signed
    0x25, 0x7F,        //     Logical Maximum (127)   <-- 8-bit signed
    0x35, 0x00,        //     Physical Minimum (0)    <-- RESET
    0x45, 0x00,        //     Physical Maximum (0)    <-- RESET
    0x75, 0x08,        //     Report Size (8)
    0x95, 0x02,        //     Report Count (2)
    0x81, 0x06,        //     Input (Data,Var,Rel)
    
    0xC0,              //   End Collection (Ends Physical Pointer)
    0xC0               // End Collection (Ends the Mouse)
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
            sizeof(DualSenseReportDescriptor) // wReportLength
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
typedef struct _VIRTUAL_MOUSE_REPORT {
    UCHAR ReportId;
    UCHAR Buttons;
    CHAR  DeltaX;  // Changed to 8-bit CHAR
    CHAR  DeltaY;  // Changed to 8-bit CHAR
} VIRTUAL_MOUSE_REPORT, * PVIRTUAL_MOUSE_REPORT;
#pragma pack(pop)


#ifdef ALLOC_PRAGMA
#pragma alloc_text (PAGE, DualSenseCreateDevice)
#endif

NTSTATUS
DualSenseCreateDevice(
    _Inout_ PWDFDEVICE_INIT DeviceInit
    )
/*++

Routine Description:

    Worker routine called to create a device and its software resources.

Arguments:

    DeviceInit - Pointer to an opaque init structure. Memory for this
                    structure will be freed by the framework when the WdfDeviceCreate
                    succeeds. So don't access the structure after that point.

Return Value:

    NTSTATUS

--*/
{
    WDF_OBJECT_ATTRIBUTES deviceAttributes;
    WDF_PNPPOWER_EVENT_CALLBACKS  pnpPowerCallbacks;
    PDEVICE_CONTEXT deviceContext;
    WDFDEVICE device;
    NTSTATUS status;

    PAGED_CODE();

    WdfFdoInitSetFilter(DeviceInit);

    WDF_PNPPOWER_EVENT_CALLBACKS_INIT(&pnpPowerCallbacks);

    pnpPowerCallbacks.EvtDevicePrepareHardware = DualSenseEvtDevicePrepareHardware;

    //
// These two callbacks start and stop the wdfusb pipe continuous reader
// as we go in and out of the D0-working state.
//
    pnpPowerCallbacks.EvtDeviceD0Entry = DualSenseEvtDeviceD0Entry;
    pnpPowerCallbacks.EvtDeviceD0Exit = DualSenseEvtDeviceD0Exit;
    

    WdfDeviceInitSetPnpPowerEventCallbacks(DeviceInit, &pnpPowerCallbacks);


    WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(&deviceAttributes, DEVICE_CONTEXT);

    deviceAttributes.EvtCleanupCallback = DualSenseEvtDeviceContextCleanup;

    status = WdfDeviceCreate(&DeviceInit, &deviceAttributes, &device);

    if (!NT_SUCCESS(status)) {
        KdPrint(("Error: WdfDeviceCreate failed 0x%x\n", status));
        return status;
    }

    if (NT_SUCCESS(status)) {
        deviceContext = GetDeviceContext(device);

        status = WdfDeviceCreateDeviceInterface(
            device,
            &GUID_DEVINTERFACE_DualSense,
            NULL
            );

        if (NT_SUCCESS(status)) {
            status = DualSenseQueueInitialize(device);
        }
    }

    return status;
}

VOID
DualSenseEvtDeviceContextCleanup(
    _In_ WDFOBJECT DeviceObject
)
{
    PDEVICE_CONTEXT deviceContext = GetDeviceContext(DeviceObject);
    if (deviceContext->DsUsbInterfaces != NULL) {
        ExFreePoolWithTag(deviceContext->DsUsbInterfaces, 'SIKT');
        deviceContext->DsUsbInterfaces = NULL;
    }
}

PCHAR
DbgDevicePowerString(
    IN WDF_POWER_DEVICE_STATE Type
)
/*++

Updated Routine Description:
    DbgDevicePowerString does not change in this stage of the function driver.

--*/
{
    switch (Type)
    {
    case WdfPowerDeviceInvalid:
        return "WdfPowerDeviceInvalid";
    case WdfPowerDeviceD0:
        return "WdfPowerDeviceD0";
    case WdfPowerDeviceD1:
        return "WdfPowerDeviceD1";
    case WdfPowerDeviceD2:
        return "WdfPowerDeviceD2";
    case WdfPowerDeviceD3:
        return "WdfPowerDeviceD3";
    case WdfPowerDeviceD3Final:
        return "WdfPowerDeviceD3Final";
    case WdfPowerDevicePrepareForHibernation:
        return "WdfPowerDevicePrepareForHibernation";
    case WdfPowerDeviceMaximum:
        return "WdfPowerDeviceMaximum";
    default:
        return "UnKnown Device Power State";
    }
}

NTSTATUS
DualSenseEvtDeviceD0Entry(
    IN  WDFDEVICE Device,
    IN  WDF_POWER_DEVICE_STATE PreviousState
)
/*++

Routine Description:

    EvtDeviceD0Entry event callback must perform any operations that are
    necessary before the specified device is used.  It will be called every
    time the hardware needs to be (re-)initialized.

    This function is not marked pageable because this function is in the
    device power up path. When a function is marked pagable and the code
    section is paged out, it will generate a page fault which could impact
    the fast resume behavior because the client driver will have to wait
    until the system drivers can service this page fault.

    This function runs at PASSIVE_LEVEL, even though it is not paged.  A
    driver can optionally make this function pageable if DO_POWER_PAGABLE
    is set.  Even if DO_POWER_PAGABLE isn't set, this function still runs
    at PASSIVE_LEVEL.  In this case, though, the function absolutely must
    not do anything that will cause a page fault.

Arguments:

    Device - Handle to a framework device object.

    PreviousState - Device power state which the device was in most recently.
        If the device is being newly started, this will be
        PowerDeviceUnspecified.

Return Value:

    NTSTATUS

--*/
{
    PDEVICE_CONTEXT     deviceContext = NULL;
    NTSTATUS            status = STATUS_SUCCESS;

    deviceContext = GetDeviceContext(Device);

    print_kd("[DualSense] DualSenseEvtDeviceD0Entry Entry - coming from %s\n", DbgDevicePowerString(PreviousState));

    status = WdfIoTargetStart(WdfUsbTargetPipeGetIoTarget(deviceContext->HidInterruptPipe));
    if (NT_SUCCESS(status)) {
        print_kd("[DualSense] Successfully started hid interrupt pipe,  status: 0x%x\n", status);
        deviceContext->IsPowerUpSwitchState = TRUE;
    }

    print_kd("[DualSense] DualSenseEvtDeviceD0Entry Exit,  status: 0x%x\n", status);

    return status;
}


NTSTATUS
DualSenseEvtDeviceD0Exit(
    IN  WDFDEVICE Device,
    IN  WDF_POWER_DEVICE_STATE TargetState
)
{
    UNREFERENCED_PARAMETER(TargetState);

    PDEVICE_CONTEXT  deviceContext = GetDeviceContext(Device);
    WdfIoTargetStop(
        WdfUsbTargetPipeGetIoTarget(deviceContext->HidInterruptPipe),
        WdfIoTargetCancelSentIo
    );
    return STATUS_SUCCESS;
}

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

    //TODO: print_kd()
    //TraceEvents(TRACE_LEVEL_VERBOSE, DBG_INIT,
    //    "HidFx2EvtDevicePrepareHardware Enter\n");

    deviceContext = GetDeviceContext(Device);

    if (deviceContext->DsUsbDevice== NULL) {
        status = WdfUsbTargetDeviceCreate(Device,
            WDF_NO_OBJECT_ATTRIBUTES,
            &deviceContext->DsUsbDevice);

        if (!NT_SUCCESS(status)) {
            /*TODO: TraceEvents(TRACE_LEVEL_ERROR, DBG_PNP,
                "WdfUsbTargetDeviceCreate failed 0x%x\n", status);*/
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
        /*TODO: TraceEvents(TRACE_LEVEL_ERROR, DBG_PNP,
            "WdfUsbTargetDeviceSelectConfig failed %!STATUS!\n",
            status);*/
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



