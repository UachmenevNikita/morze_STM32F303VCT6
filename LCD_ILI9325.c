/*LCD_ILI9325.c*/
#include "LCD_ILI9325.h"
#include "main.h"


const Tcolor aEGAcolr[16]= { 
	{"BLACK",			LCD_RGB565_CONVERT_M(0x00, 0x00, 0x00)},// 0 - чёрный
	{"BLUE",			LCD_RGB565_CONVERT_M(0x00, 0x00, 0x80)},// 1 - синий
	{"GREEN",			LCD_RGB565_CONVERT_M(0x00, 0x80, 0x00)},// 2 - зелёный
	{"CYAN",			LCD_RGB565_CONVERT_M(0x00, 0x80, 0x80)},// 3 - синезелёный TEAL
	{"RED", 			LCD_RGB565_CONVERT_M(0x80, 0x00, 0x00)},// 4 - красный
	{"MAGENTA", 		LCD_RGB565_CONVERT_M(0x80, 0x00, 0x80)},// 5 - пурпурный PURPLE
	{"BROWN", 			LCD_RGB565_CONVERT_M(0x80, 0x40, 0x00)},// 6 - коричневый	 MAROON
	{"LIGHT GRAY", 		LCD_RGB565_CONVERT_M(0x80, 0x80, 0x80)},// 7 - светло-серый	 SILVER
	{"DARK GRAY", 		LCD_RGB565_CONVERT_M(0x40, 0x40, 0x40)},// 8 - тёмно-серый	 
	{"BRIGHT BLUE", 	LCD_RGB565_CONVERT_M(0x40, 0x40, 0xFF)},// 9 - голубой	 
	{"BRIGHT GREEN", 	LCD_RGB565_CONVERT_M(0x40, 0xFF, 0x40)},// 10 - ярко-зелёный	 
	{"BRIGHT CYAN", 	LCD_RGB565_CONVERT_M(0x40, 0xFF, 0xFF)},// 11 - яркий сине-зелёный	 
	{"BRIGHT RED", 		LCD_RGB565_CONVERT_M(0xFF, 0x40, 0x40)},// 12 - ярко-красный	 
	{"BRIGHT MAGENTA", 	LCD_RGB565_CONVERT_M(0xFF, 0x40, 0xFF)},// 13 - ярко-пурпурный	 
	{"BRIGHT YELLOW", 	LCD_RGB565_CONVERT_M(0xFF, 0xFF, 0x40)},// 14 - жёлтый	 
	{"BRIGHT WHITE", 	LCD_RGB565_CONVERT_M(0xFF, 0xFF, 0xFF)},// 15 - ярко-белый	 
};

static void gpio_lcd_init(void);
static void board_lcd_reset(void);
static void bus_lcd_write_reg(uint16_t address, uint16_t value);
static void bus_lcd_read_reg(uint16_t address, uint16_t *value);
static void lcd_bus_test(void);

/*LCD pins
                    1   2       GND
LCD_D0      PD0     3   4       PD1      LCD_D1
LCD_D2      PD2     5   6       PD3      LCD_D3
LCD_D4      PD4     7   8       PD5      LCD_D5
LCD_D6      PD6     9   10      PD7      LCD_D7
LCD_D8      PD8     11  12      PD9      LCD_D9
LCD_D10     PD10    13  14      PD11     LCD_D11
LCD_D12     PD12    15  16      PD13     LCD_D13
LCD_D14     PD14    17  18      PD15     LCD_D15

LCD_CS      PB8     19  20      PB9      LCD_RS
LCD_WR      PB10    21  22      PB11     LCD_RD
LCD_RESET   PC11    23  24

BL_VDD5V    VDD     25  26      GND      BL_GND
BL_PWM      PC10    27  28      PB1      LCDTP_IRQ
LCDTP_CS    PC13    29  30      PA5      LCDTP_CLK
LCDTP_DIN   PA7     31  32      PA6      LCDTP_DOUT
            VDD     33  34      GND
*/

#define LCD_RESET_Pin 		LL_GPIO_PIN_11
#define LCD_RESET_GPIO_Port GPIOC
#define LCD_BL_PWM_Pin 		LL_GPIO_PIN_10
#define LCD_BL_PWM_GPIO_Port GPIOC

