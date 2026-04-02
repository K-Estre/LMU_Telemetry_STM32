

#include "./BSP/LCD/lcd.h"

#include "./BSP/LCD/lcd_digit_font_simhei.h"
#include "./BSP/LCD/lcdfont.h"
#include "./SYSTEM/usart/usart.h"
#include "stdio.h"
#include "stdlib.h"


#include "./BSP/LCD/lcd_ex.c"

SRAM_HandleTypeDef g_sram_handle; 


uint32_t g_point_color = 0xF800; 
uint32_t g_back_color = 0xFFFF;  


_lcd_dev lcddev;


void lcd_wr_data(volatile uint16_t data) {
  data = data; 
  LCD->LCD_RAM = data;
}


void lcd_wr_regno(volatile uint16_t regno) {
  regno = regno; 
  LCD->LCD_REG = regno; 
}


void lcd_write_reg(uint16_t regno, uint16_t data) {
  LCD->LCD_REG = regno; 
  LCD->LCD_RAM = data;  
}


static void lcd_opt_delay(uint32_t i) {
  while (i--); 
}


static uint16_t lcd_rd_data(void) {
  volatile uint16_t ram; 
  lcd_opt_delay(2);
  ram = LCD->LCD_RAM;
  return ram;
}


void lcd_write_ram_prepare(void) { LCD->LCD_REG = lcddev.wramcmd; }


uint32_t lcd_read_point(uint16_t x, uint16_t y) {
  uint16_t r = 0, g = 0, b = 0;

  if (x >= lcddev.width || y >= lcddev.height) {
    return 0; 
  }

  lcd_set_cursor(x, y); 

  if (lcddev.id == 0x5510) {
    lcd_wr_regno(0x2E00); 
  } else {
    lcd_wr_regno(0x2E); 
  }

  r = lcd_rd_data(); 

  if (lcddev.id == 0x1963) {
    return r; 
  }

  r = lcd_rd_data(); 

  if (lcddev.id == 0x7796) 
  {
    return r;
  }

  
  b = lcd_rd_data();
  g = r & 0xFF; 
  g <<= 8;

  return (((r >> 11) << 11) | ((g >> 10) << 5) |
          (b >> 11)); 
}


void lcd_display_on(void) {
  if (lcddev.id == 0x5510) {
    lcd_wr_regno(0x2900); 
  } else 
  {
    lcd_wr_regno(0x29); 
  }
}


void lcd_display_off(void) {
  if (lcddev.id == 0x5510) {
    lcd_wr_regno(0x2800); 
  } else 
  {
    lcd_wr_regno(0x28); 
  }
}


void lcd_set_cursor(uint16_t x, uint16_t y) {
  if (lcddev.id == 0x1963) {
    if (lcddev.dir == 0) 
    {
      x = lcddev.width - 1 - x;
      lcd_wr_regno(lcddev.setxcmd);
      lcd_wr_data(0);
      lcd_wr_data(0);
      lcd_wr_data(x >> 8);
      lcd_wr_data(x & 0xFF);
    } else 
    {
      lcd_wr_regno(lcddev.setxcmd);
      lcd_wr_data(x >> 8);
      lcd_wr_data(x & 0xFF);
      lcd_wr_data((lcddev.width - 1) >> 8);
      lcd_wr_data((lcddev.width - 1) & 0xFF);
    }

    lcd_wr_regno(lcddev.setycmd);
    lcd_wr_data(y >> 8);
    lcd_wr_data(y & 0xFF);
    lcd_wr_data((lcddev.height - 1) >> 8);
    lcd_wr_data((lcddev.height - 1) & 0xFF);

  } else if (lcddev.id == 0x5510) {
    lcd_wr_regno(lcddev.setxcmd);
    lcd_wr_data(x >> 8);
    lcd_wr_regno(lcddev.setxcmd + 1);
    lcd_wr_data(x & 0xFF);
    lcd_wr_regno(lcddev.setycmd);
    lcd_wr_data(y >> 8);
    lcd_wr_regno(lcddev.setycmd + 1);
    lcd_wr_data(y & 0xFF);
  } else 
  {
    lcd_wr_regno(lcddev.setxcmd);
    lcd_wr_data(x >> 8);
    lcd_wr_data(x & 0xFF);
    lcd_wr_regno(lcddev.setycmd);
    lcd_wr_data(y >> 8);
    lcd_wr_data(y & 0xFF);
  }
}


void lcd_scan_dir(uint8_t dir) {
  uint16_t regval = 0;
  uint16_t dirreg = 0;
  uint16_t temp;

  
  if ((lcddev.dir == 1 && lcddev.id != 0x1963) ||
      (lcddev.dir == 0 && lcddev.id == 0x1963)) {
    switch (dir) 
    {
      case 0:
        dir = 6;
        break;

      case 1:
        dir = 7;
        break;

      case 2:
        dir = 4;
        break;

      case 3:
        dir = 5;
        break;

      case 4:
        dir = 1;
        break;

      case 5:
        dir = 0;
        break;

      case 6:
        dir = 3;
        break;

      case 7:
        dir = 2;
        break;
    }
  }

  
  switch (dir) {
    case L2R_U2D: 
      regval |= (0 << 7) | (0 << 6) | (0 << 5);
      break;

    case L2R_D2U: 
      regval |= (1 << 7) | (0 << 6) | (0 << 5);
      break;

    case R2L_U2D: 
      regval |= (0 << 7) | (1 << 6) | (0 << 5);
      break;

    case R2L_D2U: 
      regval |= (1 << 7) | (1 << 6) | (0 << 5);
      break;

    case U2D_L2R: 
      regval |= (0 << 7) | (0 << 6) | (1 << 5);
      break;

    case U2D_R2L: 
      regval |= (0 << 7) | (1 << 6) | (1 << 5);
      break;

    case D2U_L2R: 
      regval |= (1 << 7) | (0 << 6) | (1 << 5);
      break;

    case D2U_R2L: 
      regval |= (1 << 7) | (1 << 6) | (1 << 5);
      break;
  }

  dirreg = 0x36; 

  if (lcddev.id == 0x5510) {
    dirreg = 0x3600; 
  }

  
  if (lcddev.id == 0x9341 || lcddev.id == 0x7789 || lcddev.id == 0x7796) {
    regval |= 0x08;
  }

  lcd_write_reg(dirreg, regval);

  if (lcddev.id != 0x1963) 
  {
    if (regval & 0x20) {
      if (lcddev.width < lcddev.height) 
      {
        temp = lcddev.width;
        lcddev.width = lcddev.height;
        lcddev.height = temp;
      }
    } else {
      if (lcddev.width > lcddev.height) 
      {
        temp = lcddev.width;
        lcddev.width = lcddev.height;
        lcddev.height = temp;
      }
    }
  }

  
  if (lcddev.id == 0x5510) {
    lcd_wr_regno(lcddev.setxcmd);
    lcd_wr_data(0);
    lcd_wr_regno(lcddev.setxcmd + 1);
    lcd_wr_data(0);
    lcd_wr_regno(lcddev.setxcmd + 2);
    lcd_wr_data((lcddev.width - 1) >> 8);
    lcd_wr_regno(lcddev.setxcmd + 3);
    lcd_wr_data((lcddev.width - 1) & 0xFF);
    lcd_wr_regno(lcddev.setycmd);
    lcd_wr_data(0);
    lcd_wr_regno(lcddev.setycmd + 1);
    lcd_wr_data(0);
    lcd_wr_regno(lcddev.setycmd + 2);
    lcd_wr_data((lcddev.height - 1) >> 8);
    lcd_wr_regno(lcddev.setycmd + 3);
    lcd_wr_data((lcddev.height - 1) & 0xFF);
  } else {
    lcd_wr_regno(lcddev.setxcmd);
    lcd_wr_data(0);
    lcd_wr_data(0);
    lcd_wr_data((lcddev.width - 1) >> 8);
    lcd_wr_data((lcddev.width - 1) & 0xFF);
    lcd_wr_regno(lcddev.setycmd);
    lcd_wr_data(0);
    lcd_wr_data(0);
    lcd_wr_data((lcddev.height - 1) >> 8);
    lcd_wr_data((lcddev.height - 1) & 0xFF);
  }
}


void lcd_draw_point(uint16_t x, uint16_t y, uint32_t color) {
  lcd_set_cursor(x, y);    
  lcd_write_ram_prepare(); 
  LCD->LCD_RAM = color;
}


void lcd_ssd_backlight_set(uint8_t pwm) {
  lcd_wr_regno(0xBE);      
  lcd_wr_data(0x05);       
  lcd_wr_data(pwm * 2.55); 
  lcd_wr_data(0x01);       
  lcd_wr_data(0xFF);       
  lcd_wr_data(0x00);       
  lcd_wr_data(0x00);       
}


void lcd_display_dir(uint8_t dir) {
  lcddev.dir = dir; 

  if (dir == 0) 
  {
    lcddev.width = 240;
    lcddev.height = 320;

    if (lcddev.id == 0x5510) {
      lcddev.wramcmd = 0x2C00;
      lcddev.setxcmd = 0x2A00;
      lcddev.setycmd = 0x2B00;
      lcddev.width = 480;
      lcddev.height = 800;
    } else if (lcddev.id == 0x1963) {
      lcddev.wramcmd = 0x2C; 
      lcddev.setxcmd = 0x2B; 
      lcddev.setycmd = 0x2A; 
      lcddev.width = 480;    
      lcddev.height = 800;   
    } else                   
    {
      lcddev.wramcmd = 0x2C;
      lcddev.setxcmd = 0x2A;
      lcddev.setycmd = 0x2B;
    }

    if (lcddev.id == 0x5310 || lcddev.id == 0x7796) 
    {
      lcddev.width = 320;
      lcddev.height = 480;
    }

    if (lcddev.id == 0X9806) 
    {
      lcddev.width = 480;
      lcddev.height = 800;
    }
  } else 
  {
    lcddev.width = 320;  
    lcddev.height = 240; 

    if (lcddev.id == 0x5510) {
      lcddev.wramcmd = 0x2C00;
      lcddev.setxcmd = 0x2A00;
      lcddev.setycmd = 0x2B00;
      lcddev.width = 800;
      lcddev.height = 480;
    } else if (lcddev.id == 0x1963 || lcddev.id == 0x9806) {
      lcddev.wramcmd = 0x2C; 
      lcddev.setxcmd = 0x2A; 
      lcddev.setycmd = 0x2B; 
      lcddev.width = 800;    
      lcddev.height = 480;   
    } else                   
    {
      lcddev.wramcmd = 0x2C;
      lcddev.setxcmd = 0x2A;
      lcddev.setycmd = 0x2B;
    }

    if (lcddev.id == 0x5310 || lcddev.id == 0x7796) 
    {
      lcddev.width = 480;
      lcddev.height = 320;
    }
  }

  lcd_scan_dir(DFT_SCAN_DIR); 
}


