/* -*- Mode: C -*-
 *
 * $Id: system.h,v 1.2 2007/09/06 06:09:20 jdesch Exp $
 * --------------------------------------------------------------------------
 * Copyright  (c) Dipl.-Ing. Joerg Desch
 * --------------------------------------------------------------------------
 * PROJECT: ezROBI Mega32
 * MODULE.: SYSTEM.H:  low level code
 * AUTHOR.: jdesch
 * --------------------------------------------------------------------------
 * DESCRIPTION:
 *
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

#ifndef __SYSTEM_H__
#define __SYSTEM_H__ 1

#include "defs.h"
#include "config.h"
#include "platform.h"




/*+=========================================================================+*/
/*|                      CONSTANT AND MACRO DEFINITIONS                     |*/
/*`========================================================================='*/

/* The key code
 */
#define SYS_SW1_KEY 0x01
#define SYS_SW2_KEY 0x02
#define SYS_SW3_KEY 0x04


/*             .-----------------------------------------------.             */
/* ___________/  local macro declaration                        \___________ */
/*            `-------------------------------------------------'            */

/*+=========================================================================+*/
/*|                            TYPEDECLARATIONS                             |*/
/*`========================================================================='*/
/* {{{ */

/* This is the type to hold all the keypad bits. 
 */
typedef BYTE T_Key;


/* }}} */

/*+=========================================================================+*/
/*|                            PUBLIC VARIABLES                             |*/
/*`========================================================================='*/

/*+=========================================================================+*/
/*|                     PROTOTYPES OF GLOBAL FUNCTIONS                      |*/
/*`========================================================================='*/

void sysInitHardware (void);
T_Key sysReadRawKeypad (void);

/* on-board LEDs */
void sysSetGreenLED (BOOL enable);
void sysSetRedLED (BOOL enable);
void sysSetYellowLED (BOOL enable);



/*             .-----------------------------------------------.             */
/* ___________/  Group...                                       \___________ */
/*            `-------------------------------------------------'            */


#endif
/* ==[End of file]========================================================== */

