/*timers.h*/
#ifndef TIMERS_h
#define TIMERS_h

#define MAX_TIMERS 6 //максимальное количество таймеров
//в этом разделе объявляются константы, служащие идентификаторами таймеров.
/* таймеры keypad */
#define KEYB_TIMER  0 // таймер удержания кнопок
#define KPD4_TIMER  1 // период опроса состояния кнопок
/* таймер дисплея */
#define DISPUPDT_TIMER  2 // таймер обновления экрана

#define PLAYTIMER  3
#define SOUNDTIMER  4
#define ENTERTIMER  5

//функции работы с таймерами
void ProcessTimers(void);
void InitTimers(void);
unsigned int GetTimer(char Timer);
void ResetTimer(char Timer);

#endif
