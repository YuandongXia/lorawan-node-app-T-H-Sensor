#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>
#include <libgen.h>

#include "uart.h"
#include "cfg.h"
#include "gollum.h"
#include "print.h"
#include "cmd.h"
#include "version.h"
#include "log.h"
#include "banner.h"

#define OPT_MAP_H               (1<<0)
#define OPT_MAP_F               (1<<1)
#define OPT_MAP_R               (1<<2)
#define OPT_MAP_V               (1<<3)
#define OPT_MAP_C               (1<<4)
#define OPT_MAP_L               (1<<5)
#define OPT_MAP_T               (1<<6)
#define OPT_MAP_P               (1<<7)
#define OPT_MAP_M               (1<<8)
#define OPT_MAP_D               (1<<9)
#define OPT_MAP_TP              (OPT_MAP_T|OPT_MAP_P)

//    "EU868",        // 863 ~ 870
//    "EU434",        // 433 ~ 434
//    "US915",        // 902 ~ 928
//    "US915HYBRID",  // 902 ~ 928
//    "CN780",        // 779 ~ 787
//    "AU920",        // 915 ~ 928
//    "CUSTOM",       // full

void usage(char *name)
{
    log_msg(PRIORITY_PLAIN, "\nUsage: %s [OPTIONS]... [JSON FILE]", name);
    log_msg(PRIORITY_PLAIN, "Configure RisingHF LoRaWAN devices\n");
    log_msg(PRIORITY_PLAIN, " -h    Help");
    log_msg(PRIORITY_PLAIN, " -d    Data rate scheme (EU868/EU434/US915[HYBRID]/AU920)");
    log_msg(PRIORITY_PLAIN, " -c    Get configuration from json format file");
    log_msg(PRIORITY_PLAIN, "       and set to these new configuration to connected device");
    log_msg(PRIORITY_PLAIN, " -l    Load and print configuration, do not write to device");
    log_msg(PRIORITY_PLAIN, " -f    Set device to factory default");
    log_msg(PRIORITY_PLAIN, " -r    Read all configuration from the device");
    log_msg(PRIORITY_PLAIN, " -v    Version %d.%d.%d [major.minor.patch]", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH);
    log_msg(PRIORITY_PLAIN, " -t    0:Normal 1:TXCW 2:TXCLORA 3:RXCLORA");
    log_msg(PRIORITY_PLAIN, " -p    \"%%f,%%d,%%d,%%d,%%d,%%d\"");
    log_msg(PRIORITY_PLAIN, " -m    Check mode");
}

const char opt_tab[][3] = {
    "-h",
    "-f",
    "-r",
    "-v",
    "-c",
    "-l",
    "-t",
    "-p",
    "-m",
    "-d",
};

char *mode_tab[]={
    "NORMAL",
    "TXCW",
    "TXCLORA",
    "RXCLORA",
};

int bw_tab[]={
    125,
    250,
    500
};

void opt_error(uint32_t type, uint32_t map)
{
    int i;

    for(i=0; i<32; i++){
        if(type & (1<<i)){
            printf("'%s' and", opt_tab[i]);
            break;
        }
    }

    for(i=0; i<32; i++){
        if(map & (1<<i)){
            if((1<<i) & type){
                continue;
            }
            printf(" '%s'", opt_tab[i]);
        }
    }
    printf(" can not be mixed.\n");
}