#define LCD_CONTROLBUS_PORT 	GPIOB
#define LCD_DATABUS_PORT 		GPIOD
// capture bus LCD: reset #CS
#define LCD_CAPTURE_BUS LCD_CONTROLBUS_PORT->BRR = LL_GPIO_PIN_8
// release bus LCD: set #CS
#define LCD_RELEASE_BUS LCD_CONTROLBUS_PORT->BSRR = LL_GPIO_PIN_8
// switch to index mode
#define LCD_CLEAR_RS LCD_CONTROLBUS_PORT->BRR = LL_GPIO_PIN_9
// switch to data mode
#define LCD_SET_RS LCD_CONTROLBUS_PORT->BSRR = LL_GPIO_PIN_9
// start write signal
#define LCD_CLEAR_WR LCD_CONTROLBUS_PORT->BRR = LL_GPIO_PIN_10
// stop write signal
#define LCD_SET_WR LCD_CONTROLBUS_PORT->BSRR = LL_GPIO_PIN_10
// start read signal
#define LCD_CLEAR_RD LCD_CONTROLBUS_PORT->BRR = LL_GPIO_PIN_11
// stop read signal
#define LCD_SET_RD LCD_CONTROLBUS_PORT->BSRR = LL_GPIO_PIN_11

// helper register addresses
#define LCD_REGISTER_GRAM_X_ADDR 		0x0020
#define LCD_REGISTER_GRAM_Y_ADDR 		0x0021
#define LCD_REGISTER_GRAM_WRITE_DATA 	0x0022

#define LCD_REGISTER_GRAM_X_ADDR_START 	0x0050
#define LCD_REGISTER_GRAM_X_ADDR_END 	0x0051
#define LCD_REGISTER_GRAM_Y_ADDR_START 	0x0052
#define LCD_REGISTER_GRAM_Y_ADDR_END 	0x0053

static void board_lcd_reset()
{
    // reset LCD
	LL_GPIO_ResetOutputPin(LCD_RESET_GPIO_Port, LCD_RESET_Pin);
	LL_mDelay(50);//50ms
	LL_GPIO_SetOutputPin(LCD_RESET_GPIO_Port, LCD_RESET_Pin);
}

static void bus_lcd_write_reg(uint16_t address, uint16_t value)
{
    // write register address
    LCD_CLEAR_RS; //устанавливаем низкий уровень на линии LCD_RS, т.е. на шине данных будет выставлен адресс управляющего регистра
    LCD_DATABUS_PORT->ODR = address; //устанавливаем данные на шину
    LCD_CLEAR_WR; //устанавливаем низкий уровень на линии LCD_WR, т.е. данные на шине записываются в контроллёр, LCD_RD - высокий уровень
    __NOP(); 
    LCD_SET_WR;	//устанавливаем высокий уровень на линии LCD_WR, по изменению уровня контроллер считал данные
    LCD_SET_RS; //устанавливаем высокий уровень на линии LCD_RS, т.е. на шине данных будет выставлены данные
    // write register data
    LCD_DATABUS_PORT->ODR = value;
    LCD_CLEAR_WR;//устанавливаем низкий уровень на линии LCD_WR, т.е. данные на шине записываются в контроллёр, LCD_RD - высокий уровень
    __NOP(); 
    LCD_SET_WR;
}

static void board_lcd_write_words(uint16_t address, uint16_t *data, size_t size)
{
    uint16_t* data_end;
    // write register address
    LCD_CLEAR_RS;
    LCD_DATABUS_PORT->ODR = address;
    LCD_CLEAR_WR;
    __NOP(); 
    LCD_SET_WR;
    LCD_SET_RS;
    // write register data
    data_end = data + size;
    while (data != data_end) {
        LCD_DATABUS_PORT->ODR = *data;
        LCD_CLEAR_WR;
         __NOP(); 
        LCD_SET_WR;
        data++;
    }
}

static void bus_lcd_read_reg(uint16_t address, uint16_t *value)
{
    uint32_t data_port_mode;
    volatile uint32_t i;
    // write register address
    LCD_CLEAR_RS;
    LCD_DATABUS_PORT->ODR = address;
    LCD_CLEAR_WR;
    __NOP();
    LCD_SET_WR;
    LCD_SET_RS;
    // read register
    // reconfigure data port to read mode
    data_port_mode = LCD_DATABUS_PORT->MODER;
    LCD_DATABUS_PORT->MODER = LL_GPIO_MODE_INPUT;
    // read data
    LCD_CLEAR_RD;
    for(i=0;i<3;i++) // ждём 0,2 мксек 
        __NOP();
    *value = LCD_DATABUS_PORT->IDR;
    LCD_SET_RD;
	// restore gpio mode
    LCD_DATABUS_PORT->MODER = data_port_mode;
}

