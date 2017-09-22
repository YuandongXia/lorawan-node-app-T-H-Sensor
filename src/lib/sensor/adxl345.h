/*
    _/_/_/    _/_/_/    _/_/_/  _/_/_/  _/      _/    _/_/_/  _/    _/  _/_/_/_/
   _/    _/    _/    _/          _/    _/_/    _/  _/        _/    _/  _/
  _/_/_/      _/      _/_/      _/    _/  _/  _/  _/  _/_/  _/_/_/_/  _/_/_/
 _/    _/    _/          _/    _/    _/    _/_/  _/    _/  _/    _/  _/
_/    _/  _/_/_/  _/_/_/    _/_/_/  _/      _/    _/_/_/  _/    _/  _/
    (C)2016 RisingHF, all rights reserved.
*/
#ifndef __ADXL345_H
#define __ADXL345_H

#include <stdint.h>


/******************************************************************************/
/******************************** ADXL345 *************************************/
/******************************************************************************/

/* I2C address of the device */
#define ADXL345_I2C_ADDR        (0x53<<1)
#define ADXL345_I2CADDRESS  	(0x1D)

/* SPI commands */
#define ADXL345_SPI_READ        (1 << 7)
#define ADXL345_SPI_WRITE       (0 << 7)
#define ADXL345_SPI_MB          (1 << 6)

/* ADXL345 Register Map */
#define	ADXL345_DEVID           0x00 // R   Device ID.
#define ADXL345_THRESH_TAP      0x1D // R/W Tap threshold.
#define ADXL345_OFSX            0x1E // R/W X-axis offset.
#define ADXL345_OFSY            0x1F // R/W Y-axis offset.
#define ADXL345_OFSZ            0x20 // R/W Z-axis offset.
#define ADXL345_DUR             0x21 // R/W Tap duration.
#define ADXL345_LATENT          0x22 // R/W Tap latency.
#define ADXL345_WINDOW          0x23 // R/W Tap window.
#define ADXL345_THRESH_ACT      0x24 // R/W Activity threshold.
#define ADXL345_THRESH_INACT    0x25 // R/W Inactivity threshold.
#define ADXL345_TIME_INACT      0x26 // R/W Inactivity time.
#define ADXL345_ACT_INACT_CTL   0x27 // R/W Axis enable control for activity
                                     // and inactivity detection.
#define ADXL345_THRESH_FF       0x28 // R/W Free-fall threshold.
#define ADXL345_TIME_FF         0x29 // R/W Free-fall time.
#define ADXL345_TAP_AXES        0x2A // R/W Axis control for tap/double tap.
#define ADXL345_ACT_TAP_STATUS  0x2B // R   Source of tap/double tap.
#define ADXL345_BW_RATE         0x2C // R/W Data rate and power mode control.
#define ADXL345_POWER_CTL       0x2D // R/W Power saving features control.
#define ADXL345_INT_ENABLE      0x2E // R/W Interrupt enable control.
#define ADXL345_INT_MAP         0x2F // R/W Interrupt mapping control.
#define ADXL345_INT_SOURCE      0x30 // R   Source of interrupts.
#define ADXL345_DATA_FORMAT     0x31 // R/W Data format control.
#define ADXL345_DATAX0          0x32 // R   X-Axis Data 0.
#define ADXL345_DATAX1          0x33 // R   X-Axis Data 1.
#define ADXL345_DATAY0          0x34 // R   Y-Axis Data 0.
#define ADXL345_DATAY1          0x35 // R   Y-Axis Data 1.
#define ADXL345_DATAZ0          0x36 // R   Z-Axis Data 0.
#define ADXL345_DATAZ1          0x37 // R   Z-Axis Data 1.
#define ADXL345_FIFO_CTL        0x38 // R/W FIFO control.
#define ADXL345_FIFO_STATUS     0x39 // R   FIFO status.

/* ADXL345_ACT_INACT_CTL definition */
#define ADXL345_ACT_ACDC        (1 << 7)
#define ADXL345_ACT_X_EN        (1 << 6)
#define ADXL345_ACT_Y_EN        (1 << 5)
#define ADXL345_ACT_Z_EN        (1 << 4)
#define ADXL345_INACT_ACDC      (1 << 3)
#define ADXL345_INACT_X_EN      (1 << 2)
#define ADXL345_INACT_Y_EN      (1 << 1)
#define ADXL345_INACT_Z_EN      (1 << 0)

