/*++

Module Name:

    driver.h

Abstract:

    This file contains the driver definitions.

Environment:

    Kernel-mode Driver Framework

--*/

#include <ntddk.h>
#include <wdf.h>
#include <stdarg.h>
// Core USB definitions must come FIRST
#include <usb.h>
#include <usbdlib.h>
#include <usbioctl.h>
// WDF USB extensions come AFTER
#include <wdfusb.h>
#include <initguid.h>
#include <hidport.h>

#include "util.h"
#include "device.h"
#include "queue.h"
#include "trace.h"

#define INTERRUPT_ENDPOINT_INDEX     (0)
#define SWICTHPACK_DEBOUNCE_TIME_IN_MS   10

EXTERN_C_START

//
// WDFDRIVER Events
//

DRIVER_INITIALIZE DriverEntry;
EVT_WDF_DRIVER_DEVICE_ADD DualSenseEvtDeviceAdd;
EVT_WDF_OBJECT_CONTEXT_CLEANUP DualSenseEvtDriverContextCleanup;

EXTERN_C_END