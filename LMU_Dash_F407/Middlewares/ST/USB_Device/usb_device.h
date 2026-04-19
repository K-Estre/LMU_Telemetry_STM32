#ifndef __USB_DEVICE_H
#define __USB_DEVICE_H

#include "stm32f4xx_hal.h"
#include "usbd_def.h"

extern USBD_HandleTypeDef USBD_Device;

void usb_hid_init(void);

#endif