void lcd_set_window(uint16_t sx, uint16_t sy, uint16_t width, uint16_t height) {
  uint16_t twidth, theight;
  twidth = sx + width - 1;
  theight = sy + height - 1;

  if (lcddev.id == 0x1963 &&
      lcddev.dir != 1) 
  {
    sx = lcddev.width - width - sx;
    height = sy + height - 1;
    lcd_wr_regno(lcddev.setxcmd);
    lcd_wr_data(sx >> 8);
    lcd_wr_data(sx & 0xFF);
    lcd_wr_data((sx + width - 1) >> 8);
    lcd_wr_data((sx + width - 1) & 0xFF);
    lcd_wr_regno(lcddev.setycmd);
    lcd_wr_data(sy >> 8);
    lcd_wr_data(sy & 0xFF);
    lcd_wr_data(height >> 8);
    lcd_wr_data(height & 0xFF);
  } else if (lcddev.id == 0x5510) {
    lcd_wr_regno(lcddev.setxcmd);
    lcd_wr_data(sx >> 8);
    lcd_wr_regno(lcddev.setxcmd + 1);
    lcd_wr_data(sx & 0xFF);
    lcd_wr_regno(lcddev.setxcmd + 2);
    lcd_wr_data(twidth >> 8);
    lcd_wr_regno(lcddev.setxcmd + 3);
    lcd_wr_data(twidth & 0xFF);
    lcd_wr_regno(lcddev.setycmd);
    lcd_wr_data(sy >> 8);
    lcd_wr_regno(lcddev.setycmd + 1);
    lcd_wr_data(sy & 0xFF);
    lcd_wr_regno(lcddev.setycmd + 2);
    lcd_wr_data(theight >> 8);
    lcd_wr_regno(lcddev.setycmd + 3);
    lcd_wr_data(theight & 0xFF);
  } else 
  {
    lcd_wr_regno(lcddev.setxcmd);
    lcd_wr_data(sx >> 8);
    lcd_wr_data(sx & 0xFF);
    lcd_wr_data(twidth >> 8);
    lcd_wr_data(twidth & 0xFF);
    lcd_wr_regno(lcddev.setycmd);
    lcd_wr_data(sy >> 8);
    lcd_wr_data(sy & 0xFF);
    lcd_wr_data(theight >> 8);
    lcd_wr_data(theight & 0xFF);
  }
}


void HAL_SRAM_MspInit(SRAM_HandleTypeDef* hsram) {
  GPIO_InitTypeDef gpio_init_struct;

  __HAL_RCC_FSMC_CLK_ENABLE();  
  __HAL_RCC_GPIOD_CLK_ENABLE(); 
  __HAL_RCC_GPIOE_CLK_ENABLE(); 

  
  gpio_init_struct.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_8 | GPIO_PIN_9 |
                         GPIO_PIN_10 | GPIO_PIN_14 | GPIO_PIN_15;
  gpio_init_struct.Mode = GPIO_MODE_AF_PP;       
  gpio_init_struct.Pull = GPIO_PULLUP;           
  gpio_init_struct.Speed = GPIO_SPEED_FREQ_HIGH; 
  gpio_init_struct.Alternate = GPIO_AF12_FSMC;   

  HAL_GPIO_Init(GPIOD, &gpio_init_struct); 

  
  gpio_init_struct.Pin = GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 |
                         GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 |
                         GPIO_PIN_15;
  HAL_GPIO_Init(GPIOE, &gpio_init_struct);
}


void lcd_init(void) {
  GPIO_InitTypeDef gpio_init_struct;
  FSMC_NORSRAM_TimingTypeDef fsmc_read_handle;
  FSMC_NORSRAM_TimingTypeDef fsmc_write_handle;

  LCD_CS_GPIO_CLK_ENABLE(); 
  LCD_WR_GPIO_CLK_ENABLE(); 
  LCD_RD_GPIO_CLK_ENABLE(); 
  LCD_RS_GPIO_CLK_ENABLE(); 
  LCD_BL_GPIO_CLK_ENABLE(); 

  gpio_init_struct.Pin = LCD_CS_GPIO_PIN;
  gpio_init_struct.Mode = GPIO_MODE_AF_PP;            
  gpio_init_struct.Pull = GPIO_PULLUP;                
  gpio_init_struct.Speed = GPIO_SPEED_FREQ_HIGH;      
  gpio_init_struct.Alternate = GPIO_AF12_FSMC;        
  HAL_GPIO_Init(LCD_CS_GPIO_PORT, &gpio_init_struct); 

  gpio_init_struct.Pin = LCD_WR_GPIO_PIN;
  HAL_GPIO_Init(LCD_WR_GPIO_PORT, &gpio_init_struct); 

  gpio_init_struct.Pin = LCD_RD_GPIO_PIN;
  HAL_GPIO_Init(LCD_RD_GPIO_PORT, &gpio_init_struct); 

  gpio_init_struct.Pin = LCD_RS_GPIO_PIN;
  HAL_GPIO_Init(LCD_RS_GPIO_PORT, &gpio_init_struct); 

  gpio_init_struct.Pin = LCD_BL_GPIO_PIN;
  gpio_init_struct.Mode = GPIO_MODE_OUTPUT_PP; 
  HAL_GPIO_Init(LCD_BL_GPIO_PORT,
                &gpio_init_struct); 

  g_sram_handle.Instance = FSMC_NORSRAM_DEVICE;
  g_sram_handle.Extended = FSMC_NORSRAM_EXTENDED_DEVICE;

  g_sram_handle.Init.NSBank = FSMC_NORSRAM_BANK4; 
  g_sram_handle.Init.DataAddressMux =
      FSMC_DATA_ADDRESS_MUX_DISABLE; 
  g_sram_handle.Init.MemoryDataWidth =
      FSMC_NORSRAM_MEM_BUS_WIDTH_16; 
  g_sram_handle.Init.BurstAccessMode =
      FSMC_BURST_ACCESS_MODE_DISABLE; 
  g_sram_handle.Init.WaitSignalPolarity =
      FSMC_WAIT_SIGNAL_POLARITY_LOW; 
  g_sram_handle.Init.WaitSignalActive =
      FSMC_WAIT_TIMING_BEFORE_WS; 
  g_sram_handle.Init.WriteOperation =
      FSMC_WRITE_OPERATION_ENABLE; 
  g_sram_handle.Init.WaitSignal =
      FSMC_WAIT_SIGNAL_DISABLE; 
  g_sram_handle.Init.ExtendedMode =
      FSMC_EXTENDED_MODE_ENABLE; 
  g_sram_handle.Init.AsynchronousWait =
      FSMC_ASYNCHRONOUS_WAIT_DISABLE; 
  g_sram_handle.Init.WriteBurst = FSMC_WRITE_BURST_DISABLE; 

  
  fsmc_read_handle.AddressSetupTime =
      0x0F; 
  fsmc_read_handle.AddressHoldTime = 0x00; 
  fsmc_read_handle.DataSetupTime =
      60; 
  
  fsmc_read_handle.AccessMode = FSMC_ACCESS_MODE_A; 

  
  fsmc_write_handle.AddressSetupTime =
      9; 
  fsmc_write_handle.AddressHoldTime = 0x00; 
  fsmc_write_handle.DataSetupTime =
      9; 
  
  fsmc_write_handle.AccessMode = FSMC_ACCESS_MODE_A; 

  HAL_SRAM_Init(&g_sram_handle, &fsmc_read_handle, &fsmc_write_handle);
  delay_ms(50);

  
  lcd_wr_regno(0xD3);
  lcddev.id = lcd_rd_data(); 
  lcddev.id = lcd_rd_data(); 
  lcddev.id = lcd_rd_data(); 
  lcddev.id <<= 8;
  lcddev.id |= lcd_rd_data(); 

  if (lcddev.id != 0x9341) 
  {
    lcd_wr_regno(0x04);
    lcddev.id = lcd_rd_data(); 
    lcddev.id = lcd_rd_data(); 
    lcddev.id = lcd_rd_data(); 
    lcddev.id <<= 8;
    lcddev.id |= lcd_rd_data(); 

    if (lcddev.id == 0x8552) 
    {
      lcddev.id = 0x7789;
    }

    if (lcddev.id != 0x7789) 
    {
      lcd_wr_regno(0xD4);
      lcddev.id = lcd_rd_data(); 
      lcddev.id = lcd_rd_data(); 
      lcddev.id = lcd_rd_data(); 
      lcddev.id <<= 8;
      lcddev.id |= lcd_rd_data(); 

      if (lcddev.id != 0x5310) 
      {
        lcd_wr_regno(0XD3);
        lcddev.id = lcd_rd_data(); 
        lcddev.id = lcd_rd_data(); 
        lcddev.id = lcd_rd_data(); 
        lcddev.id <<= 8;
        lcddev.id |= lcd_rd_data(); 

        if (lcddev.id != 0x7796) 
        {
          
          lcd_write_reg(0xF000, 0x0055);
          lcd_write_reg(0xF001, 0x00AA);
          lcd_write_reg(0xF002, 0x0052);
          lcd_write_reg(0xF003, 0x0008);
          lcd_write_reg(0xF004, 0x0001);

          lcd_wr_regno(0xC500);      
          lcddev.id = lcd_rd_data(); 
          lcddev.id <<= 8;

          lcd_wr_regno(0xC501);       
          lcddev.id |= lcd_rd_data(); 

          delay_ms(5); 

          if (lcddev.id != 0x5510) 
          {
            lcd_wr_regno(0XD3);
            lcddev.id = lcd_rd_data(); 
            lcddev.id = lcd_rd_data(); 
            lcddev.id = lcd_rd_data(); 
            lcddev.id <<= 8;
            lcddev.id |= lcd_rd_data(); 

            if (lcddev.id != 0x9806) 
            {
              lcd_wr_regno(0xA1);
              lcddev.id = lcd_rd_data();
              lcddev.id = lcd_rd_data(); 
              lcddev.id <<= 8;
              lcddev.id |= lcd_rd_data(); 

              if (lcddev.id == 0x5761)
                lcddev.id =
                    0x1963; 
            }
          }
        }
      }
    }
  }

  if (lcddev.id == 0x7789) {
    lcd_ex_st7789_reginit(); 
  } else if (lcddev.id == 0x9341) {
    lcd_ex_ili9341_reginit(); 
  } else if (lcddev.id == 0x5310) {
    lcd_ex_nt35310_reginit(); 
  } else if (lcddev.id == 0x7796) {
    lcd_ex_st7796_reginit(); 
  } else if (lcddev.id == 0x5510) {
    lcd_ex_nt35510_reginit(); 
  } else if (lcddev.id == 0x9806) {
    lcd_ex_ili9806_reginit(); 
  } else if (lcddev.id == 0x1963) {
    lcd_ex_ssd1963_reginit();   
    lcd_ssd_backlight_set(100); 
  }

  
  
  if (lcddev.id == 0x7789) {
    
    fsmc_write_handle.AddressSetupTime =
        3; 
    fsmc_write_handle.DataSetupTime =
        3; 
    FSMC_NORSRAM_Extended_Timing_Init(
        g_sram_handle.Extended, &fsmc_write_handle, g_sram_handle.Init.NSBank,
        g_sram_handle.Init.ExtendedMode);
  } else if (lcddev.id == 0x9806 || lcddev.id == 0x9341 ||
             lcddev.id == 0x5510) {
    
    fsmc_write_handle.AddressSetupTime =
        2; 
    fsmc_write_handle.DataSetupTime =
        2; 
    FSMC_NORSRAM_Extended_Timing_Init(
        g_sram_handle.Extended, &fsmc_write_handle, g_sram_handle.Init.NSBank,
        g_sram_handle.Init.ExtendedMode);
  } else if (lcddev.id == 0x5310 || lcddev.id == 0x7796 ||
             lcddev.id == 0x1963) {
    
    fsmc_write_handle.AddressSetupTime =
        1; 
    fsmc_write_handle.DataSetupTime =
        1; 
    FSMC_NORSRAM_Extended_Timing_Init(
        g_sram_handle.Extended, &fsmc_write_handle, g_sram_handle.Init.NSBank,
        g_sram_handle.Init.ExtendedMode);
  }

  lcd_display_dir(1); 
  LCD_BL(1);          
  lcd_clear(BLUE);
}