int main(int argc, char** argv)
{
    uint32_t opt_map;
    int i, ret, j;
    char *cpfile = NULL, *lpfile = NULL;
    gm_device_t gmsp;
    cfg_t config;

    char *ppstr = NULL, *mpdschm = NULL;
    cmd_rf_t rfcfg;
    int test;

    opt_map = 0;
    while ((i = getopt (argc, argv, "hfrvmt:c:l:p:d:")) != -1) {
		switch (i) {
        case 'h':
            opt_map |= OPT_MAP_H;
            break;
        case 'c':
            opt_map |= OPT_MAP_C;
            cpfile = optarg;
            if( access( cpfile, F_OK ) == -1 ) {
                log_msg(PRIORITY_FATAL, "Load configuration file %s fail", basename(cpfile));
                return -1;
            }
            break;
        case 'r':
            opt_map |= OPT_MAP_R;
            break;
        case 'v':
            opt_map |= OPT_MAP_V;
            break;
        case 'f':
            opt_map |= OPT_MAP_F;
            break;
        case 'l':
            opt_map |= OPT_MAP_L;
            lpfile = optarg;
            if( access( lpfile, F_OK ) == -1 ) {
                log_msg(PRIORITY_FATAL, "Load configuration file %s fail", basename(lpfile));
                return -1;
            }
            break;
        case 't':
            opt_map |= OPT_MAP_T;
            if(sscanf(optarg,"%d",&test) != 1){
                for(j=0; j<4; j++){
                    if(0 == strcasecmp(mode_tab[j], optarg)){
                        test = j;
                        break;
                    }
                }
                if(j==4){
                    log_msg(PRIORITY_FATAL, "Unknown test mode");
                    return -1;
                }
            }
            break;
        case 'p':
            opt_map |= OPT_MAP_P;
            ppstr = optarg;
            break;
        case 'd':
            opt_map |= OPT_MAP_D;
            mpdschm = optarg;
            break;
        case 'm':
            opt_map |= OPT_MAP_M;
            break;
        default:
            log_msg(PRIORITY_FATAL, "PARAMETER ERROR");
            banner();
            usage(basename(argv[0]));
            return -1;
		}
    }

    if(opt_map == 0){
        banner();
        usage(basename(argv[0]));
        return -1;
    }

//    log_msg(PRIORITY_FATAL, "%d.%d.%d fatal", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH);
//    log_msg(PRIORITY_NOTICE, "%d.%d.%d notice", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH);
//    log_msg(PRIORITY_ERROR, "%d.%d.%d error", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH);
//    log_msg(PRIORITY_WARN, "%d.%d.%d warn", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH);
//    log_msg(PRIORITY_INFO, "%d.%d.%d info", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH);
//    log_msg(PRIORITY_PLAIN, "%d.%d.%d plain", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH);

    if( opt_map&OPT_MAP_H ){
        if( opt_map&~OPT_MAP_H ){
            opt_error(OPT_MAP_H, opt_map);
            return -1;
        }
        usage(basename(argv[0]));
    }else if( opt_map&OPT_MAP_V ){
        if( opt_map&~OPT_MAP_V ){
            opt_error(OPT_MAP_V, opt_map);
            return -1;
        }
        log_msg(PRIORITY_PLAIN, "%d.%d.%d", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH);
    }else if( opt_map&OPT_MAP_R ){
        if( opt_map&~OPT_MAP_R ){
            opt_error(OPT_MAP_R, opt_map);
            return -1;
        }
        gmsp = gm_scan();
        if(gmsp != NULL){
            cfg_read(gmsp, &config);
            gm_close(gmsp);
        }
    }else if( opt_map&OPT_MAP_L ){
        if( opt_map&~OPT_MAP_L ){
            opt_error(OPT_MAP_L, opt_map);
            return -1;
        }

        ret = cfg_load(lpfile, &config);
        if(ret < 0){
            log_msg(PRIORITY_NOTICE, "Configuration parse error(%d)\n", ret);
            return -1;
        }
        print_spliter();
        log_msg(PRIORITY_INFO, "%s:",lpfile);
        cfg_puts(&config);

    }else if( opt_map&OPT_MAP_F ){
        if( opt_map&~OPT_MAP_F ){
            opt_error(OPT_MAP_F, opt_map);
            return -1;
        }
        gmsp = gm_scan();
        if(gmsp != NULL){
            if(0==cmd_fdefault(gmsp)){
                log_msg(PRIORITY_WARN, "Factory default reset successfully.");
            }
            gm_close(gmsp);
        }
    }else if( opt_map&OPT_MAP_C ){
        if( opt_map&~OPT_MAP_C ){
            opt_error(OPT_MAP_C, opt_map);
            return -1;
        }

        log_msg(PRIORITY_PLAIN, "RisingHF configuration tool\nVersion %d.%d.%d", \
            VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH);

        ret = cfg_load(cpfile, &config);
        if(ret < 0){
            log_msg(PRIORITY_NOTICE, "Configuration parse error(%d)\n", ret);
            return -1;
        }

        print_spliter();
        gmsp = gm_scan();
        if(gmsp != NULL){
            cfg_write(gmsp, &config);
            gm_close(gmsp);
        }else{
            log_msg(PRIORITY_ERROR, "No device is found.\n");
        }
    }else if(opt_map&OPT_MAP_T){
        cmd_rf_t *prfcfg = NULL;
        if( opt_map & ~(OPT_MAP_TP) ){
            opt_error(OPT_MAP_TP, opt_map);
            return -1;
        }
        if(opt_map&OPT_MAP_P){
            int ret;
            int frf,sf,bw,txpr,rxpr,pow;
            ret = sscanf(ppstr, "%d,%d,%d,%d,%d,%d", &frf,&sf,&bw,&txpr,&rxpr,&pow);
            rfcfg.frf = frf;
            rfcfg.sf = sf;
            switch(bw){
            case 125000:
                rfcfg.bw = 0;
                break;
            case 250000:
                rfcfg.bw = 1;
                break;
            case 500000:
                rfcfg.bw = 2;
                break;
            default:
                log_msg(PRIORITY_NOTICE, "Band width error");
                return -1;
            }
            rfcfg.tx_pr_len = txpr;
            rfcfg.rx_pr_len = rxpr;
            rfcfg.pow = pow;
            #if 0
            printf("%d F%d,SF%d,BW%d,TXPR%d,RXPR%d,POW%d",ret,rfcfg.frf,rfcfg.sf,rfcfg.bw,rfcfg.tx_pr_len,\
                   rfcfg.rx_pr_len,rfcfg.pow);
            #endif
            if(ret != 6){
                log_msg(PRIORITY_NOTICE, "RF parameter error");
                return -1;
            }
            prfcfg = &rfcfg;
        }
        gmsp = gm_scan();
        if(gmsp != NULL){
            if(0 == cmd_set_test(gmsp, test, prfcfg)){
                 log_msg(PRIORITY_PLAIN, "Set test mode successfully");
            }else{
                log_msg(PRIORITY_PLAIN, "Set test mode failed");
            }
            gm_close(gmsp);
        }

    }else if(opt_map&OPT_MAP_M){
        int mode;
        cmd_rf_t rfcfg_ro;
        if( opt_map & ~(OPT_MAP_M) ){
            opt_error(OPT_MAP_M, opt_map);
            return -1;
        }
        gmsp = gm_scan();
        if(gmsp != NULL){
            if(0 == cmd_get_test(gmsp, &mode, &rfcfg_ro)){
                printf("%s F%d,SF%d,BW%d,TXPR%d,RXPR%d,POW%d",mode_tab[mode],rfcfg_ro.frf,rfcfg_ro.sf,bw_tab[rfcfg_ro.bw],rfcfg_ro.tx_pr_len,\
               rfcfg_ro.rx_pr_len,rfcfg_ro.pow);
            }
            gm_close(gmsp);
        }
    }else if(opt_map&OPT_MAP_D){
        int i;
        cmd_dr_t dr;
        i = cfg_get_drschm(mpdschm);
        if( i < 0){
            log_msg(PRIORITY_ERROR, "Unknown data rate scheme");
            return -1;
        }
        dr.schm = i;
        dr.flag = CMD_FLAG_DR_SCHM;
        gmsp = gm_scan();
        if(gmsp != NULL){
            if( 0 == cmd_set_dr(gmsp, &dr) ){
                log_msg(PRIORITY_PLAIN, "Set DATA RATE SCHEME successfully");
            }else{
                log_msg(PRIORITY_ERROR, "Set DATA RATE SCHEME error");
            }
            gm_close(gmsp);
        }
        //
    }
    return 0;
}
