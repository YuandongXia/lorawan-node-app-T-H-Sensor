#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "aes.h"
#include "print.h"

void putsbuf(uint8_t *buf, int len)
{
    int i;
    for(i=0; i<len; i++){
        printf("%02X", buf[i]);
    }
}

int main(int argc, char **argv)
{
    int i;

    printf("FFFFFFFF");
    for(i=1; i<argc; i++){
        putsbuf((uint8_t *)argv[i], strlen(argv[i]));
    }
    printf("0A\n");
    return 0;
}
