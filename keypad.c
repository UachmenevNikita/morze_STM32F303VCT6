/*keypad.c*/
#include "timers.h"
#include "keypad.h"
#include <stdio.h>
#include "main.h"     

#define DEBOUNCE    10  // задержка на дребезг 10 мс
#define FIRST_DELAY 500 // задержка первого удержания кнопки 500 мс
#define AUTO_REPEAT 300 // задержка повторного удержания 300 мс

/*состояния автомата*/
#define FSMST_IDLE      0 // ожидает нажатия на кнопку
#define FSMST_DEBOUNCE  1 // ожидание завершения дребезга при замыкании контактов 
#define FSMST_KEYDOWN   2 // фиксируется нажатие и формируется сообщение MSG_KEY_PRESSED
						  // если кнопка не нажата, возврат в состояние FSMST_IDLE
#define FSMST_KEYDOWNHOLD   3 // формируется сообщение MSG_KEY_PRESSED при удержании кнопки в течении времени FIRST_DELAY
							  // если кнопка отпущена, возврат в состояние FSMST_IDLE
#define FSMST_HOLDAUTO   4 // формируется сообщение MSG_KEY_PRESSED при удержании кнопки в течении времени AUTO_REPEAT
						   // если кнопка отпущена, возврат в состояние FSMST_IDLE*/
uint16_t scankeypad(void);

static char fsmstate;   // переменная состояния автомата
static uint16_t keycode;    // переменная состояния кнопок
static uint16_t keycodeOut = 0;


uint16_t getKeycode() {
    uint16_t temp = keycodeOut;
    keycodeOut = 0;
    return temp;
}


void ProcessKeyKpd4(void)
{
	uint16_t key_code_tmp=0;
	key_code_tmp = scankeypad();
	switch (fsmstate)
	{
		case FSMST_IDLE: //ни одна кнопка не нажата
			if (key_code_tmp > 0)
			{
				keycode = key_code_tmp;
				ResetTimer(KEYB_TIMER);
				fsmstate = FSMST_DEBOUNCE;
			}
		break;
		case FSMST_DEBOUNCE: //кнопка нажата ждём окончания переходного процесса
			if (GetTimer(KEYB_TIMER) > DEBOUNCE)
                fsmstate = FSMST_KEYDOWN;
		break;
		case FSMST_KEYDOWN: //если кнопка нажата, посылаем сообщение
			if ( key_code_tmp == keycode )
			{
				ResetTimer(KEYB_TIMER);
				//SendMessage(MSG_KEY_PRESSED, (void*)&keycode);
                //SendMessage(MSG_DISPUPDTKEY, (void*)&keycode);
                keycodeOut = keycode;
                
				fsmstate = FSMST_KEYDOWNHOLD;
			}
			else
				fsmstate = FSMST_IDLE;
		break;
		case FSMST_KEYDOWNHOLD: //если кнопка удерживается, посылаем сообщение
			if ( key_code_tmp == keycode )
			{
				if (GetTimer(KEYB_TIMER) >= FIRST_DELAY)
				{
					ResetTimer(KEYB_TIMER);
					//SendMessage(MSG_KEY_PRESSED, (void*)&keycode);
                    //SendMessage(MSG_DISPUPDTKEY, (void*)&keycode);
                    
                    keycodeOut = keycode;
                    
					fsmstate = FSMST_HOLDAUTO;
				}
			}
			else
				fsmstate = FSMST_IDLE;
		break;

		case FSMST_HOLDAUTO: //если кнопка удерживается, посылаем сообщение чаще
			if (key_code_tmp == keycode)
			{
				if (GetTimer(KEYB_TIMER) >= AUTO_REPEAT)
				{

					ResetTimer(KEYB_TIMER);
					//SendMessage(MSG_KEY_PRESSED, (void*)&keycode);
                    //SendMessage(MSG_DISPUPDTKEY, (void*)&keycode);
                    
                    keycodeOut = keycode;
				}
			}
			else
				fsmstate = FSMST_IDLE;
		break;
	}
}

uint16_t scankeypad(void)
{
	uint16_t key_code=0;
	uint8_t tmpbuf=0;
	int i;
	for(i=0; i<4; i++) 
	{
		LL_GPIO_ResetOutputPin(GPIOC,1<<(4+i));
		__NOP(); __NOP(); __NOP(); __NOP();
		tmpbuf= (uint8_t)LL_GPIO_ReadInputPort(GPIOC);
		tmpbuf= ~tmpbuf & 0x0F;
		key_code |=tmpbuf<<(4*i);
		LL_GPIO_SetOutputPin(GPIOC,1<<(4+i));
	}
	return key_code;
}

void InitKpd4(void) 
{
	fsmstate=FSMST_IDLE;
	LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOC);//GPIOC
	//PC0,PC1,PC2,PC3 In
	LL_GPIO_InitTypeDef gpio_initstruct;
	gpio_initstruct.Pin        = LL_GPIO_PIN_0|LL_GPIO_PIN_1|LL_GPIO_PIN_2|LL_GPIO_PIN_3;
	gpio_initstruct.Mode       = LL_GPIO_MODE_INPUT;
	gpio_initstruct.Pull       = LL_GPIO_PULL_UP;
	if (LL_GPIO_Init(GPIOC, &gpio_initstruct) != SUCCESS)
		while (1){} 		/* Initialization Error */
	
	//PC4,PC5,PC6,PC7 Out
	gpio_initstruct.Pin        = LL_GPIO_PIN_4|LL_GPIO_PIN_5|LL_GPIO_PIN_6|LL_GPIO_PIN_7;
	gpio_initstruct.Mode       = LL_GPIO_MODE_OUTPUT;
	gpio_initstruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
	gpio_initstruct.Pull       = LL_GPIO_PULL_UP;
	if (LL_GPIO_Init(GPIOC, &gpio_initstruct) != SUCCESS)
		while (1){} 		/* Initialization Error */
	LL_GPIO_SetOutputPin (GPIOC, LL_GPIO_PIN_4|LL_GPIO_PIN_5|LL_GPIO_PIN_6|LL_GPIO_PIN_7);
}
