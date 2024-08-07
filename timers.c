#include "timers.h"

static unsigned int v_timers[MAX_TIMERS]; //переменные «виртуальных» таймеров

void ProcessTimers(void) //обработчик прерывания таймера/счетчика
{ /*увеличиваем значение всех переменных-таймеров на 1*/
    int i;
	for(i = 0; i < MAX_TIMERS; i++)
		v_timers[i]++;
}

void InitTimers(void)
{
	int i;
	for(i = 0; i < MAX_TIMERS; i++)
		v_timers[i] = 0;
}

unsigned int GetTimer(char Timer)
{
	return v_timers[Timer];
}

void ResetTimer(char Timer)
{
	v_timers[Timer] = 0;
}

void SysTick_Handler(void)
{
	ProcessTimers();
}
