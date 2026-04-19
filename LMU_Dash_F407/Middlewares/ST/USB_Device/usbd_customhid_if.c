/**
  ******************************************************************************
  * @file    USB_Device/CustomHID_Standalone/Src/usbd_customhid_if.c
  * @author  MCD Application Team
  * @brief   USB Device Custom HID interface file.
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
/* Includes ------------------------------------------------------------------*/
#include "usbd_customhid_if.h"
#include "./SYSTEM/telemetry/telemetry_hid.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
static int8_t CustomHID_Init(void);
static int8_t CustomHID_DeInit(void);
static int8_t CustomHID_OutEvent(uint8_t event_idx, uint8_t state);

/* Private variables ---------------------------------------------------------*/
volatile uint8_t CustomHID_LastReportId = 0;
volatile uint8_t CustomHID_LastReportValue = 0;
extern USBD_HandleTypeDef USBD_Device;

__ALIGN_BEGIN static uint8_t CustomHID_ReportDesc[USBD_CUSTOM_HID_REPORT_DESC_SIZE] __ALIGN_END =
{
  0x06, 0x00, 0xFF,      /* USAGE_PAGE (Vendor Defined) */
  0x09, 0x01,            /* USAGE (LMU Telemetry Dash) */
  0xA1, 0x01,            /* COLLECTION (Application) */

  0x85, HID_TELEMETRY_OUT_REPORT_ID, /* REPORT_ID (telemetry OUT) */
  0x09, 0x01,            /* USAGE (Telemetry OUT) */
  0x15, 0x00,            /* LOGICAL_MINIMUM (0) */
  0x26, 0xFF, 0x00,      /* LOGICAL_MAXIMUM (255) */
  0x75, 0x08,            /* REPORT_SIZE (8 bits) */
  0x95, HID_TELEMETRY_REPORT_PAYLOAD_SIZE, /* REPORT_COUNT (63 bytes) */
  0x91, 0x02,            /* OUTPUT (Data,Var,Abs) */

  0x85, HID_TELEMETRY_IN_REPORT_ID,  /* REPORT_ID (optional debug IN) */
  0x09, 0x02,            /* USAGE (Telemetry IN) */
  0x95, HID_TELEMETRY_REPORT_PAYLOAD_SIZE, /* REPORT_COUNT (63 bytes) */
  0x81, 0x02,            /* INPUT (Data,Var,Abs) */

  0xC0                   /* END_COLLECTION */
};

USBD_CUSTOM_HID_ItfTypeDef USBD_CustomHID_fops = 
{
  CustomHID_ReportDesc,
  CustomHID_Init,
  CustomHID_DeInit,
  CustomHID_OutEvent,
};

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  CustomHID_Init
  *         Initializes the CUSTOM HID media low layer
  * @param  None
  * @retval Result of the opeartion: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CustomHID_Init(void)
{
  return (0);
}

/**
  * @brief  CustomHID_DeInit
  *         DeInitializes the CUSTOM HID media low layer
  * @param  None
  * @retval Result of the opeartion: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CustomHID_DeInit(void)
{
  /*
  Add your deinitialization code here 
  */  
  return (0);
}


/**
  * @brief  CustomHID_OutEvent
  *         Manage the CUSTOM HID class Out Event    
  * @param  event_idx: LED Report Number
  * @param  state: LED states (ON/OFF)
  */
static int8_t CustomHID_OutEvent  (uint8_t event_idx, uint8_t state)
{ 
  USBD_CUSTOM_HID_HandleTypeDef *hhid;

  CustomHID_LastReportId = event_idx;
  CustomHID_LastReportValue = state;

  hhid = (USBD_CUSTOM_HID_HandleTypeDef *)USBD_Device.pClassData;
  if ((hhid != 0) && (hhid->Report_buf[0] == HID_TELEMETRY_OUT_REPORT_ID))
  {
    telemetry_hid_apply_report(&hhid->Report_buf[1],
                               HID_TELEMETRY_REPORT_PAYLOAD_SIZE);
  }

  /* Start next USB packet transfer once data processing is completed */
  USBD_CUSTOM_HID_ReceivePacket(&USBD_Device);

  return (0);
}

/**
  * @brief EXTI line detection callbacks
  * @param GPIO_Pin: Specifies the pins connected EXTI line
  * @retval None
  */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  UNUSED(GPIO_Pin);
}
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
