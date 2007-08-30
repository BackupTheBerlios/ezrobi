/* -*- Mode: C -*-
 *
 * $Id: swt_single.c,v 1.2 2007/08/30 16:06:03 jdesch Exp $
 * --------------------------------------------------------------------------
 * Copyright  (c) Dipl.-Ing. Joerg Desch
 * --------------------------------------------------------------------------
 * PROJECT: ezROBI Mega32
 * MODULE.: SWT_SINGLE.C: single-module Software-Timer
 * AUTHOR.: jdesch
 * --------------------------------------------------------------------------
 * DESCRIPTION:
 *
 * Implementation of the `single-module' software timers. Timer0 is used with a
 * resolution of 1000 "ticks" per second.
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

#define __SWT_SINGLE_C__

#include "cpu_avr.h"
#include "swt_single.h"
#ifdef __GNUC__
#include <avr/interrupt.h>
#endif

#ifdef DEBUG_SWT
#include "debugging.h"
#endif


/*+=========================================================================+*/
/*|                      CONSTANT AND MACRO DEFINITIONS                     |*/
/*`========================================================================='*/

/* The value ticks/second is calculated with:
 *
 *    F_CPU / (SWT_TIMER_PRESCALE_VALUE*SWT_TICKS_NEEDED)
 *
 * Here we want 1000 ticks per second. To get this count, we have the following
 * formula:
 *
 *    1000 = F_CPU / (SWT_TIMER_PRESCALE_VALUE*SWT_TICKS_NEEDED)
 *    SWT_TICKS_NEEDED = F_CPU / (SWT_TIMER_PRESCALE_VALUE*1000)
 *
 */
#define SWT_TIMER_PRESCALE TIMER_CLK_DIV64
#define SWT_TIMER_PRESCALE_VALUE 64
#define SWT_TICKS_NEEDED 250


/*             .-----------------------------------------------.             */
/* ___________/  local macro declaration                        \___________ */
/*            `-------------------------------------------------'            */


#ifdef DEBUG_SWT
#define DBG_PUTS(str) dbg_ReportStr(str)
#else
#define DBG_PUTS(str)
#endif


/*+=========================================================================+*/
/*|                          LOCAL TYPEDECLARATIONS                         |*/
/*`========================================================================='*/

struct tagEVENT
{
	WORD TimerValue;		     /* the count down timer */
	WORD ReloadValue;		     /* opt. reload value or 0 */
	BYTE ID;			     /* application ID of the event */
};
typedef struct tagEVENT T_Event;


/*+=========================================================================+*/
/*|                            PUBLIC VARIABLES                             |*/
/*`========================================================================='*/

/*+=========================================================================+*/
/*|                             LOCAL VARIABLES                             |*/
/*`========================================================================='*/


/* The stuff for managing the events.
 */

/* This array holds all possible events. All unused entries have are marked with
 * the id #SWL_UNDEFINED#. The variable #FirstUnusedEvent# points to the first
 * unused entry. All entries below this index are reserved for by the register
 * event handlers.
 */
static T_Event Events[SWT_MAX_EVENTS];
static volatile BOOL EventOccured=FALSE;

static T_TimerHandler EventHandler;

/* Some help variables
 */
static WORD OneSecond;

static DWORD volatile LocalTicks;



/*+=========================================================================+*/
/*|                      PROTOTYPES OF LOCAL FUNCTIONS                      |*/
/*`========================================================================='*/

static void enterCritical (void);
static void leaveCritical (void);


/*+=========================================================================+*/
/*|                     IMPLEMENTATION OF THE FUNCTIONS                     |*/
/*`========================================================================='*/


/*             .-----------------------------------------------.             */
/* ___________/  Setup and Configuration                        \___________ */
/*            `-------------------------------------------------'            */
/* {{{ */

