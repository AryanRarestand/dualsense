/*++

Module Name:

    device.h

Abstract:

    This file contains the device definitions.

Environment:

    Kernel-mode Driver Framework

--*/
#include "public.h"

EXTERN_C_START

//
// The device context performs the same job as a WDM device extension in the driver frameworks
//
typedef struct _DEVICE_CONTEXT
{
    //
    // USB / HID
    //
    WDFUSBDEVICE       UsbDevice;
    WDFMEMORY          UsbInterfacesMemoryHandle;
    WDFUSBINTERFACE    HidInterface;
    UCHAR              UsbInterfaceCount;
    WDFUSBPIPE         HidInterruptPipe;

    HID_DESCRIPTOR     DsHidDescriptor;
    WDFMEMORY          DsReportDescriptorHandle;
    WDFMEMORY          DsDeviceDescriptorHandle;

    //
    // Touchpad
    //
    BOOLEAN             Finger1Active;
    SHORT               Finger1PrevX;
    SHORT               Finger1PrevY;
    BOOLEAN             Finger2Active;
    SHORT               Finger2PrevX;
    SHORT               Finger2PrevY;

    //
    // Power / Device State
    //
    UCHAR               CurrentSwitchState;
    UCHAR               LatestToggledSwitch;
    BOOLEAN             IsPowerUpSwitchState;

    //
    // I/O
    //
    WDFQUEUE             InterruptMsgQueue;
    WDFTIMER             DebounceTimer;

} DEVICE_CONTEXT, *PDEVICE_CONTEXT;

//
// This macro will generate an inline function called DeviceGetContext
// which will be used to get a pointer to the device context memory
// in a type safe manner.
//
WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(DEVICE_CONTEXT, GetDeviceContext)
EVT_WDF_OBJECT_CONTEXT_CLEANUP DualSenseEvtDeviceContextCleanup;

EVT_WDF_DEVICE_D0_ENTRY DualSenseEvtDeviceD0Entry;/*++

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

EVT_WDF_DEVICE_D0_EXIT DualSenseEvtDeviceD0Exit;

//
// Function to initialize the device and its callbacks
//
NTSTATUS DualSenseCreateDevice(_Inout_ PWDFDEVICE_INIT DeviceInit);/*++

Routine Description:

    Worker routine called to create a device and its software resources.

Arguments:

    DeviceInit - Pointer to an opaque init structure. Memory for this
                    structure will be freed by the framework when the WdfDeviceCreate
                    succeeds. So don't access the structure after that point.

Return Value:

    NTSTATUS

--*/

EXTERN_C_END