/* 
   Copyright (c) 2002-2003, Eric B. Weddington
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:

   * Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.
   * Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in
     the documentation and/or other materials provided with the
     distribution.
   * Neither the name of the copyright holders nor the names of
     contributors may be used to endorse or promote products derived
     from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE. 
*/


/** \defgroup avr_boot Bootloader Support Utilities
    \code
    #include <avr/io.h>
    #include <avr/boot.h>
    \endcode

    The macros in this module provide a C language interface to the
	bootloader support functionality of certain AVR processors. These
	macros are designed to work with all sizes of flash memory.

    \note Not all AVR processors provide bootloader support. See your processor 
	datasheet to see if it provides bootloader support.*/

/*
TODO:
From email with Marek:
On smaller devices (all except ATmega64/128), SPMCR is in the I/O
space, accessible with the shorter "in" and "out" instructions -
since the boot loader has a limited size, this could be an
important optimization.
*/



#ifndef _AVR_BOOT_H_
#define _AVR_BOOT_H_	1


#include <avr/eeprom.h>
#include <avr/io.h>
#include <limits.h>



/* Check for SPM Control Register in processor. */
#ifdef SPMCSR
	#define _SPM_REG	SPMCSR
#else
	#ifdef SPMCR
		#define _SPM_REG	SPMCR
	#else
		#error AVR processor does not provide bootloader support!
	#endif
#endif

/** \ingroup avr_pgmspace
    \def BOOTLOADER_SECTION

    Used to declare a function or variable to be placed into a
	new section called .bootloader. This section and its contents
	can then be relocated to any address (such as the bootloader
	NRWW area) at link-time. */

#define BOOTLOADER_SECTION    __attribute__ ((section (".bootloader")))


/** \ingroup avr_pgmspace
    \def boot_spm_interrupt_enable()
    Enable the SPM interrupt. */

#define boot_spm_interrupt_enable()     (SPMCR |= (unsigned char)_BV(SPMIE))

/** \ingroup avr_pgmspace
    \def boot_spm_interrupt_disable()
    Disable the SPM interrupt. */

#define boot_spm_interrupt_disable()    (SPMCR &= (unsigned char)~_BV(SPMIE))

/** \ingroup avr_pgmspace
    \def boot_is_spm_interrupt()
    Check if the SPM interrupt is enabled. */

#define boot_is_spm_interrupt()         (SPMCR & (unsigned char)_BV(SPMIE))

/** \ingroup avr_pgmspace
    \def boot_rww_busy()
    Check if the RWW section is busy. */

#define boot_rww_busy()                 (SPMCR & (unsigned char)_BV(RWWSB))

/** \ingroup avr_pgmspace
    \def boot_spm_busy()
    Check if the SPM instruction is busy. */

#define boot_spm_busy()                 (SPMCR & (unsigned char)_BV(SPMEN))

/** \ingroup avr_pgmspace
    \def boot_spm_busy_wait()
    Wait while the SPM instruction is busy. */

#define boot_spm_busy_wait()            do{}while(boot_spm_busy())


#define __eeprom_write_busy()           (EECR & _BV(EEWE))

#define __eeprom_write_busy_wait()      do{}while(__eeprom_write_busy())


#define BOOT_PAGE_ERASE		(_BV(SPMEN) | _BV(PGERS))
#define BOOT_PAGE_WRITE		(_BV(SPMEN) | _BV(PGWRT))
#define BOOT_PAGE_FILL		_BV(SPMEN)
#define BOOT_RWW_ENABLE		(_BV(SPMEN) | _BV(RWWSRE))
#define BOOT_LOCK_BITS_SET	(_BV(SPMEN) | _BV(BLBSET))





#define __boot_page_fill_normal(address, data)       \
(                                                    \
    {                                                \
        boot_spm_busy_wait();                        \
		while(!eeprom_is_ready());                   \
        __asm__ __volatile__                         \
        (                                            \
            "movw  r0, %3\n\t"                       \
            "movw r30, %2\n\t"                       \
            "sts %0, %1\n\t"                         \
            "spm\n\t"                                \
            "clr  r1\n\t"                            \
            : "=m" (_SPM_REG)                        \
            : "r" ((unsigned char)BOOT_PAGE_FILL),   \
              "r" ((unsigned short)address),         \
              "r" ((unsigned short)data)             \
            : "r0", "r30", "r31"                     \
        );                                           \
    }                                                \
)


