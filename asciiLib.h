#ifndef __AsciiLib_H
#define __AsciiLib_H  
#include <stdint.h>
//шрифты LCD
void GetASCIICode(unsigned char* pBuffer,unsigned char ASCII);
void LCDprintstr(char* str, int16_t y,uint16_t bkcolor, uint16_t charcolor);
#endif
