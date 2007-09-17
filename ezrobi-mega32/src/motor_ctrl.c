/* -*- Mode: C -*-
 *
 * $Id: motor_ctrl.c,v 1.3 2007/09/17 05:50:52 jdesch Exp $
 * --------------------------------------------------------------------------
 * Copyright  (c) Dipl.-Ing. Joerg Desch
 * --------------------------------------------------------------------------
 * PROJECT: ezROBI Mega32
 * MODULE.: MOTOR_CTRL.C: Motor Controller for BA6845FS
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

#define __MOTOR_CTRL_C__

#include "config.h"
#include "system.h"
#include "motor_ctrl.h"

#ifdef __GNUC__
#include <avr/interrupt.h>
#endif

#ifdef DEBUG_MC
#include "v24_single.h"
#endif


/*+=========================================================================+*/
/*|                      CONSTANT AND MACRO DEFINITIONS                     |*/
/*`========================================================================='*/
/* {{{ */

/* The value ticks/second is calculated with:
 *
 *    F_CPU / (MC_TIMER_PRESCALE_VALUE*MC_TICKS_NEEDED)
 *
 * Here we want 1000 ticks per second. To get this count, we have the following
 * formula:
 *
 *    1000 = F_CPU / (MC_TIMER_PRESCALE_VALUE*MC_TICKS_NEEDED)
 *    MC_TICKS_NEEDED = F_CPU / (MC_TIMER_PRESCALE_VALUE*1000)
 *
 */
#define MC_TIMER_PRESCALE TIMER_CLK_DIV64
#define MC_TIMER_PRESCALE_VALUE 64
#define MC_TICKS_NEEDED  250


/* Defines how long we should wait until the `limit switch' stops the
 * motor. The value defines here are the above defined ticks. So a value of 10
 * means 10msec!
 */
#define MAX_LIMIT_COUNTS 10


/* }}} */

/*             .-----------------------------------------------.             */
/* ___________/  local macro declaration                        \___________ */
/*            `-------------------------------------------------'            */
/* {{{ */


#ifdef DEBUG_MC
#define DBG_PUTS(str) v24PutsP(str)
#else
#define DBG_PUTS(str)
#endif


/* }}} */

/*+=========================================================================+*/
/*|                          LOCAL TYPEDECLARATIONS                         |*/
/*`========================================================================='*/


/*+=========================================================================+*/
/*|                            PUBLIC VARIABLES                             |*/
/*`========================================================================='*/

/*+=========================================================================+*/
/*|                             LOCAL VARIABLES                             |*/
/*`========================================================================='*/

/* flag: set if motor is running 
 */
static volatile BOOL LeftMotorRunning = FALSE;
static volatile BOOL RightMotorRunning = FALSE;

/* flag: set if the motor is running forward 
 */
static volatile BOOL LeftMotorForward = FALSE;
static volatile BOOL RightMotorForward = FALSE;

/* flag: set if driver is enabled 
 */
static volatile BOOL LeftMotorEnabled = FALSE;
static volatile BOOL RightMotorEnabled = FALSE;

/* count ticks while front or back limit is reached */
volatile WORD FrontLimitCounter = 0;
volatile WORD BackLimitCounter = 0;

/* Some help variables
 */
static WORD mc_OneSecond;

//static DWORD volatile LocalTicks;



/*+=========================================================================+*/
/*|                      PROTOTYPES OF LOCAL FUNCTIONS                      |*/
/*`========================================================================='*/

static void enterCritical (void);
static void leaveCritical (void);
static void forceStopMotors (void);

static void initTimerTicker (void);
static void enableLeftMotor (BOOL state);
static void enableRightMotor (BOOL state);


/*+=========================================================================+*/
/*|                     IMPLEMENTATION OF THE FUNCTIONS                     |*/
/*`========================================================================='*/


/*             .-----------------------------------------------.             */
/* ___________/  Setup and Configuration                        \___________ */
/*            `-------------------------------------------------'            */
/* {{{ */

void mc_InitMotorController (void)
{
    /* Setup of variables
     */
    cpuResetWatchDog();
    DBG_PUTS(PSTR("MC:init\n"));
    enterCritical();
    mc_OneSecond=F_CPU/(MC_TIMER_PRESCALE_VALUE*MC_TICKS_NEEDED);

    /* Setup of the hardware
     */
    initTimerTicker();
    cpuInitPWM();

    /* disable both motors as default.
     */
    enableLeftMotor(FALSE);
    enableLeftMotor(FALSE);
    mc_ForceStopMotors();
    
    leaveCritical();
}

/* }}} */


/*             .-----------------------------------------------.             */
/* ___________/  Accessing the module                           \___________ */
/*            `-------------------------------------------------'            */
/* {{{ */

