#include "driver.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text (INIT, DriverEntry)
#pragma alloc_text (PAGE, DualSenseBthEvtDeviceAdd)
#pragma alloc_text (PAGE, DualSenseBthEvtDriverContextCleanup)
#endif

NTSTATUS
DriverEntry(
    _In_ PDRIVER_OBJECT  DriverObject,
    _In_ PUNICODE_STRING RegistryPath
    )
{
    WDF_DRIVER_CONFIG config;
    NTSTATUS status;
    WDF_OBJECT_ATTRIBUTES attributes;
    print_kd("[+] ---------------------------------DualSenseBth Entry---------------------------------\n");

    print_kd("[DualSenseBth] DriverEntry Entry\n");

    PAGED_CODE();
    ExInitializeDriverRuntime(DrvRtPoolNxOptIn);

    WDF_OBJECT_ATTRIBUTES_INIT(&attributes);
    attributes.EvtCleanupCallback = DualSenseBthEvtDriverContextCleanup;

    WDF_DRIVER_CONFIG_INIT(&config, DualSenseBthEvtDeviceAdd);

    status = WdfDriverCreate(DriverObject,
                             RegistryPath,
                             &attributes,
                             &config,
                             WDF_NO_HANDLE
                             );

    if (!NT_SUCCESS(status)) {
        print_kd("WdfDriverCreate failed!\n");
        return status;
    }

    print_kd("[DualSenseBth] DriverEntry Exit\n");

    return status;
}

NTSTATUS DualSenseBthEvtDeviceAdd(
    _In_    WDFDRIVER       Driver,
    _Inout_ PWDFDEVICE_INIT DeviceInit
    )
{
    NTSTATUS status;

    UNREFERENCED_PARAMETER(Driver);

    PAGED_CODE();

    print_kd("[DualSenseBth] Device Add Entry\n");

    WDFMEMORY memory;
    status = WdfFdoInitAllocAndQueryProperty(DeviceInit, DevicePropertyHardwareID, NonPagedPoolNx, WDF_NO_OBJECT_ATTRIBUTES, &memory);
    if (NT_SUCCESS(status)) {
        size_t bufferLength;
        PWCHAR hardwareId = (PWCHAR)WdfMemoryGetBuffer(memory, &bufferLength);
        
        PWCHAR currStr = hardwareId;
        BOOLEAN isVirtual = FALSE;
        while (*currStr != L'\0') {
            if (wcsstr(currStr, L"VirtualDualSense") != NULL) {
                isVirtual = TRUE;
                break;
            }
            currStr += wcslen(currStr) + 1;
        }
        WdfObjectDelete(memory);

        if (isVirtual) {
            print_kd("[DualSenseBth] Initializing as Virtual Controller\n");
            status = InitializeAsVirtualController(DeviceInit);
        } else {
            print_kd("[DualSenseBth] Initializing as Filter and Bus Driver\n");
            status = InitializeAsFilterAndBus(DeviceInit);
        }
    } else {
        print_kd("[DualSenseBth] WdfFdoInitAllocAndQueryProperty failed 0x%x. Defaulting to Filter.\n", status);
        status = InitializeAsFilterAndBus(DeviceInit);
    }

    print_kd("[DualSenseBth] Device Add Exit\n");

    return status;
}

VOID DualSenseBthEvtDriverContextCleanup(
    _In_ WDFOBJECT DriverObject
    )
{
    UNREFERENCED_PARAMETER(DriverObject);

    PAGED_CODE();

    print_kd("[DualSenseBth] DualSenseBthEvtDriverContextCleanup\n");
}
