/*
    _/_/_/    _/_/_/    _/_/_/  _/_/_/  _/      _/    _/_/_/  _/    _/  _/_/_/_/
   _/    _/    _/    _/          _/    _/_/    _/  _/        _/    _/  _/
  _/_/_/      _/      _/_/      _/    _/  _/  _/  _/  _/_/  _/_/_/_/  _/_/_/
 _/    _/    _/          _/    _/    _/    _/_/  _/    _/  _/    _/  _/
_/    _/  _/_/_/  _/_/_/    _/_/_/  _/      _/    _/_/_/  _/    _/  _/
    (C)2016 RisingHF, all rights reserved.
*/
#include "flash.h"
#include "board.h"



#define FLASH_PDKEY1               ((uint32_t)0x04152637) /*!< Flash power down key1 */
#define FLASH_PDKEY2               ((uint32_t)0xFAFBFCFD) /*!< Flash power down key2: used with FLASH_PDKEY1
                                                              to unlock the RUN_PD bit in FLASH_ACR */

#define FLASH_PEKEY1               ((uint32_t)0x89ABCDEF) /*!< Flash program erase key1 */
#define FLASH_PEKEY2               ((uint32_t)0x02030405) /*!< Flash program erase key: used with FLASH_PEKEY2
                                                               to unlock the write access to the FLASH_PECR register and
                                                               data EEPROM */

#define FLASH_PRGKEY1              ((uint32_t)0x8C9DAEBF) /*!< Flash program memory key1 */
#define FLASH_PRGKEY2              ((uint32_t)0x13141516) /*!< Flash program memory key2: used with FLASH_PRGKEY2
                                                               to unlock the program memory */

#define FLASH_OPTKEY1              ((uint32_t)0xFBEAD9C8) /*!< Flash option key1 */
#define FLASH_OPTKEY2              ((uint32_t)0x24252627) /*!< Flash option key2: used with FLASH_OPTKEY1 to
                                                              unlock the write access to the option byte block */

// Flash program erase control register (FLASH_PECR) definitions
#define FLASH_PELOCK        0x00000001          // FLASH_PECR and data memory lock
#define FLASH_PRGLOCK       0x00000002          // Program memory lock
#define FLASH_OPTLOCK       0x00000004          // Option bytes block lock
#define FLASH_PROG          0x00000008          // Program memory selection
#define FLASH_DATA          0x00000010          // Data memory selection
#define FLASH_OPT           0x00000020          // Option Bytes memory selection
#define FLASH_FIX           0x00000100          // Fixed time data write for Byte, Half Word and Word programming
#define FLASH_ERASE         0x00000200          // Page or Double Word erase mode
#define FLASH_FPRG          0x00000400          // Half Page/Double Word programming mode
#define FLASH_GBHF_ER       0x00000800          // Global Half Erase mode

// Flash status register (FLASH_SR) definitions
#define FLASH_BSY           0x00000001          // Write/erase operations in progress
#define FLASH_EOP           0x00000002          // End of operation
#define FLASH_ENDHV         0x00000004          // End of high voltage
#define FLASH_WRPERR        0x00000100          // Write protected error
#define FLASH_PGAERR        0x00000200          // Programming alignment error
#define FLASH_SIZERR        0x00000400          // Size error
#define FLASH_OPTVERR       0x00000800          // Option validity error

#define FLASH_ERRs         (FLASH_PGAERR | FLASH_WRPERR | FLASH_SIZERR | FLASH_OPTVERR)

void flash_unlock(void)
{
    FLASH->SR |= FLASH_ERRs;                  // clear error flags

    if ((FLASH->PECR & FLASH_PECR_PELOCK) != 0){
        // Unlock PECR Register
        FLASH->PEKEYR = FLASH_PEKEY1;
        FLASH->PEKEYR = FLASH_PEKEY2;
    }

    if ((FLASH->PECR & FLASH_PECR_PRGLOCK) != 0){
        // Unlock Program Matrix
        FLASH->PRGKEYR = FLASH_PRGKEY1;
        FLASH->PRGKEYR = FLASH_PRGKEY2;
    }
}

