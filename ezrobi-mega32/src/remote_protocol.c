/* -*- Mode: C -*-
 *
 * $Id: remote_protocol.c,v 1.3 2008/04/14 04:24:35 jdesch Exp $
 * -----------------------------------------------------------------------
 * Copyright (c) Dipl.-Ing. Joerg Desch
 * --------------------------------------------------------------------------
 * PROJECT: ezROBI Mega32
 * MODULE.: REMOTE_PROTOCOL.C: simple ASCII communication protocol
 * AUTHOR.: jdesch
 * -----------------------------------------------------------------------
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
 * -----------------------------------------------------------------------
 * COMPILER-FLAGS:
 *
 *
 * -----------------------------------------------------------------------
 */

#define __REMOTE_PROTOCOL_C__

#include "config.h"
#include "remote_protocol.h"


/*+=========================================================================+*/
/*|                      CONSTANT AND MACRO DEFINITIONS                     |*/
/*`========================================================================='*/

//#define DEBUG_RP 1

#if (RP_MAX_PARAMETERS < 3)
# error "fatal: remote_protocol needs at least three parameters!"
#endif

#ifdef RP_MAX_PARAMETERS
#define MAXPARMS RP_MAX_PARAMETERS
#else
#define MAXPARMS 3
#endif


/* `Inter-Command Timeout'  --  while we are inside a command, we expect, that
 * the next character is received within this timeout time. For debugging
 * purposes, this timeout is much longer.
 */
#ifdef DEBUG
#define TIMEOUT        6000       /* TimeOut-Zähler=1min (10ms steps) */
#else
#define TIMEOUT         200       /* TimeOut-Zähler=2s (10ms steps) */
#endif


/*  some common constants used for the remote protocol
 */
#define ESC             0x1B      /* the famous ESC character */
#define ACK             0x06      /* the acknowledge character */
#define EOC             '\r'      /* the EndOfCommand terminates the sequence */
#define ERR             0x15      /* a simple '*' as Not-Acknowledge */
#define STX             0x02
#define ETX             0x03


/* (C)ommunicatio(S)tates
 */
enum state_t { csSTART, csPARMS, csEXEC, csSTOP };


/* (P)arameter (T)ypes
 */
enum parm_t { ptDEZ, ptHEX };


/*             .-----------------------------------------------.             */
/* ___________/  local macro declaration                        \___________ */
/*            `-------------------------------------------------'            */

#define IS_DEZ_DIGIT(c) ( ((c)>='0') && ((c)<='9') )
#define IS_HEX_DIGIT(c) (  (((c)>='0') && ((c)<='9'))\
                        || (((c)>='A') && ((c)<='F'))\
                        || (((c)>='a') && ((c)<='f')) )


/*+=========================================================================+*/
/*|                          LOCAL TYPEDECLARATIONS                         |*/
/*`========================================================================='*/

/*+=========================================================================+*/
/*|                            PUBLIC VARIABLES                             |*/
/*`========================================================================='*/

/*+=========================================================================+*/
/*|                             LOCAL VARIABLES                             |*/
/*`========================================================================='*/

//static BYTE bTmp;
static WORD wTmp;
static BYTE ParmCnt;			     /* how many parms */
static WORD Parm[MAXPARMS];		     /* num. parameters */

// (jd) I've removed the casts of the 0, because CodeVision ignores the allocation
// of the variables in this case.
static T_RemoteCallback __callb=0;

static BOOL __fConvFailed=FALSE;


/*+=========================================================================+*/
/*|                      PROTOTYPES OF LOCAL FUNCTIONS                      |*/
/*`========================================================================='*/

static BOOL parseDezParm ( BYTE c );
static BOOL parseHexParm ( BYTE c );
static BYTE valueOfHex ( char cDigit );


/*+=========================================================================+*/
/*|                     IMPLEMENTATION OF THE FUNCTIONS                     |*/
/*`========================================================================='*/


void rp_InitRemote ( T_RemoteCallback callback )
{
    cpuResetWatchDog();
    if ( !callback )
    {
#ifdef DEBUG
	v24PutsP(PSTR("RP:init: 0 pointer!\n"));
#endif
	return;
    }
    __callb=callback;
    __fConvFailed=FALSE;
}


/* This functions starts parsing a command sequence. If a valid sequence have
 * been parsed, the local callback function is called.
 *
 */
