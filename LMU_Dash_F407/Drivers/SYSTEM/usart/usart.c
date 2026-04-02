/**
 ****************************************************************************************************
 * @file        usart.c
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.1
 * @date        2023-06-05
 * @brief       ���ڳ�ʼ������(һ���Ǵ���1)��֧��printf
 * @license     Copyright (c) 2020-2032, �������������ӿƼ����޹�˾
 ****************************************************************************************************
 * @attention
 *
 * ʵ��ƽ̨:����ԭ�� ̽���� F407������
 * ������Ƶ:www.yuanzige.com
 * ������̳:www.openedv.com
 * ��˾��ַ:www.alientek.com
 * �����ַ:openedv.taobao.com
 *
 * �޸�˵��
 * V1.0 20211014
 * ��һ�η���
 * V1.1 20230605
 * ɾ��USART_UX_IRQHandler()�����ĳ�ʱ�������޸�HAL_UART_RxCpltCallback()
 ****************************************************************************************************
 */

#include "./SYSTEM/sys/sys.h"
#include "./SYSTEM/usart/usart.h"
#include <string.h>


/* ���ʹ��os,����������ͷ�ļ����� */
#if SYS_SUPPORT_OS
#include "os.h"                               /* os ʹ�� */
#endif

/******************************************************************************************/
/* �������´���, ֧��printf����, ������Ҫѡ��use MicroLIB */

#if 1
#if (__ARMCC_VERSION >= 6010050)                    /* ʹ��AC6������ʱ */
__asm(".global __use_no_semihosting\n\t");          /* ������ʹ�ð�����ģʽ */
__asm(".global __ARM_use_no_argv \n\t");            /* AC6����Ҫ����main����Ϊ�޲�����ʽ�����򲿷����̿��ܳ��ְ�����ģʽ */

#else
/* ʹ��AC5������ʱ, Ҫ�����ﶨ��__FILE �� ��ʹ�ð�����ģʽ */
#pragma import(__use_no_semihosting)

struct __FILE
{
    int handle;
    /* Whatever you require here. If the only file you are using is */
    /* standard output using printf() for debugging, no file handling */
    /* is required. */
};

#endif

/* ��ʹ�ð�����ģʽ��������Ҫ�ض���_ttywrch\_sys_exit\_sys_command_string����,��ͬʱ����AC6��AC5ģʽ */
int _ttywrch(int ch)
{
    ch = ch;
    return ch;
}

/* ����_sys_exit()�Ա���ʹ�ð�����ģʽ */
void _sys_exit(int x)
{
    x = x;
}

char *_sys_command_string(char *cmd, int len)
{
    return NULL;
}

/* FILE �� stdio.h���涨��. */
FILE __stdout;

/* �ض���fputc����, printf�������ջ�ͨ������fputc����ַ��������� */
int fputc(int ch, FILE *f)
{
    while ((USART1->SR & 0X40) == 0);               /* �ȴ���һ���ַ�������� */

    USART1->DR = (uint8_t)ch;                       /* ��Ҫ���͵��ַ� ch д�뵽DR�Ĵ��� */
    return ch;
}
#endif
/***********************************************END*******************************************/
    
#if USART_EN_RX                                     /* ���ʹ���˽��� */

/* ���ջ���, ���USART_REC_LEN���ֽ�. */
uint8_t g_usart_rx_buf[USART_REC_LEN];

/*  ����״̬
 *  bit15��      ������ɱ�־
 *  bit14��      ���յ�0x0d
 *  bit13~0��    ���յ�����Ч�ֽ���Ŀ
*/
uint16_t g_usart_rx_sta = 0;

uint8_t g_rx_buffer[RXBUFFERSIZE];                  /* HAL��ʹ�õĴ��ڽ��ջ��� */

UART_HandleTypeDef g_uart1_handle;                  /* UART��� */
volatile uint8_t g_uart_frame_ready = 0;
volatile uart_telemetry_t g_uart_telemetry = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

static uint8_t g_uart_frame_buf[UART_FRAME_SIZE];
static uint8_t g_uart_frame_index = 0;

static void uart_reset_parser(void)
{
    g_uart_frame_index = 0;
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
            break;
    }
}


/**
 * @brief       ����X��ʼ������
 * @param       baudrate: ������, �����Լ���Ҫ���ò�����ֵ
 * @note        ע��: ����������ȷ��ʱ��Դ, ���򴮿ڲ����ʾͻ������쳣.
 *              �����USART��ʱ��Դ��sys_stm32_clock_init()�������Ѿ����ù���.
 * @retval      ��
 */
