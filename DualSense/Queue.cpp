/*++

Module Name:

    queue.c

Abstract:

    This file contains the queue entry points and callbacks.

Environment:

    Kernel-mode Driver Framework

--*/

#include "driver.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text (PAGE, DualSenseQueueInitialize)
#endif

NTSTATUS
DualSenseQueueInitialize(
    _In_ WDFDEVICE Device
)
{
    WDFQUEUE queue;
    NTSTATUS status;
    WDF_IO_QUEUE_CONFIG queueConfig;
    PDEVICE_CONTEXT deviceContext = GetDeviceContext(Device);

    PAGED_CODE();

    WDF_IO_QUEUE_CONFIG_INIT_DEFAULT_QUEUE(&queueConfig, WdfIoQueueDispatchParallel);
    queueConfig.EvtIoInternalDeviceControl = DualSenseEvtIoInternalDeviceControl;

    status = WdfIoQueueCreate(Device,
        &queueConfig,
        WDF_NO_OBJECT_ATTRIBUTES,
        &queue
    );
    if (!NT_SUCCESS(status)) {
        print_kd("[DualSense] WdfIoQueueCreate failed 0x%x\n", status);
        return status;
    }

    WDF_IO_QUEUE_CONFIG_INIT(&queueConfig, WdfIoQueueDispatchManual);
    queueConfig.PowerManaged = WdfFalse;

    status = WdfIoQueueCreate(Device,
        &queueConfig,
        WDF_NO_OBJECT_ATTRIBUTES,
        &deviceContext->InterruptMsgQueue
    );

    if (!NT_SUCCESS(status)) {
        print_kd("[DualSense] WdfIoQueueCreate failed 0x%x\n", status);
        return status;
    }

    return status;
}


VOID
DualSenseEvtIoInternalDeviceControl(
    _In_  WDFQUEUE          Queue,
    _In_  WDFREQUEST        Request,
    _In_  size_t            OutputBufferLength,
    _In_  size_t            InputBufferLength,
    _In_  ULONG             IoControlCode
) {

    NTSTATUS            status = STATUS_SUCCESS;
    WDFDEVICE           device;
    PDEVICE_CONTEXT     deviceContext;

    device = WdfIoQueueGetDevice(Queue);
    deviceContext = GetDeviceContext(device);

    UNREFERENCED_PARAMETER(InputBufferLength);
    UNREFERENCED_PARAMETER(OutputBufferLength);

    switch (IoControlCode) {
    case IOCTL_HID_GET_DEVICE_DESCRIPTOR: { 
        status = RequestCopyFromBuffer(Request,
            &DeviceUSB::DualSenseDeviceDescriptor,
            deviceContext->DsHidDescriptor.bLength);

        break;
    }
    case IOCTL_HID_GET_DEVICE_ATTRIBUTES:
    {
        HID_DEVICE_ATTRIBUTES deviceAttributes;

        // Zero out the struct and set its size
        RtlZeroMemory(&deviceAttributes, sizeof(HID_DEVICE_ATTRIBUTES));
        deviceAttributes.Size = sizeof(HID_DEVICE_ATTRIBUTES);

        // Get the raw pointer to the USB Device Descriptor we fetched in PrepareHardware
        PUSB_DEVICE_DESCRIPTOR usbDeviceDesc =
            (PUSB_DEVICE_DESCRIPTOR)WdfMemoryGetBuffer(deviceContext->DsDeviceDescriptorHandle, NULL);

        // Copy the Sony Vendor ID and DualSense Product ID over
        deviceAttributes.VendorID = usbDeviceDesc->idVendor;
        deviceAttributes.ProductID = usbDeviceDesc->idProduct;
        deviceAttributes.VersionNumber = usbDeviceDesc->bcdDevice;
        status = RequestCopyFromBuffer(Request, &deviceAttributes, sizeof(HID_DEVICE_ATTRIBUTES));
        break;
    }

    case IOCTL_HID_GET_REPORT_DESCRIPTOR:
    {
        status = RequestCopyFromBuffer(Request,
            (PVOID) &DeviceUSB::DualSenseUSBReportDescriptor,
            DeviceUSB::DualSenseDeviceDescriptor.DescriptorList[0].wReportLength);

        break;
    }

    case IOCTL_HID_READ_REPORT:
        status = WdfRequestForwardToIoQueue(Request, deviceContext->InterruptMsgQueue);

        if (!NT_SUCCESS(status)) {
            print_kd("[DualSense] WdfRequestForwardToIoQueue failed with status: 0x%x\n", status);

            WdfRequestComplete(Request, status);
        }

        return;

    case IOCTL_HID_WRITE_REPORT:            // METHOD_NEITHER TODO:
        //
        // Transmits a class driver-supplied report to the device.
        //
        //status = WriteReport(queueContext, Request);
        break;

    case IOCTL_HID_GET_FEATURE:             // METHOD_OUT_DIRECT TODO:

        //status = GetFeature(queueContext, Request);
        break;

    case IOCTL_HID_SET_FEATURE:             // METHOD_IN_DIRECT TODO:

        //status = SetFeature(queueContext, Request);
        break;

    case IOCTL_HID_GET_INPUT_REPORT:        // METHOD_OUT_DIRECT TODO:

        //status = GetInputReport(queueContext, Request);
        break;
        
    case IOCTL_HID_SET_OUTPUT_REPORT:       // METHOD_IN_DIRECT TODO:

        //status = SetOutputReport(queueContext, Request);
        break;

    case IOCTL_HID_GET_STRING:                      // METHOD_NEITHER TODO:

        //status = GetString(Request);
        break;

    case IOCTL_HID_GET_INDEXED_STRING:              // METHOD_OUT_DIRECT TODO:

        //status = GetIndexedString(Request);
        break;

    case IOCTL_HID_SEND_IDLE_NOTIFICATION_REQUEST:  // METHOD_NEITHER TODO:
        //
        // This has the USBSS Idle notification callback. If the lower driver
        // can handle it (e.g. USB stack can handle it) then pass it down
        // otherwise complete it here as not inplemented. For a virtual
        // device, idling is not needed.
        //
        // Not implemented. fall through...
        //
    case IOCTL_HID_ACTIVATE_DEVICE:                 // METHOD_NEITHER
    case IOCTL_HID_DEACTIVATE_DEVICE:               // METHOD_NEITHER
    case IOCTL_GET_PHYSICAL_DESCRIPTOR:             // METHOD_OUT_DIRECT
        //
        // We don't do anything for these IOCTLs but some minidrivers might.
        //
        // Not implemented. fall through...
        //
    default:
        status = STATUS_NOT_IMPLEMENTED;
        break;

    }

    WdfRequestComplete(Request, status);

}

