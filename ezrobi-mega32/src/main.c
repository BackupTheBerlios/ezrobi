/* -*- Mode: C -*-
 *
 * $Id: main.c,v 1.4 2007/09/03 05:10:47 jdesch Exp $
 * --------------------------------------------------------------------------
 * Copyright  (c) Dipl.-Ing. Joerg Desch
 * --------------------------------------------------------------------------
 * PROJECT: ezROBI Mega32
 * MODULE.: MAIN.C:
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

#define __MAIN_C__
#include "config.h"
#include "system.h"
#include "v24_single.h"
#include "swt_single.h"
#include "remote_protocol.h"
//#include "fifo.h"
#include "main.h"



/*+=========================================================================+*/
/*|                      CONSTANT AND MACRO DEFINITIONS                     |*/
/*`========================================================================='*/


/*             .-----------------------------------------------.             */
/* ___________/  local macro declaration                        \___________ */
/*            `-------------------------------------------------'            */

/*+=========================================================================+*/
/*|                          LOCAL TYPEDECLARATIONS                         |*/
/*`========================================================================='*/
/* {{{ */

enum LOCAL_TIMER_EVENTS
{
    EV_ALIVE=200,			     /* cyclic actions */
    EV_KEYPAD
};


enum LOCAL_MESSAGES
{
    MSG_THE_FIRST_ONE=0,
    MAX_MESSAGES
};


/* }}} */

/*+=========================================================================+*/
/*|                            PUBLIC VARIABLES                             |*/
/*`========================================================================='*/
/* {{{ */
const flash_chr ProjectId[] = 
{PRJNAME" "__VERSION"\n"};
const flash_chr ProjectId_More[] = 
{"BLD-"__BUILDLEVEL"  Compiled "__DATE__" "__TIME__"\n"};

//T_SysVar SysVar;


/* }}} */

/*+=========================================================================+*/
/*|                             LOCAL VARIABLES                             |*/
/*`========================================================================='*/
/* {{{ */

static BOOL AliveLED_State = FALSE;

#if 0

/* temporary keypad handling
 */
static BYTE tmp_prev_key_io=0x00;


/* the 'system messages' FIFO
 */
MAKE_LOCAL_FIFO(BYTE)

#endif

/* }}} */

/*+=========================================================================+*/
/*|                      PROTOTYPES OF LOCAL FUNCTIONS                      |*/
/*`========================================================================='*/
/* {{{ */

static void setupSystem (void);
static BOOL handleCommands (BYTE cmd, WORD* Parm, WORD cnt);
static void handleDebugCommand (WORD* Parm, WORD cnt);
static void localTimerHandler (BYTE event);

#if 0
static void handleMessages (void);
#endif

/* }}} */

/*+=========================================================================+*/
/*|                     IMPLEMENTATION OF THE FUNCTIONS                     |*/
/*`========================================================================='*/
/* {{{ */


int main (void)
{
    setupSystem();
    app_SystemStarted();
    while (1)
    {
	cpuResetWatchDog();
	app_Idle();
	
	/* SOFTWARE TIMER
	 */
	swt_DispatchTimer();			/* call dispatcher */

	/* SERIAL PROTOCOL INTERFACES
	 */
	if ( v24HaveData() )
	    rp_ParseCommand();			/* start parsing command */
	
#if 0
	/* SYSTEM MESSAGES
	 */
	handleMessages();
	
	/* KEYPAD HANDLER
	 */
	//pad_HandleKeypad();		     /* call dispatcher */
#endif

    }
    return 0;
}




/* }}} */



/*             .-----------------------------------------------.             */
/* ___________/  Keypad related stuff                           \___________ */
/*            `-------------------------------------------------'            */
/* {{{ */

#if 0

BOOL handleKeyPress ( BYTE KeyCode, BOOL IsRepeat )
{
    cpuResetWatchDog();
    if ( !IsRepeat )
    {
    }
    else
    {
    }
    return TRUE;
}


BOOL handleKeyRelease ( BYTE KeyCode )
{
    cpuResetWatchDog();
    return TRUE;
}

#endif

/* }}} */

/*             .-----------------------------------------------.             */
/* ___________/  System Message Handling                        \___________ */
/*            `-------------------------------------------------'            */
/* {{{ */

#if 0

static void handleMessages (void)
{
    BYTE msg;
    
    if ( fifo_empty() )
	return;

    cpuResetWatchDog();
    msg = fifo_get();
    switch ( msg )
    {
    }    
}

#endif

/* }}} */

