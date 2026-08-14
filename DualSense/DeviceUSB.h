#ifndef DUALSENSE_DEVICEUSB_H
#define DUALSENSE_DEVICEUSB_H


namespace DeviceUSB{
    NTSTATUS
    DualSenseConfigContReaderForInterruptEndPoint(PDEVICE_CONTEXT DeviceContext); /*++

    Routine Description:

        This routine configures a continuous reader on the
        interrupt endpoint. It's called from the PrepareHarware event.

    Arguments:

        DeviceContext - Pointer to device context structure

    Return Value:

        NT status value

    --*/

    EVT_WDF_USB_READER_COMPLETION_ROUTINE DualSenseEvtUsbInterruptPipeReadComplete;

    EVT_WDF_DEVICE_PREPARE_HARDWARE DualSenseEvtDevicePrepareHardware;
}


#endif //DUALSENSE_DEVICEUSB_H