NTSTATUS
RequestCopyFromBuffer(
    _In_  WDFREQUEST        Request,
    _In_  PVOID             SourceBuffer,
    _When_(NumBytesToCopyFrom == 0, __drv_reportError(NumBytesToCopyFrom cannot be zero))
    _In_  size_t            NumBytesToCopyFrom
)
/*++

Routine Description:

    A helper function to copy specified bytes to the request's output memory

Arguments:

    Request - A handle to a framework request object.

    SourceBuffer - The buffer to copy data from.

    NumBytesToCopyFrom - The length, in bytes, of data to be copied.

Return Value:

    NTSTATUS

--*/
{
    NTSTATUS                status;
    WDFMEMORY               memory;
    size_t                  outputBufferLength;

    status = WdfRequestRetrieveOutputMemory(Request, &memory);
    if (!NT_SUCCESS(status)) {
        print_kd("[DualSense] WdfRequestRetrieveOutputMemory failed 0x%x\n", status);
        return status;
    }

    WdfMemoryGetBuffer(memory, &outputBufferLength);
    if (outputBufferLength < NumBytesToCopyFrom) {
        status = STATUS_INVALID_BUFFER_SIZE;
        print_kd("[DualSense] RequestCopyFromBuffer: buffer too small. Size %d, expect %d\n",
            (int)outputBufferLength, (int)NumBytesToCopyFrom);
        return status;
    }

    status = WdfMemoryCopyFromBuffer(memory,
        0,
        SourceBuffer,
        NumBytesToCopyFrom);

    if (!NT_SUCCESS(status)) {
        print_kd("[DualSense] WdfMemoryCopyFromBuffer failed 0x%x\n", status);
        return status;
    }

    WdfRequestSetInformation(Request, NumBytesToCopyFrom);
    return status;
}