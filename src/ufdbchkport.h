/*
 * ufdbchkport.h - URLfilterDB
 *
 * ufdbGuard is copyrighted (C) 2005-2009 by URLfilterDB with all rights reserved.
 * 
 * Parts of the ufdbGuard daemon are based on squidGuard.
 * This module is NOT based on squidGuard.
 *
 * RCS $Id: ufdbchkport.h,v 1.5 2010/05/10 17:15:55 root Exp root $
 */

#ifndef UFDB_UFDBCHKPORT_H_INCLUDED
#define UFDB_UFDBCHKPORT_H_INCLUDED

#include "ufdb.h"


int UFDBloadAllowedHTTPSsites( char * filename );

#define UFDBgetTunnelCheckMethod()		UFDBglobalTunnelCheckMethod
void UFDBsetTunnelCheckMethod( int method );

int UFDBinitHTTPSchecker( void );

/* Check for a tunnel.
 *
 * Valid flags are:
 * UFDB_API_ALLOW_QUEUING
 *
 * return values are:
 * UFDB_API_OK:          regular https traffic
 * UFDB_API_REQ_QUEUED:  request to queued for an other thread
 * UFDB_API_ERR_TUNNEL:  https channel is tunneled
 *
 * NOTE: This function may take 3 seconds to complete when the "agressive" option is used !
 */
int UFDBcheckForHTTPStunnel( char * hostname, int portnumber, int flags );

void * UFDBhttpsTunnelVerifier( void * ptr );


struct httpsInfo 
{
   int     status;
   time_t  t;
};

#endif

