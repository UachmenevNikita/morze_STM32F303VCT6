/* displayctrl.c */
#include "timers.h"
#include "main.h"   
#include "AsciiLib.h"
#include "LCD_ILI9325.h"
#include "displayctrl.h"
#include "keypad.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define SOUND_PITCH 4

void dispkypd4(uint16_t kypd4status);

 ////////////////////////////////////////////////////////////////////////////////
 
 /* reverse:  переворачиваем строку s на месте */
 void reverse(char s[])
 {
     int i, j;
     char c;
 
     for (i = 0, j = strlen(s)-1; i<j; i++, j--) {
         c = s[i];
         s[i] = s[j];
         s[j] = c;
     }
 }

 /* itoa:  конвертируем n в символы в s */
 void itoa(int n, char s[])
 {
     int i, sign;
 
     if ((sign = n) < 0)  /* записываем знак */
         n = -n;          /* делаем n положительным числом */
     i = 0;
     do {       /* генерируем цифры в обратном порядке */
         s[i++] = n % 10 + '0';   /* берем следующую цифру */
     } while ((n /= 10) > 0);     /* удаляем */
     if (sign < 0)
         s[i++] = '-';
     s[i] = '\0';
     reverse(s);
 }
 
 ////////////////////////////////////////////////////////////////////////////////
 
const char morseCode[50][6] = {
    {'.','-','0','0','0','0'}, //1 a
    {'-','.','.','.','0','0'}, //2 b
    {'-','.','-','.','0','0'}, //3 c
    {'-','.','.','0','0','0'}, //4 d
    {'.','0','0','0','0','0'}, //5 e
    {'.','.','-','.','0','0'}, //6 f
    {'-','-','.','0','0','0'}, //7 g
    {'.','.','.','.','0','0'}, //8 h
    {'.','.','0','0','0','0'}, //9 i
    {'.','-','-','-','0','0'}, //10 j
    {'-','.','-','0','0','0'}, //11 k
    {'.','-','.','.','0','0'}, //12 l
    {'-','-','0','0','0','0'}, //13 m
    {'-','.','0','0','0','0'}, //14 n
    {'-','-','-','0','0','0'}, //15 o
    {'.','-','-','.','0','0'}, //16 p
    {'-','-','.','-','0','0'}, //17 q
    {'.','-','.','0','0','0'}, //18 r
    {'.','.','.','0','0','0'}, //19 s
    {'-','0','0','0','0','0'}, //20 t
    {'.','.','-','0','0','0'}, //21 u
    {'.','.','.','-','0','0'}, //22 v
    {'.','-','-','0','0','0'}, //23 w
    {'-','.','.','-','0','0'}, //24 x
    {'-','.','-','-','0','0'}, //25 y
    {'-','-','.','.','0','0'}, //26 z
    {'0','0','0','0','0','0'}, //27 space
    {'.','-','.','-','.','-'}, //28 .
    {'-','-','.','.','-','-'}, //29 ,
    {'-','-','-','.','.','.'}, //30 :
    {'.','.','-','-','.','.'}, //31 ?
    {'.','-','-','-','-','.'}, //32 '
    {'-','.','.','.','.','-'}, //33 -
    {'-','.','.','-','.','0'}, //34 /
    {'-','.','-','-','.','0'}, //35 (
    {'-','.','-','-','.','-'}, //36 )
    {'.','-','.','.','-','.'}, //37 "
    {'-','.','.','.','-','0'}, //38 =
    {'.','-','.','-','.','0'}, //39 +
    {'.','-','-','.','-','.'}, //40 @
    {'.','-','-','-','-','0'}, //41 1
    {'.','.','-','-','-','0'}, //42 2
    {'.','.','.','-','-','0'}, //43 3
    {'.','.','.','.','-','0'}, //44 4
    {'.','.','.','.','.','0'}, //45 5
    {'-','.','.','.','.','0'}, //46 6
    {'-','-','.','.','.','0'}, //47 7
    {'-','-','-','.','.','0'}, //48 8
    {'-','-','-','-','.','0'}, //49 9
    {'-','-','-','-','-','0'}, //50 0
};

