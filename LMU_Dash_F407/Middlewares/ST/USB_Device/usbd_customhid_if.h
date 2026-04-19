/**
  ******************************************************************************
  * @file    USB_Device/CustomHID_Standalone/Inc/usbd_customhid_if.h
  * @author  MCD Application Team
  * @brief   Header for usbd_customhid_if.c file.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2017 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */ 

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USBD_CUSTOMHID_IF_H
#define __USBD_CUSTOMHID_IF_H

/* Includes ------------------------------------------------------------------*/
#include "usbd_customhid.h"
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
#define HID_TELEMETRY_OUT_REPORT_ID       0x01
#define HID_TELEMETRY_IN_REPORT_ID        0x02
#define HID_TELEMETRY_REPORT_SIZE         64U
#define HID_TELEMETRY_REPORT_PAYLOAD_SIZE 63U

/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
extern USBD_CUSTOM_HID_ItfTypeDef USBD_CustomHID_fops;
extern volatile uint8_t CustomHID_LastReportId;
extern volatile uint8_t CustomHID_LastReportValue;

#endif /* __USBD_CUSTOMHID_IF_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
