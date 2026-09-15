#include "driver.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text (PAGE, InitializeAsVirtualController)
#pragma alloc_text (PAGE, InitializeAsFilterAndBus)
#endif

NTSTATUS
InitializeAsVirtualController(
    _Inout_ PWDFDEVICE_INIT DeviceInit
    )
{
    WDF_OBJECT_ATTRIBUTES deviceAttributes;
    WDFDEVICE device;
    NTSTATUS status;

    PAGED_CODE();

    WdfFdoInitSetFilter(DeviceInit);

    WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(&deviceAttributes, DEVICE_CONTEXT);
    deviceAttributes.EvtCleanupCallback = DualSenseBthEvtDeviceContextCleanup;

    status = WdfDeviceCreate(&DeviceInit, &deviceAttributes, &device);
    if (!NT_SUCCESS(status)) {
        print_kd("[DualSenseBth] Error: WdfDeviceCreate failed 0x%x\n", status);
        return status;
    }

	status = DualSenseBthQueueInitialize(device);
    if (!NT_SUCCESS(status)) {
        print_kd("[DualSenseBth] Error: DualSenseBthQueueInitialize failed 0x%x\n", status);
        return status;
	}

    return status;
}

VOID
PdoReadCompletion(
    _In_ WDFREQUEST Request,
    _In_ WDFIOTARGET Target,
    _In_ PWDF_REQUEST_COMPLETION_PARAMS Params,
    _In_ WDFCONTEXT Context
    )
{
    UNREFERENCED_PARAMETER(Target);
    WDFREQUEST OriginalRequest = (WDFREQUEST)Context;

    NTSTATUS status = Params->IoStatus.Status;
    ULONG_PTR information = Params->IoStatus.Information;

    // We MUST delete the new request BEFORE completing the original request.
    // The new request holds a reference to the original request's memory buffer.
    // Completing the original request while references exist causes BugCheck 0x10D!
    WdfObjectDelete(Request);

    WdfRequestCompleteWithInformation(OriginalRequest, status, information);
}

VOID
PdoEvtIoInternalDeviceControl(
    _In_ WDFQUEUE Queue,
    _In_ WDFREQUEST Request,
    _In_ size_t OutputBufferLength,
    _In_ size_t InputBufferLength,
    _In_ ULONG IoControlCode
    )
{
    WDFDEVICE pdo = WdfIoQueueGetDevice(Queue);
    WDFDEVICE filterDevice = WdfPdoGetParent(pdo);
    PDEVICE_CONTEXT filterContext = DeviceGetContext(filterDevice);
    NTSTATUS status;

    UNREFERENCED_PARAMETER(InputBufferLength);
    UNREFERENCED_PARAMETER(OutputBufferLength);

    if (IoControlCode == IOCTL_HID_READ_REPORT) {
        // print_kd("[DualSense] IOCTL_HID_READ_REPORT received on the filter\n");
        WDFMEMORY outputMem;
        status = WdfRequestRetrieveOutputMemory(Request, &outputMem);
        if (NT_SUCCESS(status)) {
            
            // We MUST create a new request because the original request belongs to the 
            // Virtual Controller's device stack and does not have enough I/O stack locations
            // to travel down the physical controller's device stack!
            WDFREQUEST newRequest;
            status = WdfRequestCreate(WDF_NO_OBJECT_ATTRIBUTES, filterContext->RemoteTarget, &newRequest);
            if (NT_SUCCESS(status)) {
                status = WdfIoTargetFormatRequestForRead(
                    filterContext->RemoteTarget,
                    newRequest,
                    outputMem,
                    NULL,
                    NULL
                );
                
                if (NT_SUCCESS(status)) {
                    // Pass the ORIGINAL request as the context so we can complete it later!
                    WdfRequestSetCompletionRoutine(newRequest, PdoReadCompletion, Request);
                    if (WdfRequestSend(newRequest, filterContext->RemoteTarget, WDF_NO_SEND_OPTIONS) == FALSE) {
                        status = WdfRequestGetStatus(newRequest);
                        print_kd("[DualSense] WdfRequestSend failed 0x%x\n", status);
                        // Delete newRequest BEFORE completing original request
                        WdfObjectDelete(newRequest);
                        WdfRequestComplete(Request, status);
                    }
                    return;
                } else {
                    print_kd("[DualSense] WdfIoTargetFormatRequestForRead failed 0x%x\n", status);
                    WdfObjectDelete(newRequest);
                }
            } else {
                print_kd("[DualSense] WdfRequestCreate failed 0x%x\n", status);
            }
        }
        else {
            print_kd("[DualSense] WdfRequestRetrieveOutputMemory failed 0x%x\n", status);
        }
        WdfRequestComplete(Request, status);
        return;
    }

    WdfRequestComplete(Request, STATUS_NOT_SUPPORTED);
}


