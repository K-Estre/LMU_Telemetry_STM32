#include "./SYSTEM/sys/sys.h"
#include "./SYSTEM/usart/usart.h"
#include <string.h>

#if SYS_SUPPORT_OS
#include "os.h"
#endif

#if 1
#if (__ARMCC_VERSION >= 6010050)
__asm(".global __use_no_semihosting\n\t");
__asm(".global __ARM_use_no_argv \n\t");
#else
#pragma import(__use_no_semihosting)

struct __FILE
{
    int handle;
};
#endif

int _ttywrch(int ch)
{
    (void)ch;
    return ch;
}

void _sys_exit(int x)
{
    (void)x;
}

char *_sys_command_string(char *cmd, int len)
{
    (void)cmd;
    (void)len;
    return NULL;
}

FILE __stdout;

int fputc(int ch, FILE *f)
{
    (void)f;
    while ((USART1->SR & 0X40) == 0)
    {
    }

    USART1->DR = (uint8_t)ch;
    return ch;
}
#endif

#if USART_EN_RX

uint8_t g_usart_rx_buf[USART_REC_LEN];
uint16_t g_usart_rx_sta = 0;

UART_HandleTypeDef g_uart1_handle;
DMA_HandleTypeDef g_uart1_rx_dma_handle;
volatile uint8_t g_uart_frame_ready = 0;
volatile uart_telemetry_t g_uart_telemetry = {0};

static uint8_t g_uart_frame_buf[UART_FRAME_SIZE];
static uint8_t g_uart_frame_index = 0;
static uint8_t g_uart_dma_rx_buf[UART_DMA_RX_BUFFER_SIZE];
static uint16_t g_uart_dma_last_pos = 0;

static void uart_reset_parser(void)
{
    g_uart_frame_index = 0;
}

static void uart_apply_frame(void)
{
    uint16_t speed;
    uint16_t rpm;
    uint16_t brake_temp_fl;
    uint16_t brake_temp_fr;
    uint16_t brake_temp_rl;
    uint16_t brake_temp_rr;
    uint16_t rpm_pct_x10;
    uint32_t best_lap_ms;
    uint32_t current_lap_ms;

    speed = (uint16_t)g_uart_frame_buf[3] | ((uint16_t)g_uart_frame_buf[4] << 8);
    rpm = (uint16_t)g_uart_frame_buf[5] | ((uint16_t)g_uart_frame_buf[6] << 8);
    brake_temp_fl = (uint16_t)g_uart_frame_buf[11] | ((uint16_t)g_uart_frame_buf[12] << 8);
    brake_temp_fr = (uint16_t)g_uart_frame_buf[13] | ((uint16_t)g_uart_frame_buf[14] << 8);
    brake_temp_rl = (uint16_t)g_uart_frame_buf[15] | ((uint16_t)g_uart_frame_buf[16] << 8);
    brake_temp_rr = (uint16_t)g_uart_frame_buf[17] | ((uint16_t)g_uart_frame_buf[18] << 8);
    best_lap_ms = (uint32_t)g_uart_frame_buf[21] |
                  ((uint32_t)g_uart_frame_buf[22] << 8) |
                  ((uint32_t)g_uart_frame_buf[23] << 16) |
                  ((uint32_t)g_uart_frame_buf[24] << 24);
    current_lap_ms = (uint32_t)g_uart_frame_buf[25] |
                     ((uint32_t)g_uart_frame_buf[26] << 8) |
                     ((uint32_t)g_uart_frame_buf[27] << 16) |
                     ((uint32_t)g_uart_frame_buf[28] << 24);
    rpm_pct_x10 = (uint16_t)g_uart_frame_buf[29] |
                  ((uint16_t)g_uart_frame_buf[30] << 8);

    g_uart_telemetry.gear = g_uart_frame_buf[2];
    g_uart_telemetry.speed = speed;
    g_uart_telemetry.rpm = rpm;
    g_uart_telemetry.tire_temp_fl = g_uart_frame_buf[7];
    g_uart_telemetry.tire_temp_fr = g_uart_frame_buf[8];
    g_uart_telemetry.tire_temp_rl = g_uart_frame_buf[9];
    g_uart_telemetry.tire_temp_rr = g_uart_frame_buf[10];
    g_uart_telemetry.brake_temp_fl = brake_temp_fl;
    g_uart_telemetry.brake_temp_fr = brake_temp_fr;
    g_uart_telemetry.brake_temp_rl = brake_temp_rl;
    g_uart_telemetry.brake_temp_rr = brake_temp_rr;
    g_uart_telemetry.water_temp = g_uart_frame_buf[19];
    g_uart_telemetry.oil_temp = g_uart_frame_buf[20];
    g_uart_telemetry.best_lap_ms = best_lap_ms;
    g_uart_telemetry.current_lap_ms = current_lap_ms;
    g_uart_telemetry.rpm_pct_x10 = rpm_pct_x10;
    g_uart_telemetry.fuel_liters = g_uart_frame_buf[31];
    g_uart_telemetry.fuel_pct = g_uart_frame_buf[32];
    g_uart_telemetry.throttle_pct = g_uart_frame_buf[33];
    g_uart_telemetry.brake_pct = g_uart_frame_buf[34];

    memcpy(g_usart_rx_buf, g_uart_frame_buf, UART_FRAME_SIZE);
    g_usart_rx_sta = UART_FRAME_SIZE;
    g_uart_frame_ready = 1;
    uart_reset_parser();
}