static void lcd_bus_test(void)
{
    uint16_t i;
    uint16_t act_val = 0;
    for (i = 0xEE; i < 0xF1; i++) {
        bus_lcd_write_reg(0x0020, i);
        bus_lcd_read_reg(0x0020, &act_val);
        if (act_val != i) {
            /* Initialization Error */
			while (1){}
        }
    }
	bus_lcd_write_reg(0x0020, 0x00);
}

static void gpio_lcd_init(void)
{
	LL_GPIO_InitTypeDef gpio_initstruct = {0};
	/* GPIO Ports Clock Enable */
	LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOD);
	LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB);
	LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOC);
	/*Configure GPIO pin lcd bus control 
	PB.10(WR), PB.11(RD) , PB.08(CS), PB.09(RS)*/
	LL_GPIO_SetOutputPin(LCD_CONTROLBUS_PORT, 	 LL_GPIO_PIN_8 | LL_GPIO_PIN_9 
											|LL_GPIO_PIN_10 | LL_GPIO_PIN_11);
	gpio_initstruct.Pin = 	 LL_GPIO_PIN_8 | LL_GPIO_PIN_9 
							|LL_GPIO_PIN_10 | LL_GPIO_PIN_11;
	gpio_initstruct.Mode = LL_GPIO_MODE_OUTPUT;
	gpio_initstruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
	gpio_initstruct.Pull = LL_GPIO_PULL_NO;
	gpio_initstruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
	if (LL_GPIO_Init(LCD_CONTROLBUS_PORT, &gpio_initstruct) != SUCCESS)
	{
		/* Initialization Error */
		while (1){}
	}
	/*Configure GPIO pin data bus 
	PD.00(D0), PD.01(D1), PD.02(D2), PD.03(D3), 
	PD.04(D4), PD.05(D5), PD.06(D6), PD.07(D7), 
	PD.08(D8), PD.09(D9), PD.10(D10), PD.11(D11),
	PD.12(D12), PD.13(D13), PD.14(D14), PD.15(D15)*/
	LL_GPIO_SetOutputPin(LCD_DATABUS_PORT, LL_GPIO_PIN_ALL);
	gpio_initstruct.Pin = LL_GPIO_PIN_ALL;
	gpio_initstruct.Mode = LL_GPIO_MODE_OUTPUT;
	gpio_initstruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
	gpio_initstruct.Pull = LL_GPIO_PULL_NO;
	gpio_initstruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
 	if (LL_GPIO_Init(LCD_DATABUS_PORT, &gpio_initstruct) != SUCCESS)
	{
		/* Initialization Error */
		while (1){}
	}
	/*Configure GPIO pin reset, backlight control PWM: PC.11(RST)  PC.10(BL_PWM) */
	gpio_initstruct.Pin =  LL_GPIO_PIN_11 		| LL_GPIO_PIN_10;
	gpio_initstruct.Mode = LL_GPIO_MODE_OUTPUT;
	gpio_initstruct.OutputType = LL_GPIO_OUTPUT_OPENDRAIN;
	gpio_initstruct.Pull = LL_GPIO_PULL_UP;
	gpio_initstruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
	if (LL_GPIO_Init(GPIOC, &gpio_initstruct) != SUCCESS)
	{
		/* Initialization Error */
		while (1){}
	}
	LL_GPIO_SetOutputPin(LCD_BL_PWM_GPIO_Port,LCD_BL_PWM_Pin);//Максимальная яркость
}

