$content = Get-Content C:\Users\aryan\source\repos\DualSenseBth\DualSenseBth\Device.cpp -Raw -Encoding Unicode
$original = '(?s)NTSTATUS\r?\nPnpNotificationCallback.*?return STATUS_SUCCESS;\r?\n\}'
$replacement = @'
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
        
        PWSTR symbolicLinkList = NULL;
        // Query the device interfaces registered specifically for OUR physical device object
        NTSTATUS status = IoGetDeviceInterfaces(
            (LPGUID)&GUID_DEVINTERFACE_HID_LOCAL, 
            WdfDeviceWdmGetPhysicalDevice(Device), 
            0, 
            &symbolicLinkList
        );
        
        if (NT_SUCCESS(status) && symbolicLinkList != NULL) {
            BOOLEAN found = FALSE;
            PWSTR currentLink = symbolicLinkList;
            
            // IoGetDeviceInterfaces returns a MULTI_SZ (double null-terminated) string
            while (*currentLink != L'\0') {
                UNICODE_STRING linkString;
                RtlInitUnicodeString(&linkString, currentLink);
                
                // If the arriving interface matches our device's interface
                if (RtlEqualUnicodeString(Notification->SymbolicLinkName, &linkString, TRUE)) {
                    found = TRUE;
                    break;
                }
                currentLink += wcslen(currentLink) + 1;
            }
            
            ExFreePool(symbolicLinkList);
            
            if (found) {
                print_kd("[DualSense] Our specific Device Interface Arrived: %wZ\n", Notification->SymbolicLinkName);
                
                WDF_IO_TARGET_OPEN_PARAMS openParams;
                WDF_IO_TARGET_OPEN_PARAMS_INIT_OPEN_BY_NAME(&openParams, Notification->SymbolicLinkName, GENERIC_READ);
                openParams.ShareAccess = FILE_SHARE_READ | FILE_SHARE_WRITE;

                status = WdfIoTargetOpen(deviceContext->RemoteTarget, &openParams);
                if (NT_SUCCESS(status)) {
                    print_kd("[DualSense] Remote Target OPENED successfully via PnP Callback!\n");
                } else {
                    print_kd("[DualSense] WdfIoTargetOpen in PnP Callback failed 0x%x\n", status);
                }
            }
        }
    }
    return STATUS_SUCCESS;
}
'@
$newContent = $content -replace $original, $replacement
Set-Content C:\Users\aryan\source\repos\DualSenseBth\DualSenseBth\Device.cpp -Value $newContent -Encoding Unicode
