

#include "./BSP/LCD/lcd.h"
#include "./BSP/LED/led.h"
#include "./SYSTEM/delay/delay.h"
#include "./SYSTEM/sys/sys.h"
#include "./SYSTEM/usart/usart.h"

int main(void) {
  uart_telemetry_t telemetry = {0, 0, 0};
  int last_gear = -1;
  int last_rpm = -1;
  int last_speed = -1;
  int last_tire_temp_fl = -1;
  int last_tire_temp_fr = -1;
  int last_tire_temp_rl = -1;
  int last_tire_temp_rr = -1;
  int last_brake_temp_fl = -1;
  int last_brake_temp_fr = -1;
  int last_brake_temp_rl = -1;
  int last_brake_temp_rr = -1;
  int last_water_temp = -1;
  int last_oil_temp = -1;
  long last_best_lap_ms = -1;
  long last_current_lap_ms = -1;
  int last_fuel_liters = -1;
  int last_fuel_pct = -1;
  int last_rpm_pct_x10 = -1;
  int last_throttle_pct = -1;
  int last_brake_pct = -1;

  HAL_Init();                         
  sys_stm32_clock_init(336, 8, 2, 7); 
  delay_init(168);                    
  usart_init(115200);                 
  led_init();                         
  lcd_init();                         
  g_point_color = BLUE;
  lcd_draw_lmh_demo();
  lcd_draw_lmh_shift_lights(0);
  lcd_draw_lmh_gear(0);
  lcd_draw_lmh_rpm(0);
  lcd_draw_lmh_speed(0);
  lcd_draw_lmh_brake_temps(0, 0, 0, 0);
  lcd_draw_lmh_tire_temps(0, 0, 0, 0);
  lcd_draw_lmh_engine_temps(0, 0);
  lcd_draw_lmh_lap_times(0, 0);
  lcd_draw_lmh_fuel_status(0, 0);
  lcd_draw_lmh_pedals(0, 0);

  while (1) {
    if (g_uart_frame_ready != 0) {
      __disable_irq();
      telemetry.gear = g_uart_telemetry.gear;
      telemetry.speed = g_uart_telemetry.speed;
      telemetry.rpm = g_uart_telemetry.rpm;
      telemetry.tire_temp_fl = g_uart_telemetry.tire_temp_fl;
      telemetry.tire_temp_fr = g_uart_telemetry.tire_temp_fr;
      telemetry.tire_temp_rl = g_uart_telemetry.tire_temp_rl;
      telemetry.tire_temp_rr = g_uart_telemetry.tire_temp_rr;
      telemetry.brake_temp_fl = g_uart_telemetry.brake_temp_fl;
      telemetry.brake_temp_fr = g_uart_telemetry.brake_temp_fr;
      telemetry.brake_temp_rl = g_uart_telemetry.brake_temp_rl;
      telemetry.brake_temp_rr = g_uart_telemetry.brake_temp_rr;
      telemetry.water_temp = g_uart_telemetry.water_temp;
      telemetry.oil_temp = g_uart_telemetry.oil_temp;
      telemetry.best_lap_ms = g_uart_telemetry.best_lap_ms;
      telemetry.current_lap_ms = g_uart_telemetry.current_lap_ms;
      telemetry.rpm_pct_x10 = g_uart_telemetry.rpm_pct_x10;
      telemetry.fuel_liters = g_uart_telemetry.fuel_liters;
      telemetry.fuel_pct = g_uart_telemetry.fuel_pct;
      telemetry.throttle_pct = g_uart_telemetry.throttle_pct;
      telemetry.brake_pct = g_uart_telemetry.brake_pct;
      g_uart_frame_ready = 0;
      __enable_irq();
    }

    if ((int)telemetry.gear != last_gear) {
      lcd_draw_lmh_gear(telemetry.gear);
      last_gear = telemetry.gear;
    }

    if ((int)telemetry.rpm_pct_x10 != last_rpm_pct_x10) {
      lcd_draw_lmh_shift_lights(telemetry.rpm_pct_x10);
      last_rpm_pct_x10 = telemetry.rpm_pct_x10;
    }

    if ((int)telemetry.rpm != last_rpm) {
      lcd_draw_lmh_rpm(telemetry.rpm);
      last_rpm = telemetry.rpm;
    }

    if ((int)telemetry.speed != last_speed) {
      lcd_draw_lmh_speed(telemetry.speed);
      last_speed = telemetry.speed;
    }

    if (((int)telemetry.brake_temp_fl != last_brake_temp_fl) ||
        ((int)telemetry.brake_temp_fr != last_brake_temp_fr) ||
        ((int)telemetry.brake_temp_rl != last_brake_temp_rl) ||
        ((int)telemetry.brake_temp_rr != last_brake_temp_rr)) {
      lcd_draw_lmh_brake_temps(
          telemetry.brake_temp_fl, telemetry.brake_temp_fr,
          telemetry.brake_temp_rl, telemetry.brake_temp_rr);
      last_brake_temp_fl = telemetry.brake_temp_fl;
      last_brake_temp_fr = telemetry.brake_temp_fr;
      last_brake_temp_rl = telemetry.brake_temp_rl;
      last_brake_temp_rr = telemetry.brake_temp_rr;
    }

    if (((int)telemetry.tire_temp_fl != last_tire_temp_fl) ||
        ((int)telemetry.tire_temp_fr != last_tire_temp_fr) ||
        ((int)telemetry.tire_temp_rl != last_tire_temp_rl) ||
        ((int)telemetry.tire_temp_rr != last_tire_temp_rr)) {
      lcd_draw_lmh_tire_temps(telemetry.tire_temp_fl, telemetry.tire_temp_fr,
                              telemetry.tire_temp_rl, telemetry.tire_temp_rr);
      last_tire_temp_fl = telemetry.tire_temp_fl;
      last_tire_temp_fr = telemetry.tire_temp_fr;
      last_tire_temp_rl = telemetry.tire_temp_rl;
      last_tire_temp_rr = telemetry.tire_temp_rr;
    }

    if (((int)telemetry.water_temp != last_water_temp) ||
        ((int)telemetry.oil_temp != last_oil_temp)) {
      lcd_draw_lmh_engine_temps(telemetry.water_temp, telemetry.oil_temp);
      last_water_temp = telemetry.water_temp;
      last_oil_temp = telemetry.oil_temp;
    }

    if (((long)telemetry.best_lap_ms != last_best_lap_ms) ||
        ((long)telemetry.current_lap_ms != last_current_lap_ms)) {
      lcd_draw_lmh_lap_times(telemetry.current_lap_ms, telemetry.best_lap_ms);
      last_best_lap_ms = telemetry.best_lap_ms;
      last_current_lap_ms = telemetry.current_lap_ms;
    }

    if (((int)telemetry.fuel_liters != last_fuel_liters) ||
        ((int)telemetry.fuel_pct != last_fuel_pct)) {
      lcd_draw_lmh_fuel_status(telemetry.fuel_liters, telemetry.fuel_pct);
      last_fuel_liters = telemetry.fuel_liters;
      last_fuel_pct = telemetry.fuel_pct;
    }

    if (((int)telemetry.throttle_pct != last_throttle_pct) ||
        ((int)telemetry.brake_pct != last_brake_pct)) {
      lcd_draw_lmh_pedals(telemetry.throttle_pct, telemetry.brake_pct);
      last_throttle_pct = telemetry.throttle_pct;
      last_brake_pct = telemetry.brake_pct;
    }

    delay_ms(5);
  }
}