char alphavite1[27] = "abcdefghijklmnopqrstuvwxyz";
char alphavite2[25] = " .,:?'-/()\"=+@1234567890";

int selectedX, selectedY, currentX;
int morseX, morseSize, instructionsIndex;
char isPlayedMorse;
char isHideSignal, isHideSignalSound;
char selectedStr[31];
char currentText[31];
char morseText[31];
char instructions[23];

int speed;
int speedInterval;

////////////////////////////////////////////////////////////////////////////////

int toSpeedInterval(int speed) {
    float symToSec = (float)speed / 60.0f;
    int si = 500.0f / symToSec;
    return si;
}

void initSelectedStr(int x, int y) {
    for (int i=0; i<30; ++i) {
        selectedStr[i] = ' ';
    }
    if (y == -1 || y == selectedY) {
        if (x >= 0) {
            selectedStr[x] = '^';
        }
    }
}

void initData() {
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN | RCC_AHBENR_GPIOEEN | RCC_AHBENR_GPIOFEN;
    GPIOE->MODER |= 0x55550000;
    char roll=0x1F;
    
    SET_BIT(RCC->AHBENR,RCC_AHBENR_GPIOFEN);
    
    //устанавливаем работу линий PB11 на вывод
	SET_BIT(GPIOA->MODER, GPIO_MODER_MODER4_0);
    SET_BIT(GPIOF->MODER, GPIO_MODER_MODER2_0);
    
    // подтягиваем PB11 к питанию Pull-up
    //SET_BIT(GPIOA->PUPDR, GPIO_PUPDR_PUPDR4_0);
    //SET_BIT(GPIOF->PUPDR, GPIO_PUPDR_PUPDR2_0);
    
    
    
    selectedX = 0;
    selectedY = 0;
    currentX = 0;
    morseX = 0;
    isPlayedMorse = 0;
    isHideSignal = 0;
    isHideSignalSound = 0;
    for (int i=0; i<30; ++i) {
        selectedStr[i] = ' ';
        currentText[i] = ' ';
        morseText[i] = '\0';
    }
    selectedStr[30] = '\0';
    currentText[30] = '\0';
    morseText[30] = '\0';
    instructions[22] = '\0';

    speed = 120;
    speedInterval = toSpeedInterval(speed);
}

void selectLeft() {
    --selectedX;
    if (selectedX < 0) selectedX = 0;
}
void selectRight() {
    ++selectedX;
    if (selectedY == 0) {
        if (selectedX > 25) selectedX = 25;
    }
    else {
        if (selectedX > 23) selectedX = 23;
    }
}
void selectUp() {
    --selectedY;
    if (selectedY < 0) selectedY = 0;
}
void selectDown() {
    ++selectedY;
    if (selectedY > 1) selectedY = 1;
    if (selectedX > 23) selectedX = 23;
}

void addChar() {
    if (currentX == 29) return;

    if (selectedY == 0) {
        currentText[currentX] = alphavite1[selectedX];
    }
    else {
        currentText[currentX] = alphavite2[selectedX];
    }
    ++currentX;
}

void deleteChar() {
    --currentX;
    if (currentX < 0) {
        currentX = 0;
        return;
    }

    if (currentText[currentX+1] == '_') {
        currentText[currentX+1] = ' ';
        currentText[currentX] = '_';
    }
    else {
        currentText[currentX] = ' ';
    }
}

void changeEnterSym() {
    if (currentText[currentX] == '_') {
        currentText[currentX] = ' ';
    }
    else {
        currentText[currentX] = '_';
    }
}