void lcd_clear(uint16_t color) {
  uint32_t index = 0;
  uint32_t totalpoint = lcddev.width;

  totalpoint *= lcddev.height;  
  lcd_set_cursor(0x00, 0x0000); 
  lcd_write_ram_prepare();      

  for (index = 0; index < totalpoint; index++) {
    LCD->LCD_RAM = color;
  }
}


void lcd_fill(uint16_t sx, uint16_t sy, uint16_t ex, uint16_t ey,
              uint32_t color) {
  uint16_t i, j;
  uint16_t xlen = 0;
  xlen = ex - sx + 1;

  for (i = sy; i <= ey; i++) {
    lcd_set_cursor(sx, i);   
    lcd_write_ram_prepare(); 

    for (j = 0; j < xlen; j++) {
      LCD->LCD_RAM = color; 
    }
  }
}


void lcd_color_fill(uint16_t sx, uint16_t sy, uint16_t ex, uint16_t ey,
                    uint16_t* color) {
  uint16_t height, width;
  uint16_t i, j;

  width = ex - sx + 1;  
  height = ey - sy + 1; 

  for (i = 0; i < height; i++) {
    lcd_set_cursor(sx, sy + i); 
    lcd_write_ram_prepare();    

    for (j = 0; j < width; j++) {
      LCD->LCD_RAM = color[i * width + j]; 
    }
  }
}


void lcd_draw_line(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2,
                   uint16_t color) {
  uint16_t t;
  int xerr = 0, yerr = 0, delta_x, delta_y, distance;
  int incx, incy, row, col;
  delta_x = x2 - x1; 
  delta_y = y2 - y1;
  row = x1;
  col = y1;

  if (delta_x > 0) {
    incx = 1; 
  } else if (delta_x == 0) {
    incx = 0; 
  } else {
    incx = -1;
    delta_x = -delta_x;
  }

  if (delta_y > 0) {
    incy = 1;
  } else if (delta_y == 0) {
    incy = 0; 
  } else {
    incy = -1;
    delta_y = -delta_y;
  }

  if (delta_x > delta_y) {
    distance = delta_x; 
  } else {
    distance = delta_y;
  }

  for (t = 0; t <= distance + 1; t++) 
  {
    lcd_draw_point(row, col, color); 
    xerr += delta_x;
    yerr += delta_y;

    if (xerr > distance) {
      xerr -= distance;
      row += incx;
    }

    if (yerr > distance) {
      yerr -= distance;
      col += incy;
    }
  }
}


void lcd_draw_hline(uint16_t x, uint16_t y, uint16_t len, uint16_t color) {
  if ((len == 0) || (x > lcddev.width) || (y > lcddev.height)) {
    return;
  }

  lcd_fill(x, y, x + len - 1, y, color);
}


void lcd_draw_rectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2,
                        uint16_t color) {
  lcd_draw_line(x1, y1, x2, y1, color);
  lcd_draw_line(x1, y1, x1, y2, color);
  lcd_draw_line(x1, y2, x2, y2, color);
  lcd_draw_line(x2, y1, x2, y2, color);
}


void lcd_draw_circle(uint16_t x0, uint16_t y0, uint8_t r, uint16_t color) {
  int a, b;
  int di;

  a = 0;
  b = r;
  di = 3 - (r << 1); 

  while (a <= b) {
    lcd_draw_point(x0 + a, y0 - b, color); 
    lcd_draw_point(x0 + b, y0 - a, color); 
    lcd_draw_point(x0 + b, y0 + a, color); 
    lcd_draw_point(x0 + a, y0 + b, color); 
    lcd_draw_point(x0 - a, y0 + b, color); 
    lcd_draw_point(x0 - b, y0 + a, color);
    lcd_draw_point(x0 - a, y0 - b, color); 
    lcd_draw_point(x0 - b, y0 - a, color); 
    a++;

    
    if (di < 0) {
      di += 4 * a + 6;
    } else {
      di += 10 + 4 * (a - b);
      b--;
    }
  }
}


void lcd_fill_circle(uint16_t x, uint16_t y, uint16_t r, uint16_t color) {
  uint32_t i;
  uint32_t imax = ((uint32_t)r * 707) / 1000 + 1;
  uint32_t sqmax = (uint32_t)r * (uint32_t)r + (uint32_t)r / 2;
  uint32_t xr = r;

  lcd_draw_hline(x - r, y, 2 * r, color);

  for (i = 1; i <= imax; i++) {
    if ((i * i + xr * xr) > sqmax) {
      
      if (xr > imax) {
        lcd_draw_hline(x - i + 1, y + xr, 2 * (i - 1), color);
        lcd_draw_hline(x - i + 1, y - xr, 2 * (i - 1), color);
      }

      xr--;
    }

    
    lcd_draw_hline(x - xr, y + i, 2 * xr, color);
    lcd_draw_hline(x - xr, y - i, 2 * xr, color);
  }
}


void lcd_show_char(uint16_t x, uint16_t y, char chr, uint8_t size, uint8_t mode,
                   uint16_t color) {
  uint8_t temp, t1, t;
  uint16_t y0 = y;
  uint8_t csize = 0;
  uint8_t* pfont = 0;

  csize = (size / 8 + ((size % 8) ? 1 : 0)) *
          (size / 2); 
  chr = chr - ' '; 

  switch (size) {
    case 12:
      pfont = (uint8_t*)asc2_1206[chr]; 
      break;

    case 16:
      pfont = (uint8_t*)asc2_1608[chr]; 
      break;

    case 24:
      pfont = (uint8_t*)asc2_2412[chr]; 
      break;

    case 32:
      pfont = (uint8_t*)asc2_3216[chr]; 
      break;

    default:
      return;
  }

  for (t = 0; t < csize; t++) {
    temp = pfont[t]; 

    for (t1 = 0; t1 < 8; t1++) 
    {
      if (temp & 0x80) 
      {
        lcd_draw_point(x, y, color); 
      } else if (mode == 0)          
      {
        lcd_draw_point(
            x, y, g_back_color); 
      }

      temp <<= 1; 
      y++;

      if (y >= lcddev.height) return; 

      if ((y - y0) == size) 
      {
        y = y0; 
        x++;    

        if (x >= lcddev.width) {
          return; 
        }

        break;
      }
    }
  }
}


static uint32_t lcd_pow(uint8_t m, uint8_t n) {
  uint32_t result = 1;

  while (n--) {
    result *= m;
  }

  return result;
}


void lcd_show_num(uint16_t x, uint16_t y, uint32_t num, uint8_t len,
                  uint8_t size, uint16_t color) {
  uint8_t t, temp;
  uint8_t enshow = 0;

  for (t = 0; t < len; t++) 
  {
    temp = (num / lcd_pow(10, len - t - 1)) % 10; 

    if (enshow == 0 && t < (len - 1)) 
    {
      if (temp == 0) {
        lcd_show_char(x + (size / 2) * t, y, ' ', size, 0,
                      color); 
        continue;             
      } else {
        enshow = 1; 
      }
    }

    lcd_show_char(x + (size / 2) * t, y, temp + '0', size, 0,
                  color); 
  }
}


void lcd_show_xnum(uint16_t x, uint16_t y, uint32_t num, uint8_t len,
                   uint8_t size, uint8_t mode, uint16_t color) {
  uint8_t t, temp;
  uint8_t enshow = 0;

  for (t = 0; t < len; t++) 
  {
    temp = (num / lcd_pow(10, len - t - 1)) % 10; 

    if (enshow == 0 && t < (len - 1)) 
    {
      if (temp == 0) {
        if (mode & 0x80) 
        {
          lcd_show_char(x + (size / 2) * t, y, '0', size, mode & 0x01,
                        color); 
        } else {
          lcd_show_char(x + (size / 2) * t, y, ' ', size, mode & 0x01,
                        color); 
        }

        continue;
      } else {
        enshow = 1; 
      }
    }

    lcd_show_char(x + (size / 2) * t, y, temp + '0', size, mode & 0x01, color);
  }
}


