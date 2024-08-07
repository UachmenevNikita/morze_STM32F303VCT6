/*keypad.h*/

#include "stdint.h"

#ifndef KEYPAD_H
#define KEYPAD_H

void InitKpd4(void);
void ProcessKeyKpd4(void);

uint16_t getKeycode();

#endif
