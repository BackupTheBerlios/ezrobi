/* -*- Mode: C -*-
 *
 * $Id: remote_protocol.h,v 1.1 2007/08/29 12:42:24 jdesch Exp $
 * -----------------------------------------------------------------------
 * Copyright (c) Dipl.-Ing. Joerg Desch
 * --------------------------------------------------------------------------
 * PROJECT: ezROBI Mega32
 * MODULE.: REMOTE_PROTOCOL.H: simple ASCII communication protocol
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
 * -----------------------------------------------------------------------
 */


#ifndef __REMOTE_PROTOCOL_H__
#define __REMOTE_PROTOCOL_H__ 1

#include "defs.h"
#include "v24_single.h"


/*+=========================================================================+*/
/*|                      CONSTANT AND MACRO DEFINITIONS                     |*/
/*`========================================================================='*/

#define RP_PARM_SEPARATOR   ';'

/* The internaly used command codes
 */
#define RP_CMD_INFO         'I'
#define RP_CMD_ECHO         '!'
#define RP_CMD_DEBUG        'D'
#define RP_CMD_READ_SYSVAR  'R'
#define RP_CMD_WRITE_SYSVAR 'W'
// more to come...


/*+=========================================================================+*/
/*|                            TYPEDECLARATIONS                             |*/
/*`========================================================================='*/

/* The prototype of the callback function passed as handler to the
 * remote protocol initialiser.
 */
typedef BOOL (*T_RemoteCallback)(BYTE cmd, WORD* args, WORD cnt);


/*+=========================================================================+*/
/*|                            PUBLIC VARIABLES                             |*/
/*`========================================================================='*/

/*+=========================================================================+*/
/*|                     PROTOTYPES OF GLOBAL FUNCTIONS                      |*/
/*`========================================================================='*/


/** Initialize the protocol parser.
 *
 *
 * @param callback  the command handler.
 */
void rp_InitRemote ( T_RemoteCallback callback );


/** Parse command sequences. The application must initialize the serial port,
 * and then wait for incoming characters. If there is something in the
 * \emph{receive queue}, the application must call this function to parse a
 * possible command sequence.
 *
 * Once called, #rp_ParseCommand# scans the input for a valid sequence. The
 * command code and it's parameters are separated. With the completion of a
 * sequence, the previously installed handler is called. The command code and
 * the parameters are passed to the handler.
 *
 * If the doesn't know the command, it must return #FALSE#. This leads to an
 * \emph{NAK} in the protocol. This is also the case, if the structure of the
 * sequence is damaged.
 *
 * If the handler returns #TRUE#, the whole process is completed. As a result
 * of this success, a \emph{ACK} is sent.
 */
void rp_ParseCommand ( void );

#endif
/* ==[End of file]========================================================== */