void swt_InitTimers ( T_TimerHandler callback )
{
    short i;

    /* Setup of variables
     */
    cpuResetWatchDog();
    if ( !callback )
    {
	DBG_PUTS("null pointer\n");
	return;
    }
    enterCritical();
    EventOccured=FALSE;
    for ( i=0; i<SWT_MAX_EVENTS; i++ )
	Events[i].ID=SWT_INVALID;
    EventHandler=callback;
    LocalTicks=0;

    /* Setup of the hardware
     */
#ifdef __GNUC__
  #if defined (__AVR_ATmega128__) || defined (__AVR_ATmega32__)
    TCCR0 = ((TCCR0 & ~TIMER_PRESCALE_MASK) | SWT_TIMER_PRESCALE);
    TCNT0 = 0xFF - SWT_TICKS_NEEDED;
  #elif defined (__AVR_ATmega1280__) || defined (__AVR_ATmega323__) || defined (__AVR_ATmega2560__)
    TCCR0B = ((TCCR0B & ~TIMER_PRESCALE_MASK) | SWT_TIMER_PRESCALE);
    TCNT0 = 0xFF - SWT_TICKS_NEEDED;
  #else
    #warning "SWT-Modul: unsupported MCU type!"
  #endif
#elif __CODEVISIONAVR__
  #if defined _CHIP_ATMEGA128_ | defined _CHIP_ATMEGA32
    TCCR0 = ((TCCR0 & ~TIMER_PRESCALE_MASK) | SWT_TIMER_PRESCALE);
    TCNT0 = 0xFF - SWT_TICKS_NEEDED;
  #elif defined _CHIP_ATMEGA1280_ | defined _CHIP_ATMEGA323_ | defined _CHIP_ATMEGA2560_
    TCCR0B = ((TCCR0B & ~TIMER_PRESCALE_MASK) | SWT_TIMER_PRESCALE);
    TCNT0 = 0xFF - SWT_TICKS_NEEDED;
  #else
    #warning "SWT-Modul: unsupported MCU type!"
  #endif
#else
  #error "SWT-Modul: unknown compiler"
#endif
    OneSecond=F_CPU/(SWT_TIMER_PRESCALE_VALUE*SWT_TICKS_NEEDED);

    leaveCritical();
}


/* }}} */


/*             .-----------------------------------------------.             */
/* ___________/  Accessing the timers                           \___________ */
/*            `-------------------------------------------------'            */
/* {{{ */

BOOL swt_AddTimer ( BYTE id, WORD time, BYTE type )
{
    BYTE free_one=SWT_MAX_EVENTS;
    BYTE i;

    cpuResetWatchDog();
    if ( id==SWT_INVALID || type>=SWT_LAST_TIMERTYPE )
	return FALSE;
    free_one=SWT_MAX_EVENTS;
    for ( i=0; i<SWT_MAX_EVENTS; i++ )
    {
	if ( Events[i].ID == id )
	{
	    /* The time we have to add is already running. So we restart it!
	     */
	    enterCritical();
	    Events[i].TimerValue=time;
	    if ( type==SWT_RELOAD )
		Events[i].ReloadValue=time;
	    else
		Events[i].ReloadValue=0;
	    leaveCritical();
	    return TRUE;
	}
	else if ( Events[i].ID==SWT_INVALID )
	{
	    /* if we find a free timer, remember the first one for later use!
	     */
	    if ( free_one == SWT_MAX_EVENTS )
		free_one = i;
	}
    }

    /* At this point, we have not found a running timer. If we have found a
     * free one, start this!
     */
    cpuResetWatchDog();
    if ( free_one != SWT_MAX_EVENTS )
    {
	enterCritical();
	Events[free_one].TimerValue=time;
	if ( type==SWT_RELOAD )
	    Events[free_one].ReloadValue=time;
	else
	    Events[free_one].ReloadValue=0;
	Events[free_one].ID=id;
	leaveCritical();
	return TRUE;
    }
    return FALSE;
}


BOOL swt_KillTimer ( BYTE id )
{
    BYTE i;

    cpuResetWatchDog();
    if ( id == SWT_INVALID )
	return FALSE;
    for ( i=0; i<SWT_MAX_EVENTS; i++ )
    {
	if ( Events[i].ID==id )
	{
	    enterCritical();
	    Events[i].TimerValue=0;
	    Events[i].ReloadValue=0;
	    Events[i].ID=SWT_INVALID;
	    leaveCritical();
	    return TRUE;
	}
    }
    cpuResetWatchDog();
    return FALSE;
}

void swt_DispatchTimer (void)
{
    BYTE curr_id;
    BYTE i;

    cpuResetWatchDog();
    if ( !EventOccured )
	return;
    enterCritical();
    EventOccured=FALSE;
    leaveCritical();

    for ( i=0; i<SWT_MAX_EVENTS; i++ )
    {
	/* looking for an expired timer
	 */
	if ( Events[i].ID!=SWT_INVALID && Events[i].TimerValue==0 )
	{
	    /* mark the event as `invalid'. This means, it is processed now. We
	     * have to do this before calling the handler, so that it is
	     * possible to restart it from inside the handler!
	     *
	     * Note: direct access is save here, because this timer isn't
	     * running anymore.
	     */
	    curr_id=Events[i].ID;
	    Events[i].ID=SWT_INVALID;

	    /* If this is a auto reloading timer-entry, we have to reload the
	     * timer value.
	     */
	    if ( Events[i].ReloadValue )
	    {
		enterCritical();
		Events[i].ID=curr_id;
		Events[i].TimerValue=Events[i].ReloadValue;
		leaveCritical();
	    }

	    /* If one is found, call it's handler (if possible). If the handler
	     * isn't valid, the event gets lost.
	     */
	    if ( EventHandler )
	    {
		cpuResetWatchDog();
		EventHandler(curr_id);
	    }
	}
    }
}