void init_lcd_ili9325(void)
{
    uint16_t device_code = 0;
	gpio_lcd_init();
	// reset LCD
    board_lcd_reset();
    // read device code
	LCD_CAPTURE_BUS;
    bus_lcd_read_reg(0x0000, &device_code);
    if (device_code == 0x9325 || device_code == 0x9328) {
        // test write operation
		lcd_bus_test();
        bus_lcd_write_reg(0xE5, 0x78F0); /* set SRAM internal timing */
        bus_lcd_write_reg(0x01, 0x0100); /* set Driver Output Control */
        bus_lcd_write_reg(0x02, 0x0700); /* set 1 line inversion */
        bus_lcd_write_reg(0x03, 0x1030); /* set GRAM write direction and BGR=1 */
        bus_lcd_write_reg(0x04, 0x0000); /* Resize register */
        bus_lcd_write_reg(0x08, 0x0207); /* set the back porch and front porch */
        bus_lcd_write_reg(0x09, 0x0000); /* set non-display area refresh cycle ISC[3:0] */
        bus_lcd_write_reg(0x0A, 0x0000); /* FMARK function */
        bus_lcd_write_reg(0x0C, 0x0000); /* RGB interface setting */
        bus_lcd_write_reg(0x0D, 0x0000); /* Frame marker Position */
        bus_lcd_write_reg(0x0F, 0x0000); /* RGB interface polarity */
        /*************Power On sequence ****************/
        bus_lcd_write_reg(0x10, 0x0000); /* SAP, BT[3:0], AP, DSTB, SLP, STB */
        bus_lcd_write_reg(0x11, 0x0007); /* DC1[2:0], DC0[2:0], VC[2:0] */
        bus_lcd_write_reg(0x12, 0x0000); /* VREG1OUT voltage */
        bus_lcd_write_reg(0x13, 0x0000); /* VDV[4:0] for VCOM amplitude */
        bus_lcd_write_reg(0x07, 0x0001);
        LL_mDelay(50);/* Delay50ms */
        /* Dis-charge capacitor power voltage */
        bus_lcd_write_reg(0x10, 0x1090); /* SAP, BT[3:0], AP, DSTB, SLP, STB */
        bus_lcd_write_reg(0x11, 0x0227); /* Set DC1[2:0], DC0[2:0], VC[2:0] */
        LL_mDelay(50); /* Delay 50ms */
        bus_lcd_write_reg(0x12, 0x001F);
        LL_mDelay(50); /* Delay 50ms */
        bus_lcd_write_reg(0x13, 0x1500); /* VDV[4:0] for VCOM amplitude */
        bus_lcd_write_reg(0x29, 0x0027); /* 04 VCM[5:0] for VCOMH */
        bus_lcd_write_reg(0x2B, 0x000D); /* Set Frame Rate */
        LL_mDelay(50); /* Delay 50ms */
        bus_lcd_write_reg(0x20, 0x0000); /* GRAM horizontal Address */
        bus_lcd_write_reg(0x21, 0x0000); /* GRAM Vertical Address */
        /* ----------- Adjust the Gamma Curve ---------- */
        bus_lcd_write_reg(0x30, 0x0000);
        bus_lcd_write_reg(0x31, 0x0707);
        bus_lcd_write_reg(0x32, 0x0307);
        bus_lcd_write_reg(0x35, 0x0200);
        bus_lcd_write_reg(0x36, 0x0008);
        bus_lcd_write_reg(0x37, 0x0004);
        bus_lcd_write_reg(0x38, 0x0000);
        bus_lcd_write_reg(0x39, 0x0707);
        bus_lcd_write_reg(0x3C, 0x0002);
        bus_lcd_write_reg(0x3D, 0x1D04);
        /* ------------------ Set GRAM area --------------- */
        bus_lcd_write_reg(0x50, 0x0000); /* Horizontal GRAM Start Address */
        bus_lcd_write_reg(0x51, 0x00EF); /* Horizontal GRAM End Address */
        bus_lcd_write_reg(0x52, 0x0000); /* Vertical GRAM Start Address */
        bus_lcd_write_reg(0x53, 0x013F); /* Vertical GRAM Start Address */
        bus_lcd_write_reg(0x60, 0xA700); /* Gate Scan Line */
        bus_lcd_write_reg(0x61, 0x0001); /* NDL,VLE, REV */
        bus_lcd_write_reg(0x6A, 0x0000); /* set scrolling line */
        /* -------------- Partial Display Control --------- */
        bus_lcd_write_reg(0x80, 0x0000);
        bus_lcd_write_reg(0x81, 0x0000);
        bus_lcd_write_reg(0x82, 0x0000);
        bus_lcd_write_reg(0x83, 0x0000);
        bus_lcd_write_reg(0x84, 0x0000);
        bus_lcd_write_reg(0x85, 0x0000);
        /* -------------- Panel Control ------------------- */
        bus_lcd_write_reg(0x90, 0x0010);
        bus_lcd_write_reg(0x92, 0x0600);
        bus_lcd_write_reg(0x07, 0x0133); /* 262K color and display ON */
    }
	LCD_RELEASE_BUS;
	lcd_fill_color(0, 0, LCD_WIDTH,LCD_HEIGTH, LCD_COLOR_BLACK);
}

