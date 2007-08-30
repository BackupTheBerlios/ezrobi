/* -*- Mode: C -*-
 *
 * $Id: motor_ctrl.h,v 1.1 2007/08/30 16:06:28 jdesch Exp $
 * --------------------------------------------------------------------------
 * Copyright  (c) Dipl.-Ing. Joerg Desch
 * --------------------------------------------------------------------------
 * PROJECT: ezROBI Mega32
 * MODULE.: MOTOR_CTRL.H: Header of the `motor control' module
 * AUTHOR.: jdesch
 * --------------------------------------------------------------------------
 * DESCRIPTION:
 *
 * 
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
 *
 * --------------------------------------------------------------------------
 */

#ifndef __MOTOR_CTRL_H__
#define __MOTOR_CTRL_H__ 1

#include "config.h"


/*+=========================================================================+*/
/*|                      CONSTANT AND MACRO DEFINITIONS                     |*/
/*`========================================================================='*/

/*             .-----------------------------------------------.             */
/* ___________/  local macro declaration                        \___________ */
/*            `-------------------------------------------------'            */

/*+=========================================================================+*/
/*|                            TYPEDECLARATIONS                             |*/
/*`========================================================================='*/

enum MC_MOTORSTATES
{
    MC_MOTOR_DISABLED=0,		     /* motor is disabled ;-) */
    MC_MOTOR_STOPPED,			     /* motor is not started */
    MC_MOTOR_RUNNING,			     /* motor is running ok */
    MC_MOTOR_SHUTDOWN,			     /* motor fault detected */
    MC_MOTOR_OVERLOAD			     /* motor current to high */
};


/*+=========================================================================+*/
/*|                            PUBLIC VARIABLES                             |*/
/*`========================================================================='*/

/*+=========================================================================+*/
/*|                     PROTOTYPES OF GLOBAL FUNCTIONS                      |*/
/*`========================================================================='*/

/*             .-----------------------------------------------.             */
/* ___________/  Setup and Configuration                        \___________ */
/*            `-------------------------------------------------'            */

void mc_InitMotorController (void);


/*             .-----------------------------------------------.             */
/* ___________/  Group...                                       \___________ */
/*            `-------------------------------------------------'            */

void mc_EnableMotors (void);

void mc_DisableMotors (void);

void mc_StartLeftMotor (BYTE Speed, BOOL Forward);

void mc_StartRightMotor (BYTE Speed, BOOL Forward);

void mc_StopLeftMotor (void);

void mc_StopRightMotor (void);

void mc_ForceStopMotors (void);

int mc_CheckLeftMotorState (void);

int mc_CheckRightMotorState (void);



#endif
/* ==[End of file]========================================================== */