void initInstructions(char sym) {
    int si = -1;

    for (int i=0; i<26; ++i) {
        if (sym == alphavite1[i]) {
            si = i;
            break;
        }
    }
    if (si == -1) {
        for (int i=0; i<24; ++i) {
            if (sym == alphavite2[i]) {
                si = i + 26;
                break;
            }
        }
    }

    for (int i=0; i<22; ++i) {
        instructions[i] = '\0';
    }

    int ii = 0;

    if (sym == ' ') {
        instructions[ii] = '0';
        instructions[ii+1] = '0';
        instructions[ii+2] = '0';
        instructions[ii+3] = '0';
    }
    else {
        for (int i=0; i<6; ++i) {
            if (morseCode[si][i] == '.') {
                instructions[ii] = '1';
                ++ii;
                instructions[ii] = '0';
                ++ii;
            }
            else if (morseCode[si][i] == '-') {
                instructions[ii] = '1';
                ++ii;
                instructions[ii] = '1';
                ++ii;
                instructions[ii] = '1';
                ++ii;
                instructions[ii] = '0';
                ++ii;
            }
            else if (morseCode[si][i] == '0') {
                instructions[ii] = '0';
                ++ii;
                instructions[ii] = '0';
                ++ii;
                break;
            }
            if (i == 5) {
                instructions[ii] = '0';
                ++ii;
                instructions[ii] = '0';
                ++ii;
            }
        }
    }
}

void playSignal();

void startSignal() {
    //showRect = true; //
    char roll=0xFF;
    GPIOE->BSRR = roll<<8;
    //isHideSignal = 1;
    //isHideSignalSound = 0;
    //ResetTimer(SOUNDTIMER);
    
    isHideSignal = 1;
    isHideSignalSound = 1;
    GPIOA->BSRR= GPIO_ODR_4;
    ResetTimer(SOUNDTIMER);
}
void endSignal() {
    //showRect = false; //
    char roll=0xFF;
    GPIOE->BRR = (roll)<<8;
    //isHideSignal = 0;
    //isHideSignalSound = 0;
    
    isHideSignal = 0;
    isHideSignalSound = 0;
    GPIOA->BRR = GPIO_ODR_4;
}

void startMorse() {
    morseSize = currentX;
    int i = 0;
    for (; i<morseSize; ++i) {
        morseText[i] = currentText[i];
    }
    for (; i<31; ++i) {
        morseText[i] = '\0';
    }
    morseX = 0;
    instructionsIndex = 0;

    if (morseSize == 0) {
        for (int i=0; i<22; ++i) {
            instructions[i] = '\0';
        }
        isPlayedMorse = 0;
        endSignal();
        return;
    }

    initInstructions(morseText[morseX]);
    isPlayedMorse = 1;

    ResetTimer(PLAYTIMER);

    instructionsIndex = -1;
    playSignal();
}

void playSignal() {
    ++instructionsIndex;

    if (instructions[instructionsIndex] == '1') {
        startSignal();
    }
    else if (instructions[instructionsIndex] == '0') {
        endSignal();
    }
    else if (instructions[instructionsIndex] == '\0') {
        ++morseX;

        if (morseX >= morseSize) {
            morseX = 0;
            isPlayedMorse = 0;
            endSignal();
            instructionsIndex = 0;

            for (int i=0; i<30; ++i) {
                morseText[i] = '\0';
            }
            for (int i=0; i<22; ++i) {
                instructions[i] = '\0';
            }
        }
        else {
            instructionsIndex = -1;
            initInstructions(morseText[morseX]);
            playSignal();
        }
    }
}

void speedUp() {
    speed = speed + 10;
    if (speed > 240) {
        speed = 240;
    }
    speedInterval = toSpeedInterval(speed);
}

void speedDown() {
    speed = speed - 10;
    if (speed < 60) {
        speed = 60;
    }
    speedInterval = toSpeedInterval(speed);
}

////////////////////////////////////////////////////////////////////////////////

void InitDisplay(void)
{
    initData();
    
    init_lcd_ili9325();
}