void mc_EnableMotors (void)
{
    cpuResetWatchDog();
    DBG_PUTS(PSTR("MC:enable-motors\n"));
    enableLeftMotor(TRUE);
    enableRightMotor(TRUE);
    mc_ForceStopMotors();
}

void mc_DisableMotors (void)
{
    cpuResetWatchDog();
    DBG_PUTS(PSTR("MC:disable-motors\n"));
    mc_ForceStopMotors();
    enableLeftMotor(FALSE);
    enableRightMotor(FALSE);
}

void mc_StartLeftMotor ( BYTE Speed, BOOL Forward )
{
    cpuResetWatchDog();
    DBG_PUTS(PSTR("MC:start-left\n"));
    if ( !LeftMotorEnabled )
    {
        DBG_PUTS(PSTR("`--abort\n"));
	return;
    }
    LeftMotorForward = Forward;
    out_MOTOR1_DIR = Forward;
    
    enterCritical();
    cpuSetPWM1(Speed);

    /* TODO: For now we clear the counters. Later we must check the direction
     * and than clear the right one.
     *
     * TODO: we must also check the bumper and supress the start if the
     * direction is "blocked".
     */
    FrontLimitCounter = 0;
    BackLimitCounter = 0;

    LeftMotorRunning = TRUE;
    leaveCritical();
}

void mc_StartRightMotor ( BYTE Speed, BOOL Forward )
{
    cpuResetWatchDog();
    DBG_PUTS(PSTR("MC:start-right\n"));
    if ( !RightMotorEnabled )
    {
        DBG_PUTS(PSTR("`--abort\n"));
	return;
    }
    RightMotorForward = Forward;
    out_MOTOR2_DIR = Forward;
    
    enterCritical();
    cpuSetPWM2(Speed);
    
    /* TODO: For now we clear the counters. Later we must check the direction
     * and than clear the right one.
     *
     * TODO: we must also check the bumper and supress the start if the
     * direction is "blocked".
     */
    FrontLimitCounter = 0;
    BackLimitCounter = 0;

    RightMotorRunning = TRUE;
    leaveCritical();
}

void mc_StopLeftMotor ( void )
{
    cpuResetWatchDog();
    DBG_PUTS(PSTR("MC:stop-left\n"));
    if ( LeftMotorRunning )
    {
	enterCritical();
	cpuSetPWM1(0);
	LeftMotorRunning = TRUE;
	leaveCritical();
        DBG_PUTS(PSTR("`--done\n"));	
    }
}

void mc_StopRightMotor ( void )
{
    cpuResetWatchDog();
    DBG_PUTS(PSTR("MC:stop-right\n"));
    if ( RightMotorRunning )
    {
	enterCritical();
	cpuSetPWM2(0);
	RightMotorRunning = TRUE;
	leaveCritical();
        DBG_PUTS(PSTR("`--done\n"));
    }
}

/* Stop both motors without any conditions.
 *
 */
void mc_ForceStopMotors ( void )
{
    cpuResetWatchDog();
    DBG_PUTS(PSTR("MC:force-stop\n"));
    enterCritical();
    forceStopMotors();
    leaveCritical();
}


/* Check the `health state' of the left motor. If possible, we query the motor
 * driver...
 *
 * @return (int) the state of motor (MC_MOTORSTATES).
 */
int mc_CheckLeftMotorState ( void )
{
    DBG_PUTS(PSTR("MC:check-left\n"));
    if ( !LeftMotorEnabled )
	return MC_MOTOR_SHUTDOWN;
    return (LeftMotorRunning)?MC_MOTOR_RUNNING:MC_MOTOR_STOPPED;
}


/* Check the `health state' of the right motor. If possible, we query the motor
 * driver (this is not supported by the BA6845FS).
 *
 * @return (int) the state of motor (MC_MOTORSTATES).
 */
int mc_CheckRightMotorState ( void )
{
    DBG_PUTS(PSTR("MC:check-right\n"));
    if ( !RightMotorEnabled )
	return MC_MOTOR_SHUTDOWN;
    return (RightMotorRunning)?MC_MOTOR_RUNNING:MC_MOTOR_STOPPED;
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
    cbi(TIMSK, TOIE2);			// enable TCNT2 overflow
  #elif defined (__AVR_ATmega1280__) || defined (__AVR_ATmega323__) || defined (__AVR_ATmega2560__)
    cbi(TIMSK2, TOIE2);
  #endif
#elif __CODEVISIONAVR__
  // CodeVisionAVR has no declaration for TOIE2 
  #if defined _CHIP_ATMEGA128_ | defined _CHIP_ATMEGA32
    cbi(TIMSK, 6);			// enable TCNT2 overflow
  #elif defined _CHIP_ATMEGA1280_ | defined _CHIP_ATMEGA323_ | defined _CHIP_ATMEGA2560_
    cbi(TIMSK2, 6);
  #endif
#else
  #error "MC-Module: unknown compiler"
#endif
}