static void uart_handle_rx_byte(uint8_t byte)
{
    switch (g_uart_frame_index)
    {
        case 0:
            if (byte == UART_FRAME_HEADER1)
            {
                g_uart_frame_buf[g_uart_frame_index++] = byte;
            }
            break;

        case 1:
            if (byte == UART_FRAME_HEADER2)
            {
                g_uart_frame_buf[g_uart_frame_index++] = byte;
            }
            else if (byte == UART_FRAME_HEADER1)
            {
                g_uart_frame_buf[0] = byte;
                g_uart_frame_index = 1;
            }
            else
            {
                uart_reset_parser();
            }
            break;

        default:
            g_uart_frame_buf[g_uart_frame_index++] = byte;

            if (g_uart_frame_index >= UART_FRAME_SIZE)
            {
                uart_apply_frame();
            }
            break;
    }
}

static uint16_t uart_dma_get_write_pos(void)
{
    uint16_t remaining = (uint16_t)__HAL_DMA_GET_COUNTER(&g_uart1_rx_dma_handle);
    uint16_t current = (uint16_t)(UART_DMA_RX_BUFFER_SIZE - remaining);

    if (current >= UART_DMA_RX_BUFFER_SIZE)
    {
        current = 0;
    }

    return current;
}

void usart_init(uint32_t baudrate)
{
    g_uart1_handle.Instance = USART_UX;
    g_uart1_handle.Init.BaudRate = baudrate;
    g_uart1_handle.Init.WordLength = UART_WORDLENGTH_8B;
    g_uart1_handle.Init.StopBits = UART_STOPBITS_1;
    g_uart1_handle.Init.Parity = UART_PARITY_NONE;
    g_uart1_handle.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    g_uart1_handle.Init.Mode = UART_MODE_TX_RX;
    HAL_UART_Init(&g_uart1_handle);

    memset((void *)g_usart_rx_buf, 0, sizeof(g_usart_rx_buf));
    memset((void *)&g_uart_telemetry, 0, sizeof(g_uart_telemetry));
    memset((void *)g_uart_frame_buf, 0, sizeof(g_uart_frame_buf));
    memset((void *)g_uart_dma_rx_buf, 0, sizeof(g_uart_dma_rx_buf));
    g_usart_rx_sta = 0;
    g_uart_frame_ready = 0;
    g_uart_dma_last_pos = 0;
    uart_reset_parser();

    HAL_UART_Receive_DMA(&g_uart1_handle, g_uart_dma_rx_buf, UART_DMA_RX_BUFFER_SIZE);
    __HAL_DMA_DISABLE_IT(&g_uart1_rx_dma_handle, DMA_IT_HT | DMA_IT_TC | DMA_IT_TE);
}

