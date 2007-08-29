/* -*- Mode: C -*-
 *
 * $Id: v24_single.c,v 1.1 2007/08/29 12:38:07 jdesch Exp $
 * --------------------------------------------------------------------------
 * Copyright  (c) Dipl.-Ing. Joerg Desch
 * --------------------------------------------------------------------------
 * PROJECT: ezROBI Mega32
 * MODULE.: V24_SINGLE.C: single UART module
 * AUTHOR.: jdesch
 * --------------------------------------------------------------------------
 * DESCRIPTION:
 * This module implements a reduced and simplified V24 module. The 
 * initialisation of the serial port and the ISR must be done inside
 * the SYSTEM module. The V24 parameters are defines inside config.h!
 *
 * NOTE: this implentation do not use the TxD interrupt!
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

#define __V24_SINGLE_C__
#include "config.h"
#include "v24_single.h"
#include "cpu_avr.h"                         /* CPU specific functions */
#ifdef __GNUC__
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#endif


#define WANT_PARITY_ERRORS 0            // RxD-ISR checks for frame errors


/*+=========================================================================+*/
/*|                      CONSTANT AND MACRO DEFINITIONS                     |*/
/*`========================================================================='*/

#ifndef V24_BUFFSIZE
# define V24_BUFFSIZE 0x08
# define V24_BUFFMASK 0x07
#endif

#ifdef __CODEVISIONAVR__
  // CodeVisionAVR has no bit declaration
  /* UCSRB */
#  define RXCIE   7
#  define TXCIE   6
#  define UDRIE   5
#  define RXEN    4
#  define TXEN    3
#  define UCSZ2   2
#  define RXB8    1
#  define TXB8    0
#endif

#ifdef __GNUC__
#define FRAMING_ERROR (1<<FE)
#define PARITY_ERROR (1<<PE)
#define DATA_OVERRUN (1<<DOR)
#elif __CODEVISIONAVR__
#define FRAMING_ERROR (1<<FE)
#define PARITY_ERROR (1<<OPE)
#define DATA_OVERRUN (1<<OVR)
#else
#error "V24-Module: unknown compiler"
#endif



/*             .-----------------------------------------------.             */
/* ___________/  local macro declaration                        \___________ */
/*            `-------------------------------------------------'            */

/*+=========================================================================+*/
/*|                          LOCAL TYPEDECLARATIONS                         |*/
/*`========================================================================='*/


struct tagUART_ACCESS
{
        BYTE Buffer[V24_BUFFSIZE];
        BYTE IdxHead;
        BYTE IdxTail;
        BYTE Avail;
        int  TimeoutMS;          /* timeout-time in [ms] */
        BYTE OpenFlags;
        BYTE Errno;
        BOOL Opened;
        BOOL TimedOut;
};
typedef struct tagUART_ACCESS T_UartAccess;


/*+=========================================================================+*/
/*|                             LOCAL VARIABLES                             |*/
/*`========================================================================='*/

/* Die Arbeitsvariablen der geöffneten Ports befinden sich in dieser Struktur.
 */
static volatile T_UartAccess Uart;

static const char Hex8[]= {"0123456789ABCDEF"};


/*+=========================================================================+*/
/*|                      PROTOTYPES OF LOCAL FUNCTIONS                      |*/
/*`========================================================================='*/

static void __waitMS (void);
static void enterCritical (void);
static void leaveCritical (void);


/*+=========================================================================+*/
/*|                     IMPLEMENTATION OF THE FUNCTIONS                     |*/
/*`========================================================================='*/


void v24InitModule (void)
/* {{{  */
{
    cpuResetWatchDog();
	Uart.IdxHead=Uart.IdxTail=0;
	Uart.Avail=V24_BUFFSIZE;
	Uart.TimeoutMS=1000;      /* timeout-time in [ms] */
	Uart.Errno=V24_E_NONE;
	Uart.Opened=TRUE;
	Uart.OpenFlags=0;
	Uart.TimedOut=FALSE;
}
/* }}} */


BOOL v24SetTimeouts ( int TenthOfSeconds )
/* {{{  */
{
    cpuResetWatchDog();
    if ( !Uart.Opened )
        return FALSE;
    Uart.Errno=V24_E_NONE;
    Uart.TimeoutMS = TenthOfSeconds*100;
    return TRUE;
}
/* }}} */


