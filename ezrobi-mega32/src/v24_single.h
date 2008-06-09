/* -*- Mode: C -*-
 *
 * $Id: v24_single.h,v 1.2 2008/06/09 07:55:04 jdesch Exp $
 * --------------------------------------------------------------------------
 * Copyright  (c) Dipl.-Ing. Joerg Desch
 * --------------------------------------------------------------------------
 * PROJECT: ezROBI Mega32
 * MODULE.: V24-SINGLE.H: single UART module
 * AUTHOR.: jdesch
 * --------------------------------------------------------------------------
 * DESCRIPTION:
 *
 * This module implements the handling of exactly one serial port (UART).
 * The setup of the port and the serial parameters is done in system.c 
 * (sysInitHardware). You can control the baudrate in config.h.
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

#ifndef __V24_SINGLE_H__
#define __V24_SINGLE_H__ 1

//#include <stddef.h>

#include "config.h"          /* defines the hardware we use */





/*+=========================================================================+*/
/*|                      CONSTANT AND MACRO DEFINITIONS                     |*/
/*`========================================================================='*/


/* The following declarations are a mapping of the baudrate selection
 * parameter. This is necessary to avoid uncommon baudrates.
 */
//@{
#define V24_B1200   1200L      /* 1200 baud */
#define V24_B2400   2400L      /* 2400 baud */
#define V24_B4800   4800L      /* 4800 baud */
#define V24_B9600   9600L      /* 9600 baud */
#define V24_B19200 19200L      /* 19200 baud */
#define V24_B38400 38400L      /* 38400 baud */
//@}


/*             .-----------------------------------------------.             */
/* ___________/  local macro declaration                        \___________ */
/*            `-------------------------------------------------'            */

/*+=========================================================================+*/
/*|                            TYPEDECLARATIONS                             |*/
/*`========================================================================='*/


typedef unsigned char v24_port_t;


enum __V24_ERROR_CODES
{
    V24_E_NONE=0,
    V24_E_ILLHANDLE,
    V24_E_ILLBAUD,
    V24_E_ILLDATASZ,
    V24_E_ILLPARITY,
    V24_E_FRAME,
    V24_E_OVERFLOW,
    V24_E_TIMEOUT,
    V24_E_ILLEGAL,
    V24_E_NOT_OPEN,
    V24_E_NOT_IMPLEMENTED=100
};


/** The open flags control some "special behaviour" of the v24 module. The
 * constants here must be declares as bitmask. Therefore they must have 2^x
 * values (1,2,4,8,16..)
 */
enum __V24_OPEN_FLAGS
{
    V24_OF_DEFAULT=0,
    V24_OF_WAITFORTX=1,   // bit0: putc waits til char is sent.
    // V24_OF_DISABLERXWHILETX=2,
    V24_OF_MAXOPENFLAGS
};


enum __V24_DATASIZE
{
    V24_7BIT=0,
    V24_8BIT,
    V24_NUM_DATASIZES          /* number of datasize values */
};

enum __V24_PARITY_FLAGS
{
    V24_NONE=0,            /* disable parity bit */
    V24_EVEN,            /* even parity */
    V24_ODD,             /* odd parity */
    V24_NUM_PARITIES           /* number of parity values */
};


/*+=========================================================================+*/
/*|                            PUBLIC VARIABLES                             |*/
/*`========================================================================='*/

/*+=========================================================================+*/
/*|                     PROTOTYPES OF GLOBAL FUNCTIONS                      |*/
/*`========================================================================='*/

void v24InitModule (void);
BOOL v24SetTimeouts ( int TenthOfSeconds );
int  v24Getc ( void );
int  v24GetQueue ( void );
BOOL v24Putc ( unsigned char TheData );
int  v24Gets ( char* Buffer, int BuffSize );
int  v24Puts ( const_str Buffer );
int  v24PutsP ( flash_str PtrBuffer );
BOOL v24HaveData ( void );
BOOL v24FlushRxQueue ( void );
int  v24SetDTR ( int NewState );
BOOL v24TimedOut ( void );
BYTE v24QueryErrno ( void );

#if V24_NEED_PUT_UNSIGNED
void v24PutUnsigned ( WORD num );
#endif
#if V24_NEED_PUT_WORD
void v24PutWord ( WORD num );
#endif
#if V24_NEED_PUT_BYTE
void v24PutByte ( BYTE num );
#endif


/*             .-----------------------------------------------.             */
/* ___________/  Group...                                       \___________ */
/*            `-------------------------------------------------'            */

#endif
/* ==[End of file]========================================================== */