#define __boot_page_fill_alternate(address, data)    \
(                                                    \
    {                                                \
        boot_spm_busy_wait();                        \
		while(!eeprom_is_ready());                   \
        __asm__ __volatile__                         \
        (                                            \
            "movw  r0, %3\n\t"                       \
            "movw r30, %2\n\t"                       \
            "sts %0, %1\n\t"                         \
            "spm\n\t"                                \
            ".word 0xffff\n\t"                       \
            "nop\n\t"                                \
            "clr  r1\n\t"                            \
            : "=m" (_SPM_REG)                        \
            : "r" ((unsigned char)BOOT_PAGE_FILL),   \
              "r" ((unsigned short)address),         \
              "r" ((unsigned short)data)             \
            : "r0", "r30", "r31"                     \
        );                                           \
    }                                                \
)

#define __boot_page_fill_extended(address, data)     \
(                                                    \
    {                                                \
        boot_spm_busy_wait();                        \
		while(!eeprom_is_ready());                   \
        __asm__ __volatile__                         \
        (                                            \
            "movw  r0, %4\n\t"                       \
            "movw r30, %A3\n\t"                      \
            "sts %1, %C3\n\t"                        \
            "sts %0, %2\n\t"                         \
            "spm\n\t"                                \
            "clr  r1\n\t"                            \
            : "=m" (_SPM_REG),                       \
              "=m" (RAMPZ)                           \
            : "r" ((unsigned char)BOOT_PAGE_FILL),   \
              "r" ((unsigned long)address),          \
              "r" ((unsigned short)data)             \
            : "r0", "r30", "r31"                     \
        );                                           \
    }                                                \
)

#define __boot_page_erase_normal(address)            \
(                                                    \
    {                                                \
        boot_spm_busy_wait();                        \
		while(!eeprom_is_ready());                   \
        __asm__ __volatile__                         \
        (                                            \
            "movw r30, %2\n\t"                       \
            "sts %0, %1\n\t"                         \
            "spm\n\t"                                \
            : "=m" (_SPM_REG)                        \
            : "r" ((unsigned char)BOOT_PAGE_ERASE),  \
              "r" ((unsigned short)address)          \
            : "r30", "r31"                           \
        );                                           \
    }                                                \
)


#define __boot_page_erase_alternate(address)         \
(                                                    \
    {                                                \
        boot_spm_busy_wait();                        \
		while(!eeprom_is_ready());                   \
        __asm__ __volatile__                         \
        (                                            \
            "movw r30, %2\n\t"                       \
            "sts %0, %1\n\t"                         \
            "spm\n\t"                                \
            ".word 0xffff\n\t"                       \
            "nop\n\t"                                \
            : "=m" (_SPM_REG)                        \
            : "r" ((unsigned char)BOOT_PAGE_ERASE),  \
              "r" ((unsigned short)address)          \
            : "r30", "r31"                           \
        );                                           \
    }                                                \
)



#define __boot_page_erase_extended(address)          \
(                                                    \
    {                                                \
        boot_spm_busy_wait();                        \
		while(!eeprom_is_ready());                   \
        __asm__ __volatile__                         \
        (                                            \
            "movw r30, %A3\n\t"                      \
            "sts  %1, %C3\n\t"                       \
            "sts %0, %2\n\t"                         \
            "spm\n\t"                                \
            : "=m" (_SPM_REG),                       \
              "=m" (RAMPZ)                           \
            : "r" ((unsigned char)BOOT_PAGE_ERASE),  \
              "r" ((unsigned long)address)           \
            : "r30", "r31"                           \
        );                                           \
    }                                                \
)


#define __boot_page_write_normal(address)            \
(                                                    \
    {                                                \
        boot_spm_busy_wait();                        \
		while(!eeprom_is_ready());                   \
        __asm__ __volatile__                         \
        (                                            \
            "movw r30, %2\n\t"                       \
            "sts %0, %1\n\t"                         \
            "spm\n\t"                                \
            : "=m" (_SPM_REG)                        \
            : "r" ((unsigned char)BOOT_PAGE_WRITE),  \
              "r" ((unsigned short)address)          \
            : "r30", "r31"                           \
        );                                           \
    }                                                \
)