int v24Getc ( void )
/* {{{  */
{
    int WaitLoop;
    /* BYTE btmp;*/

    cpuResetWatchDog();
    Uart.TimedOut=FALSE;
    Uart.Errno=V24_E_NONE;
    if ( Uart.Avail<V24_BUFFSIZE )
        return v24GetQueue();

    for ( WaitLoop=0; WaitLoop<Uart.TimeoutMS; ++WaitLoop )
    {
        cpuResetWatchDog();
        __waitMS();                          /* wait 1ms */
        if ( Uart.Avail<V24_BUFFSIZE )
            return v24GetQueue();
    }
    Uart.Errno=V24_E_TIMEOUT;
    Uart.TimedOut=TRUE;
    return -2;
}
/* }}} */


int v24GetQueue ( void )
/* {{{  */
{
    BYTE btmp;

    cpuResetWatchDog();
    if ( Uart.Avail==V24_BUFFSIZE )
        return -1;
    if ( !Uart.Opened )
        return -1;
    enterCritical();
    ++Uart.Avail;
    btmp = Uart.Buffer[Uart.IdxTail++];
    Uart.IdxTail &= V24_BUFFMASK;
    leaveCritical();
    return (int)btmp;
}
/* }}} */


/* Simply send a byte over the UART. The functions waits for an empty
 * transmit register. If the "open flag" V24_OF_WAITFORTX is set, the
 * function waits for the transmittion shift-register to become empty
 * too. This way we can ensure, that after the function returns, the
 * the byte is definitly sent.
 */
BOOL v24Putc ( unsigned char txData )
/* {{{  */
{
    cpuResetWatchDog();
    if ( !Uart.Opened )
        return FALSE;
	while(!(UCSRA & (1<<UDRE)))
	   ;
	outb(UDR, txData);
#if CFG_V24_WAIT_FOR_TX
	cpuResetWatchDog();
	while(!(UCSRA & (1<<UDRE)))    // first wait for this, than
	   ;
	while( !(UCSRA & (1<<TXC)) )   // wait for shift-register to become empty
	   ;
	sbi(UCSRA,TXC);                // write 1 to clear. 8-)
#endif
    return TRUE;
}
/* }}} */


int v24Gets ( char* Buffer, int BuffSize )
/* {{{  */
{
    int i, rc=0;

    cpuResetWatchDog();
    if ( !Buffer )
        return -1;
    if ( !Uart.Opened )
        return -1;
    for ( i=0; i<(BuffSize-1); ++i )
    {
        rc=v24Getc();
        if ( rc<0 )
            break;
        Buffer[i]=(BYTE)rc;
        if ( rc==0 || rc=='\n' )
            break;
    }
    Buffer[i]='\0';                          /* term. string even for timeouts */
    return (rc<0) ? rc : i;                  /* return error code or length */
}
/* }}} */


int v24Puts ( const_str PtrBuffer )
/* {{{  */
{
    int i;

    cpuResetWatchDog();
    if ( !PtrBuffer )
        return -1;
    if ( !Uart.Opened )
        return -1;
    i=0;
    while ( PtrBuffer[i] )
    {
        if ( PtrBuffer[i]=='\n' )            /* must send a LF? */
            v24Putc('\r');                   /* send a CR first */
        v24Putc(PtrBuffer[i]);
        ++i;
    }
    return i;
}
/* }}} */


int v24PutsP ( flash_str PtrBuffer )
/* {{{  */
{
    int i;
    char c;

    cpuResetWatchDog();
    if ( !PtrBuffer )
        return -1;
    if ( !Uart.Opened )
        return -1;
    i=0;
#ifdef __GNUC__

    c=pgm_read_byte(((uint8_t*)PtrBuffer)+i);
    while ( c!=0 )
#elif __CODEVISIONAVR__
    while ( (c=PtrBuffer[i])!=0 )
#else
  #error "V24-Module: unknown compiler"
#endif
    {
        if ( c=='\n' )                       /* must send a LF? */
            v24Putc('\r');                   /* send a CR first */
        v24Putc(c);
        ++i;
#ifdef __GNUC__
        c=pgm_read_byte(((uint8_t*)PtrBuffer)+i);
#endif
    }
    return i;
}
/* }}} */


BOOL v24HaveData ( void )
/* {{{  */
{
    BOOL result;

    cpuResetWatchDog();
    if ( !Uart.Opened )
        return FALSE;
    enterCritical();
    result = (Uart.Avail==V24_BUFFSIZE) ? FALSE : TRUE;
    leaveCritical();
    return result;
}
/* }}} */