void lcd_show_string(uint16_t x, uint16_t y, uint16_t width, uint16_t height,
                     uint8_t size, char* p, uint16_t color) {
  uint8_t x0 = x;

  width += x;
  height += y;

  while ((*p <= '~') && (*p >= ' ')) 
  {
    if (x >= width) {
      x = x0;
      y += size;
    }

    if (y >= height) {
      break; 
    }

    lcd_show_char(x, y, *p, size, 1, color);
    x += size / 2;
    p++;
  }
}

void lcd_show_char_scaled(uint16_t x, uint16_t y, char chr, uint8_t size,
                          uint8_t scale, uint16_t color) {
  uint8_t temp, t1, t;
  uint8_t csize = 0;
  uint8_t* pfont = 0;
  uint16_t draw_x = x;
  uint16_t draw_y = y;
  uint16_t y0 = y;

  if (scale == 0) {
    return;
  }

  csize = (size / 8 + ((size % 8) ? 1 : 0)) * (size / 2);
  chr = chr - ' ';

  switch (size) {
    case 12:
      pfont = (uint8_t*)asc2_1206[(uint8_t)chr];
      break;
    case 16:
      pfont = (uint8_t*)asc2_1608[(uint8_t)chr];
      break;
    case 24:
      pfont = (uint8_t*)asc2_2412[(uint8_t)chr];
      break;
    case 32:
      pfont = (uint8_t*)asc2_3216[(uint8_t)chr];
      break;
    default:
      return;
  }

  for (t = 0; t < csize; t++) {
    temp = pfont[t];

    for (t1 = 0; t1 < 8; t1++) {
      if (temp & 0x80) {
        lcd_fill(draw_x, draw_y, draw_x + scale - 1, draw_y + scale - 1, color);
      }

      temp <<= 1;
      draw_y += scale;

      if ((draw_y - y0) == (uint16_t)(size * scale)) {
        draw_y = y0;
        draw_x += scale;
        break;
      }
    }
  }
}


void lcd_show_string_scaled(uint16_t x, uint16_t y, char* p, uint8_t size,
                            uint8_t scale, uint16_t color) {
  while ((*p <= '~') && (*p >= ' ')) {
    lcd_show_char_scaled(x, y, *p, size, scale, color);
    x += (size / 2) * scale;
    p++;
  }
}

void lcd_show_char_scaled_bold(uint16_t x, uint16_t y, char chr, uint8_t size,
                               uint8_t scale, uint16_t color) {
  uint8_t temp, t1, t;
  uint8_t csize = 0;
  uint8_t* pfont = 0;
  uint16_t draw_x = x;
  uint16_t draw_y = y;
  uint16_t y0 = y;
  uint16_t block_right;
  uint16_t block_bottom;

  if (scale == 0) {
    return;
  }

  csize = (size / 8 + ((size % 8) ? 1 : 0)) * (size / 2);
  chr = chr - ' ';

  switch (size) {
    case 12:
      pfont = (uint8_t*)asc2_1206[(uint8_t)chr];
      break;
    case 16:
      pfont = (uint8_t*)asc2_1608[(uint8_t)chr];
      break;
    case 24:
      pfont = (uint8_t*)asc2_2412[(uint8_t)chr];
      break;
    case 32:
      pfont = (uint8_t*)asc2_3216[(uint8_t)chr];
      break;
    default:
      return;
  }

  for (t = 0; t < csize; t++) {
    temp = pfont[t];

    for (t1 = 0; t1 < 8; t1++) {
      if (temp & 0x80) {
        block_right = draw_x + scale;
        block_bottom = draw_y + scale;

        if (block_right >= lcddev.width) {
          block_right = lcddev.width - 1;
        }

        if (block_bottom >= lcddev.height) {
          block_bottom = lcddev.height - 1;
        }

        lcd_fill(draw_x, draw_y, block_right, block_bottom, color);
      }

      temp <<= 1;
      draw_y += scale;

      if ((draw_y - y0) == (uint16_t)(size * scale)) {
        draw_y = y0;
        draw_x += scale;
        break;
      }
    }
  }
}


void lcd_show_string_scaled_bold(uint16_t x, uint16_t y, char* p, uint8_t size,
                                 uint8_t scale, uint16_t color) {
  while ((*p <= '~') && (*p >= ' ')) {
    lcd_show_char_scaled_bold(x, y, *p, size, scale, color);
    x += (size / 2) * scale;
    p++;
  }
}

static uint8_t lcd_bold_int_strlen(const char* buf);
static void lcd_bold_int_to_str(int32_t num, char* buf);
static uint16_t g_lmh_rect_buf[104 * 96];
static const uint16_t g_lmh_center_bg = WHITE;
static const uint16_t g_lmh_gear_color = BLACK;
static const uint16_t g_lmh_rpm_color = 0x52AA;
static const uint16_t g_lmh_speed_color = 0x4208;
static const uint16_t g_lmh_brake_temp_bg = 0x7C8F;
static const uint16_t g_lmh_brake_temp_color = WHITE;
static const uint16_t g_lmh_tire_bg = 0x85B9;
static const uint16_t g_lmh_tire_color = WHITE;
static const uint16_t g_lmh_bar_bg = 0x8410;
static const uint16_t g_lmh_brake_color = 0xF800;
static const uint16_t g_lmh_throttle_color = 0x07E0;
static const uint16_t g_lmh_water_label_color = LIGHTBLUE;
static const uint16_t g_lmh_oil_label_color = YELLOW;
static const uint16_t g_lmh_lap_bg = 0x1A06;
static const uint16_t g_lmh_fuel_bar_color = 0xFFE0;
static const uint16_t g_lmh_shift_off_color = 0x3186;
static const uint16_t g_lmh_shift_cyan_color = BLUE;
static const unsigned char g_lmh_small_letter_l_mask
    [SIMHEI_DIGIT_SMALL_WIDTH * SIMHEI_DIGIT_SMALL_HEIGHT] = {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0,
        0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0};

static void lcd_draw_lmh_pressure_badge(void);
static void lcd_draw_lmh_temp_badge(void);
static void lcd_draw_lmh_static_texts(void);
static uint8_t lcd_ascii_strlen(const char* str);
static void lcd_format_lap_time(uint32_t lap_ms, char* buf);
static uint16_t lcd_center_text_x(uint16_t region_x, uint16_t region_width,
                                  const char* str, uint8_t size);
static const uint8_t* lcd_get_ascii_font(char chr, uint8_t size,
                                         uint8_t* csize);
static void lcd_draw_char_to_buf(uint16_t* buf, uint16_t buf_width,
                                 uint16_t buf_height, uint16_t x, uint16_t y,
                                 char chr, uint8_t size, uint16_t color);
static void lcd_draw_string_to_buf(uint16_t* buf, uint16_t buf_width,
                                   uint16_t buf_height, uint16_t x,
                                   uint16_t y, const char* str, uint8_t size,
                                   uint16_t color);
static void lcd_compose_vertical_bar_rect(uint16_t rect_width,
                                          uint16_t rect_height,
                                          uint8_t fill_pct,
                                          uint16_t fill_color,
                                          uint16_t bg_color, uint16_t* buf);

static void lcd_fill_round_rect(uint16_t x1, uint16_t y1, uint16_t x2,
                                uint16_t y2, uint16_t radius, uint16_t color) {
  uint16_t width;
  uint16_t height;

  if ((x2 <= x1) || (y2 <= y1)) {
    return;
  }

  width = x2 - x1 + 1;
  height = y2 - y1 + 1;

  if ((radius * 2) > width) {
    radius = width / 2;
  }

  if ((radius * 2) > height) {
    radius = height / 2;
  }

  if (radius == 0) {
    lcd_fill(x1, y1, x2, y2, color);
    return;
  }

  lcd_fill(x1 + radius, y1, x2 - radius, y2, color);
  lcd_fill(x1, y1 + radius, x1 + radius - 1, y2 - radius, color);
  lcd_fill(x2 - radius + 1, y1 + radius, x2, y2 - radius, color);

  lcd_fill_circle(x1 + radius, y1 + radius, radius, color);
  lcd_fill_circle(x2 - radius, y1 + radius, radius, color);
  lcd_fill_circle(x1 + radius, y2 - radius, radius, color);
  lcd_fill_circle(x2 - radius, y2 - radius, radius, color);
}

static void lcd_draw_round_frame(uint16_t x1, uint16_t y1, uint16_t x2,
                                 uint16_t y2, uint16_t radius,
                                 uint16_t thickness, uint16_t frame_color,
                                 uint16_t fill_color) {
  if ((x2 <= x1) || (y2 <= y1)) {
    return;
  }

  lcd_fill_round_rect(x1, y1, x2, y2, radius, frame_color);

  if (((x2 - x1) > (thickness * 2)) && ((y2 - y1) > (thickness * 2))) {
    lcd_fill_round_rect(
        x1 + thickness, y1 + thickness, x2 - thickness, y2 - thickness,
        (radius > thickness) ? (radius - thickness) : 0, fill_color);
  }
}

static void lcd_draw_open_round_frame(uint16_t x1, uint16_t y1, uint16_t x2,
                                      uint16_t y2, uint16_t radius,
                                      uint16_t thickness, uint16_t frame_color,
                                      uint16_t fill_color,
                                      uint8_t open_bottom) {
  lcd_draw_round_frame(x1, y1, x2, y2, radius, thickness, frame_color,
                       fill_color);

  if (open_bottom) {
    lcd_fill(x1 + radius, y2 - thickness + 1, x2 - radius, y2, fill_color);
    lcd_fill(x1, y2 - radius, x1 + radius - 1, y2, fill_color);
    lcd_fill(x2 - radius + 1, y2 - radius, x2, y2, fill_color);
  }
}

