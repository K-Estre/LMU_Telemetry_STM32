#ifndef _USART_H
#define _USART_H

#include "stdio.h"
#include "./SYSTEM/sys/sys.h"

#define USART_TX_GPIO_PORT              GPIOA
#define USART_TX_GPIO_PIN               GPIO_PIN_9
#define USART_TX_GPIO_AF                GPIO_AF7_USART1
#define USART_TX_GPIO_CLK_ENABLE()      do{ __HAL_RCC_GPIOA_CLK_ENABLE(); }while(0)

#define USART_RX_GPIO_PORT              GPIOA
#define USART_RX_GPIO_PIN               GPIO_PIN_10
#define USART_RX_GPIO_AF                GPIO_AF7_USART1
#define USART_RX_GPIO_CLK_ENABLE()      do{ __HAL_RCC_GPIOA_CLK_ENABLE(); }while(0)

#define USART_UX                        USART1
#define USART_UX_IRQn                   USART1_IRQn
#define USART_UX_IRQHandler             USART1_IRQHandler
#define USART_UX_CLK_ENABLE()           do{ __HAL_RCC_USART1_CLK_ENABLE(); }while(0)

#define USART_RX_DMA_STREAM             DMA2_Stream2
#define USART_RX_DMA_CHANNEL            DMA_CHANNEL_4
#define USART_RX_DMA_CLK_ENABLE()       do{ __HAL_RCC_DMA2_CLK_ENABLE(); }while(0)
#define UART_DMA_RX_BUFFER_SIZE         256U

#define USART_REC_LEN                   200
#define USART_EN_RX                     1
#define UART_FRAME_HEADER1              0xAA
#define UART_FRAME_HEADER2              0x55
#define UART_FRAME_PAYLOAD_SIZE         33
#define UART_FRAME_SIZE                 (2 + UART_FRAME_PAYLOAD_SIZE)

extern UART_HandleTypeDef g_uart1_handle;
extern DMA_HandleTypeDef g_uart1_rx_dma_handle;

extern uint8_t  g_usart_rx_buf[USART_REC_LEN];
extern uint16_t g_usart_rx_sta;

typedef struct
{
    uint8_t gear;
    uint16_t speed;
    uint16_t rpm;
    uint8_t tire_temp_fl;
    uint8_t tire_temp_fr;
    uint8_t tire_temp_rl;
    uint8_t tire_temp_rr;
    uint16_t brake_temp_fl;
    uint16_t brake_temp_fr;
    uint16_t brake_temp_rl;
    uint16_t brake_temp_rr;
    uint8_t water_temp;
    uint8_t oil_temp;
    uint32_t best_lap_ms;
    uint32_t current_lap_ms;
    uint16_t rpm_pct_x10;
    uint8_t fuel_liters;
    uint8_t fuel_pct;
    uint8_t throttle_pct;
    uint8_t brake_pct;
} uart_telemetry_t;

extern volatile uint8_t g_uart_frame_ready;
extern volatile uart_telemetry_t g_uart_telemetry;

void usart_init(uint32_t baudrate);
void uart_telemetry_poll(void);
uint8_t uart_telemetry_fetch(uart_telemetry_t *out);

#endif
