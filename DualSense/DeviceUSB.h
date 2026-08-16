#ifndef DUALSENSE_DEVICEUSB_H
#define DUALSENSE_DEVICEUSB_H



namespace DeviceUSB{
    extern const unsigned char DualSenseUSBReportDescriptor[];
    extern HID_DESCRIPTOR DualSenseDeviceDescriptor;

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
    NTSTATUS DualSenseInitUsbTarget(_In_ WDFDEVICE Device, _In_ PDEVICE_CONTEXT DeviceContext);
    NTSTATUS DualSenseInitializeHidInterface(_In_ WDFDEVICE Device, _In_ PDEVICE_CONTEXT DeviceContext, _Out_ PUCHAR HidInterfaceNumber);
    NTSTATUS DualSenseFetchHidAndReportDescriptors(_In_ WDFDEVICE Device, _In_ PDEVICE_CONTEXT DeviceContext, _In_ UCHAR HidInterfaceNumber);
    NTSTATUS DualSenseInitInterruptPipe(_In_ PDEVICE_CONTEXT DeviceContext);
}


#endif //DUALSENSE_DEVICEUSB_H