void lcd_draw_lmh_demo(void) {
  uint16_t panel_light_green = 0x7C8F;
  uint16_t panel_light_blue = 0x85B9;
  uint16_t panel_dark_green = 0x1A06;
  uint16_t line_blue = 0x2196;
  uint16_t line_green = 0xAFE0;
  uint16_t line_gray = 0xC638;

  lcd_clear(BLACK);

  
  lcd_fill_round_rect(0, 24, 107, 103, 5, panel_light_green);
  lcd_fill(108, 24, 211, 175, g_lmh_center_bg);
  lcd_fill_round_rect(212, 24, 319, 103, 5, panel_light_blue);
  lcd_draw_round_frame(0, 104, 107, 175, 6, 3, line_blue, BLACK);
  lcd_draw_round_frame(212, 104, 319, 175, 6, 3, line_blue, panel_dark_green);

  
  lcd_fill(6, 64, 101, 66, line_gray);
  lcd_fill(52, 29, 55, 100, line_gray);
  lcd_draw_lmh_pressure_badge();
  lcd_fill(6, 173, 101, 175, line_blue);

  lcd_fill(218, 64, 313, 66, line_gray);
  lcd_fill(265, 29, 267, 100, line_gray);
  lcd_fill(218, 173, 313, 175, line_blue);
  lcd_draw_lmh_temp_badge();

  
  lcd_draw_open_round_frame(80, 176, 239, 239, 7, 3, line_green, BLACK, 1);
  lcd_fill(80, 183, 82, 239, line_green);
  lcd_fill(237, 183, 239, 239, line_green);

  lcd_draw_lmh_center_values(0, 0);
  lcd_draw_lmh_static_texts();
}

static void lcd_blit_rgb565_rect(uint16_t x, uint16_t y, uint16_t width,
                                 uint16_t height, const uint16_t* buf) {
  uint32_t total = (uint32_t)width * height;
  uint32_t i;

  lcd_set_window(x, y, width, height);
  lcd_write_ram_prepare();

  for (i = 0; i < total; i++) {
    LCD->LCD_RAM = buf[i];
  }
}

static const unsigned char* lcd_get_digit_mask(char digit, uint16_t width,
                                               uint16_t height) {
  uint8_t index;

  if ((width == SIMHEI_DIGIT_LARGE_WIDTH) &&
      (height == SIMHEI_DIGIT_LARGE_HEIGHT)) {
    if (digit == 'N') {
      return g_simhei_gear_letter_n_mask;
    }

    if (digit == 'R') {
      return g_simhei_gear_letter_r_mask;
    }
  }

  if ((width == SIMHEI_DIGIT_SMALL_WIDTH) &&
      (height == SIMHEI_DIGIT_SMALL_HEIGHT) && (digit == 'L')) {
    return g_lmh_small_letter_l_mask;
  }

  if ((digit < '0') || (digit > '9')) {
    return 0;
  }

  index = digit - '0';

  if ((width == SIMHEI_DIGIT_LARGE_WIDTH) &&
      (height == SIMHEI_DIGIT_LARGE_HEIGHT)) {
    return g_simhei_digits_large_mask[index];
  }

  if ((width == SIMHEI_DIGIT_SMALL_WIDTH) &&
      (height == SIMHEI_DIGIT_SMALL_HEIGHT)) {
    return g_simhei_digits_small_mask[index];
  }

  return 0;
}

static void lcd_compose_bold_int_rect(uint16_t rect_width, uint16_t rect_height,
                                      int32_t num, uint16_t digit_width,
                                      uint16_t digit_height, uint16_t spacing,
                                      uint16_t fg_color, uint16_t bg_color,
                                      uint16_t* buf) {
  char str[12];
  uint8_t len;
  uint16_t total_width;
  uint16_t start_x;
  uint16_t start_y;
  uint16_t i;
  uint32_t total_pixels;
  uint8_t digit_idx;

  lcd_bold_int_to_str(num, str);
  len = lcd_bold_int_strlen(str);
  total_width = len * digit_width;

  if (len > 1) {
    total_width += (len - 1) * spacing;
  }

  start_x = 0;
  start_y = 0;

  if (rect_width > total_width) {
    start_x = (rect_width - total_width) / 2;
  }

  if (rect_height > digit_height) {
    start_y = (rect_height - digit_height) / 2;
  }

  total_pixels = (uint32_t)rect_width * rect_height;
  for (i = 0; i < total_pixels; i++) {
    buf[i] = bg_color;
  }

  for (digit_idx = 0; digit_idx < len; digit_idx++) {
    const unsigned char* mask =
        lcd_get_digit_mask(str[digit_idx], digit_width, digit_height);
    uint16_t digit_x = start_x + digit_idx * (digit_width + spacing);
    uint16_t row;
    uint16_t col;

    if (mask == 0) {
      continue;
    }

    for (row = 0; row < digit_height; row++) {
      uint16_t dst_y = start_y + row;
      for (col = 0; col < digit_width; col++) {
        if (mask[row * digit_width + col]) {
          uint16_t dst_x = digit_x + col;
          buf[dst_y * rect_width + dst_x] = fg_color;
        }
      }
    }
  }
}

static void lcd_compose_bold_char_rect(uint16_t rect_width,
                                       uint16_t rect_height, char chr,
                                       uint16_t digit_width,
                                       uint16_t digit_height, uint16_t fg_color,
                                       uint16_t bg_color, uint16_t* buf) {
  const unsigned char* mask =
      lcd_get_digit_mask(chr, digit_width, digit_height);
  uint16_t start_x = 0;
  uint16_t start_y = 0;
  uint32_t total_pixels;
  uint16_t row;
  uint16_t col;
  uint32_t i;

  total_pixels = (uint32_t)rect_width * rect_height;
  for (i = 0; i < total_pixels; i++) {
    buf[i] = bg_color;
  }

  if (mask == 0) {
    return;
  }

  if (rect_width > digit_width) {
    start_x = (rect_width - digit_width) / 2;
  }

  if (rect_height > digit_height) {
    start_y = (rect_height - digit_height) / 2;
  }

  for (row = 0; row < digit_height; row++) {
    uint16_t dst_y = start_y + row;
    for (col = 0; col < digit_width; col++) {
      if (mask[row * digit_width + col]) {
        uint16_t dst_x = start_x + col;
        buf[dst_y * rect_width + dst_x] = fg_color;
      }
    }
  }
}

static void lcd_compose_bold_fuel_rect(uint16_t rect_width,
                                       uint16_t rect_height, uint8_t fuel_liters,
                                       uint16_t fg_color, uint16_t bg_color,
                                       uint16_t* buf) {
  char str[12];
  uint8_t len;
  uint16_t digits_width;
  uint16_t total_width;
  uint16_t start_x;
  uint16_t digits_y;
  uint16_t letter_y;
  uint8_t idx;
  uint32_t total_pixels;
  uint32_t i;

  lcd_bold_int_to_str(fuel_liters, str);
  len = lcd_bold_int_strlen(str);
  digits_width = (uint16_t)(len * SIMHEI_DIGIT_SMALL_WIDTH);
  total_width = (uint16_t)(digits_width + SIMHEI_DIGIT_SMALL_WIDTH);
  start_x = 0;
  digits_y = 0;
  letter_y = 0;

  if (rect_width > total_width) {
    start_x = (uint16_t)((rect_width - total_width) / 2);
  }

  if (rect_height > SIMHEI_DIGIT_SMALL_HEIGHT) {
    digits_y = (uint16_t)((rect_height - SIMHEI_DIGIT_SMALL_HEIGHT) / 2);
    letter_y = digits_y;
  }

  total_pixels = (uint32_t)rect_width * rect_height;
  for (i = 0; i < total_pixels; i++) {
    buf[i] = bg_color;
  }

  for (idx = 0; idx < len; idx++) {
    const unsigned char* mask = lcd_get_digit_mask(
        str[idx], SIMHEI_DIGIT_SMALL_WIDTH, SIMHEI_DIGIT_SMALL_HEIGHT);
    uint16_t digit_x = (uint16_t)(start_x + idx * SIMHEI_DIGIT_SMALL_WIDTH);
    uint16_t row;
    uint16_t col;

    if (mask == 0) {
      continue;
    }

    for (row = 0; row < SIMHEI_DIGIT_SMALL_HEIGHT; row++) {
      uint16_t dst_y = (uint16_t)(digits_y + row);
      for (col = 0; col < SIMHEI_DIGIT_SMALL_WIDTH; col++) {
        if (mask[row * SIMHEI_DIGIT_SMALL_WIDTH + col]) {
          uint16_t dst_x = (uint16_t)(digit_x + col);
          buf[dst_y * rect_width + dst_x] = fg_color;
        }
      }
    }
  }

  {
    const unsigned char* mask = lcd_get_digit_mask(
        'L', SIMHEI_DIGIT_SMALL_WIDTH, SIMHEI_DIGIT_SMALL_HEIGHT);
    uint16_t letter_x = (uint16_t)(start_x + digits_width);
    uint16_t row;
    uint16_t col;

    if (mask != 0) {
      for (row = 0; row < SIMHEI_DIGIT_SMALL_HEIGHT; row++) {
        uint16_t dst_y = (uint16_t)(letter_y + row);
        for (col = 0; col < SIMHEI_DIGIT_SMALL_WIDTH; col++) {
          if (mask[row * SIMHEI_DIGIT_SMALL_WIDTH + col]) {
            uint16_t dst_x = (uint16_t)(letter_x + col);
            buf[dst_y * rect_width + dst_x] = fg_color;
          }
        }
      }
    }
  }
}

