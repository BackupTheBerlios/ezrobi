/* -*- Mode: C -*-
 *
 * $Id: cpu_avr.c,v 1.6 2008/02/01 14:15:06 jdesch Exp $
 * --------------------------------------------------------------------------
 * Copyright  (c) Dipl.-Ing. Joerg Desch
 * --------------------------------------------------------------------------
 * PROJECT: ezROBI Mega32
 * MODULE.: CPU_AVR.C: CPU specific declarations and functions
 * AUTHOR.: jdesch
 * --------------------------------------------------------------------------
 * DESCRIPTION:
 *
 * This module defines and declares all the AVR related stuff. This means, all
 * the code here is independent from the application code.
 * 
 * All the low level code that is part of the application has to be put into
 * system.c!
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

#define __CPU_AVR_C__
#include "config.h"
#include "cpu_avr.h"



/*+=========================================================================+*/
/*|                      CONSTANT AND MACRO DEFINITIONS                     |*/
/*`========================================================================='*/
/* {{{ */

#ifdef __CODEVISIONAVR__
/* bits of the ADMUX register */
#define ADLAR 5
#define REFS0 6
#define REFS1 7
/* bits of the ADCSRA register */
#define ADEN    7
#define ADSC    6
#define ADATE   5
#define ADIF    4
#define ADIE    3
/* bits of the ACSR register */
#define ACD     7
#define ACBG    6
#define ACO     5
#define ACI     4
#define ACIE    3
#define ACIC    2
#define ACIS1   1
#define ACIS0   0
#endif

#define ADC_PRESCALE_16 4

/* }}} */

/*+=========================================================================+*/
/*|                             LOCAL VARIABLES                             |*/
/*`========================================================================='*/
/* {{{ */

/* Copy of the setup (the upper part)
 */
static BYTE Curr_ADMUX;

/* }}} */

/*+=========================================================================+*/
/*|                    IMPLEMENTATION OF LOCAL FUNCTIONS                    |*/
/*`========================================================================='*/

/*             .-----------------------------------------------.             */
/* ___________/  HANDLE THE WATCHDOG                            \___________ */
/*            `-------------------------------------------------'            */
/* {{{ */

void cpuInitWatchDog (void)
{
#if !CFG_USE_XWATCHDOG
    // dummy
    BYTE i;
    i=1;
#endif
}


void cpuResetWatchDog (void)
{
#if CFG_USE_XWATCHDOG
#  ifndef out_XWATCHDOG
#    error "define out_XWATCHDOG in platform.h as PIN for the extern watchdog."
#  endif
    out_XWATCHDOG = 0;
    out_XWATCHDOG = 1;
#else
#  ifdef __GNUC__
    __asm__ __volatile__ ("wdr" ::);
#  elif __CODEVISIONAVR__
   #asm("wdr")
#  else
#    error "cpu_avr: unknown compiler"
#  endif
#endif
}


/* }}} */

/*             .-----------------------------------------------.             */
/* ___________/  GENERATE A DELAY                               \___________ */
/*            `-------------------------------------------------'            */
/* {{{ */

/**
 *
 * @param time  delay as 10ms steps.
 */
void cpuDelay ( unsigned int time )
{
    unsigned int i, j;

    for ( i=0; i<time; i++ )
    {
	cpuResetWatchDog();
	for ( j=0; j<100; j++ )
	    cpuDelay_us(100);
    }
    cpuResetWatchDog();
}




/** Very short delays. Generate a delay for a minimum of #time_us#
 * microseconds. The time resolution is dependent on the time the loop
 * takes e.g. with 4Mhz and 5 cycles per loop, the resolution is 1.25us.
 *
 * TODO: 1000 loops are 1.42ms on a ATmega2560 !!!!
 *
 * @param time  delay as 1us steps.
 */
void cpuDelay_us ( unsigned short time_us )
{
    unsigned short delay_loops;
    register unsigned short i;

    delay_loops = (time_us+3)/5*CYCLES_PER_US; // +3 for rounding up (dirty)

    // one loop takes 5 cpu cycles
    for (i=0; i<delay_loops; i++) 
    	;

    // TODO: 1000 loop sind 1,42ms !!!! ATmega2560
}

/* }}} */

/*             .-----------------------------------------------.             */
/* ___________/   HANDLE THE ADC                                \___________ */
/*            `-------------------------------------------------'            */
/* {{{ */