void ProcessDisplay(void)
{
    
    if (GetTimer(ENTERTIMER) > 500) {
        ResetTimer(ENTERTIMER);
        changeEnterSym();
    }
    
    if (isPlayedMorse == 1 && GetTimer(PLAYTIMER) > speedInterval) {
        ResetTimer(PLAYTIMER);
        playSignal();
    }
    
    if (isHideSignal == 1) {
        if (GetTimer(SOUNDTIMER) > SOUND_PITCH) { // 4
            ResetTimer(SOUNDTIMER);
        
            if (isHideSignalSound == 1) {
                isHideSignalSound = 0;
                GPIOA->BRR = GPIO_ODR_4;
            }
            else {
                isHideSignalSound = 1;
                GPIOA->BSRR= GPIO_ODR_4;
            }
        }
    }
    
	/* каждые 25 мс обновляем поля экрана: 40 кадр/сек */
    if(GetTimer(DISPUPDT_TIMER)>=25) 
	{
        ResetTimer(DISPUPDT_TIMER);

        uint16_t key = getKeycode();
        if (key != 0) {
            dispkypd4(key);
        }
        
        int y = 10;
        LCDprintstr("Alphavite:", y, LCD_COLOR_BLACK, LCD_COLOR_WHITE);
        y = y + 16;
        
        LCDprintstr(alphavite1, y, LCD_COLOR_BLACK, LCD_COLOR_WHITE);
        initSelectedStr(selectedX, 0);
        LCDprintstr(selectedStr, y+16, LCD_COLOR_BLACK, LCD_COLOR_WHITE);
        y = y + 16 + 16;
        
        LCDprintstr(alphavite2, y, LCD_COLOR_BLACK, LCD_COLOR_WHITE);
        initSelectedStr(selectedX, 1);
        LCDprintstr(selectedStr, y+16, LCD_COLOR_BLACK, LCD_COLOR_WHITE);
        y = y + 16 + 16;
        
        y = y + 10;
        LCDprintstr("Text:", y, LCD_COLOR_BLACK, LCD_COLOR_WHITE);
        LCDprintstr(currentText, y+16, LCD_COLOR_BLACK, LCD_COLOR_WHITE);
        y = y + 16 + 16;
        
        y = y + 10;
        LCDprintstr("Code Morse:", y, LCD_COLOR_BLACK, LCD_COLOR_WHITE);
        LCDprintstr(morseText, y+16, LCD_COLOR_BLACK, LCD_COLOR_WHITE);
        initSelectedStr(morseX, -1);
        LCDprintstr(selectedStr, y+16+16, LCD_COLOR_BLACK, LCD_COLOR_WHITE);
        y = y + 16 + 16;
        
        y = y + 10;
        LCDprintstr("Signal:", y, LCD_COLOR_BLACK, LCD_COLOR_WHITE);
        LCDprintstr(instructions, y+16, LCD_COLOR_BLACK, LCD_COLOR_WHITE);
        initSelectedStr(instructionsIndex, -1);
        LCDprintstr(selectedStr, y+16+16, LCD_COLOR_BLACK, LCD_COLOR_WHITE);
        y = y + 16 + 16;
        
        char number [5];
        itoa(speed, number);
        
        y = y + 10;
        LCDprintstr("Symbols per minute:", y, LCD_COLOR_BLACK, LCD_COLOR_WHITE);
        LCDprintstr(number, y+16, LCD_COLOR_BLACK, LCD_COLOR_WHITE);
    }

}




void dispkypd4(uint16_t kypd4status)
{
    uint32_t i;
    char textbuf[17]="";
    int count=0;
    
    for(i=0;i<16;i++)
    {
        if( kypd4status & (1<<i))
        {
			if(i==4) {
                selectLeft();
            }
            if(i==6) {
                selectRight();
            }
            if(i==1) {
                selectUp();
            }
            if(i==5) {
                selectDown();
            }
            if(i==2) {
                addChar();
            }
            if(i==0) {
                deleteChar();
            }
            if(i==3) {
                startMorse();
            }
            if(i==8) {
                speedDown();
            }
            if(i==9) {
                speedUp();
            }
        
            textbuf[count+0] = 'K';
            textbuf[count+1] = 0x30+((i+1)/10);
            textbuf[count+2] = 0x30+((i+1)%10);
            textbuf[count+3] = ' ';
            count += 4;
        }
    }
    textbuf[count] = 0;
    if( count > 16  )
        strcpy(textbuf, "Error");
}
