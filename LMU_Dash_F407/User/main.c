#include "./BSP/LCD/lcd.h"
#include "./BSP/LED/led.h"
#include "./SYSTEM/delay/delay.h"
#include "./SYSTEM/sys/sys.h"
#include "./SYSTEM/usart/usart.h"

#define UART_POLL_INTERVAL_MS 8U

int main(void) {
  uart_telemetry_t telemetry = {0};
  dashboard_view_state_t view_state = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                                       -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};
  uint32_t last_uart_poll_ms = 0;

  HAL_Init();
  sys_stm32_clock_init(336, 8, 2, 7);
  delay_init(168);
  usart_init(115200);
  led_init();
  lcd_init();
  dashboard_init_screen();

  while (1) {
    uint32_t now = HAL_GetTick();

    if ((now - last_uart_poll_ms) >= UART_POLL_INTERVAL_MS) {
      last_uart_poll_ms = now;
      uart_telemetry_poll();

      if (uart_telemetry_fetch(&telemetry) != 0U) {
        dashboard_update(&telemetry, &view_state);
      }
    }
  }
}
