#include "main.h"
#include "timers.h"
#include "keypad.h"
#include "displayctrl.h"



void SystemClock_Config(void);

int main(void)
{
	SystemClock_Config();
	//InitMessages();	//инициализация механизма обработки сообщений
	InitTimers();	//инициализация таймеров
	InitKpd4();		//инициализация автомата Kpd4
    InitDisplay();  //инициализация автомата Kpd4
    LL_SYSTICK_EnableIT();

    while (1)
	{
        if(GetTimer(KPD4_TIMER)>=5) //каждые 5 мс
		{
			ProcessKeyKpd4(); //итерация автомата Kpd4
			ResetTimer(KPD4_TIMER);
		}
        ProcessDisplay();
	}
}

void SystemClock_Config(void)
{   // PLL (HSE) = SYSCLK = HCLK(Hz) = 72000000
	LL_FLASH_SetLatency(LL_FLASH_LATENCY_2); /* Set FLASH latency */ 
	LL_RCC_HSE_Enable(); /* Enable HSE and wait for activation*/
	while(LL_RCC_HSE_IsReady() != 1) {}
	LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_HSE_DIV_1, LL_RCC_PLL_MUL_9);
	LL_RCC_PLL_Enable();/* Main PLL configuration and activation */
	while(LL_RCC_PLL_IsReady() != 1) {}
	LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1); //AHB Prescaler=1
	LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);
	while(LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL){}
	LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_2); //APB1 Prescaler=2
	LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_1); //APB2 Prescaler=1
	LL_Init1msTick(72000000);
	LL_SetSystemCoreClock(72000000);
}
