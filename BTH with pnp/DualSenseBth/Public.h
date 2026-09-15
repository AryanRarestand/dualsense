/*++

Module Name:

    public.h

Abstract:

    This module contains the common declarations shared by driver
    and user applications.

Environment:

    user and kernel

--*/

//
// Define an Interface Guid so that apps can find the device and talk to it.
//

extern const unsigned char DualSenseBTHReportDescriptor[];
extern HID_DESCRIPTOR DualSenseBTHDeviceDescriptor;
extern HID_DEVICE_ATTRIBUTES DualSenseBTHDeviceAttributes;


DEFINE_GUID (GUID_DEVINTERFACE_DualSenseBth,
    0x3bfe3ec4,0xb9b5,0x4114,0x8c,0x17,0xe8,0x88,0xe4,0x85,0xe8,0xbb);
// {3bfe3ec4-b9b5-4114-8c17-e888e485e8bb}
