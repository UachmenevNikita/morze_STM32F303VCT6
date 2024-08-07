#ifndef __LCD_ILI9325_H
#define __LCD_ILI9325_H

#include <stdint.h>

#define LCD_WIDTH 240
#define LCD_HEIGTH 320

void init_lcd_ili9325(void);

void lcd_fill_region(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t *colors_data);
void lcd_fill_color(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color);
void lcd_set_pixel(int16_t x, int16_t y, uint16_t color);

#define        LCD_RGB565_CONVERT_M(red, green, blue) (((red >> 3) << 11) | ((green >> 2) << 5) | (blue >> 3))

// Some color constants
#define LCD_COLOR_BLACK 	LCD_RGB565_CONVERT_M(0x00, 0x00, 0x00)
#define LCD_COLOR_MAROON    LCD_RGB565_CONVERT_M(0x80, 0x00, 0x00)
#define LCD_COLOR_GREEN     LCD_RGB565_CONVERT_M(0x00, 0x80, 0x00)
#define LCD_COLOR_OLIVE     LCD_RGB565_CONVERT_M(0x80, 0x80, 0x00)
#define LCD_COLOR_NAVY      LCD_RGB565_CONVERT_M(0x00, 0x00, 0x80)
#define LCD_COLOR_PURPLE    LCD_RGB565_CONVERT_M(0x80, 0x00, 0x80)
#define LCD_COLOR_TEAL      LCD_RGB565_CONVERT_M(0x00, 0x80, 0x80)
#define LCD_COLOR_SILVER    LCD_RGB565_CONVERT_M(0xC0, 0xC0, 0xC0)
#define LCD_COLOR_GRAY      LCD_RGB565_CONVERT_M(0x80, 0x80, 0x80)
#define LCD_COLOR_RED       LCD_RGB565_CONVERT_M(0xFF, 0x00, 0x00)
#define LCD_COLOR_LIME      LCD_RGB565_CONVERT_M(0xFF, 0xFF, 0x00)
#define LCD_COLOR_BLUE      LCD_RGB565_CONVERT_M(0x00, 0x00, 0xFF)
#define LCD_COLOR_FUCHSIA   LCD_RGB565_CONVERT_M(0xFF, 0x00, 0xFF)
#define LCD_COLOR_AQUA      LCD_RGB565_CONVERT_M(0x00, 0xFF, 0xFF)
#define LCD_COLOR_WHITE     LCD_RGB565_CONVERT_M(0xFF, 0xFF, 0xFF)

typedef  struct {
	char cnamecolor[16];
	uint16_t uwcolor;
} Tcolor;

extern const Tcolor aEGAcolr[16];
#endif
