#ifndef __CFG_H
#define __CFG_H

#include <stdint.h>
#include <stdbool.h>


#define LW_DEVADDR_LEN          (4)
#define LW_APPEUI_LEN           (8)
#define LW_DEVEUI_LEN           (8)

#define LW_NWKSKEY_LEN          (16)
#define LW_APPSKEY_LEN          (16)
#define LW_APPKEY_LEN           (16)

#define LW_CHANNELS_MAX_NUM     (72)

#include "cmd.h"

typedef struct{
    struct{
        uint8_t val;
        bool flag;
    }mode;

    struct{
        uint8_t val;
        bool flag;
    }drschm;

    struct{
        struct{
            uint8_t buf[LW_DEVADDR_LEN];
            bool flag;
        }devaddr;
        struct{
            uint8_t buf[LW_DEVEUI_LEN];
            bool flag;
        }deveui;
        struct{
            uint8_t buf[LW_APPEUI_LEN];
            bool flag;
        }appeui;
    }id;

    struct{
        struct{
            uint8_t buf[LW_NWKSKEY_LEN];
            bool flag;
        }nwkskey;
        struct{
            uint8_t buf[LW_APPSKEY_LEN];
            bool flag;
        }appskey;
        struct{
            uint8_t buf[LW_APPKEY_LEN];
            bool flag;
        }appkey;
    }key;

    struct{
        bool flag;
        int freq;
        int dr;
    }rxwin2;

    struct{
        bool flag;
        bool val;
    }adr;

    struct{
        int val;
        bool flag;
    }dr;

    struct{
        int val;
        bool flag;
    }power;

    struct{
        int val;
        bool flag;
    }rept;

    struct{
        int val;
        bool flag;
    }retry;

    struct{
        int val;
        bool flag;
    }port;

    // class
    struct{
        int val;
        bool flag;
    }clss;

    struct{
        int val;
        bool flag;
    }threshold;

    uint16_t chmsk[6];
    struct{
        int freq;
        int dr_min;
        int dr_max;
    }channels[LW_CHANNELS_MAX_NUM];

    uint16_t rxwin1msk[6];
    struct{
        int freq;
    }rxwin1[LW_CHANNELS_MAX_NUM];

    struct{
        bool flag;
        int32_t val;
    }period;

    struct{
        bool flag;
        int32_t val;
    }period_offset;

    struct{
        bool flag;
        uint8_t val;
    }ulfmt;

    struct{
		bool flag;
		int16_t val;
	}factor;

	struct{
		bool flag;
		int32_t val;
	}me1initpul;

	struct{
		bool flag;
		int32_t val;
	}me2initpul;

	struct{
        bool flag;
        float longitude;
        float latitude;
	}gps;

    struct{
        bool flag;
        struct{
            bool flag;
            uint8_t devaddr[4];
            uint8_t nwkskey[16];
            uint8_t appskey[16];
            uint32_t dfcnt;
        }val;
    }mc;
}cfg_t;

// Load configuration from json format configure file
int cfg_load(const char *file, cfg_t *config);

// Write configuration to device
void cfg_write(gm_device_t gmsp, cfg_t *config);

// Write configuration to device
void cfg_read(gm_device_t gmsp, cfg_t *config);

void cfg_puts(cfg_t *config);

int cfg_get_drschm(char *drschm);

#endif // __CFG_H
