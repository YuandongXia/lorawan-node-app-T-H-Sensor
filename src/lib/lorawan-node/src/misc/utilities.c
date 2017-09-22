/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
    (C)2013 Semtech

Description: Helper functions implementation

License: Revised BSD License, see LICENSE.TXT file include in the project

Maintainer: Miguel Luis and Gregory Cristian
*/
#include <stdlib.h>
#include <stdio.h>
#include "board.h"
#include "utilities.h"

/*!
 * Redefinition of rand() and srand() standard C functions.
 * These functions are redefined in order to get the same behavior across
 * different compiler toolchains implementations.
 */
// Standard random functions redefinition start
#define RAND_LOCAL_MAX 2147483647L

static uint32_t next = 1;

int32_t rand1( void )
{
    return ( ( next = next * 1103515245L + 12345L ) % RAND_LOCAL_MAX );
}

void srand1( uint32_t seed )
{
    next = seed;
}
// Standard random functions redefinition end

int32_t randr( int32_t min, int32_t max )
{
    return ( int32_t )rand1( ) % ( max - min + 1 ) + min;
}

void memcpy1( uint8_t *dst, const uint8_t *src, uint16_t size )
{
    while( size-- )
    {
        *dst++ = *src++;
    }
}

void memcpyr( uint8_t *dst, const uint8_t *src, uint16_t size )
{
    dst = dst + ( size - 1 );
    while( size-- )
    {
        *dst-- = *src++;
    }
}

void memset1( uint8_t *dst, uint8_t value, uint16_t size )
{
    while( size )
    {
        *dst++ = value;
        size--;
    }
}

int8_t Nibble2HexChar( uint8_t a )
{
    if( a < 10 )
    {
        return '0' + a;
    }
    else if( a < 16 )
    {
        return 'A' + ( a - 10 );
    }
    else
    {
        return '?';
    }
}

uint32_t u8tou32_b(const uint8_t *u_8)
{
    return ( ((uint32_t)u_8[0]<<24) | ((uint32_t)u_8[1]<<16) | ((uint32_t)u_8[2]<<8) | ((uint32_t)u_8[3]<<0) );
}

uint32_t u8tou32_l(const uint8_t *u_8)
{
    return ( ((uint32_t)u_8[0]<<0) | ((uint32_t)u_8[1]<<8) | ((uint32_t)u_8[2]<<16) | ((uint32_t)u_8[3]<<24) );
}

uint32_t u32tou8_b(uint8_t *u_8, uint32_t u_32)
{
    u_8[0] = (uint8_t)(u_32>>24);
    u_8[1] = (uint8_t)(u_32>>16);
    u_8[2] = (uint8_t)(u_32>>8);
    u_8[3] = (uint8_t)(u_32>>0);
    return u_32;
}

uint32_t u32tou8_l(uint8_t *u_8, uint32_t u_32)
{
    u_8[0] = (uint8_t)(u_32>>0);
    u_8[1] = (uint8_t)(u_32>>8);
    u_8[2] = (uint8_t)(u_32>>16);
    u_8[3] = (uint8_t)(u_32>>24);
    return u_32;
}

uint16_t u8tou16_b(const uint8_t *u_8)
{
    return ( ((uint16_t)u_8[0]<<8) | ((uint16_t)u_8[1]<<0) );
}

uint16_t u8tou16_l(const uint8_t *u_8)
{
    return ( ((uint16_t)u_8[0]<<0) | ((uint16_t)u_8[1]<<8) );
}

uint16_t u16tou8_b(uint8_t *u_8, uint16_t u_16)
{
    u_8[0] = (uint8_t)(u_16>>8);
    u_8[1] = (uint8_t)(u_16>>0);
    return u_16;
}

uint16_t u16tou8_l(uint8_t *u_8, uint16_t u_16)
{
    u_8[0] = (uint8_t)(u_16>>0);
    u_8[1] = (uint8_t)(u_16>>8);
    return u_16;
}