#define __boot_page_write_alternate(address)         \
(                                                    \
    {                                                \
        boot_spm_busy_wait();                        \
		while(!eeprom_is_ready());                   \
        __asm__ __volatile__                         \
        (                                            \
            "movw r30, %2\n\t"                       \
            "sts %0, %1\n\t"                         \
            "spm\n\t"                                \
            ".word 0xffff\n\t"                       \
            "nop\n\t"                                \
            : "=m" (_SPM_REG)                        \
            : "r" ((unsigned char)BOOT_PAGE_WRITE),  \
              "r" ((unsigned short)address)          \
            : "r30", "r31"                           \
        );                                           \
    }                                                \
)



#define __boot_page_write_extended(address)          \
(                                                    \
    {                                                \
        boot_spm_busy_wait();                        \
		while(!eeprom_is_ready());                   \
        __asm__ __volatile__                         \
        (                                            \
            "movw r30, %A3\n\t"                      \
            "sts %1, %C3\n\t"                        \
            "sts %0, %2\n\t"                         \
            "spm\n\t"                                \
            : "=m" (_SPM_REG),                       \
              "=m" (RAMPZ)                           \
            : "r" ((unsigned char)BOOT_PAGE_WRITE),  \
              "r" ((unsigned long)address)           \
            : "r30", "r31"                           \
        );                                           \
    }                                                \
)


#define __boot_rww_enable()                          \
(                                                    \
    {                                                \
        boot_spm_busy_wait();                        \
		while(!eeprom_is_ready());                   \
        __asm__ __volatile__                         \
        (                                            \
            "sts %0, %1\n\t"                         \
            "spm\n\t"                                \
            : "=m" (_SPM_REG)                        \
            : "r" ((unsigned char)BOOT_RWW_ENABLE)   \
        );                                           \
    }                                                \
)


#define __boot_rww_enable_alternate()                \
(                                                    \
    {                                                \
        boot_spm_busy_wait();                        \
		while(!eeprom_is_ready());                   \
        __asm__ __volatile__                         \
        (                                            \
            "sts %0, %1\n\t"                         \
            "spm\n\t"                                \
            ".word 0xffff\n\t"                       \
            "nop\n\t"                                \
            : "=m" (_SPM_REG)                        \
            : "r" ((unsigned char)BOOT_RWW_ENABLE)   \
        );                                           \
    }                                                \
)


#define __boot_lock_bits_set(lock_bits)                        \
(                                                              \
    {                                                          \
        unsigned char value = (unsigned char)lock_bits | 0xC3; \
        boot_spm_busy_wait();                                  \
		while(!eeprom_is_ready());                             \
        __asm__ __volatile__                                   \
        (                                                      \
            "ldi r30, 1\n\t"                                   \
            "ldi r31, 0\n\t"                                   \
            "mov r0, %2\n\t"                                   \
            "sts %0, %1\n\t"                                   \
            "spm\n\t"                                          \
            : "=m" (_SPM_REG)                                  \
            : "r" ((unsigned char)BOOT_LOCK_BITS_SET),         \
              "r" (value)                                      \
            : "r0", "r30", "r31"                               \
        );                                                     \
    }                                                          \
)


#define __boot_lock_bits_set_alternate(lock_bits)              \
(                                                              \
    {                                                          \
        unsigned char value = (unsigned char)lock_bits | 0xC3; \
        boot_spm_busy_wait();                                  \
		while(!eeprom_is_ready());                             \
        __asm__ __volatile__                                   \
        (                                                      \
            "ldi r30, 1\n\t"                                   \
            "ldi r31, 0\n\t"                                   \
            "mov r0, %2\n\t"                                   \
            "sts %0, %1\n\t"                                   \
            "spm\n\t"                                          \
            ".word 0xffff\n\t"                                 \
            "nop\n\t"                                          \
            : "=m" (_SPM_REG)                                  \
            : "r" ((unsigned char)BOOT_LOCK_BITS_SET),         \
              "r" (value)                                      \
            : "r0", "r30", "r31"                               \
        );                                                     \
    }                                                          \
)


#define __boot_mem_read_normal(address)        \
(                                            \
    {                                        \
        unsigned char result;                \
        __asm__ __volatile__                 \
        (                                    \
            "movw r30, %1\n\t"               \
            "lpm %0, Z\n\t"                  \
            : "=r" ((unsigned char)result)   \
            : "r" ((unsigned short)address)  \
            : "r30", "r31"                   \
        );                                   \
        result;                              \
    }                                        \
)

