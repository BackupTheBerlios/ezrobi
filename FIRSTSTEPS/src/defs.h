/* -*- Mode: C -*-
 *
 * $Id: defs.h,v 1.1 2007/08/29 12:38:05 jdesch Exp $
 * --------------------------------------------------------------------------
 * Copyright  (c) Dipl.-Ing. Joerg Desch
 * --------------------------------------------------------------------------
 * PROJECT: ezROBI Mega32
 * MODULE.: DEFS.H: global declarations and definitions
 * AUTHOR.: jdesch
 * --------------------------------------------------------------------------
 * DESCRIPTION:
 *
 * Some very important (mostly) internally used declarations and 
 * definitions. There is (normaly) no need to change something here.
 *
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * --------------------------------------------------------------------------
 * COMPILER-FLAGS:
 *
 * --------------------------------------------------------------------------
 */

#ifndef __DEFS_H__
#define __DEFS_H__ 1

#include <avr/pgmspace.h> // need PSTR() and maybe prog_char


/*+=========================================================================+*/
/*|                      CONSTANT AND MACRO DEFINITIONS                     |*/
/*`========================================================================='*/

/*             .-----------------------------------------------.             */
/* ___________/  declaration of MCU specific macrs              \___________ */
/*            `-------------------------------------------------'            */

#ifdef __GNUC__
#define ATTR_PACKED __attribute__((__packed__))
#else 
#define ATTR_PACKED
#endif

/* portable definition of a constant char pointer placed in the flash area
 * of the MCU. This topic is a little bit complicated. 
 *
 * (1) GNU-C can't use the const identifier together with the attribute
 * progmem inside typedefs!
 *  
 * (2) GNU-C can't use the attribute progmem within function parameter
 * declarations! Thats my experience.
 *
 * So we need to declare strings as #const flash_chr xx[]# and pass them to
 * function parameter declared as #flash_str#. #flash_str# doesn't need to
 * have the PROGMEM attribute. It's a simple pointer and the called
 * function must know how to handle it.
 * 
 * CodeVision is much smarter. It uses the attribute flash to do all the
 * stuff. It uses const also as flag attribute, which is incompatible to
 * ANSI-C standard and to most `good code'. ;-) So I've tried to map it.
 */
#ifdef __GNUC__
typedef const char* flash_str;
//typedef char flash_chr __attribute__((__progmem__));
typedef prog_char flash_chr; // needs include <avr/pgmspace.h>
#elif __CODEVISIONAVR__
typedef char flash flash_chr;
typedef char flash* flash_str;
#endif

/* portable definition of a constant char pointer placed in the RAM area of the
 * MCU. The CodeVision-AVR moves "const" variables into the FLASH! So we must
 * ignore it.
 */
#ifdef __GNUC__
typedef const char* const_str;
#elif __CODEVISIONAVR__
typedef char* const_str;
#endif


/* Theres no need to have a PSTR macro for strings. CodeVision put them allways
 * into the flash mem!
 */
#ifdef __CODEVISIONAVR__
#define PSTR(s) s
#endif


#ifndef outb
#define	outb(addr, data)	addr = (data)
#endif
#ifndef inb
#define	inb(addr)		(addr)
#endif
#ifndef BV
#define BV(bit)			(1<<(bit))
#endif
#ifndef cbi
#define cbi(reg,bit)	        reg &= ~(BV(bit))
#endif
#ifndef sbi
#define sbi(reg,bit)	        reg |= (BV(bit))
#endif
#ifdef __GNUC__
  #ifndef cli
    #define cli()		__asm__ __volatile__ ("cli" ::)
  #endif
  #ifndef sei
    #define sei()		__asm__ __volatile__ ("sei" ::)
  #endif
#else
  #define cli()			#asm("cli")
  #define sei()			#asm("sei")
#endif


/*             .-----------------------------------------------.             */
/* ___________/  declaration of general macros                  \___________ */
/*            `-------------------------------------------------'            */


