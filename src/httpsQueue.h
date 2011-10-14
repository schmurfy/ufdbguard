/*
 * httpsQueue.h - URLfilterDB
 *
 * ufdbGuard is copyrighted (C) 2005-2010 by URLfilterDB with all rights reserved.
 * 
 * Parts of the ufdbGuard daemon are based on squidGuard.
 * This module is NOT based on squidGuard.
 *
 * RCS $Id: httpsQueue.h,v 1.5 2010/05/10 17:06:44 root Exp root $
 */

#ifndef UFDB_HTTPSQUEUE_H_INCLUDED
#define UFDB_HTTPSQUEUE_H_INCLUDED

#include "ufdb.h"

/* A HTTPS/SSL verifier thread mostly waits for network I/O
 * and uses relatively little CPU.  To have a high throughput
 * in the SSL verification, a "high" number of threads are used.
 */
#define UFDB_NUM_HTTPS_VERIFIERS   14

void initHttpsQueue( void );
int ufdbHttpsQueueRequest( char * hostname, int portnumber );
void ufdbGetHttpsRequest( char * hostname, int * portnumber );
int UFDBhttpsVerificationQueued( void );

#endif

