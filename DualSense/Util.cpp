#include "Driver.h"

void print_kd(_In_z_ _Printf_format_string_ PCSTR format, ...) {
    va_list args;
    va_start(args, format);

    vDbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, format, args);

    va_end(args);
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
