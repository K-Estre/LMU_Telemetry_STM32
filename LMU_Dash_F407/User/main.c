

#include "./BSP/LCD/lcd.h"
#include "./BSP/LED/led.h"
#include "./SYSTEM/delay/delay.h"
#include "./SYSTEM/sys/sys.h"
#include "./SYSTEM/usart/usart.h"

int main(void) {
  uart_telemetry_t telemetry = {0, 0, 0};
  dashboard_view_state_t view_state = {
      -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};

  HAL_Init();                         
  sys_stm32_clock_init(336, 8, 2, 7); 
  delay_init(168);                    
  usart_init(115200);                 
  led_init();                         
  lcd_init();                         
  dashboard_init_screen();

  while (1) {
    uart_telemetry_fetch(&telemetry);
    dashboard_update(&telemetry, &view_state);

    delay_ms(5);
  }
}


