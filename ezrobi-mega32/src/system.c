/* -*- Mode: C -*-
 *
 * $Id: system.c,v 1.1 2007/08/29 12:42:25 jdesch Exp $
 * --------------------------------------------------------------------------
 * Copyright  (c) Dipl.-Ing. Joerg Desch
 * --------------------------------------------------------------------------
 * PROJECT: ezROBI Mega32
 * MODULE.: SYSTEM.C:
 * AUTHOR.: jdesch
 * --------------------------------------------------------------------------
 * DESCRIPTION:
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

#define __SYSTEM_C__
#include "system.h"
#include "cpu_avr.h"



/*+=========================================================================+*/
/*|                      CONSTANT AND MACRO DEFINITIONS                     |*/
/*`========================================================================='*/
/* {{{ */

/* }}} */

/*             .-----------------------------------------------.             */
/* ___________/  local macro declaration                        \___________ */
/*            `-------------------------------------------------'            */
/* {{{ */


/* }}} */

/*+=========================================================================+*/
/*|                          LOCAL TYPEDECLARATIONS                         |*/
/*`========================================================================='*/
/* {{{ */

/* }}} */

/*+=========================================================================+*/
/*|                            PUBLIC VARIABLES                             |*/
/*`========================================================================='*/
/* {{{ */

/* }}} */

/*+=========================================================================+*/
/*|                             LOCAL VARIABLES                             |*/
/*`========================================================================='*/
/* {{{ */

static BYTE led_state = 0;	/* bitflags which remember the LED state */

/* }}} */

/*+=========================================================================+*/
/*|                      PROTOTYPES OF LOCAL FUNCTIONS                      |*/
/*`========================================================================='*/
/* {{{ */

static void setupCPU_Ports (void);
static WORD mergedKeypadPins (void);

/* }}} */


/*+=========================================================================+*/
/*|                     IMPLEMENTATION OF THE FUNCTIONS                     |*/
/*`========================================================================='*/


/*             .-----------------------------------------------.             */
/* ___________/  Initialization                                 \___________ */
/*            `-------------------------------------------------'            */
/* {{{ */

void sysInitHardware (void)
{
    cpuInitWatchDog();
    setupCPU_Ports();
 
    cli();			     /* don't disturbe our circles ;) */
    cpuInitADC();
    cpuInitPWM();

#if 0
    // External Interrupt(s) initialization
    // INT0: On
    // INT0 Mode: Falling Edge
    // INT1: On
    // INT1 Mode: Falling Edge
    // INT2: Off
    GICR|=0xC0;
    MCUCR=0x0A;
    MCUCSR=0x00;
    GIFR=0xC0;

    // Timer(s)/Counter(s) Interrupt(s) initialization
    TIMSK=0x41;
#endif

    // USART initialization
    // Communication Parameters: 8 Data, 1 Stop, No Parity
    // USART Receiver: On
    // USART Transmitter: On
    // USART Mode: Asynchronous
    // USART Baud rate: 57600 (Double Speed Mode)
    // USART RX-Interrupt On
    UCSRA=0x02;
    UCSRB=0x98;
    UCSRC=0x86;
#ifdef __CODEVISIONAVR__
#error "TODO: we have to parse the baudrate with CodeVision"
#endif    
#if (F_CPU == 16000000)
#if (CFG_V24_BAUD == 57600)
    UBRRH=0;  UBRRL=34;
#endif
#if (CFG_V24_BAUD == 38400)
    UBRRH=0;  UBRRL=51;
#endif
#if (CFG_V24_BAUD == 19200)
    UBRRH=0;  UBRRL=103;
#endif
#else
#  error "baudrate: unsupported CPU speed (F_CPU)" 
#endif

    /* The IRQ Stuff
     */
    sei();
    
    /* init the local variables...
     */
    led_state = 0;
}


/* }}} */

/*             .-----------------------------------------------.             */
/* ___________/  ACCESSING THE HARDWARE                         \___________ */
/*            `-------------------------------------------------'            */
/* {{{ */

void sysSetGreenLED (BOOL enable)
{
    cpuResetWatchDog();
    if (enable) led_state |= 0x01;
    else led_state &= (~0x01);
    out_GREEN_LED = (enable)?0:1;
}

void sysSetRedLED (BOOL enable)
{
    cpuResetWatchDog();
    if (enable) led_state |= 0x02;
    else led_state &= (~0x02);
    out_RED_LED = (enable)?0:1;
}

void sysSetYellowLED (BOOL enable)
{
    cpuResetWatchDog();
    if (enable) led_state |= 0x04;
    else led_state &= (~0x04);
    out_YELLOW_LED = (enable)?0:1;
}


/* }}} */

/*             .-----------------------------------------------.             */
/* ___________/  Accessing the Keypad                           \___________ */
/*            `-------------------------------------------------'            */
/* {{{ */

/** Returns the state of the keypad-port. The signals are already in positive
 * logic. So a set bit represents a pressed key.
 *
 * @return (WORD) the inputs of the keypad port.
 * @memo internal function.
 */
WORD sysReadRawKeypad (void)
{
    WORD key_io;

    cpuResetWatchDog();
    key_io = mergedKeypadPins();
    if ( key_io )
    {
	cpuDelay_us(100);
	key_io = mergedKeypadPins();
    }
    return key_io;
}



/* }}} */

/*+=========================================================================+*/
/*|                    IMPLEMENTATION OF LOCAL FUNCTIONS                    |*/
/*`========================================================================='*/
/* {{{ */


static void setupCPU_Ports (void)
{
    PORTA = VAL_PORT_A;  DDRA = DIR_PORT_A;
    PORTB = VAL_PORT_B;  DDRB = DIR_PORT_B;
    PORTC = VAL_PORT_C;  DDRC = DIR_PORT_C;
    PORTD = VAL_PORT_D;  DDRD = DIR_PORT_D;
}


/* Simply return the merged pins of the keypad. This means, all bits are
 * collected into one WORD. Make sure that the bits are in positive logic.
 * So a pressed key must be 1!
 *
 * In this design, the LED outputs are shared with the keypad inputs. To get
 * the keys, we have to set the led outputs to 1, switch to inputs, read the
 * pin and restore the outputs.
 */
static WORD mergedKeypadPins (void)
{
    WORD w=0;
    
    out_GREEN_LED = 1;
    out_RED_LED = 1;
    out_YELLOW_LED = 1;
    SET_SW1_IN();
    SET_SW2_IN();
    SET_SW3_IN();

    if ( in_SW1==0 ) w |= SYS_SW1_KEY;
    if ( in_SW2==0 ) w |= SYS_SW2_KEY;
    if ( in_SW3==0 ) w |= SYS_SW3_KEY;

    SET_SW1_OUT();
    SET_SW2_OUT();
    SET_SW3_OUT();
    out_GREEN_LED = (led_state&0x01)? 0:1;
    out_RED_LED = (led_state&0x02)? 0:1;;
    out_YELLOW_LED = (led_state&0x04)? 0:1;;
    return 0;
}

/* }}} */

/*             .-----------------------------------------------.             */
/* ___________/  INTERRUPT SERVICE ROUTINES                     \___________ */
/*            `-------------------------------------------------'            */
/* {{{ */

/* }}} */

/* ==[End of file]========================================================== */

