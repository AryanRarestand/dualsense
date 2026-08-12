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


extern const unsigned char DualSenseReportDescriptor[];
extern HID_DESCRIPTOR DualSenseDeviceDescriptor;

//
// The device context performs the same job as
// a WDM device extension in the driver frameworks
//
typedef struct _DEVICE_CONTEXT
{

    //
    //WDF handles for USB Target 
    //
    WDFUSBDEVICE      DsUsbDevice;
    WDFUSBINTERFACE*  DsUsbInterfaces;
    WDFUSBINTERFACE   DsHidUsbInterface;
    UCHAR             NumDsUsbInterfaces;
    WDFUSBPIPE        HidInterruptPipe;

    HID_DESCRIPTOR    DsHidDescriptor;
    WDFMEMORY         DsReportDescriptorHandle;

    BOOLEAN Finger1Active;
    SHORT   Finger1PrevX;
    SHORT   Finger1PrevY;
    BOOLEAN Finger2Active;
    SHORT   Finger2PrevX;
    SHORT   Finger2PrevY;

    //
    //Device descriptor for the USB device
    //
    WDFMEMORY DsDeviceDescriptorHandle;

    //
    // Switch state.
    //
    UCHAR    CurrentSwitchState;

    //
    // This variable stores state for the swicth that got toggled most recently
    // (the device returns the state of all the switches and not just the 
    // one that got toggled).
    //
    UCHAR    LatestToggledSwitch;

    //
    // Interrupt endpoints sends switch state when first started 
    // or when resuming from suspend. We need to ignore that data.
    //
    BOOLEAN  IsPowerUpSwitchState;

    //
    // WDF Queue for read IOCTLs from hidclass that get satisfied from 
    // USB interrupt endpoint
    //
    WDFQUEUE   InterruptMsgQueue;

    //
    // Handle debouncing of switchpack
    //
    WDFTIMER DebounceTimer;

} DEVICE_CONTEXT, *PDEVICE_CONTEXT;

//
// This macro will generate an inline function called DeviceGetContext
// which will be used to get a pointer to the device context memory
// in a type safe manner.
//
WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(DEVICE_CONTEXT, GetDeviceContext)


EVT_WDF_DEVICE_PREPARE_HARDWARE DualSenseEvtDevicePrepareHardware;
EVT_WDF_OBJECT_CONTEXT_CLEANUP DualSenseEvtDeviceContextCleanup;

EVT_WDF_DEVICE_D0_ENTRY DualSenseEvtDeviceD0Entry;
EVT_WDF_DEVICE_D0_EXIT DualSenseEvtDeviceD0Exit;

EVT_WDF_USB_READER_COMPLETION_ROUTINE DualSenseEvtUsbInterruptPipeReadComplete;

NTSTATUS
DualSenseConfigContReaderForInterruptEndPoint(
    PDEVICE_CONTEXT DeviceContext
    );

//
// Function to initialize the device and its callbacks
//
NTSTATUS
DualSenseCreateDevice(
    _Inout_ PWDFDEVICE_INIT DeviceInit
    );

EXTERN_C_END
