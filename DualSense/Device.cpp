/*++

Module Name:

    device.c - Device handling events for example driver.

Abstract:

   This file contains the device entry points and callbacks.
    
Environment:

    Kernel-mode Driver Framework

--*/

#include "driver.h"

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
{
    WDF_OBJECT_ATTRIBUTES deviceAttributes;
    WDF_PNPPOWER_EVENT_CALLBACKS  pnpPowerCallbacks;
    PDEVICE_CONTEXT deviceContext;
    WDFDEVICE device;
    NTSTATUS status;

    PAGED_CODE();

    WdfFdoInitSetFilter(DeviceInit);

    WDF_PNPPOWER_EVENT_CALLBACKS_INIT(&pnpPowerCallbacks);

    pnpPowerCallbacks.EvtDevicePrepareHardware = DeviceUSB::DualSenseEvtDevicePrepareHardware;

    // These two callbacks start and stop the wdfusb pipe continuous reader as we go in and out of the D0-working state.

    pnpPowerCallbacks.EvtDeviceD0Entry = DualSenseEvtDeviceD0Entry;
    pnpPowerCallbacks.EvtDeviceD0Exit = DualSenseEvtDeviceD0Exit;

    WdfDeviceInitSetPnpPowerEventCallbacks(DeviceInit, &pnpPowerCallbacks);

    WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(&deviceAttributes, DEVICE_CONTEXT);

    deviceAttributes.EvtCleanupCallback = DualSenseEvtDeviceContextCleanup;

    //WDF Device
    status = WdfDeviceCreate(&DeviceInit, &deviceAttributes, &device);
    if (!NT_SUCCESS(status)) {
        print_kd(("[DualSense] Error: WdfDeviceCreate failed 0x%x\n", status));
        return status;
    }

    //Device Interface
    deviceContext = GetDeviceContext(device);
    status = WdfDeviceCreateDeviceInterface(device, &GUID_DEVINTERFACE_DualSense, NULL);
    if (!NT_SUCCESS(status)) {
        print_kd(("[DualSense] Error: Failed to create device interface 0x%x\n", status));
        return status;
    }

    //I/O Queue Initialize
    status = DualSenseQueueInitialize(device);
    if (!NT_SUCCESS(status)) {
        print_kd(("[DualSense] Error: DualSenseQueueInitialize failed 0x%x\n", status));
        return status;
    }

    return status;

    //NOTE: deviceContext is not used, so it can be removed
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


NTSTATUS
DualSenseEvtDeviceD0Entry(
    IN  WDFDEVICE Device,
    IN  WDF_POWER_DEVICE_STATE PreviousState
)
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