void lcd_fill_region(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t *colors_data)
{
    size_t total_pixes;
    // set window
    if (x1 > x2) {
        int16_t x_tmp = x1;
        x1 = x2;
        x2 = x_tmp;
    }
    if (y1 > y2) {
        int16_t y_tmp = y1;
        y1 = y2;
        y2 = y_tmp;
    }
	LCD_CAPTURE_BUS;
    // set gram coordinates
    bus_lcd_write_reg(LCD_REGISTER_GRAM_X_ADDR, x1);
    bus_lcd_write_reg(LCD_REGISTER_GRAM_Y_ADDR, y1);
    bus_lcd_write_reg(LCD_REGISTER_GRAM_X_ADDR_START, x1);
    bus_lcd_write_reg(LCD_REGISTER_GRAM_X_ADDR_END, x2 - 1);
    bus_lcd_write_reg(LCD_REGISTER_GRAM_Y_ADDR_START, y1);
    bus_lcd_write_reg(LCD_REGISTER_GRAM_Y_ADDR_END, y2 - 1);

    // write pixels
    total_pixes = (x2 - x1) * (y2 - y1);
    board_lcd_write_words(LCD_REGISTER_GRAM_WRITE_DATA, colors_data, total_pixes);
	LCD_RELEASE_BUS;
}

void  lcd_fill_color(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color)
{
    size_t i;
    size_t total_pixes;
    const size_t max_buff_size = 128;
    size_t buff_size;
    uint16_t buff[max_buff_size];
   
    // set window
    if (x1 > x2) {
        int16_t x_tmp = x1;
        x1 = x2;
        x2 = x_tmp;
    }
    if (y1 > y2) {
        int16_t y_tmp = y1;
        y1 = y2;
        y2 = y_tmp;
    }
    // set gram coordinates
	LCD_CAPTURE_BUS;
    bus_lcd_write_reg(LCD_REGISTER_GRAM_X_ADDR, x1);
    bus_lcd_write_reg(LCD_REGISTER_GRAM_Y_ADDR, y1);
    bus_lcd_write_reg(LCD_REGISTER_GRAM_X_ADDR_START, x1);
    bus_lcd_write_reg(LCD_REGISTER_GRAM_X_ADDR_END, x2 - 1);
    bus_lcd_write_reg(LCD_REGISTER_GRAM_Y_ADDR_START, y1);
    bus_lcd_write_reg(LCD_REGISTER_GRAM_Y_ADDR_END, y2 - 1);
    
    // prepare buffer
    total_pixes = (x2 - x1) * (y2 - y1);
    buff_size = total_pixes < max_buff_size ? total_pixes : max_buff_size;
    for (i = 0; i < buff_size; i++) {
        buff[i] = color;
    }

    i = total_pixes;
    while (i > 0) {
        size_t pixels_to_update = i >= buff_size ? buff_size : i;
        // write pixels
        board_lcd_write_words(LCD_REGISTER_GRAM_WRITE_DATA, buff, pixels_to_update);
        i -= pixels_to_update;
    }
	LCD_RELEASE_BUS;
}

void lcd_set_pixel(int16_t x, int16_t y, uint16_t color)
{
	LCD_CAPTURE_BUS;
    // set gram coordinates
    bus_lcd_write_reg(LCD_REGISTER_GRAM_X_ADDR, x);
    bus_lcd_write_reg(LCD_REGISTER_GRAM_Y_ADDR, y);
	bus_lcd_write_reg(LCD_REGISTER_GRAM_X_ADDR_START, x);
    bus_lcd_write_reg(LCD_REGISTER_GRAM_X_ADDR_END, x);
    bus_lcd_write_reg(LCD_REGISTER_GRAM_Y_ADDR_START, y);
    bus_lcd_write_reg(LCD_REGISTER_GRAM_Y_ADDR_END, y);
    // write pixel
    bus_lcd_write_reg(LCD_REGISTER_GRAM_WRITE_DATA, color);
    LCD_RELEASE_BUS;
}
