#include "./SYSTEM/telemetry/telemetry_hid.h"

#include "./SYSTEM/usart/usart.h"
#include "string.h"

void telemetry_hid_apply_report(const uint8_t *report, uint16_t report_len)
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

    if ((report == 0) || (report_len < UART_FRAME_SIZE))
    {
        return;
    }

    if ((report[0] != UART_FRAME_HEADER1) || (report[1] != UART_FRAME_HEADER2))
    {
        return;
    }

    speed = (uint16_t)report[3] | ((uint16_t)report[4] << 8);
    rpm = (uint16_t)report[5] | ((uint16_t)report[6] << 8);
    brake_temp_fl = (uint16_t)report[11] | ((uint16_t)report[12] << 8);
    brake_temp_fr = (uint16_t)report[13] | ((uint16_t)report[14] << 8);
    brake_temp_rl = (uint16_t)report[15] | ((uint16_t)report[16] << 8);
    brake_temp_rr = (uint16_t)report[17] | ((uint16_t)report[18] << 8);
    best_lap_ms = (uint32_t)report[21] |
                  ((uint32_t)report[22] << 8) |
                  ((uint32_t)report[23] << 16) |
                  ((uint32_t)report[24] << 24);
    current_lap_ms = (uint32_t)report[25] |
                     ((uint32_t)report[26] << 8) |
                     ((uint32_t)report[27] << 16) |
                     ((uint32_t)report[28] << 24);
    rpm_pct_x10 = (uint16_t)report[29] | ((uint16_t)report[30] << 8);

    g_uart_telemetry.gear = report[2];
    g_uart_telemetry.speed = speed;
    g_uart_telemetry.rpm = rpm;
    g_uart_telemetry.tire_temp_fl = report[7];
    g_uart_telemetry.tire_temp_fr = report[8];
    g_uart_telemetry.tire_temp_rl = report[9];
    g_uart_telemetry.tire_temp_rr = report[10];
    g_uart_telemetry.brake_temp_fl = brake_temp_fl;
    g_uart_telemetry.brake_temp_fr = brake_temp_fr;
    g_uart_telemetry.brake_temp_rl = brake_temp_rl;
    g_uart_telemetry.brake_temp_rr = brake_temp_rr;
    g_uart_telemetry.water_temp = report[19];
    g_uart_telemetry.oil_temp = report[20];
    g_uart_telemetry.best_lap_ms = best_lap_ms;
    g_uart_telemetry.current_lap_ms = current_lap_ms;
    g_uart_telemetry.rpm_pct_x10 = rpm_pct_x10;
    g_uart_telemetry.fuel_liters = report[31];
    g_uart_telemetry.fuel_pct = report[32];
    g_uart_telemetry.throttle_pct = report[33];
    g_uart_telemetry.brake_pct = report[34];

    memcpy(g_usart_rx_buf, report, UART_FRAME_SIZE);
    g_usart_rx_sta = UART_FRAME_SIZE;
    g_uart_frame_ready = 1;
}