void uart_telemetry_poll(void)
{
    uint16_t current_pos = uart_dma_get_write_pos();

    while (g_uart_dma_last_pos != current_pos)
    {
        uart_handle_rx_byte(g_uart_dma_rx_buf[g_uart_dma_last_pos]);
        g_uart_dma_last_pos++;

        if (g_uart_dma_last_pos >= UART_DMA_RX_BUFFER_SIZE)
        {
            g_uart_dma_last_pos = 0;
        }
    }
}

uint8_t uart_telemetry_fetch(uart_telemetry_t *out)
{
    if ((out == NULL) || (g_uart_frame_ready == 0))
    {
        return 0;
    }

    memcpy(out, (const void *)&g_uart_telemetry, sizeof(uart_telemetry_t));
    g_uart_frame_ready = 0;
    return 1;
}

void HAL_UART_MspInit(UART_HandleTypeDef *huart)
{
    GPIO_InitTypeDef gpio_init_struct;

    if (huart->Instance == USART_UX)
    {
        USART_UX_CLK_ENABLE();
        USART_TX_GPIO_CLK_ENABLE();
        USART_RX_GPIO_CLK_ENABLE();
        USART_RX_DMA_CLK_ENABLE();

        gpio_init_struct.Pin = USART_TX_GPIO_PIN;
        gpio_init_struct.Mode = GPIO_MODE_AF_PP;
        gpio_init_struct.Pull = GPIO_PULLUP;
        gpio_init_struct.Speed = GPIO_SPEED_FREQ_HIGH;
        gpio_init_struct.Alternate = USART_TX_GPIO_AF;
        HAL_GPIO_Init(USART_TX_GPIO_PORT, &gpio_init_struct);

        gpio_init_struct.Pin = USART_RX_GPIO_PIN;
        gpio_init_struct.Alternate = USART_RX_GPIO_AF;
        HAL_GPIO_Init(USART_RX_GPIO_PORT, &gpio_init_struct);

        g_uart1_rx_dma_handle.Instance = USART_RX_DMA_STREAM;
        g_uart1_rx_dma_handle.Init.Channel = USART_RX_DMA_CHANNEL;
        g_uart1_rx_dma_handle.Init.Direction = DMA_PERIPH_TO_MEMORY;
        g_uart1_rx_dma_handle.Init.PeriphInc = DMA_PINC_DISABLE;
        g_uart1_rx_dma_handle.Init.MemInc = DMA_MINC_ENABLE;
        g_uart1_rx_dma_handle.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
        g_uart1_rx_dma_handle.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
        g_uart1_rx_dma_handle.Init.Mode = DMA_CIRCULAR;
        g_uart1_rx_dma_handle.Init.Priority = DMA_PRIORITY_HIGH;
        g_uart1_rx_dma_handle.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
        g_uart1_rx_dma_handle.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_FULL;
        g_uart1_rx_dma_handle.Init.MemBurst = DMA_MBURST_SINGLE;
        g_uart1_rx_dma_handle.Init.PeriphBurst = DMA_PBURST_SINGLE;
        HAL_DMA_Init(&g_uart1_rx_dma_handle);

        __HAL_LINKDMA(huart, hdmarx, g_uart1_rx_dma_handle);
    }
}

void USART_UX_IRQHandler(void)
{
#if SYS_SUPPORT_OS
    OSIntEnter();
#endif

    HAL_UART_IRQHandler(&g_uart1_handle);

#if SYS_SUPPORT_OS
    OSIntExit();
#endif
}

#endif