static void lcd_compose_bold_2digit_rect(uint16_t rect_width,
                                         uint16_t rect_height, int32_t num,
                                         uint16_t digit_width,
                                         uint16_t digit_height,
                                         uint16_t spacing, uint16_t fg_color,
                                         uint16_t bg_color, uint16_t* buf) {
  char str[2];
  uint16_t total_width = (digit_width * 2) + spacing;
  uint16_t start_x = 0;
  uint16_t start_y = 0;
  uint32_t total_pixels = (uint32_t)rect_width * rect_height;
  uint32_t i;
  uint8_t digit_idx;

  if (num < 0) {
    num = 0;
  } else if (num > 99) {
    num = 99;
  }

  str[0] = (char)('0' + ((num / 10) % 10));
  str[1] = (char)('0' + (num % 10));

  if (rect_width > total_width) {
    start_x = (rect_width - total_width) / 2;
  }

  if (rect_height > digit_height) {
    start_y = (rect_height - digit_height) / 2;
  }

  for (i = 0; i < total_pixels; i++) {
    buf[i] = bg_color;
  }

  for (digit_idx = 0; digit_idx < 2; digit_idx++) {
    const unsigned char* mask =
        lcd_get_digit_mask(str[digit_idx], digit_width, digit_height);
    uint16_t digit_x = start_x + digit_idx * (digit_width + spacing);
    uint16_t row;
    uint16_t col;

    if (mask == 0) {
      continue;
    }

    for (row = 0; row < digit_height; row++) {
      uint16_t dst_y = start_y + row;
      for (col = 0; col < digit_width; col++) {
        if (mask[row * digit_width + col]) {
          uint16_t dst_x = digit_x + col;
          buf[dst_y * rect_width + dst_x] = fg_color;
        }
      }
    }
  }
}

static void lcd_compose_bold_3digit_rect(uint16_t rect_width,
                                         uint16_t rect_height, int32_t num,
                                         uint16_t digit_width,
                                         uint16_t digit_height,
                                         uint16_t spacing, uint16_t fg_color,
                                         uint16_t bg_color, uint16_t* buf) {
  char str[3];
  uint16_t total_width = (digit_width * 3) + (spacing * 2);
  uint16_t start_x = 0;
  uint16_t start_y = 0;
  uint32_t total_pixels = (uint32_t)rect_width * rect_height;
  uint32_t i;
  uint8_t digit_idx;

  if (num < 0) {
    num = 0;
  } else if (num > 999) {
    num = 999;
  }

  str[0] = (char)('0' + ((num / 100) % 10));
  str[1] = (char)('0' + ((num / 10) % 10));
  str[2] = (char)('0' + (num % 10));

  if (rect_width > total_width) {
    start_x = (rect_width - total_width) / 2;
  }

  if (rect_height > digit_height) {
    start_y = (rect_height - digit_height) / 2;
  }

  for (i = 0; i < total_pixels; i++) {
    buf[i] = bg_color;
  }

  for (digit_idx = 0; digit_idx < 3; digit_idx++) {
    const unsigned char* mask =
        lcd_get_digit_mask(str[digit_idx], digit_width, digit_height);
    uint16_t digit_x = start_x + digit_idx * (digit_width + spacing);
    uint16_t row;
    uint16_t col;

    if (mask == 0) {
      continue;
    }

    for (row = 0; row < digit_height; row++) {
      uint16_t dst_y = start_y + row;
      for (col = 0; col < digit_width; col++) {
        if (mask[row * digit_width + col]) {
          uint16_t dst_x = digit_x + col;
          buf[dst_y * rect_width + dst_x] = fg_color;
        }
      }
    }
  }
}

static void lcd_draw_lmh_brake_temp_cell(uint16_t x, uint16_t y,
                                         uint16_t width, uint16_t height,
                                         int temp) {
  lcd_compose_bold_3digit_rect(width, height, temp,
                               SIMHEI_DIGIT_SMALL_WIDTH,
                               SIMHEI_DIGIT_SMALL_HEIGHT, 0,
                               g_lmh_brake_temp_color, g_lmh_brake_temp_bg,
                               g_lmh_rect_buf);
  lcd_blit_rgb565_rect(x, y, width, height, g_lmh_rect_buf);
}

static void lcd_draw_lmh_pressure_badge(void) {
  lcd_fill(47, 56, 60, 73, 0xC638);
  lcd_show_string_scaled_bold(50, 57, "B", 16, 1, BLACK);
}

static void lcd_draw_lmh_temp_badge(void) {
  lcd_fill(260, 56, 273, 73, 0xC638);
  lcd_show_string_scaled_bold(263, 57, "T", 16, 1, BLACK);
}

static void lcd_draw_lmh_static_texts(void) {
  uint16_t tc_col0_center = 22;
  uint16_t tc_col1_center = 54;
  uint16_t tc_col2_center = 86;

  lcd_show_string(19, 219, 8, 16, 16, "W", g_lmh_water_label_color);
  lcd_show_string(51, 219, 8, 16, 16, "O", g_lmh_oil_label_color);

  lcd_show_string(8, 110, 24, 12, 12, "TC L", 0x33FF);
  lcd_show_string(40, 110, 24, 12, 12, "TC C", 0xFD20);
  lcd_show_string(72, 110, 24, 12, 12, "TC S", YELLOW);

  lcd_compose_bold_int_rect(24, 20, 7, SIMHEI_DIGIT_SMALL_WIDTH,
                            SIMHEI_DIGIT_SMALL_HEIGHT, 0, WHITE, BLACK,
                            g_lmh_rect_buf);
  lcd_blit_rgb565_rect((uint16_t)(tc_col0_center - 12), 124, 24, 20,
                       g_lmh_rect_buf);

  lcd_compose_bold_int_rect(24, 20, 8, SIMHEI_DIGIT_SMALL_WIDTH,
                            SIMHEI_DIGIT_SMALL_HEIGHT, 0, WHITE, BLACK,
                            g_lmh_rect_buf);
  lcd_blit_rgb565_rect((uint16_t)(tc_col1_center - 12), 124, 24, 20,
                       g_lmh_rect_buf);

  lcd_compose_bold_int_rect(24, 20, 7, SIMHEI_DIGIT_SMALL_WIDTH,
                            SIMHEI_DIGIT_SMALL_HEIGHT, 0, WHITE, BLACK,
                            g_lmh_rect_buf);
  lcd_blit_rgb565_rect((uint16_t)(tc_col2_center - 12), 124, 24, 20,
                       g_lmh_rect_buf);

  lcd_fill(14, 148, 94, 149, 0x4208);

  lcd_show_string(14, 153, 8, 16, 16, "F", 0x07E0);
  lcd_show_string(26, 153, 8, 16, 16, "9", WHITE);
  lcd_show_string(42, 153, 24, 16, 16, "ARB", WHITE);
  lcd_show_string(76, 153, 8, 16, 16, "R", RED);
  lcd_show_string(88, 153, 8, 16, 16, "5", WHITE);

  lcd_show_string(286, 110, 30, 12, 12, "BEST", WHITE);
  lcd_show_string(274, 140, 42, 12, 12, "CURRENT", WHITE);

  lcd_show_string(251, 220, 32, 16, 16, "FUEL", WHITE);
}

static uint8_t lcd_ascii_strlen(const char* str) {
  uint8_t len = 0;

  while (str[len] != '\0') {
    len++;
  }

  return len;
}

static void lcd_format_lap_time(uint32_t lap_ms, char* buf) {
  uint32_t total_seconds = lap_ms / 1000U;
  uint32_t milliseconds = lap_ms % 1000U;
  uint16_t minutes = total_seconds / 60;
  uint16_t seconds = total_seconds % 60;

  buf[0] = '\0';

  if (lap_ms == 0) {
    buf[0] = '-';
    buf[1] = '-';
    buf[2] = ':';
    buf[3] = '-';
    buf[4] = '-';
    buf[5] = '.';
    buf[6] = '-';
    buf[7] = '-';
    buf[8] = '-';
    buf[9] = '\0';
    return;
  }

  sprintf(buf, "%u:%02u.%03u", minutes, seconds, milliseconds);
}

static uint16_t lcd_center_text_x(uint16_t region_x, uint16_t region_width,
                                  const char* str, uint8_t size) {
  uint16_t text_width = (uint16_t)(lcd_ascii_strlen(str) * (size / 2));

  if (region_width > text_width) {
    return (uint16_t)(region_x + ((region_width - text_width) / 2));
  }

  return region_x;
}

static const uint8_t* lcd_get_ascii_font(char chr, uint8_t size,
                                         uint8_t* csize) {
  if ((chr < ' ') || (chr > '~')) {
    return 0;
  }

  *csize = (uint8_t)((size / 8 + ((size % 8) ? 1 : 0)) * (size / 2));
  chr = (char)(chr - ' ');

  switch (size) {
    case 12:
      return (const uint8_t*)asc2_1206[(uint8_t)chr];
    case 16:
      return (const uint8_t*)asc2_1608[(uint8_t)chr];
    case 24:
      return (const uint8_t*)asc2_2412[(uint8_t)chr];
    case 32:
      return (const uint8_t*)asc2_3216[(uint8_t)chr];
    default:
      return 0;
  }
}

static void lcd_draw_char_to_buf(uint16_t* buf, uint16_t buf_width,
                                 uint16_t buf_height, uint16_t x, uint16_t y,
                                 char chr, uint8_t size, uint16_t color) {
  const uint8_t* pfont;
  uint8_t csize;
  uint8_t temp;
  uint8_t t1;
  uint8_t t;
  uint16_t draw_x = x;
  uint16_t draw_y = y;
  uint16_t y0 = y;

  pfont = lcd_get_ascii_font(chr, size, &csize);
  if (pfont == 0) {
    return;
  }

  for (t = 0; t < csize; t++) {
    temp = pfont[t];

    for (t1 = 0; t1 < 8; t1++) {
      if ((temp & 0x80) && (draw_x < buf_width) && (draw_y < buf_height)) {
        buf[draw_y * buf_width + draw_x] = color;
      }

      temp <<= 1;
      draw_y++;

      if ((draw_y - y0) == size) {
        draw_y = y0;
        draw_x++;
        break;
      }
    }
  }
}

static void lcd_draw_string_to_buf(uint16_t* buf, uint16_t buf_width,
                                   uint16_t buf_height, uint16_t x,
                                   uint16_t y, const char* str, uint8_t size,
                                   uint16_t color) {
  while (*str != '\0') {
    lcd_draw_char_to_buf(buf, buf_width, buf_height, x, y, *str, size, color);
    x = (uint16_t)(x + (size / 2));
    str++;
  }
}

