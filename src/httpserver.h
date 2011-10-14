/*
 * httpserver.h - URLfilterDB
 *
 * ufdbGuard is copyrighted (C) 2005-2009 by URLfilterDB with all rights reserved.
 *
 * RCS $Id: httpserver.h,v 1.5 2009/12/04 12:20:56 root Exp root $
 */

#ifndef UFDB_HTTPSERVER_H_INCLUDED
#define UFDB_HTTPSERVER_H_INCLUDED

#define UFDBHTTPD_PID_FILE	"/var/tmp/ufdbhttpd.pid"

void ufdbSimulateHttpServer( 
	const char * interface, 
	int port, 
	const char * username,
	const char * imagesDirectory, 
	int flags );

#endif

