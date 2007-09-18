/* -*- Mode: C -*-
 *
 * $Id: application.c,v 1.2 2007/09/18 12:42:12 jdesch Exp $
 * --------------------------------------------------------------------------
 * Copyright  (c) Dipl.-Ing. Joerg Desch
 * --------------------------------------------------------------------------
 * PROJECT: ezROBI Mega32
 * MODULE.: APPLICATION.C: the main application stuff
 * AUTHOR.: jdesch
 * --------------------------------------------------------------------------
 * DESCRIPTION:
 *
 * Here it is. The main entry point for the users of ezROBI. The point
 * of your interest. ;-)
 * 
 * The whole Firmware is designed to provide you with the stuff you need
 * to edit ONLY this file. All the other stuff is invible (at least if you
 * are not looking at it ;) ).
 *
 * In the normal distribution of ezROBI, this file come as a template with
 * some code. The serial remote protocol knows the command 'i' and returns
 * the application id string. The startup code in app_SystemStarted() creates
 * a sample software time event. This events occures only once (it's a 
 * countdown timer) one second after the start-up. That's all for now.
 * 
 * IMPORTANT: all the functions starting with app_ must be defined. At 
 * least as dummies as you see them here. Don't delete them. If you define
 * your own stuff, declare your function static and put them below the
 * "IMPLEMENTATION OF YOUR LOCAL FUNCTIONS" bar!
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

#define __APPLICATION_C__
#include "main.h"


/*+=========================================================================+*/
/*|                      CONSTANT AND MACRO DEFINITIONS                     |*/
/*`========================================================================='*/

/*+=========================================================================+*/
/*|                          LOCAL TYPEDECLARATIONS                         |*/
/*`========================================================================='*/

/* If you need a time reference, use swt_AddTime() with one of these
 * IDs. If the time is gone, app_HandleEvent() with the ID of the timer
 * is called.
 */
enum APP_TIMER_EVENTS
{
    EV_MY_FIRST=0,
    EV_MY_SECOND
};


/*+=========================================================================+*/
/*|                             LOCAL VARIABLES                             |*/
/*`========================================================================='*/

/* Name and Release of the content of *this* file. Here you can define
 * a unique name for the stuff you are doing here.
 */
const flash_chr ApplicationId[] = 
{"DEMO-APP 0.1"};


/*+=========================================================================+*/
/*|                      PROTOTYPES OF LOCAL FUNCTIONS                      |*/
/*`========================================================================='*/

// add the prototypes of your static functions here


/*+=========================================================================+*/
/*|                IMPLEMENTATION OF THE 'MUST HAVE' FUNCTIONS              |*/
/*`========================================================================='*/


void app_SystemStarted (void)
{
    // called only once at startup. If you have nothing to initialise,
    // leave it as is.
    v24PutsP(PSTR("**Hello world, here's your application**\n"));
    
    // sample for an single (one time) timer event. This occures in 1 second.
    swt_AddTimer(EV_MY_FIRST,swt_OneSecond(),SWT_COUNTDOWN);

    // if we don't want to way for a key, this is a possilbe place to globally
    // enable the motor drivers. The drawback is the power consumption until
    // the real start of your application...
    mc_EnableMotors();
}


void app_Idle (void)
{
    cpuResetWatchDog();
    // the idle function is called from within the main loop. So it
    // is called as often as possible. This will be your working horse.
    // Because ezRobi run the loop for you, *don't* use a endless loop
    // here in your code!
}


BOOL app_HandleCommand (BYTE cmd, WORD* args, WORD cnt)
{
    // if you want own serial commands, this is the place to handle them.
    // cmd hold the command character, and cnt the number of numerical
    // paramters stored in args[]. Your own command characters should
    // only be lowercase letters! ('a'..'z')
    cpuResetWatchDog();
    v24PutsP(PSTR("**HandleCommand not implemented**\n"));

    switch (cmd)
    {
    	case 'i':		// return the ApplicationId!
	    v24PutsP(ApplicationId);
	    break;
	// add your stuff here
	default:
	    return FALSE;	// means `unkown command'.
    }
    return TRUE;		// means `execution ok'
}


void app_HandleKeyPress (BYTE key)
{
    // the name says all. If a key is pressed, this function is called.
    // Key can have the values 1..3.
    cpuResetWatchDog();
    v24PutsP(PSTR("**HandleKeyPress not implemented**\n"));
}

#if CFG_USE_KEY_RELEASE
void app_HandleKeyRelease (BYTE key)
{
    // the name says all. If a pressed key was released, this function is
    // called. Key can have the values 1..3.
    cpuResetWatchDog();
    v24PutsP(PSTR("**HandleKeyRelease not implemented**\n"));
}
#endif


void app_HandleEvent (BYTE event)
{
    // here comes your 'timer events voodoo' ;-)
    cpuResetWatchDog();
    v24PutsP(PSTR("**HandleEvent not implemented**\n"));
    switch (event)
    {
    	case EV_MY_FIRST:
    	    v24PutsP(PSTR("EV_MY_FIRST occured\n"));
    	    break;
    }
}


/*+=========================================================================+*/
/*|                  IMPLEMENTATION OF YOUR LOCAL FUNCTIONS                 |*/
/*`========================================================================='*/


// Implement your local (static)  functions here!


/* ==[End of file]========================================================== */

