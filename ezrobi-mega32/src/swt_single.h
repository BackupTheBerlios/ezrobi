/* -*- Mode: C -*-
 *
 * $Id: swt_single.h,v 1.1 2007/08/29 12:42:25 jdesch Exp $
 * --------------------------------------------------------------------------
 * Copyright  (c) Dipl.-Ing. Joerg Desch
 * --------------------------------------------------------------------------
 * PROJECT: ezROBI Mega32
 * MODULE.: SWT_SINGLE.H: Header of the Software-Timer-Moduls
 * AUTHOR.: jdesch
 * --------------------------------------------------------------------------
 * DESCRIPTION:
 *
 * The indention of this modul is the implementation of multiple, software
 * based timers. Therefore, the hardware `timebase' timer is used. All modules
 * can create or delete multiple timer events. The application must ensure, that
 * the polling function of this modul is called periodically. This function is
 * responsible for the dispatching of occured timer events.
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
 */

#ifndef __SWT_SINGLE_H__
#define __SWT_SINGLE_H__ 1

#include "config.h"			     /* defines the hardware we use  */


/** The indention of the #swt_timer# modul is the implementation of multiple,
 * software based timers. Therefore, the hardware #timebase# timer is
 * used. Several moduls could register their own \emph{timer event
 * handler}. For this handler, multiple timer events can be created or
 * deleted. The application must ensure, that the polling function of this
 * modul is called periodically. This function is responsible for the
 * dispatching of occured timer events.
 *
 * @memo Implementation of `software timers'
 */
/*@{ --------------------------------====> Start of DOC++ Scope of the Modul */


/*+=========================================================================+*/
/*|                      CONSTANT AND MACRO DEFINITIONS                     |*/
/*`========================================================================='*/

#ifndef SWT_MAX_EVENTS
#define SWT_MAX_EVENTS   16		/* max. number of all events */
#endif
#define SWT_INVALID     0xFF		/* ID for undefined events */


/*             .-----------------------------------------------.             */
/* ___________/  local macro declaration                        \___________ */
/*            `-------------------------------------------------'            */

/*+=========================================================================+*/
/*|                            TYPEDECLARATIONS                             |*/
/*`========================================================================='*/

typedef void (*T_TimerHandler)(BYTE Event);

enum __SWT_TIMERTYPES
{
    SWT_COUNTDOWN=0,			     /* simple countdown timer */
    SWT_RELOAD,				     /* auto reloading timer */
    SWT_LAST_TIMERTYPE
};


/*+=========================================================================+*/
/*|                            PUBLIC VARIABLES                             |*/
/*`========================================================================='*/

/*+=========================================================================+*/
/*|                     PROTOTYPES OF GLOBAL FUNCTIONS                      |*/
/*`========================================================================='*/

/** Initialize the modul. Before calling any of the below functions, you have
 * to call this initialization function. All data is cleared, and the timer
 * interrupts are enabled. Nothing more an nothing less.
 *
 * The handler  #callback# is a pointer to a function that accepts a BYTE as
 * parameter. This function is called for each occured timer event. The id of
 * the event is given as parameter.
 * 
 * @param callback pointer to the timer event handler.
 * @memo
 */
void swt_InitTimers (T_TimerHandler callback);



/** This function is used to add or restart a timer.  #id# is the id code of
 * the timer, and #time# is the amount of time. The \emph{time} is calculated
 * as timer ticks. To get the right time base, use \Ref{swt_OneSecond}.
 *
 * The type of the timer is given in #type#. It defined the dehaviour of the
 * timer termination. If #SWT_COUNTDOWN# is used, the timer runs only
 * once. It's cleared after the event occures. Timers of the type #SWT_RELOAD#
 * never stopps. After the event occures, the timer is automagically
 * restarted. See also \Ref{swt_DispatchTimer}.
 *
 * If there is already a timer with the id #id# active, it is restarted. This
 * also works, if the type of the timer changes.
 *
 * @param id      id code of the timer event.
 * @param time    amount of time the timer runs.
 * @param         type of the timer.
 * @return (BOOL) #TRUE# is no error happens.
 * @memo
 */
BOOL swt_AddTimer (BYTE id, WORD time, BYTE type);


/** The function stopps and clears the timer with the #id#.
 *
 * @param id      id code of the timer event.
 * @return (BOOL) #TRUE# is no error happens.
 * @memo
 */
BOOL swt_KillTimer (BYTE id);


/** The function returns the state of the compiler. This means, it returns
 * #TRUE# if the timer is still active. The return value is #FALSE#, if the
 * timer event is already happened, or if no such timer id is defined.
 *
 * @param id      id code of the timer event.
 * @return (BOOL) #TRUE# if timer is still running.
 * @memo
 */
BOOL swt_QueryTimerState (BYTE id);


/** This is the \emph{working horse} of the modul. The dispatcher \textbf{must}
 * be called on a regular base! If possible, put it into you \emph{idle proc}.
 * The function is looking for occured timer events. For each of this events,
 * the registered \emph{handler function} (the \emph{callback function}) is
 * called. The ID of the timer event is passed as argument to the callback.
 *
 * Timer of the type #SWT_COUNTDOWN# are cleared right after the call to the
 * handler. It's save to restart it again by calling \Ref{swt_AddTimer} from
 * inside the handler.
 *
 * The second type of timers is #SWT_RELOAD#. These timers are never
 * cleared. They are restarted just before the call to the handler. Note, that
 * it's also save to call \Ref{swt_AddTimer} to modify the timer from within
 * the handler.
 *
 * @memo
 */
void swt_DispatchTimer (void);


/** This function simply returns the timer value for one second. It's important
 * to know, that this value depends on the configuration of the modul. So you
 * should always use this function to get a valid reference.
 *
 * @return (WORD) timer ticks for one second.
 * @memo
 */
WORD swt_OneSecond (void);


/** Read the internal timer ticker.
 *
 */
DWORD swt_GetTicker (void);


/** Calculate the ticker difference. You can use \Ref{swt_OneSecond} to convert
 * ticker into seconds.
 */
DWORD swt_GetTickerDiff (DWORD PrevTick);


/*             .-----------------------------------------------.             */
/* ___________/  Group...                                       \___________ */
/*            `-------------------------------------------------'            */



/*@} ----------------------------------====> End of DOC++ Scope of the Modul */
#endif
/* ==[End of file]========================================================== */

