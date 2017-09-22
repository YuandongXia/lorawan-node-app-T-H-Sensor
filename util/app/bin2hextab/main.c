#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

FILE *fin;
FILE *fout;
FILE *fdout;

uint8_t inbuf[1024*1024];
uint8_t outbuf[1024*1024];

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
    int ret;
    int i;

    fin = fopen(argv[1], "rb");
    if(fin==NULL){
        printf("Can't open %s\n", argv[1]);
        return -1;
    }

    while( !feof( fin ) ){
        ret = fread(inbuf, 1, 1024*1024, fin);
        if( ferror( fin ) ){
            perror( "Read error" );
            break;
        }
        if(ret > 0 && ret < 1024*1024){
            printf("FILE size %d\n", ret);
        }else{
            printf("Error size %d\n", ret);
        }
    }

    fout = fopen("out.hextab", "wb");
    if(fout==NULL){
        printf("Can't creat output file\n");
        fclose(fin);
        return -1;
    }

    i = 0;
    fprintf(fout, "    ");
    do{
        if(i%16 == 0){
            fprintf(fout, "\r\n    ");
        }
        fprintf(fout, "0x%02X, ", inbuf[i]);
        i++;
    }while(i<ret);

    //fwrite(outbuf, sizeof(uint8_t), len, fout);

    fclose(fout);
    fclose(fin);

    return 0;
}