void cpuInitADC ( void )
{
    cpuResetWatchDog();
    ACSR = (1<<ACD);			 // analog comparator disabled.
    SFIOR = 0x00;			 // free running mode
#if CFG_USE_8BIT_ADC
    Curr_ADMUX = (2<<REFS0)|(1<<ADLAR);  // VRef=VCC; left-adjusted
#else
    Curr_ADMUX = (2<<REFS0)|(1<<ADLAR);  // VRef=VCC; right-adjusted
#endif
    ADMUX = Curr_ADMUX;
    ADCSRA = (1<<ADEN)|ADC_PRESCALE_16;  // ADC enab; no INT; DIV 16
}


T_ADC cpuReadADC ( BYTE Channel )
{
    T_ADC tmp;
 
    // IMPORTANT: DON'T RESET WATCHDOG HERE!!! THIS CODE COULD BE CALLED
    // FROM WITH AN ISR!   
    ADMUX = Curr_ADMUX|Channel;
    ADCSRA |= (1<<ADSC); 		 // Start the AD conversion
    while ( (ADCSRA&(1<<ADIF))==0 )	 // Wait for the AD conversion to complete
	;
    ADCSRA |= (1<<ADIF);
#if CFG_USE_8BIT_ADC
    tmp = ADCH;
#else
    tmp = ADCL;
    tmp |= (((T_ADC)ADCH)<<8);
#endif
    return tmp;
}

/* }}} */

/*             .-----------------------------------------------.             */
/* ___________/   HANDLE THE PWM                                \___________ */
/*            `-------------------------------------------------'            */
/* {{{ */

/* Setup the PWm generator of the CPU.
 * 
 * AVR: 8bit mode, output inverted, no interrupt, no noise canceler
 */
void cpuInitPWM ( void )
{
#ifdef __GNUC__
  #if defined (__AVR_ATmega128__) || defined (__AVR_ATmega32__)
    /* For this MCU, the value of CFG_PWM_SPEED can be used directly!
     * PWM frequency: 1=31kHz 2=3.9kHz 3=490Hz 
     */
    TCCR1A = 0xA1; TCCR1B = CFG_PWM_SPEED;
    //ICR1H = 0; ICR1L = 0;
    TCNT1H = 0; TCNT1L = 0;
    OCR1AH = 0; OCR1AL = 0;
    OCR1BH = 0; OCR1BL = 0;
    #if defined (__AVR_ATmega128__)
    OCR1CH = 0; OCR1CL = 0;
    #endif
  #else
    #warning "cpu_avr: unsupported MCU type!"
  #endif
#elif __CODEVISIONAVR__
  #if defined _CHIP_ATMEGA128_ | defined _CHIP_ATMEGA32_
    /* For this MCU, the value of CFG_PWM_SPEED can be used directly!
     * PWM frequency: 1=31kHz 2=3.9kHz 3=490Hz 
     */
    TCCR1A = 0xA1; TCCR1B = CFG_PWM_SPEED;
    //ICR1H = 0; ICR1L = 0;
    TCNT1H = 0; TCNT1L = 0;
    OCR1AH = 0; OCR1AL = 0;
    OCR1BH = 0; OCR1BL = 0;
    #if defined _CHIP_ATMEGA128_
    OCR1CH = 0; OCR1CL = 0;
    #endif
  #else
    #warning "cpu_avr: unsupported MCU type!"
  #endif
#else
  #error "cpu_avr: unknown compiler"
#endif
}

void cpuSetPWM1 ( BYTE Speed )
{
    OCR1AH = 0;
    OCR1AL = Speed;
}

void cpuSetPWM2 ( BYTE Speed )
{
    OCR1BH = 0;
    OCR1BL = Speed;
}

void cpuSetPWM3 ( BYTE Speed )
{
#ifdef __GNUC__
  #if defined (__AVR_ATmega128__)
    OCR1CH = 0; OCR1CL = Speed;
  #endif
#elif __CODEVISIONAVR__
  #if defined _CHIP_ATMEGA128_
    OCR1CH = 0; OCR1CL = Speed;
  #endif
#endif
}




#if 0


/* PWM TIMER ISR -- this ISR is (normally) not used. But you never know ;-)
 */
#ifdef __GNUC__
ISR(SIG_OVERFLOW1)
#elif __CODEVISIONAVR__
interrupt [TIM1_OVF] void timer1_ovf_isr(void)
#else
#  error "MC-Module: unknown compiler"
#endif
{
    // not enabled!
}


#endif


/* }}} */

/* ==[End of file]========================================================== */

