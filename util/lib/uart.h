/*-
 * Free/Libre Near Field Communication (NFC) library
 *
 * Libnfc historical contributors:
 * Copyright (C) 2009      Roel Verdult
 * Copyright (C) 2009-2013 Romuald Conty
 * Copyright (C) 2010-2012 Romain Tartière
 * Copyright (C) 2010-2013 Philippe Teuwen
 * Copyright (C) 2012-2013 Ludovic Rousseau
 * See AUTHORS file for a more comprehensive list of contributors.
 * Additional contributors of this file:
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 *
 */

/**
 * @file uart.h
 * @brief UART driver header
 */

#ifndef __NFC_BUS_UART_H__
#  define __NFC_BUS_UART_H__

#  include <sys/time.h>

#  include <stdio.h>
#  include <string.h>
#  include <stdlib.h>
#  include <stdint.h>
#  include <stdbool.h>

/* Error codes */
/** @ingroup error
 * @hideinitializer
 * Success (no error)
 */
#define NFC_SUCCESS			 0
/** @ingroup error
 * @hideinitializer
 * Input / output error, device may not be usable anymore without re-open it
 */
#define NFC_EIO				-1
/** @ingroup error
 * @hideinitializer
 * Invalid argument(s)
 */
#define NFC_EINVARG			-2
/** @ingroup error
 * @hideinitializer
 *  Operation not supported by device
 */
#define NFC_EDEVNOTSUPP			-3
/** @ingroup error
 * @hideinitializer
 * No such device
 */
#define NFC_ENOTSUCHDEV			-4
/** @ingroup error
 * @hideinitializer
 * Buffer overflow
 */
#define NFC_EOVFLOW			-5
/** @ingroup error
 * @hideinitializer
 * Operation timed out
 */
#define NFC_ETIMEOUT			-6
/** @ingroup error
 * @hideinitializer
 * Operation aborted (by user)
 */
#define NFC_EOPABORTED			-7
/** @ingroup error
 * @hideinitializer
 * Not (yet) implemented
 */
#define NFC_ENOTIMPL			-8
/** @ingroup error
 * @hideinitializer
 * Target released
 */
#define NFC_ETGRELEASED			-10
/** @ingroup error
 * @hideinitializer
 * Error while RF transmission
 */
#define NFC_ERFTRANS			-20
/** @ingroup error
 * @hideinitializer
 * MIFARE Classic: authentication failed
 */
#define NFC_EMFCAUTHFAIL		-30
/** @ingroup error
 * @hideinitializer
 * Software error (allocation, file/pipe creation, etc.)
 */
#define NFC_ESOFT			-80
/** @ingroup error
 * @hideinitializer
 * Device's internal chip error
 */
#define NFC_ECHIP			-90


// Define shortcut to types to make code more readable
typedef void *serial_port;
#  define INVALID_SERIAL_PORT (void*)(~1)
#  define CLAIMED_SERIAL_PORT (void*)(~2)

serial_port uart_open(const char *pcPortName);
void    uart_close(const serial_port sp);
void    uart_flush_input(const serial_port sp, bool wait);

void    uart_set_speed(serial_port sp, const uint32_t uiPortSpeed);
uint32_t uart_get_speed(const serial_port sp);

int     uart_receive(serial_port sp, uint8_t *pbtRx, const size_t szRx, void *abort_p, int timeout);
int     uart_send(serial_port sp, const uint8_t *pbtTx, const size_t szTx, int timeout);

char  **uart_list_ports(void);
void uart_list_ports_free(char **availablePorts);

#endif // __NFC_BUS_UART_H__
