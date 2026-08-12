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
DEFINE_GUID (GUID_DEVINTERFACE_DualSense,
    0x663648f0,0x7f9c,0x4965,0xa9,0x76,0xee,0x69,0x11,0x53,0x16,0xbc);
// {663648f0-7f9c-4965-a976-ee69115316bc}
