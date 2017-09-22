/*
    _/_/_/    _/_/_/    _/_/_/  _/_/_/  _/      _/    _/_/_/  _/    _/  _/_/_/_/
   _/    _/    _/    _/          _/    _/_/    _/  _/        _/    _/  _/
  _/_/_/      _/      _/_/      _/    _/  _/  _/  _/  _/_/  _/_/_/_/  _/_/_/
 _/    _/    _/          _/    _/    _/    _/_/  _/    _/  _/    _/  _/
_/    _/  _/_/_/  _/_/_/    _/_/_/  _/      _/    _/_/_/  _/    _/  _/
    (C)2016 RisingHF, all rights reserved.
*/
#include "oled.h"
#include "hal-ssd1306.h"
#include "font.h"

void oled_init()
{

}

void oled_putchar(uint8_t x, uint8_t y, char c)
{
    hal_ssd1306_draw(x, y, (uint8_t *)&font5x8[5*(c-' ')], 5, 8);
}

/** MAX_LEN: 8x21 = 168 */
void oled_putstr(uint8_t x, uint8_t y, char *str)
{
    while(*str){
        oled_putchar(x, y, *str);
        x+=6;
        str++;
        if( x>(127-5) ){
            y+=8;
            x=0;
            if(y>(63-7)){
                break;
            }
        }
    }
}

void oled_putchar_big(uint8_t x, uint8_t y, char c)
{
    hal_ssd1306_draw(x, y, (uint8_t *)&font8x16[16*(c-' ')], 8, 16);
}

/** MAX_LEN: 4x14 = 56 */
void oled_putstr_big(uint8_t x, uint8_t y, char *str)
{
    while(*str){
        oled_putchar_big(x, y, *str);
        x+=9;
        str++;
        if( x>(127-8) ){
            y+=16;
            x=0;
            if(y>(63-15)){
                break;
            }
        }
    }
}

void oled_putchar_huge(uint8_t x, uint8_t y, char c)
{
    hal_ssd1306_draw(x, y, (uint8_t *)&font16x32[64*(c-' ')], 16, 32);
}

/** MAX_LEN: 2x8 = 16 */
void oled_putstr_huge(uint8_t x, uint8_t y, char *str)
{
    while(*str){
        oled_putchar_huge(x, y, *str);
        x+=16;
        str++;
        if( x> (127-8) ){
            y+=32;
            x=0;
            if(y>(63-31)){
                break;
            }
        }
    }
}

void oled_draw_rect(uint8_t x, uint8_t y, uint8_t width, uint8_t height)
{
    int i, j;
    for(i=0; i<width; i++){
        for(j=0; j<height; j++){
            hal_ssd1306_draw_pixel_buf(x+i, y+j, 1);
        }
    }
    hal_ssd1306_update(x, y, width, height);
}