static void lcd_fill_round_bar_to_buf(uint16_t* buf, uint16_t buf_width,
                                      uint16_t buf_height, uint16_t x,
                                      uint16_t y, uint16_t width,
                                      uint16_t height, uint16_t color) {
  uint16_t row;
  uint16_t col;

  for (row = 0; row < height; row++) {
    for (col = 0; col < width; col++) {
      uint8_t fill = 1;
      uint16_t dst_x = (uint16_t)(x + col);
      uint16_t dst_y = (uint16_t)(y + row);

      if ((dst_x >= buf_width) || (dst_y >= buf_height)) {
        continue;
      }

      if ((((col == 0) || (col == (width - 1))) &&
           ((row < 2) || (row >= (height - 2)))) ||
          (((col == 1) || (col == (width - 2))) &&
           ((row == 0) || (row == (height - 1))))) {
        fill = 0;
      }

      if (fill != 0) {
        buf[dst_y * buf_width + dst_x] = color;
      }
    }
  }
}

static uint16_t lcd_get_shift_light_color(uint8_t index, uint16_t rpm_pct_x10) {
  static const uint16_t thresholds[10] = {865, 865, 884, 884, 902,
                                          902, 921, 921, 940, 940};
  static const uint16_t colors[10] = {
      0x07E0, 0x07E0, 0xFFE0, 0xFFE0, 0xF800,
      0xF800, 0xF800, 0xF800, 0x07FF, 0x07FF};

  if (rpm_pct_x10 >= 940) {
    return g_lmh_shift_cyan_color;
  }

  if ((index < 10) && (rpm_pct_x10 >= thresholds[index])) {
    return colors[index];
  }

  return g_lmh_shift_off_color;
}

static void lcd_compose_vertical_bar_rect(uint16_t rect_width,
                                          uint16_t rect_height,
                                          uint8_t fill_pct,
                                          uint16_t fill_color,
                                          uint16_t bg_color, uint16_t* buf) {
  uint16_t fill_height;
  uint16_t x;
  uint16_t y;
  uint32_t i;
  uint32_t total_pixels = (uint32_t)rect_width * rect_height;

  if (fill_pct > 100) {
    fill_pct = 100;
  }

  fill_height = (uint16_t)(((uint32_t)rect_height * fill_pct) / 100U);

  for (i = 0; i < total_pixels; i++) {
    buf[i] = bg_color;
  }

  for (y = 0; y < fill_height; y++) {
    uint16_t dst_y = (uint16_t)(rect_height - 1 - y);
    for (x = 0; x < rect_width; x++) {
      buf[dst_y * rect_width + x] = fill_color;
    }
  }
}

static void lcd_compose_bar_rect(uint16_t rect_width, uint16_t rect_height,
                                 uint8_t fill_pct, uint16_t fill_color,
                                 uint16_t bg_color, uint16_t* buf) {
  uint32_t total_pixels = (uint32_t)rect_width * rect_height;
  uint16_t fill_width;
  uint16_t x;
  uint16_t y;
  uint32_t i;

  if (fill_pct > 100) {
    fill_pct = 100;
  }

  fill_width = (uint16_t)(((uint32_t)rect_width * fill_pct) / 100U);

  for (i = 0; i < total_pixels; i++) {
    buf[i] = bg_color;
  }

  for (y = 0; y < rect_height; y++) {
    for (x = 0; x < fill_width; x++) {
      buf[y * rect_width + x] = fill_color;
    }
  }
}

static void lcd_draw_lmh_tire_temp_cell(uint16_t x, uint16_t y,
                                        uint16_t width, uint16_t height,
                                        int temp) {
  lcd_compose_bold_2digit_rect(width, height, temp, SIMHEI_DIGIT_SMALL_WIDTH,
                               SIMHEI_DIGIT_SMALL_HEIGHT, 0,
                               g_lmh_tire_color, g_lmh_tire_bg,
                               g_lmh_rect_buf);
  lcd_blit_rgb565_rect(x, y, width, height, g_lmh_rect_buf);
}

void lcd_draw_lmh_center_values(int gear, int speed) {
  lcd_draw_lmh_rpm(0);
  lcd_draw_lmh_gear(gear);
  lcd_draw_lmh_speed(speed);
}

void dashboard_init_screen(void) {
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
}

void dashboard_update(const uart_telemetry_t *telemetry,
                      dashboard_view_state_t *state) {
  if ((telemetry == NULL) || (state == NULL)) {
    return;
  }

  if ((int)telemetry->gear != state->gear) {
    lcd_draw_lmh_gear(telemetry->gear);
    state->gear = telemetry->gear;
  }

  if ((int)telemetry->rpm_pct_x10 != state->rpm_pct_x10) {
    lcd_draw_lmh_shift_lights(telemetry->rpm_pct_x10);
    state->rpm_pct_x10 = telemetry->rpm_pct_x10;
  }

  if ((int)telemetry->rpm != state->rpm) {
    lcd_draw_lmh_rpm(telemetry->rpm);
    state->rpm = telemetry->rpm;
  }

  if ((int)telemetry->speed != state->speed) {
    lcd_draw_lmh_speed(telemetry->speed);
    state->speed = telemetry->speed;
  }

  if (((int)telemetry->brake_temp_fl != state->brake_temp_fl) ||
      ((int)telemetry->brake_temp_fr != state->brake_temp_fr) ||
      ((int)telemetry->brake_temp_rl != state->brake_temp_rl) ||
      ((int)telemetry->brake_temp_rr != state->brake_temp_rr)) {
    lcd_draw_lmh_brake_temps(
        telemetry->brake_temp_fl, telemetry->brake_temp_fr,
        telemetry->brake_temp_rl, telemetry->brake_temp_rr);
    state->brake_temp_fl = telemetry->brake_temp_fl;
    state->brake_temp_fr = telemetry->brake_temp_fr;
    state->brake_temp_rl = telemetry->brake_temp_rl;
    state->brake_temp_rr = telemetry->brake_temp_rr;
  }

  if (((int)telemetry->tire_temp_fl != state->tire_temp_fl) ||
      ((int)telemetry->tire_temp_fr != state->tire_temp_fr) ||
      ((int)telemetry->tire_temp_rl != state->tire_temp_rl) ||
      ((int)telemetry->tire_temp_rr != state->tire_temp_rr)) {
    lcd_draw_lmh_tire_temps(telemetry->tire_temp_fl, telemetry->tire_temp_fr,
                            telemetry->tire_temp_rl, telemetry->tire_temp_rr);
    state->tire_temp_fl = telemetry->tire_temp_fl;
    state->tire_temp_fr = telemetry->tire_temp_fr;
    state->tire_temp_rl = telemetry->tire_temp_rl;
    state->tire_temp_rr = telemetry->tire_temp_rr;
  }

  if (((int)telemetry->water_temp != state->water_temp) ||
      ((int)telemetry->oil_temp != state->oil_temp)) {
    lcd_draw_lmh_engine_temps(telemetry->water_temp, telemetry->oil_temp);
    state->water_temp = telemetry->water_temp;
    state->oil_temp = telemetry->oil_temp;
  }

  if (((long)telemetry->best_lap_ms != state->best_lap_ms) ||
      ((long)telemetry->current_lap_ms != state->current_lap_ms)) {
    lcd_draw_lmh_lap_times(telemetry->current_lap_ms, telemetry->best_lap_ms);
    state->best_lap_ms = telemetry->best_lap_ms;
    state->current_lap_ms = telemetry->current_lap_ms;
  }

  if (((int)telemetry->fuel_liters != state->fuel_liters) ||
      ((int)telemetry->fuel_pct != state->fuel_pct)) {
    lcd_draw_lmh_fuel_status(telemetry->fuel_liters, telemetry->fuel_pct);
    state->fuel_liters = telemetry->fuel_liters;
    state->fuel_pct = telemetry->fuel_pct;
  }

  if (((int)telemetry->throttle_pct != state->throttle_pct) ||
      ((int)telemetry->brake_pct != state->brake_pct)) {
    lcd_draw_lmh_pedals(telemetry->throttle_pct, telemetry->brake_pct);
    state->throttle_pct = telemetry->throttle_pct;
    state->brake_pct = telemetry->brake_pct;
  }
}

void lcd_draw_lmh_rpm(int rpm) {
  lcd_compose_bold_int_rect(104, 24, rpm, 13, 19, 0, g_lmh_rpm_color,
                            g_lmh_center_bg, g_lmh_rect_buf);
  lcd_blit_rgb565_rect(108, 32, 104, 24, g_lmh_rect_buf);
}

void lcd_draw_lmh_gear(int gear) {
  if (gear == 0) {
    lcd_compose_bold_char_rect(104, 72, 'N', 44, 64, g_lmh_gear_color,
                               g_lmh_center_bg, g_lmh_rect_buf);
  } else if ((gear < 0) || (gear == 255)) {
    lcd_compose_bold_char_rect(104, 72, 'R', 44, 64, g_lmh_gear_color,
                               g_lmh_center_bg, g_lmh_rect_buf);
  } else {
    lcd_compose_bold_int_rect(104, 72, gear, 44, 64, 0, g_lmh_gear_color,
                              g_lmh_center_bg, g_lmh_rect_buf);
  }
  lcd_blit_rgb565_rect(108, 64, 104, 72, g_lmh_rect_buf);
}

void lcd_draw_lmh_speed(int speed) {
  lcd_compose_bold_int_rect(104, 24, speed, 13, 19, 0, g_lmh_speed_color,
                            g_lmh_center_bg, g_lmh_rect_buf);
  lcd_blit_rgb565_rect(108, 144, 104, 24, g_lmh_rect_buf);
}

void lcd_draw_lmh_shift_lights(uint16_t rpm_pct_x10) {
  uint16_t region_x = 0;
  uint16_t region_y = 0;
  uint16_t region_w = 320;
  uint16_t region_h = 24;
  uint16_t led_w = 24;
  uint16_t led_h = 10;
  uint16_t gap = 6;
  uint16_t strip_w = (uint16_t)(10 * led_w + 9 * gap);
  uint16_t start_x = (uint16_t)((region_w - strip_w) / 2);
  uint16_t start_y = (uint16_t)((region_h - led_h) / 2);
  uint32_t i;
  uint8_t led;

  for (i = 0; i < ((uint32_t)region_w * region_h); i++) {
    g_lmh_rect_buf[i] = BLACK;
  }

  for (led = 0; led < 10; led++) {
    uint16_t led_x = (uint16_t)(start_x + led * (led_w + gap));
    uint16_t color = lcd_get_shift_light_color(led, rpm_pct_x10);
    lcd_fill_round_bar_to_buf(g_lmh_rect_buf, region_w, region_h, led_x,
                              start_y, led_w, led_h, color);
  }

  lcd_blit_rgb565_rect(region_x, region_y, region_w, region_h, g_lmh_rect_buf);
}