uint32_t flash_erase_page(uint32_t Page_Address)
{
    flash_unlock();

    FLASH->PECR |= FLASH_ERASE;                   // Page or Double Word Erase enabled
    FLASH->PECR |= FLASH_PROG;                    // Program memory selected

    *(uint32_t *)(Page_Address) = 0x00000000;                 // write '0' to the first address to erase page

    while (FLASH->SR & FLASH_BSY) {
        /* wait until sector erase finish*/
    }

    FLASH->PECR &= ~FLASH_ERASE;                  // Page or Double Word Erase disabled
    FLASH->PECR &= ~FLASH_PROG;                   // Program memory deselected

    return (0);                                   // Done
}

#if defined ( __CC_ARM   )
/* ARM Compiler
   ------------
   RAM functions are defined using the toolchain options.
   Functions that are executed in RAM should reside in a separate source module.
   Using the 'Options for File' dialog you can simply change the 'Code / Const'
   area of a module to a memory space in physical RAM.
   Available memory areas are declared in the 'Target' tab of the 'Options for Target'
   dialog.
*/
#define __RAM_FUNC

#elif defined ( __ICCARM__ )
/* ICCARM Compiler
   ---------------
   RAM functions are defined using a specific toolchain keyword "__ramfunc".
*/
#define __RAM_FUNC __ramfunc

#elif defined   (  __GNUC__  )
/* GNU Compiler
   ------------
  RAM functions are defined using a specific toolchain attribute
   "__attribute__((section(".RamFunc")))".
*/
#define __RAM_FUNC __attribute__((section(".RamFunc")))

#endif

__RAM_FUNC int flash_program_page(unsigned long pageAddr,
                      unsigned long size,
                      unsigned char *buf)
{
	  unsigned long  cnt = 64;

	  size = (size + 127) & ~127;            // adjust programming size

	  FLASH->PECR |= FLASH_FPRG;                    // Half Page programming mode enabled
	  FLASH->PECR |= FLASH_PROG;                    // Program memory selected

	  // program first half page
	  cnt = 64;
	  while (size && cnt) {
	     *(uint32_t *)(pageAddr) = *((uint32_t *)buf);        // Program Word
	     pageAddr += 4;
	     buf += 4;
	     size  -= 4;
	     cnt -= 4;
	  }

	  while (FLASH->SR & FLASH_BSY) {
	    //IWDG->KR = 0xAAAA;                          // Reload IWDG
	  }

	  // Check for Errors
	  if (FLASH->SR & (FLASH_ERRs)) {
	    FLASH->SR |= FLASH_ERRs;                    // clear error flags
	    return (1);                                 // Failed
	  }

	  FLASH->PECR &= ~FLASH_FPRG;                   // Half Page programming mode disabled
	  FLASH->PECR &= ~FLASH_PROG;                   // Program memory deselected

	  FLASH->PECR |= FLASH_FPRG;                    // Half Page programming mode enabled
	  FLASH->PECR |= FLASH_PROG;                    // Program memory selected

	  // program second half page
	  cnt = 64;
	  while (size && cnt) {
	    (*(uint32_t *)(pageAddr)) = *((uint32_t *)buf);         // Program Word
	    pageAddr += 4;
	    buf += 4;
	    size  -= 4;
	    cnt -= 4;
	  }

	  while (FLASH->SR & FLASH_BSY) {
	    //IWDG->KR = 0xAAAA;                          // Reload IWDG
	  }

	  // Check for Errors
	  if (FLASH->SR & (FLASH_ERRs)) {
	    FLASH->SR |= FLASH_ERRs;                    // clear error flags
	    return (1);                                 // Failed
	  }

	  FLASH->PECR &= ~FLASH_FPRG;                   // Half Page programming mode disabled
	  FLASH->PECR &= ~FLASH_PROG;                   // Program memory deselected

	  return (0);                                   // Done
}
