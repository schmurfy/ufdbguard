/*
 * ufdbpeek.c - URLfilterDB
 *
 * ufdbGuard is copyrighted (C) 2005-2011 by URLfilterDB with all rights reserved.
 *
 * Parts of the ufdbGuard daemon are based on squidGuard.
 *
 * RCS: $Id: ufdbpeek.c,v 1.7 2011/05/10 19:36:21 root Exp root $
 */

#include "ufdb.h"
#include "ufdbchkport.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "sg.h"


static void usage( void )
{
   fprintf( stderr, "usage: ufdbpeek [-p <port>] [-C CAcertsFile] [-A] [-G] [-S] [-Y] <domain>\n" );
   exit( 1 );
}


int main( int argc, char * argv[] )
{
   char * t;
   char * url;
   int    opt;
   int    port;

   strcpy( progname, "ufdbpeek" );
   globalPid = getpid();
   ufdbSetGlobalErrorLogFile();
   globalErrorLog = NULL;

   if (argc <= 1)
      usage();

   UFDBglobalDebug = 1;
   UFDBglobalHttpsOfficialCertificate = 1;
   UFDBglobalHttpsWithHostname = 1;
   UFDBsetTunnelCheckMethod( UFDB_API_HTTPS_CHECK_AGRESSIVE );

   port = 443;
   url = NULL;

   while ((opt = getopt( argc, argv, "?C:hp:AGSY" )) > 0)
   {
      switch (opt) {
      case 'C':
      	 strcpy( UFDBglobalCAcertsFile, optarg );
	 break;
      case 'p':
         port = atoi( optarg );
	 break;
      case 'A':
	 UFDBglobalDebugAim = 1;
	 UFDBglobalAimOverHttps = 1;
         break;
      case 'G':
	 UFDBglobalDebugGtalk = 1;
	 UFDBglobalGtalkOverHttps = 1;
         break;
      case 'S':
	 UFDBglobalDebugSkype = 1;
	 UFDBglobalSkypeOverHttps = 1;
         break;
      case 'Y':
	 UFDBglobalDebugYahooMsg = 1;
	 UFDBglobalYahooMsgOverHttps = 1;
         break;
      case '?':
      case 'h':
          usage();
	  exit( 0 );
	  break;
      }
   }

   if (optind < argc)
      url = argv[optind];
   else
      usage();

   UFDBinitHTTPSchecker();
   if (strncmp( url, "http://", 7 ) == 0)
      url += 7;
   if (strncmp( url, "https://", 8 ) == 0)
      url += 8;
   if ((t = strchr( url, '@' )) != NULL)
      *t = '\0';
   if ((t = strchr( url, ':' )) != NULL)
      *t = '\0';
   if ((t = strchr( url, '/' )) != NULL)
      *t = '\0';

   (void) UFDBcheckForHTTPStunnel( url, port, UFDB_API_VERBOSE_OUTPUT );

   ufdbLogError( "" );

   if (globalErrorLog != NULL)
   {
      fflush( globalErrorLog );
      fclose( globalErrorLog );
   }

   return 0;
}