void lcd_draw_lmh_engine_temps(int water_temp, int oil_temp) {
  uint16_t number_y = 195;

  lcd_compose_bold_int_rect(32, 20, water_temp, SIMHEI_DIGIT_SMALL_WIDTH,
                            SIMHEI_DIGIT_SMALL_HEIGHT, 0, WHITE, BLACK,
                            g_lmh_rect_buf);
  lcd_blit_rgb565_rect(8, number_y, 32, 20, g_lmh_rect_buf);

  lcd_compose_bold_int_rect(32, 20, oil_temp, SIMHEI_DIGIT_SMALL_WIDTH,
                            SIMHEI_DIGIT_SMALL_HEIGHT, 0, WHITE, BLACK,
                            g_lmh_rect_buf);
  lcd_blit_rgb565_rect(40, number_y, 32, 20, g_lmh_rect_buf);
}

void lcd_draw_lmh_lap_times(uint32_t current_lap_ms, uint32_t best_lap_ms) {
  char best_buf[12];
  char current_buf[12];
  uint16_t region_x = 216;
  uint16_t region_width = 100;
  uint16_t time_height = 16;
  uint16_t best_time_y = 122;
  uint16_t current_time_y = 152;
  uint32_t i;

  lcd_format_lap_time(best_lap_ms, best_buf);
  lcd_format_lap_time(current_lap_ms, current_buf);

  for (i = 0; i < ((uint32_t)region_width * time_height); i++) {
    g_lmh_rect_buf[i] = g_lmh_lap_bg;
  }
  lcd_draw_string_to_buf(
      g_lmh_rect_buf, region_width, time_height,
      (uint16_t)(lcd_center_text_x(0, region_width, best_buf, 16)), 0, best_buf,
      16, WHITE);
  lcd_blit_rgb565_rect(region_x, best_time_y, region_width, time_height,
                       g_lmh_rect_buf);

  for (i = 0; i < ((uint32_t)region_width * time_height); i++) {
    g_lmh_rect_buf[i] = g_lmh_lap_bg;
  }
  lcd_draw_string_to_buf(
      g_lmh_rect_buf, region_width, time_height,
      (uint16_t)(lcd_center_text_x(0, region_width, current_buf, 16)), 0,
      current_buf, 16, WHITE);
  lcd_blit_rgb565_rect(region_x, current_time_y, region_width, time_height,
                       g_lmh_rect_buf);
}

void lcd_draw_lmh_fuel_status(uint8_t fuel_liters, uint8_t fuel_pct) {
  char pct_buf[8];
  uint8_t pct_len;
  uint16_t liters_x = 240;
  uint16_t liters_y = 190;
  uint16_t liters_w = 52;
  uint16_t liters_h = 20;
  uint16_t bar_width = 10;
  uint16_t bar_height = 34;
  uint16_t bar_x = 293;
  uint16_t bar_y = 184;
  uint16_t pct_x = 289;
  uint16_t pct_y = 220;
  uint16_t pct_w = 31;
  uint16_t pct_h = 16;
  uint32_t i;

  sprintf(pct_buf, "%u", fuel_pct);
  pct_len = lcd_ascii_strlen(pct_buf);

  lcd_compose_bold_fuel_rect(liters_w, liters_h, fuel_liters, WHITE, BLACK,
                             g_lmh_rect_buf);
  lcd_blit_rgb565_rect(liters_x, liters_y, liters_w, liters_h, g_lmh_rect_buf);

  lcd_compose_vertical_bar_rect(bar_width, bar_height, fuel_pct,
                                g_lmh_fuel_bar_color, g_lmh_bar_bg,
                                g_lmh_rect_buf);
  lcd_blit_rgb565_rect(bar_x, bar_y, bar_width, bar_height, g_lmh_rect_buf);

  for (i = 0; i < ((uint32_t)pct_w * pct_h); i++) {
    g_lmh_rect_buf[i] = BLACK;
  }
  lcd_draw_string_to_buf(g_lmh_rect_buf, pct_w, pct_h, 0, 0, pct_buf, 16,
                         WHITE);
  lcd_draw_string_to_buf(g_lmh_rect_buf, pct_w, pct_h,
                         (uint16_t)(pct_len * (16 / 2) + 2), 0, "%", 16,
                         WHITE);
  lcd_blit_rgb565_rect(pct_x, pct_y, pct_w, pct_h, g_lmh_rect_buf);
}

void lcd_draw_lmh_pedals(int throttle_pct, int brake_pct) {
  lcd_compose_bar_rect(130, 14, (uint8_t)brake_pct, g_lmh_brake_color,
                       g_lmh_bar_bg, g_lmh_rect_buf);
  lcd_blit_rgb565_rect(95, 192, 130, 14, g_lmh_rect_buf);

  lcd_compose_bar_rect(130, 14, (uint8_t)throttle_pct, g_lmh_throttle_color,
                       g_lmh_bar_bg, g_lmh_rect_buf);
  lcd_blit_rgb565_rect(95, 214, 130, 14, g_lmh_rect_buf);
}

void lcd_draw_lmh_brake_temps(int fl, int fr, int rl, int rr) {
  lcd_draw_lmh_brake_temp_cell(6, 29, 41, 35, fl);
  lcd_draw_lmh_brake_temp_cell(61, 29, 41, 35, fr);
  lcd_draw_lmh_brake_temp_cell(6, 67, 41, 34, rl);
  lcd_draw_lmh_brake_temp_cell(61, 67, 41, 34, rr);
}

void lcd_draw_lmh_tire_temps(int fl, int fr, int rl, int rr) {
  lcd_draw_lmh_tire_temp_cell(218, 29, 42, 35, fl);
  lcd_draw_lmh_tire_temp_cell(274, 29, 40, 35, fr);
  lcd_draw_lmh_tire_temp_cell(218, 67, 42, 34, rl);
  lcd_draw_lmh_tire_temp_cell(274, 67, 40, 34, rr);
}

static void lcd_draw_mask_bitmap_full(uint16_t x, uint16_t y,
                                      uint16_t mask_width, uint16_t mask_height,
                                      const unsigned char* mask,
                                      uint16_t fg_color, uint16_t bg_color) {
  uint32_t total = (uint32_t)mask_width * mask_height;
  uint32_t i;

  lcd_set_window(x, y, mask_width, mask_height);
  lcd_write_ram_prepare();

  for (i = 0; i < total; i++) {
    LCD->LCD_RAM = mask[i] ? fg_color : bg_color;
  }
}
static uint8_t lcd_bold_int_strlen(const char* buf) {
  uint8_t len = 0;
  while ((buf[len] <= '9') && (buf[len] >= '0')) {
    len++;
  }
  return len;
}
static void lcd_bold_int_to_str(int32_t num, char* buf) {
  uint32_t value;
  uint8_t len = 0;
  uint8_t i;

  if (num < 0) {
    value = (uint32_t)(-num);
  } else {
    value = (uint32_t)num;
  }

  if (value == 0) {
    buf[0] = '0';
    buf[1] = '\0';
    return;
  }

  while (value > 0) {
    buf[len++] = (value % 10) + '0';
    value /= 10;
  }

  for (i = 0; i < len / 2; i++) {
    char temp = buf[i];
    buf[i] = buf[len - 1 - i];
    buf[len - 1 - i] = temp;
  }

  buf[len] = '\0';
}

void lcd_draw_bold_digit(uint16_t x, uint16_t y, char digit, uint16_t width,
                         uint16_t height, uint16_t color) {
  uint8_t index;

  if ((digit < '0') || (digit > '9')) {
    return;
  }

  index = digit - '0';

  if ((width == SIMHEI_DIGIT_LARGE_WIDTH) &&
      (height == SIMHEI_DIGIT_LARGE_HEIGHT)) {
    lcd_draw_mask_bitmap_full(
        x, y, SIMHEI_DIGIT_LARGE_WIDTH, SIMHEI_DIGIT_LARGE_HEIGHT,
        g_simhei_digits_large_mask[index], color, g_lmh_center_bg);
  } else if ((width == SIMHEI_DIGIT_SMALL_WIDTH) &&
             (height == SIMHEI_DIGIT_SMALL_HEIGHT)) {
    lcd_draw_mask_bitmap_full(
        x, y, SIMHEI_DIGIT_SMALL_WIDTH, SIMHEI_DIGIT_SMALL_HEIGHT,
        g_simhei_digits_small_mask[index], color, g_lmh_center_bg);
  }
}

void lcd_draw_bold_number(uint16_t x, uint16_t y, char* str,
                          uint16_t digit_width, uint16_t digit_height,
                          uint16_t spacing, uint16_t color) {
  while ((*str <= '9') && (*str >= '0')) {
    lcd_draw_bold_digit(x, y, *str, digit_width, digit_height, color);
    x += digit_width + spacing;
    str++;
  }
}
void lcd_draw_bold_int(uint16_t x, uint16_t y, int32_t num,
                       uint16_t digit_width, uint16_t digit_height,
                       uint16_t spacing, uint16_t color) {
  char buf[12];
  lcd_bold_int_to_str(num, buf);
  lcd_draw_bold_number(x, y, buf, digit_width, digit_height, spacing, color);
}

void lcd_draw_bold_int_in_rect(uint16_t x, uint16_t y, uint16_t rect_width,
                               uint16_t rect_height, int32_t num,
                               uint16_t digit_width, uint16_t digit_height,
                               uint16_t spacing, uint16_t color) {
  char buf[12];
  uint8_t len = 0;
  uint16_t total_width;
  uint16_t draw_x = x;
  uint16_t draw_y = y;

  lcd_bold_int_to_str(num, buf);
  while ((buf[len] <= '9') && (buf[len] >= '0')) {
    len++;
  }

  total_width = len * digit_width;
  if (len > 1) {
    total_width += (len - 1) * spacing;
  }

  if (rect_width > total_width) {
    draw_x = x + (rect_width - total_width) / 2;
  }

  if (rect_height > digit_height) {
    draw_y = y + (rect_height - digit_height) / 2;
  }

  lcd_draw_bold_number(draw_x, draw_y, buf, digit_width, digit_height, spacing,
                       color);
}

