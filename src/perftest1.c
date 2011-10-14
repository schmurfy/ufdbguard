/*
 * perftest1.c 
 *
 * ufdbGuard is copyrighted (C) 2005-2010 by URLfilterDB with all rights reserved.
 *
 * Performance test for http://website/cgi-bin/URLblocked.cgi :
 * Open as many sockets to ufdbhttpd or a web server as possible.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/times.h>

#include "ufdblib.h"

#if 0
FILE * globalErrorLog = NULL;
int    globalFatalError = 0;
int    globalDebugTimeDelta = 0;
struct LogFile * globalLogFile = NULL;
int    globalPid = 0;
#endif

char   progname[80] = "perftest1";


int main( 
   int        argc, 
   char *     argv[] )
{
   int        s;
   int        i;
   int        n, len;
   struct tms t;
   int        port;
   char       host[128];
   char       buffer[8192];

#ifdef UFDB_TEST_CGIBIN
   port = 80;
   strcpy( host, "cgibin.urlfilterdb.com" );
#else
   port = 8081;
   strcpy( host, "localhost" );
#endif

   printf( "perftest1: connecting to %s:%d with 4 clients\n", host, port );

   (void) fork();	/* now we are with 2 */
   (void) fork();	/* now we are with 4 */

   UFDBglobalDebug = 1;
   UFDBtimerInit( &t );

   for (i = 0; i < 10000; i++)
   {
      s = UFDBopenSocket( host, port );
      if (s < 0)
      {
         fprintf( stderr, "UFDBopenSocket failed: i=%d %s\n", i, strerror(errno) );
	 exit( 1 );
      }

      sprintf( buffer, "GET /cgi-bin/URLblocked.cgi?mode=normal&color=black&category=proxies&url=www.aap.nl/%06d\r\n"
		       "Referer: http://www.google.com/\r\n"
		       "User-Agent: Mozilla/5.0 (X11; U; Linux i686; en-US; rv:1.8.0.7) Gecko/20061011 Fedora/1.5.0.7-7.fc6 Firefox/1.5.0.7\r\n"
		       "Accept: */*\r\n"
		       "Host: cgibin.urlfilterdb.com\r\n"
		       "Accept-Language: en;q=0.8, nl;q=0.7, de;q=0.6, it;q=0.5, es;q=0.4, *;q=0.1\r\n"
                       "Connection: close\r\n"
		       "\r\n",
		       i );
      len = strlen( buffer );
      n = write( s, buffer, len );
      if (n != len)
      {
         fprintf( stderr, "write failed: i=%d n=%d len=%d %s\n", i, n, len, strerror(errno) );
	 exit( 2 );
      }

      n = read( s, buffer, 8192 );
      if (n <= 0)
      {
         fprintf( stderr, "read failed: i=%d n=%d %s\n", i, n, strerror(errno) );
	 exit( 3 );
      }

      close( s );
   }

   UFDBtimerStop( &t );
   UFDBtimerPrint( &t, "perftest1 10000" );

   return 0;
}