BOOL v24FlushRxQueue ( void )
/* {{{  */
{
    cpuResetWatchDog();
    if ( !Uart.Opened )
        return FALSE;
    enterCritical();
    Uart.IdxHead=Uart.IdxTail=0;
    Uart.Avail=V24_BUFFSIZE;
    leaveCritical();
    return TRUE;
}
/* }}} */


BOOL v24TimedOut ( void )
/* {{{  */
{
    cpuResetWatchDog();
    return Uart.TimedOut;
}
/* }}} */


BYTE v24QueryErrno ( void )
/* {{{  */
{
    cpuResetWatchDog();
    return Uart.Errno;
}
/* }}} */


/*             .-----------------------------------------------.             */
/* ___________/  Additional Functions                           \___________ */
/*            `-------------------------------------------------'            */

#if V24_NEED_PUT_UNSIGNED
void v24PutUnsigned ( v24_port_t port, WORD num )
{
    BOOL SentOne=FALSE;		     /* um trailing zeros zu vermeiden */
    WORD CurrDigit=10000;
    WORD Remain;
    BYTE d;
    
    if ( num==0 )
    {
	v24Putc('0');
	return;
    }
    
    while ( CurrDigit )
    {
	if ( !SentOne && num<CurrDigit )
	{
	    CurrDigit /= 10;		/* supress leading zeros */
	}
	else
	{
	    Remain=num/CurrDigit;
	    d = (BYTE)(Remain%10);
	    v24Putc('0'+d);
	    CurrDigit /= 10;
	    SentOne=TRUE;
	}
/* 	printf(">CurrDigit=%d\n",CurrDigit); */
/* 	printf("Remain=%d\n",Remain); */
/* 	printf("Remain\%10=%d\n",Remain%10); */
    }
}
#endif

#if V24_NEED_PUT_WORD
void v24PutWord ( WORD num )
{
    v24Putc(Hex8[(num>>12)&0x000F]);
    v24Putc(Hex8[(num>>8)&0x000F]);
    v24Putc(Hex8[(num>>4)&0x000F]);
    v24Putc(Hex8[(num)&0x000F]);
}
#endif

#if V24_NEED_PUT_BYTE
void v24PutByte ( BYTE num )
{
    v24Putc(Hex8[(num>>4)&0x0F]);
    v24Putc(Hex8[(num)&0x0F]);
}
#endif


/*+=========================================================================+*/
/*|                    IMPLEMENTATION OF LOCAL FUNCTIONS                    |*/
/*`========================================================================='*/


static void __waitMS (void)
/* {{{ */
{
    cpuDelay_us(1000);          // wait 1ms
}
/* }}} */


static void enterCritical ( void )
/* {{{  */
{
    if ( !Uart.Opened )
        return;
	cbi(UCSRB,RXCIE);
	//cbi(UCSRB,TXCIE);
}
/* }}} */


static void leaveCritical ( void )
/* {{{  */
{
    if ( !Uart.Opened )
        return;
	sbi(UCSRB,RXCIE);
	//sbi(UCSRB,TXCIE);
}
/* }}} */


/*+=========================================================================+*/
/*|                       INTERRUPT SERVICE FUNCTIONS                       |*/
/*`========================================================================='*/


#ifdef __GNUC__
ISR(SIG_USART_RECV)
#elif __CODEVISIONAVR__
interrupt [USART_RXC] void usart_rx_isr(void)
#else
  #error "V24-Module: unknown compiler"
#endif
{
    register BYTE b;
    BYTE s;

    s = (UCSRA)&(FRAMING_ERROR|PARITY_ERROR|DATA_OVERRUN);
    b = UDR;
#if WANT_PARITY_ERRORS
    if ( s )
    {
        Uart.Errno=V24_E_FRAME;
    }
    else
#endif
    {
        if ( Uart.Avail )
        {
            --Uart.Avail;
            Uart.Buffer[Uart.IdxHead++] = b;
            Uart.IdxHead &= V24_BUFFMASK;
            Uart.Errno=V24_E_NONE;
        }
        else
        {
            Uart.Errno=V24_E_OVERFLOW;
        }
    }
}

/* WE DON'T USE TXD-INTERRUPTS FOR NOW
#ifdef __GNUC__
SIGNAL((SIG_USART_TRANS)
#else
interrupt [USART_TXC] void usart_tx_isr(void)
#endif
{
}
*/



/* ==[End of file]========================================================== */