#define __boot_mem_read_extended(address)      \
(                                            \
    {                                        \
        unsigned char result;                \
        __asm__ __volatile__                 \
        (                                    \
            "movw r30, %A2\n\t"              \
            "sts  %1, %C2\n\t"               \
            "elpm %0, Z\n\t"                 \
            : "=r" ((unsigned char)result),  \
              "=m" (RAMPZ)                   \
            : "r" ((unsigned long)address)   \
            : "r30", "r31"                   \
        );                                   \
        result;                              \
    }                                        \
)

/** \ingroup avr_pgmspace
    \fn boot_page_fill(address, data)

    Fill the bootloader temporary page buffer for flash 
	address with data word. 
	
	\note The address is a byte address. The data is a word. The AVR 
	writes data to the buffer a word at a time, but addresses the buffer
	per byte! So, increment your address by 2 between calls, and send 2
	data bytes in a word format! The LSB of the data is written to the lower 
	address; the MSB of the data is written to the higher address.*/
	
/** \ingroup avr_pgmspace
    \fn boot_page_erase(address)

    Erase the flash page that contains address.

	\note address is a byte address in flash, not a word address. */

	
/** \ingroup avr_pgmspace
    \fn boot_page_write(address)

	Write the bootloader temporary page buffer 
	to flash page that contains address.
	
	\note address is a byte address in flash, not a word address. */


/** \ingroup avr_pgmspace
    \fn boot_mem_read(address)

    Read a byte from flash at address.

	\note address is a byte address in flash, not a word address. */

	


/*
Normal versions of the macros use 16-bit addresses.
Extended versions of the macros use 32-bit addresses.
Alternate versions of the macros use 16-bit addresses and require special instruction
   sequences after LPM.

FLASHEND is defined in the ioXXXX.h file.
USHRT_MAX is defined in <limits.h>.
*/ 

#if defined(__AVR_ATmega161__) || defined(__AVR_ATmega163__) || defined(__AVR_ATmega323__)

    /* Alternate = ATmega161/163/323 and 16 bit address */
	#define boot_page_fill(address, data)   __boot_page_fill_alternate(address, data)
	#define boot_page_erase(address)        __boot_page_erase_alternate(address)
	#define boot_page_write(address)        __boot_page_write_alternate(address)
    #define boot_rww_enable()               __boot_rww_enable_alternate()
    #define boot_lock_bits_set(lock_bits)   __boot_lock_bits_set_alternate(lock_bits)
    
#elif (FLASHEND > USHRT_MAX)

	/* Extended = >16 bit address */
	#define boot_page_fill(address, data)   __boot_page_fill_extended(address, data)
	#define boot_page_erase(address)        __boot_page_erase_extended(address)
	#define boot_page_write(address)        __boot_page_write_extended(address)
    #define boot_rww_enable()               __boot_rww_enable()
    #define boot_lock_bits_set(lock_bits)   __boot_lock_bits_set(lock_bits)
	#define boot_mem_read(address)	__boot_mem_read_extended(address)
    
#else

	/* Normal = 16 bit address */
	#define boot_page_fill(address, data)   __boot_page_fill_normal(address, data)
	#define boot_page_erase(address)        __boot_page_erase_normal(address)
	#define boot_page_write(address)        __boot_page_write_normal(address)
    #define boot_rww_enable()               __boot_rww_enable()
    #define boot_lock_bits_set(lock_bits)   __boot_lock_bits_set(lock_bits)
	#define boot_mem_read(address)	__boot_mem_read_normal(address)
    
#endif


/* Example bootloader test function for the ATmega128.
#define ADDRESS		0x1C000UL
static void boot_test(void)
{
    unsigned char buffer[8];
	
    cli();

    // Erase page.
    boot_page_erase((unsigned long)ADDRESS);
    while(boot_rww_busy())
    {
        boot_rww_enable();
    }


    // Write data to buffer a word at a time. Note incrementing address by 2.
    for(unsigned long i = ADDRESS; i < ADDRESS + SPM_PAGESIZE; i += 2)
    {
        boot_page_fill(i, (i-ADDRESS) + ((i-ADDRESS+1) << 8));
    }


    // Write page.
    boot_page_write((unsigned long)ADDRESS);
    while(boot_rww_busy())
    {
        boot_rww_enable();
    }
	
    sei();


    // Read back the values and display.
    for(unsigned long i = ADDRESS; i < ADDRESS + 256; i++)
    {
        show(utoa(pgm_read_byte(i), buffer, 16));
    }
	
    return;
}
*/



#endif