BOOL swt_QueryTimerState (BYTE id)
{
    BYTE i;

    cpuResetWatchDog();
    if ( id == SWT_INVALID )
	return FALSE;
    for ( i=0; i<SWT_MAX_EVENTS; i++ )
    {
	if ( Events[i].ID==id )
	    return (Events[i].TimerValue!=0)?TRUE:FALSE;
    }
    return FALSE;
}


WORD swt_OneSecond (void)
{
    return OneSecond;
}


/* }}} */

/*             .-----------------------------------------------.             */
/* ___________/  Group...                                       \___________ */
/*            `-------------------------------------------------'            */
/* {{{ */

DWORD swt_GetTicker(void)
{
    DWORD ticks;
    enterCritical();
    ticks = LocalTicks;        /* read tick counter                */
    leaveCritical();
    return ticks;
}

DWORD swt_GetTickerDiff (DWORD PrevTick)
{
    DWORD now;            
    
    now = swt_GetTicker();
    if (now >= PrevTick)
        return now - PrevTick;
    else
        return 0xffffffff - PrevTick + now + 1;
}

/* }}} */

/*+=========================================================================+*/
/*|                    IMPLEMENTATION OF LOCAL FUNCTIONS                    |*/
/*`========================================================================='*/
/* {{{ */

static void enterCritical (void)
{
#ifdef __GNUC__
  #if defined (__AVR_ATmega128__) || defined (__AVR_ATmega32__)
    cbi(TIMSK, TOIE0);			// enable TCNT1 overflow
  #elif defined (__AVR_ATmega1280__) || defined (__AVR_ATmega323__) || defined (__AVR_ATmega2560__)
    cbi(TIMSK0, TOIE0);
  #endif
#elif __CODEVISIONAVR__
  // CodeVisionAVR has no declaration for TOIE0 
  #if defined _CHIP_ATMEGA128_ | defined _CHIP_ATMEGA32
    cbi(TIMSK, 0);			// enable TCNT1 overflow
  #elif defined _CHIP_ATMEGA1280_ | defined _CHIP_ATMEGA323_ | defined _CHIP_ATMEGA2560_
    cbi(TIMSK0, 0);
  #endif
#else
  #error "SWT-Module: unknown compiler"
#endif
}

static void leaveCritical (void)
{
#ifdef __GNUC__
  #if defined (__AVR_ATmega128__) || defined (__AVR_ATmega32__)
    sbi(TIMSK, TOIE0);			// enable TCNT1 overflow
  #elif defined (__AVR_ATmega1280__) || defined (__AVR_ATmega323__) || defined (__AVR_ATmega2560__)
    sbi(TIMSK0, TOIE0);
  #endif
#elif __CODEVISIONAVR__
  // CodeVisionAVR has no declaration for TOIE0 
  #if defined _CHIP_ATMEGA128_ | defined _CHIP_ATMEGA32
    sbi(TIMSK, 0);
  #elif defined _CHIP_ATMEGA1280_ | defined _CHIP_ATMEGA323_ | defined _CHIP_ATMEGA2560_
    sbi(TIMSK0, 0);
  #endif
#else
  #error "SWT-Module: unknown compiler"
#endif
}

/* }}} */

/*             .-----------------------------------------------.             */
/* ___________/  INTERRUPT SERVICE FUNCTIONS                    \___________ */
/*            `-------------------------------------------------'            */
/* {{{ */

#ifdef __GNUC__
ISR(SIG_OVERFLOW0)
#elif __CODEVISIONAVR__
interrupt [TIM0_OVF] void timer0_ovf_isr(void)
#else
  #error "SWT-Module: unknown compiler"
#endif
{
    BYTE i;

    ++LocalTicks;
    for ( i=0; i<SWT_MAX_EVENTS; i++ )
    {
	if ( Events[i].ID!=SWT_INVALID && Events[i].TimerValue )
	{
	    Events[i].TimerValue--;
	    if ( Events[i].TimerValue==0 )
		EventOccured=TRUE;
	}
    }
}


/* }}} */


/* ==[End of file]========================================================== */