/*+=========================================================================+*/
/*|                    IMPLEMENTATION OF LOCAL FUNCTIONS                    |*/
/*`========================================================================='*/

/*             .-----------------------------------------------.             */
/* ___________/  Setup / Configuration                          \___________ */
/*            `-------------------------------------------------'            */
/* {{{ */

static void setupSystem (void)
{
    sysSetRedLED(1);
    sysInitHardware();	
    swt_InitTimers(localTimerHandler);
    v24InitModule();
    v24PutsP(ProjectId);
#ifdef DEBUG
    v24PutsP(PSTR("**DEBUG-VERSION**\n"));
#endif
    rp_InitRemote(handleCommands);
    mc_InitMotorController();
    //fifo_init();
    swt_AddTimer(EV_ALIVE,swt_OneSecond(),SWT_RELOAD);
    sysSetRedLED(0);
    sysSetGreenLED(1);
    return;
}

/* }}} */

/*             .-----------------------------------------------.             */
/* ___________/  Locale Handler                                 \___________ */
/*            `-------------------------------------------------'            */
/* {{{ */

static BOOL handleCommands ( BYTE cmd, WORD* Parm, WORD cnt )
{
    WORD i;
    cpuResetWatchDog();
    switch(cmd)
    {
	case RP_CMD_INFO:
	    v24PutsP(ProjectId);
	    if ( cnt==1 && Parm[0]==1 )
	    {
	        v24PutsP(ProjectId_More);
	    }
	    break;
	case RP_CMD_ECHO:
	    v24PutsP(PSTR("Parms:\n"));
	    for ( i=0; i<cnt; i++ )
	    {
		v24PutWord(Parm[i]);
		v24PutsP(PSTR("\n"));
	    }
	    break;
	case RP_CMD_READ_SYSVAR:
	    // parms: var-id; type; value;
	    break;
	case RP_CMD_WRITE_SYSVAR:
	    // parms: var-id; type; value;
	    break;
	case RP_CMD_DEBUG:
	    handleDebugCommand(Parm,cnt);
	    break;
	default:
	    // call the handler of the application logic...
	    app_HandleCommand(cmd,Parm,cnt);
    }
    return TRUE;
}


static void handleDebugCommand ( WORD* Parm, WORD cnt )
{
    if ( !Parm || !cnt )
    	return;
#ifdef ENAB_DEBUG_CMDS
    switch ( Parm[0] )
    {
	case 1:			// switch motor drivers: parm[1]=on/off
	    if ( cnt>1 )
	    {
	    	v24PutsP(PSTR("DBG:motor-driver "));
		if (Parm[1]) {mc_EnableMotors(); v24PutsP(PSTR("on\n"));}
		else {mc_DisableMotors(); v24PutsP(PSTR("off\n"));}
	    }
	    break;
	case 2:			// start left motor: parm[1]=speed parm[2]=dir
	    if ( cnt>2 )
	    {
	    	v24PutsP(PSTR("DBG:start-motor\n"));
		mc_StartLeftMotor(Parm[1]&0x00FF,Parm[2]);
	    }
	    break;
	case 3:			// stop left motor
    	    v24PutsP(PSTR("DBG:stop-motor\n"));
	    mc_StopLeftMotor();
	    break;
	case 4:			// start right motor: parm[1]=speed parm[2]=dir
	    if ( cnt>2 )
	    {
		mc_StartLeftMotor(Parm[1]&0x00FF,Parm[2]);
	    }
	    break;
	case 5:			// stop right motor
	    mc_StopRightMotor();
	    break;
	case 6:			// read raw keypad inputs
	    v24PutsP(PSTR("DBG:keys\n"));
	    v24PutWord(sysReadRawKeypad());
	    break;
	case 7:			// read raw keypad inputs
	    v24PutsP(PSTR("DBG:PWM1(150)\n"));
	    cpuSetPWM1(150);
	    break;

    	// here's the place to add 'debug only' command
    }
#endif
}

static void localTimerHandler ( BYTE event )
{
    cpuResetWatchDog();
    switch (event)
    {
	case EV_ALIVE:
	    // dbg_ReportStrP(PSTR("*ALIVE*\n"));
	    AliveLED_State = AliveLED_State?FALSE:TRUE;
	    sysSetGreenLED(AliveLED_State);
	    break;

	case EV_KEYPAD:
	    /* TEMPORARY keypad handling
	     */
	    break;

	default:
	    // call the handler of the application logic...
	    app_HandleEvent(event);
    }
}

/* }}} */


/* ==[End of file]========================================================== */

