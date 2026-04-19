#include "usb_device.h"

#include "usbd_core.h"
#include "usbd_customhid.h"
#include "usbd_customhid_if.h"
#include "usbd_desc.h"

USBD_HandleTypeDef USBD_Device;

void usb_hid_init(void)
{
    USBD_Init(&USBD_Device, &HID_Desc, 0);
    USBD_RegisterClass(&USBD_Device, &USBD_CUSTOM_HID);
    USBD_CUSTOM_HID_RegisterInterface(&USBD_Device, &USBD_CustomHID_fops);
    USBD_Start(&USBD_Device);
}