void rp_ParseCommand ( void )
{
    BYTE Command=0;
    BOOL HaveParm=FALSE;
    BOOL NewParmStarted=FALSE;
    BOOL send_err;
    BYTE ParmType=ptDEZ;
    BYTE state;
    BYTE c;
    int rc;

    cpuResetWatchDog();
    rc=v24Getc();
    if ( rc<0 )				    /* read failed */
	return;
    c = (BYTE)rc;

    if ( c == EOC )                         /* ACK is forced. */
    {
	v24Putc(ACK);
        return;
    }

    if ( c < 0x20 )                         /* seq started with printable */
        return;

    state=csSTART;
    cpuResetWatchDog();
    do
    {
        /*  The parsing state machine
         */
        switch (state)
	{
            case csSTART:		     /* --- The Beginning */
#ifdef DEBUG_RP
	        v24PutsP(PSTR("RP:ccSTART\n-->"));
	        v24Putc(c); v24PutsP(PSTR("<--\n")); 
#endif
                Command=c;
                ParmCnt=0;		     /* clear old parms */
                Parm[ParmCnt]=0;
                HaveParm=FALSE;
		NewParmStarted=TRUE;
		ParmType=ptDEZ;
                state=csPARMS;
                break;

            case csPARMS:		     /* --- collect parameters */
                /* wait for new characters
                 */
#ifdef DEBUG_RP
		v24PutsP(PSTR("RP:ccPARMS\n"));
#endif
                wTmp=TIMEOUT;
                cpuResetWatchDog();
                while ( --wTmp )
		{
                    cpuResetWatchDog();
                    if ( v24HaveData() )
			break;
		    else
			cpuDelay_ms(10);
                }
                if ( wTmp==0 )
		{
#ifdef DEBUG_RP
		    v24PutsP(PSTR("RP:timeout\n"));
#endif
                    Command=0;
                    return;
                }
                c=(BYTE)v24GetQueue();

                /* parse character
                 */
                cpuResetWatchDog();
                if ( c == RP_PARM_SEPARATOR )    /* Parameter termination? */
		{
                    if ( HaveParm )	         /* not the first parm? */
		    {
			++ParmCnt;	         /* than go on */
#ifdef DEBUG_RP
		        v24PutsP(PSTR("RP:++ParmCnt\n"));
#endif
		    }
		    ParmType=ptDEZ;	         /* our default type */
		    NewParmStarted=TRUE;
                    if ( ParmCnt==MAXPARMS )
		    {
#ifdef DEBUG_RP
		        v24PutsP(PSTR("RP:MAXPARMS\n"));
#endif
                        ParmCnt=Command=0;
			v24Putc(ERR);
                        return;
                    }
                    Parm[ParmCnt]=0;
                }
		else if ( c == EOC )
		{
                    if (HaveParm)
		    {
			++ParmCnt;               /* terminate parm */
		    }
#ifdef DEBUG_RP
		    v24PutsP(PSTR("RP:EOC\n--->#"));
	            v24Putc('0'+ParmCnt); v24PutsP(PSTR("<--\n")); 
#endif
                    state=csEXEC;                /* next state.. */
                    break;
                }
		else if ( c == ESC )             /* seq. aborted? */
		{
#ifdef DEBUG_RP
		    v24PutsP(PSTR("RP:abort\n"));
#endif
                    HaveParm=FALSE;
		    NewParmStarted=FALSE;
                    Command=ParmCnt=0;
                    return;
                }
		else if ( NewParmStarted && c=='#' )
		{
#ifdef DEBUG_RP
		    v24PutsP(PSTR("RP:hextype\n"));
#endif
		    ParmType=ptHEX;
		}
		else
		{
                    HaveParm=TRUE;
		    NewParmStarted=FALSE;
		    send_err=FALSE;
		    switch (ParmType)
		    {
			case ptHEX:
			    if ( !parseHexParm(c) )
				send_err=TRUE;
			    break;
			default:
			    if ( !parseDezParm(c) )
				send_err=TRUE;
		    }
		    if ( send_err )
		    {
#ifdef DEBUG_RP
		        v24PutsP(PSTR("RP:parse-err\n"));
#endif
			ParmCnt=Command=0;
                        state = csSTOP;
			v24Putc(ERR);
		    }
		}
		break;

            case csEXEC:		     /* --- Execute Command */
#ifdef DEBUG_RP
		v24PutsP(PSTR("RP:ccEXEC\n"));
#endif
		cpuResetWatchDog();
    		if ( !__callb  )
    		{
#ifdef DEBUG
	v24PutsP(PSTR("RP:init: 0 pointer!\n"));
#endif
		    v24Putc(ERR);
		    return;
		}
		if ( __callb(Command,Parm,ParmCnt) )
		    v24Putc(ACK);
		else
		    v24Putc(ERR);
                ParmCnt = 0;
                state = csSTOP;
                break;

            case csSTOP:		     /* --- Parser finished */
#ifdef DEBUG_RP
		v24PutsP(PSTR("RP:ccSTOP\n"));
#endif
                break;
            default:
                state=csSTOP;
                break;
        }
    } while ( state!=csSTOP );
    cpuResetWatchDog();
}



/*+=========================================================================+*/
/*|                    IMPLEMENTATION OF LOCAL FUNCTIONS                    |*/
/*`========================================================================='*/


static BOOL parseDezParm ( BYTE c )
{
#ifdef DEBUG_RP
    v24PutsP(PSTR("RP:parseDezParm("));
    v24Putc(c);
    v24PutsP(PSTR(")\n"));
#endif
    if ( ! IS_DEZ_DIGIT(c) )
	return FALSE;
    if ( Parm[ParmCnt]==6553 && c>'5' )
	return FALSE;
    if ( Parm[ParmCnt]>6553 )
	return FALSE;
    Parm[ParmCnt] *= 10;
    Parm[ParmCnt] += (c-'0');
    return TRUE;
}


static BOOL parseHexParm ( BYTE c )
{
#ifdef DEBUG_RP
    v24PutsP(PSTR("RP:parseHexParm("));
    v24Putc(c);
    v24PutsP(PSTR(")\n"));
#endif
    if ( ! IS_HEX_DIGIT(c) )
	return FALSE;
    if ( Parm[ParmCnt] > 0xFFF )
	return FALSE;

    Parm[ParmCnt] *= 16;
    Parm[ParmCnt] += valueOfHex(c);
    return TRUE;
}


static BYTE valueOfHex (char cDigit)
{
    __fConvFailed=FALSE;
    if ( IS_DEZ_DIGIT(cDigit) )
    {
        return (cDigit-'0');
    }
    else if ( cDigit>='A' && cDigit<='F' )
    {
        return (cDigit-'A'+10);
    }
    else if ( cDigit>='a' && cDigit<='f' )
    {
        return (cDigit-'a'+10);
    }
    else
    {
        __fConvFailed=TRUE;
        return 0;
    }
}


/* ==[End of file]========================================================== */