/* ADXL345_TAP_AXES definition */
#define ADXL345_SUPPRESS        (1 << 3)
#define ADXL345_TAP_X_EN        (1 << 2)
#define ADXL345_TAP_Y_EN        (1 << 1)
#define ADXL345_TAP_Z_EN        (1 << 0)

/* ADXL345_ACT_TAP_STATUS definition */
#define ADXL345_ACT_X_SRC       (1 << 6)
#define ADXL345_ACT_Y_SRC       (1 << 5)
#define ADXL345_ACT_Z_SRC       (1 << 4)
#define ADXL345_ASLEEP          (1 << 3)
#define ADXL345_TAP_X_SRC       (1 << 2)
#define ADXL345_TAP_Y_SRC       (1 << 1)
#define ADXL345_TAP_Z_SRC       (1 << 0)

/* ADXL345_BW_RATE definition */
#define ADXL345_LOW_POWER       (1 << 4)
#define ADXL345_RATE(x)         ((x) & 0xF)

/* ADXL345_POWER_CTL definition */
#define ADXL345_PCTL_LINK       (1 << 5)
#define ADXL345_PCTL_AUTO_SLEEP (1 << 4)
#define ADXL345_PCTL_MEASURE    (1 << 3)
#define ADXL345_PCTL_SLEEP      (1 << 2)
#define ADXL345_PCTL_WAKEUP(x)  ((x) & 0x3)

/* ADXL345_INT_ENABLE / ADXL345_INT_MAP / ADXL345_INT_SOURCE definition */
#define ADXL345_DATA_READY      (1 << 7)
#define ADXL345_SINGLE_TAP      (1 << 6)
#define ADXL345_DOUBLE_TAP      (1 << 5)
#define ADXL345_ACTIVITY        (1 << 4)
#define ADXL345_INACTIVITY      (1 << 3)
#define ADXL345_FREE_FALL       (1 << 2)
#define ADXL345_WATERMARK       (1 << 1)
#define ADXL345_OVERRUN         (1 << 0)

/* ADXL345_DATA_FORMAT definition */
#define ADXL345_SELF_TEST       (1 << 7)
#define ADXL345_SPI             (1 << 6)
#define ADXL345_INT_INVERT      (1 << 5)
#define ADXL345_FULL_RES        (1 << 3)
#define ADXL345_JUSTIFY         (1 << 2)
#define ADXL345_RANGE(x)        ((x) & 0x3)

/* ADXL345_RANGE(x) options */
#define ADXL345_RANGE_PM_2G     0
#define ADXL345_RANGE_PM_4G     1
#define ADXL345_RANGE_PM_8G     2
#define ADXL345_RANGE_PM_16G    3

/* ADXL345_FIFO_CTL definition */
#define ADXL345_FIFO_MODE(x)    (((x) & 0x3) << 6)
#define ADXL345_TRIGGER         (1 << 5)
#define ADXL345_SAMPLES(x)      ((x) & 0x1F)

/* ADXL345_FIFO_MODE(x) options */
#define ADXL345_FIFO_BYPASS     0
#define ADXL345_FIFO_FIFO       1
#define ADXL345_FIFO_STREAM     2
#define ADXL345_FIFO_TRIGGER    3

/* ADXL345_FIFO_STATUS definition */
#define ADXL345_FIFO_TRIG       (1 << 7)
#define ADXL345_ENTRIES(x)      ((x) & 0x3F)

/* ADXL345 ID */
#define ADXL345_ID              0xE5

/* ADXL345 Full Resolution Scale Factor */
#define ADXL345_SCALE_FACTOR    0.0039


uint8_t adxl345_get_id(void);
int adxl345_set_mode(uint8_t mode);
int adxl345_set_range(uint8_t gRange, uint8_t fullRes);
int adxl345_read_xyz(uint8_t *buf);

int adxl345_write(uint8_t addr, uint8_t val);

#endif