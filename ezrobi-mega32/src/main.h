/* -*- Mode: C -*-
 *
 * $Id: main.h,v 1.1 2007/08/29 12:42:24 jdesch Exp $
 * --------------------------------------------------------------------------
 * Copyright  (c) Dipl.-Ing. Joerg Desch
 * --------------------------------------------------------------------------
 * PROJECT: ezROBI Mega32
 * MODULE.: MAIN.H: the main header for the application.
 * AUTHOR.: jdesch
 * --------------------------------------------------------------------------
 * DESCRIPTION:
 *
 * This is the main header of the application. It is the glue bewteen
 * the modules, the main file and the "real" user application.
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

#ifndef __MAIN_H__
#define __MAIN_H__ 1

#include "config.h"
#include "system.h"
#include "v24_single.h"
#include "swt_single.h"


/*+=========================================================================+*/
/*|                      CONSTANT AND MACRO DEFINITIONS                     |*/
/*`========================================================================='*/

/*+=========================================================================+*/
/*|                            TYPEDECLARATIONS                             |*/
/*`========================================================================='*/

/*+=========================================================================+*/
/*|                            PUBLIC VARIABLES                             |*/
/*`========================================================================='*/

/*+=========================================================================+*/
/*|                     PROTOTYPES OF GLOBAL FUNCTIONS                      |*/
/*`========================================================================='*/


/*             .-----------------------------------------------.             */
/* ___________/  EXPORTED FROM MAIN.C                           \___________ */
/*            `-------------------------------------------------'            */


/*             .-----------------------------------------------.             */
/* ___________/  EXPORTED FROM APPLICATION.C                    \___________ */
/*            `-------------------------------------------------'            */

void app_SystemStarted (void);

void app_Idle (void);

BOOL app_HandleCommand (BYTE cmd, WORD* args, WORD cnt);

void app_HandleKeypress (BYTE key);

void app_HandleEvent (BYTE event);


#endif
/* ==[End of file]========================================================== */