NTSTATUS
InitializeAsFilterAndBus(
    _Inout_ PWDFDEVICE_INIT DeviceInit
    )
{
    WDF_OBJECT_ATTRIBUTES deviceAttributes;
    WDFDEVICE device;
    NTSTATUS status;

    PAGED_CODE();

    WdfFdoInitSetFilter(DeviceInit);

    WDF_PNPPOWER_EVENT_CALLBACKS pnpCallbacks;
    WDF_PNPPOWER_EVENT_CALLBACKS_INIT(&pnpCallbacks);
    pnpCallbacks.EvtDeviceSelfManagedIoInit = FilterEvtDeviceSelfManagedIoInit;
    pnpCallbacks.EvtDeviceSelfManagedIoCleanup = FilterEvtDeviceSelfManagedIoCleanup;
    WdfDeviceInitSetPnpPowerEventCallbacks(DeviceInit, &pnpCallbacks);

    WDF_FILEOBJECT_CONFIG fileConfig;
    WDF_FILEOBJECT_CONFIG_INIT(&fileConfig,
                               DualSenseBthEvtDeviceFileCreate,
                               WDF_NO_EVENT_CALLBACK,
                               WDF_NO_EVENT_CALLBACK);
    
    WdfDeviceInitSetFileObjectConfig(DeviceInit, &fileConfig, WDF_NO_OBJECT_ATTRIBUTES);

    WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(&deviceAttributes, DEVICE_CONTEXT);
    deviceAttributes.EvtCleanupCallback = DualSenseBthEvtDeviceContextCleanup;

    status = WdfDeviceCreate(&DeviceInit, &deviceAttributes, &device);
    if (!NT_SUCCESS(status)) {
        return status;
    }

    PDEVICE_CONTEXT deviceContext = DeviceGetContext(device);

    status = WdfIoTargetCreate(device, WDF_NO_OBJECT_ATTRIBUTES, &deviceContext->RemoteTarget);
    if (!NT_SUCCESS(status)) {
        return status;
    }

    PWDFDEVICE_INIT pdoInit = WdfPdoInitAllocate(device);
    if (pdoInit == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    DECLARE_CONST_UNICODE_STRING(hwId, L"BTHENUM\\VirtualDualSense");
    status = WdfPdoInitAssignDeviceID(pdoInit, &hwId);
    status = WdfPdoInitAddHardwareID(pdoInit, &hwId);
    DECLARE_CONST_UNICODE_STRING(deviceLocation, L"Virtual DualSense Controller");
    status = WdfPdoInitAddDeviceText(pdoInit, &deviceLocation, &deviceLocation, 0x409);

    WDFDEVICE pdo;
    WDF_OBJECT_ATTRIBUTES pdoAttributes;
    WDF_OBJECT_ATTRIBUTES_INIT(&pdoAttributes);
    
    status = WdfDeviceCreate(&pdoInit, &pdoAttributes, &pdo);
    if (!NT_SUCCESS(status)) {
        WdfDeviceInitFree(pdoInit);
        return status;
    }

    status = WdfFdoAddStaticChild(device, pdo);
    if (!NT_SUCCESS(status)) {
        WdfObjectDelete(pdo);
        return status;
    }

    deviceContext->ChildPdo = pdo;

    WDF_IO_QUEUE_CONFIG pdoQueueConfig;
    WDF_IO_QUEUE_CONFIG_INIT_DEFAULT_QUEUE(&pdoQueueConfig, WdfIoQueueDispatchParallel);
    pdoQueueConfig.PowerManaged = WdfFalse;
    pdoQueueConfig.EvtIoInternalDeviceControl = PdoEvtIoInternalDeviceControl;

    status = WdfIoQueueCreate(pdo, &pdoQueueConfig, WDF_NO_OBJECT_ATTRIBUTES, &deviceContext->PdoManualQueue);
    
    return status;
}

VOID
DualSenseBthEvtDeviceFileCreate(
    _In_ WDFDEVICE Device,
    _In_ WDFREQUEST Request,
    _In_ WDFFILEOBJECT FileObject
    )
{
    UNREFERENCED_PARAMETER(FileObject);

    KPROCESSOR_MODE mode = WdfRequestGetRequestorMode(Request);
    
    if (mode == UserMode) {
        print_kd("[DualSenseBth] User-mode IRP_MJ_CREATE blocked!\n");
        WdfRequestComplete(Request, STATUS_ACCESS_DENIED);
        return;
    }

    WDF_REQUEST_SEND_OPTIONS options;
    WDF_REQUEST_SEND_OPTIONS_INIT(&options, WDF_REQUEST_SEND_OPTION_SEND_AND_FORGET);
    WdfRequestFormatRequestUsingCurrentType(Request);
    BOOLEAN ret = WdfRequestSend(Request, WdfDeviceGetIoTarget(Device), &options);
    if (ret == FALSE) {
        WdfRequestComplete(Request, WdfRequestGetStatus(Request));
    }
}

VOID
DualSenseBthEvtDeviceContextCleanup(
    _In_ WDFOBJECT DeviceObject
)
{
    UNREFERENCED_PARAMETER(DeviceObject);
}

const GUID GUID_DEVINTERFACE_HID_LOCAL = { 0x4D1E55B2, 0xF16F, 0x11CF, { 0x88, 0xCB, 0x00, 0x11, 0x11, 0x00, 0x00, 0x30 } };
const GUID GUID_DEVICE_INTERFACE_ARRIVAL_LOCAL = { 0xcb3a4004, 0x46f0, 0x11d0, { 0xb0, 0x8f, 0x00, 0x60, 0x97, 0x13, 0x05, 0x3f } };

NTSTATUS
PnpNotificationCallback(
    _In_ PVOID NotificationStructure,
    _Inout_opt_ PVOID Context
    )
{
    PDEVICE_INTERFACE_CHANGE_NOTIFICATION Notification = (PDEVICE_INTERFACE_CHANGE_NOTIFICATION)NotificationStructure;
    WDFDEVICE Device = (WDFDEVICE)Context;
    PDEVICE_CONTEXT deviceContext = DeviceGetContext(Device);

    if (IsEqualGUID(Notification->Event, GUID_DEVICE_INTERFACE_ARRIVAL_LOCAL)) {
        print_kd("[DualSense] Device Interface Arrived: %wZ\n", Notification->SymbolicLinkName);
        
        WDF_IO_TARGET_OPEN_PARAMS openParams;
        WDF_IO_TARGET_OPEN_PARAMS_INIT_OPEN_BY_NAME(&openParams, Notification->SymbolicLinkName, GENERIC_READ);
        openParams.ShareAccess = FILE_SHARE_READ | FILE_SHARE_WRITE;

        NTSTATUS status = WdfIoTargetOpen(deviceContext->RemoteTarget, &openParams);
        if (NT_SUCCESS(status)) {
            print_kd("[DualSense] Remote Target OPENED successfully via PnP Callback!\n");
        } else {
            print_kd("[DualSense] WdfIoTargetOpen in PnP Callback failed 0x%x\n", status);
        }
    }
    return STATUS_SUCCESS;
}

NTSTATUS
FilterEvtDeviceSelfManagedIoInit(
    _In_ WDFDEVICE Device
    )
{
    PDEVICE_CONTEXT deviceContext = DeviceGetContext(Device);
    NTSTATUS status;

    status = IoRegisterPlugPlayNotification(
        EventCategoryDeviceInterfaceChange,
        PNPNOTIFY_DEVICE_INTERFACE_INCLUDE_EXISTING_INTERFACES,
        (PVOID)&GUID_DEVINTERFACE_HID_LOCAL,
        WdfDriverWdmGetDriverObject(WdfGetDriver()),
        PnpNotificationCallback,
        (PVOID)Device,
        &deviceContext->NotificationEntry
    );

    if (!NT_SUCCESS(status)) {
        print_kd("[DualSense] IoRegisterPlugPlayNotification failed 0x%x\n", status);
    } else {
        print_kd("[DualSense] PnP Notification Registered!\n");
    }

    return STATUS_SUCCESS;
}

VOID
FilterEvtDeviceSelfManagedIoCleanup(
    _In_ WDFDEVICE Device
    )
{
    PDEVICE_CONTEXT deviceContext = DeviceGetContext(Device);
    
    if (deviceContext->NotificationEntry) {
        IoUnregisterPlugPlayNotificationEx(deviceContext->NotificationEntry);
        deviceContext->NotificationEntry = NULL;
    }

    WdfIoTargetClose(deviceContext->RemoteTarget);
}