void usart_init(uint32_t baudrate)
{
    g_uart1_handle.Instance = USART_UX;                         /* USART1 */
    g_uart1_handle.Init.BaudRate = baudrate;                    /* ������ */
    g_uart1_handle.Init.WordLength = UART_WORDLENGTH_8B;        /* �ֳ�Ϊ8λ���ݸ�ʽ */
    g_uart1_handle.Init.StopBits = UART_STOPBITS_1;             /* һ��ֹͣλ */
    g_uart1_handle.Init.Parity = UART_PARITY_NONE;              /* ����żУ��λ */
    g_uart1_handle.Init.HwFlowCtl = UART_HWCONTROL_NONE;        /* ��Ӳ������ */
    g_uart1_handle.Init.Mode = UART_MODE_TX_RX;                 /* �շ�ģʽ */
    HAL_UART_Init(&g_uart1_handle);                             /* HAL_UART_Init()��ʹ��UART1 */
    g_usart_rx_sta = 0;
    g_uart_frame_ready = 0;
    g_uart_telemetry.gear = 0;
    g_uart_telemetry.speed = 0;
    g_uart_telemetry.rpm = 0;
    g_uart_telemetry.tire_temp_fl = 0;
    g_uart_telemetry.tire_temp_fr = 0;
    g_uart_telemetry.tire_temp_rl = 0;
    g_uart_telemetry.tire_temp_rr = 0;
    g_uart_telemetry.brake_temp_fl = 0;
    g_uart_telemetry.brake_temp_fr = 0;
    g_uart_telemetry.brake_temp_rl = 0;
    g_uart_telemetry.brake_temp_rr = 0;
    g_uart_telemetry.water_temp = 0;
    g_uart_telemetry.oil_temp = 0;
    g_uart_telemetry.best_lap_ms = 0;
    g_uart_telemetry.current_lap_ms = 0;
    g_uart_telemetry.rpm_pct_x10 = 0;
    g_uart_telemetry.fuel_liters = 0;
    g_uart_telemetry.fuel_pct = 0;
    g_uart_telemetry.throttle_pct = 0;
    g_uart_telemetry.brake_pct = 0;
    uart_reset_parser();
    
    /* �ú����Ὺ�������жϣ���־λUART_IT_RXNE���������ý��ջ����Լ����ջ��������������� */
    HAL_UART_Receive_IT(&g_uart1_handle, (uint8_t *)g_rx_buffer, RXBUFFERSIZE);
}

uint8_t uart_telemetry_fetch(uart_telemetry_t *out)
{
    if ((out == NULL) || (g_uart_frame_ready == 0))
    {
        return 0;
    }

    __disable_irq();
    memcpy(out, (const void *)&g_uart_telemetry, sizeof(uart_telemetry_t));
    g_uart_frame_ready = 0;
    __enable_irq();

    return 1;
}

/**
 * @brief       UART�ײ��ʼ������
 * @param       huart: UART�������ָ��
 * @note        �˺����ᱻHAL_UART_Init()����
 *              ���ʱ��ʹ�ܣ��������ã��ж�����
 * @retval      ��
 */
void HAL_UART_MspInit(UART_HandleTypeDef *huart)
{
    GPIO_InitTypeDef gpio_init_struct;
    if(huart->Instance == USART_UX)                             /* ����Ǵ���1�����д���1 MSP��ʼ�� */
    {
        USART_UX_CLK_ENABLE();                                  /* USART1 ʱ��ʹ�� */
        USART_TX_GPIO_CLK_ENABLE();                             /* ��������ʱ��ʹ�� */
        USART_RX_GPIO_CLK_ENABLE();                             /* ��������ʱ��ʹ�� */

        gpio_init_struct.Pin = USART_TX_GPIO_PIN;               /* TX���� */
        gpio_init_struct.Mode = GPIO_MODE_AF_PP;                /* ����������� */
        gpio_init_struct.Pull = GPIO_PULLUP;                    /* ���� */
        gpio_init_struct.Speed = GPIO_SPEED_FREQ_HIGH;          /* ���� */
        gpio_init_struct.Alternate = USART_TX_GPIO_AF;          /* ����ΪUSART1 */
        HAL_GPIO_Init(USART_TX_GPIO_PORT, &gpio_init_struct);   /* ��ʼ���������� */

        gpio_init_struct.Pin = USART_RX_GPIO_PIN;               /* RX���� */
        gpio_init_struct.Alternate = USART_RX_GPIO_AF;          /* ����ΪUSART1 */
        HAL_GPIO_Init(USART_RX_GPIO_PORT, &gpio_init_struct);   /* ��ʼ���������� */

#if USART_EN_RX
        HAL_NVIC_EnableIRQ(USART_UX_IRQn);                      /* ʹ��USART1�ж�ͨ�� */
        HAL_NVIC_SetPriority(USART_UX_IRQn, 3, 3);              /* ��ռ���ȼ�3�������ȼ�3 */
#endif
    }
}

/**
 * @brief       Rx����ص�����
 * @param       huart: UART�������ָ��
 * @retval      ��
 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if(huart->Instance == USART_UX)             /* ����Ǵ���1 */
    {
        uart_handle_rx_byte(g_rx_buffer[0]);
        HAL_UART_Receive_IT(&g_uart1_handle, (uint8_t *)g_rx_buffer, RXBUFFERSIZE);
    }
}

/**
 * @brief       ����1�жϷ�����
 * @param       ��
 * @retval      ��
 */
void USART_UX_IRQHandler(void)
{ 
#if SYS_SUPPORT_OS                              /* ʹ��OS */
    OSIntEnter();    
#endif

    HAL_UART_IRQHandler(&g_uart1_handle);       /* ����HAL���жϴ������ú��� */

#if SYS_SUPPORT_OS                              /* ʹ��OS */
    OSIntExit();
#endif
}

#endif


 

 