/* following macro gives the best code for single bits to be toggled
 * All other arithmetic constructs will always cast to integer and not use
 * bit instructions.
 * Applies to CC907 and FCC907S compilers
 */
#define TOGGLEBIT(b) {if (b) b=0; else b=1;}

#define BITVAL(a) (1<<a)

/* This macro converts a sequence of 0 and 1 into a BYTE. The sequence start
 * with the MSB! Make shure that you allways define all 8 bits!
 * Sample: BIN2BYTE(11001010) is converted to 0xCA (1100->C & 1010->A).
 */
#define BIN2BYTE(a) ( ((0x##a##L>>21) & 0x80) + ((0x##a##L>>18) & 0x40) \
                    + ((0x##a##L>>15) & 0x20) + ((0x##a##L>>12) & 0x10) \
                    + ((0x##a##L>>9) & 0x08) + ((0x##a##L>>6) & 0x04)  \
                    + ((0x##a##L>>3) & 0x02) + (0x##a##L & 0x01))

/*#define BIN2WORD(h,l) ( (BIN2BYTE(h) << 16) + BIN2BYTE(l)) */

#define MAXVAL(x,y) ( (x)>(y) ? (x):(y) )
#define MINVAL(x,y) ( (x)<(y) ? (x):(y) )

#define LOBYTE(x) ((x)&0x00FF)
#define HIBYTE(x) (((x)&0xFF00)>>8)


/*             .-----------------------------------------------.             */
/* ___________/  Compiler Portability Issues                    \___________ */
/*            `-------------------------------------------------'            */
/* {{{ */


/* Structure to access the bits of an byte.
 */
struct tagBITADDRESSABLEBYTE
{
	char bit0:1;
	char bit1:1;
	char bit2:1;
	char bit3:1;
	char bit4:1;
	char bit5:1;
	char bit6:1;
	char bit7:1;
} ATTR_PACKED;


/* The AVR MCU doesn't use a direct access to an single pin. As we want to
 * declare a pin with a single define, we need this feature. One possible
 * solution is the usage of bit-fields. We have to cast the SFR address to an
 * pointer to a bitfield. Than we can access a bit with #___PIN(PORTA).bit0#!
 *
 * The "official way" would be the declaration of the pin number and may be
 * the alias of an port. So we have to use #sbi(port_LED1,out_LED1)#.
 *
 * The CodeVision compiler distinguishs between SFR and memory mapped SFRs. The
 * last one can't be accessed as normal SFR. So I've implemented the __MPIN
 * macro. For avr-gcc, there is no such distinction!
 */
#define ___PIN(___port) (*(struct tagBITADDRESSABLEBYTE*)(_SFR_ADDR(___port)))

/* There are (at least now) two different ways to directly access the port
 * pins. To hide this ways, I've defined this macro. Just use
 *
 *   __PIN(PORTG,0)=1;
 *
 */
#ifdef __GNUC__
#define __PIN(port__,bit__) ___PIN(port__).bit##bit__
#define __MPIN(port__,bit__) ___PIN(port__).bit##bit__
#endif
#ifdef __CODEVISIONAVR__
#define __PIN(port__,bit__) port__.##bit__
#define __MPIN(port__,bit__) (*(struct tagBITADDRESSABLEBYTE*)(&(port__))).bit##bit__
#endif

/* }}} */



/*+=========================================================================+*/
/*|                            TYPEDECLARATIONS                             |*/
/*`========================================================================='*/

/* define some really elementary (platform dependent) data types.
 */
typedef unsigned char  BYTE;
typedef unsigned short WORD;
typedef unsigned long  DWORD;

#ifndef TRUE
typedef unsigned char BOOL;
#define TRUE 1 /* better would be !FALSE */
#define FALSE 0
#endif



#endif
/* ==[End of file]========================================================== */

