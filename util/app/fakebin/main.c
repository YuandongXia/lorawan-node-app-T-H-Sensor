#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "aes.h"
#include "print.h"

uint8_t key[]={
    0x15, 0xF9, 0xC1, 0x11, 0x17, 0xA2, 0x1F, 0x63,
    0x24, 0x58, 0x58, 0x77, 0xE1, 0x15, 0x16, 0x28,
};

FILE *fout;
FILE *fdout;

uint8_t inbuf[1024*1024];
uint8_t outbuf[1024*1024];
uint8_t doutbuf[1024*1024];

int encrypt(uint8_t *out, uint8_t *in, uint8_t *key)
{
    aes_context aesContext;
    memset(&aesContext, 0, sizeof(aesContext));
	aes_set_key(key, 16, &aesContext);
    aes_encrypt(in, out, &aesContext);

    return 0;
}

int decrypt(uint8_t *out, uint8_t *in, uint8_t *key)
{
    aes_context aesContext;
    memset(&aesContext, 0, sizeof(aesContext));
	aes_set_key(key, 16, &aesContext);
    aes_decrypt(in, out, &aesContext);

    return 0;
}

int block_encrypt(uint8_t *out, uint8_t *in, int len, uint8_t *key)
{
    int i;

    for(i=0; i<len; i+=16){
        decrypt(out+i, in+i, key);
    }

    return i;
}

int block_decrypt(uint8_t *out, uint8_t *in, int len, uint8_t *key)
{
    int i;

    for(i=0; i<len; i+=16){
        encrypt(out+i, in+i, key);
    }
    return i;
}

void putsbuf(uint8_t *buf, int len)
{
    int i;
    for(i=0; i<len; i++){
        printf("%02X ", buf[i]);
    }
    printf("\n");
}

int main(int argc, char **argv)
{
    int i, ret, len, dlen, count;

    ret = 120*1024;
    for(i=0, count=0; i<ret; i++){
        inbuf[i] = (uint8_t)i;
        if(inbuf[i] == 0){
            inbuf[i] = count;
            count++;
        }
    }
    if(ret > 0 && ret < 1024*1024){
        printf("FILE size %d\n", ret);
        len = block_encrypt(outbuf, inbuf, ret, key);
        printf("encrypt size %d\n", len);
    }else{
        printf("Error size %d\n", ret);
    }

    dlen = block_decrypt(doutbuf, outbuf, len, key);
    printf("decrypt size %d\n", dlen);

    fout = fopen("out.ebin.bin", "wb");
    if(fout==NULL){
        printf("Can't creat output file\n");
        return -1;
    }
    fdout = fopen("dout.bin", "wb");
    if(fdout==NULL){
        printf("Can't creat output file\n");
        fclose(fout);
        return -1;
    }

    fwrite(outbuf, sizeof(uint8_t), len, fout);
    fwrite(doutbuf, sizeof(uint8_t), len, fdout);

    fclose(fout);

    return 0;
}