static void leaveCritical (void)
{
#ifdef __GNUC__
  #if defined (__AVR_ATmega128__) || defined (__AVR_ATmega32__)
    sbi(TIMSK, TOIE2);			// enable TCNT2 overflow
  #elif defined (__AVR_ATmega1280__) || defined (__AVR_ATmega323__) || defined (__AVR_ATmega2560__)
    sbi(TIMSK2, TOIE2);
  #endif
#elif __CODEVISIONAVR__
  // CodeVisionAVR has no declaration for TOIE2 
  #if defined _CHIP_ATMEGA128_ | defined _CHIP_ATMEGA32
    sbi(TIMSK, 6);
  #elif defined _CHIP_ATMEGA1280_ | defined _CHIP_ATMEGA323_ | defined _CHIP_ATMEGA2560_
    sbi(TIMSK2, 6);
  #endif
#else
  #error "MC-Module: unknown compiler"
#endif
}


/* Stop both motors without any conditions.
 *
 * CAUTION: this functions is called from within an ISR. So don't use any
 * printf nor reset the watchdog here.
 */
static void forceStopMotors ( void )
{
    LeftMotorRunning = FALSE;
    RightMotorRunning = FALSE;
    cpuSetPWM1(0);
    cpuSetPWM2(0);
}


/* }}} */

/*             .-----------------------------------------------.             */
/* ___________/  INITIALISATION OF THE COMPONENTS               \___________ */
/*            `-------------------------------------------------'            */
/* {{{ */

static void initTimerTicker (void)
{
#ifdef __GNUC__
  #if defined (__AVR_ATmega128__) || defined (__AVR_ATmega32__)
    TCCR2 = ((TCCR2 & ~TIMER_PRESCALE_MASK) | MC_TIMER_PRESCALE);
    TCNT2 = 0xFF-MC_TICKS_NEEDED;
  #elif defined (__AVR_ATmega1280__) || defined (__AVR_ATmega323__) || defined (__AVR_ATmega2560__)
    TCCR2B = ((TCCR2B & ~TIMER_PRESCALE_MASK) | MC_TIMER_PRESCALE);
    TCNT2 = 0xFF-MC_TICKS_NEEDED;
  #else
    #warning "MC-Modul: unsupported MCU type!"
  #endif
#elif __CODEVISIONAVR__
  #if defined _CHIP_ATMEGA128_ | defined _CHIP_ATMEGA32
    TCCR2 = ((TCCR2 & ~TIMER_PRESCALE_MASK) | MC_TIMER_PRESCALE);
    TCNT2 = 0xFF-MC_TICKS_NEEDED;
  #elif defined _CHIP_ATMEGA1280_ | defined _CHIP_ATMEGA323_ | defined _CHIP_ATMEGA2560_
    TCCR2B = ((TCCR2B & ~TIMER_PRESCALE_MASK) | MC_TIMER_PRESCALE);
    TCNT2 = 0xFF-MC_TICKS_NEEDED;
  #else
    #warning "MC-Modul: unsupported MCU type!"
  #endif
#else
  #error "MC-Modul: unknown compiler"
#endif

}

static void enableLeftMotor ( BOOL state )
{
    out_MOTOR1_ENAB = state; 
    LeftMotorEnabled = state;
}

static void enableRightMotor ( BOOL state )
{
    out_MOTOR2_ENAB = state; 
    RightMotorEnabled = state;
}

/* }}} */

/*             .-----------------------------------------------.             */
/* ___________/  INTERRUPT SERVICE FUNCTIONS                    \___________ */
/*            `-------------------------------------------------'            */
/* {{{ */

/* TIMER TICKER ISR. This is the place, where the `limit switched' are
 * observed.
 */
#ifdef __GNUC__
ISR(SIG_OVERFLOW2)
#elif __CODEVISIONAVR__
interrupt [TIM2_OVF] void timer2_ovf_isr(void)
#else
#  error "MC-Module: unknown compiler"
#endif
{


#if 0 
// ==================== OLD CODE! MUST BE REPLACED 
    if ( MotorRunning )
    {
	if ( DirectionForward )
	{
	    if ( in_SW3 )
	    {
		++LimitCounter;
		if ( LimitCounter >= MAX_LIMIT_COUNTS )
		    forceStopMotor();
	    }
	}
	else
	{
	    if ( in_SW4 )
	    {
		++LimitCounter;
		if ( LimitCounter >= MAX_LIMIT_COUNTS )
		    forceStopMotor();
	    }
	}	
    }
// ==================== OLD CODE! MUST BE REPLACED 
#endif


}


/* }}} */


/* ==[End of file]========================================================== */

