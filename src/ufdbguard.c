/*
 * ufdbguard,c - URLfilterDB
 *
 * ufdbGuard is copyrighted (C) 2005-2011 by URLfilterDB with all rights reserved.
 *
 * Parts of the ufdbGuard daemon are based on squidGuard.
 * squidGuard is copyrighted (C) 1998 by
 * ElTele Øst AS, Oslo, Norway, with all rights reserved.
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License (version 2) as
 * published by the Free Software Foundation.  It is distributed in the
 * hope that it will be useful, but WITHOUT ANY WARRANTY; without even the
 * implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License (GPL) for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * (GPL) along with this program.
 */

#include "ufdb.h"
#include "sg.h"
#include "ufdblib.h"
#include "ufdbchkport.h"

#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <libgen.h>
#include <signal.h>
#include <sys/time.h>

/* use linebuffer output: flush() is not needed any more */
#if 0
#define FLUSH(f)   fflush(f)
#else
#define FLUSH(f)   /* */
#endif


static int parseOnly = 0;
static int noRedirects = 0;



static void usage( void )
{
  fprintf( stderr, 
	   "Usage: ufdbGuard [-C block] [-t time] [-c file] [-v] [-d]\n" );
  fprintf( stderr, "Options:\n" );
  fprintf( stderr, "  -v          : show version number\n" );
  fprintf( stderr, "  -d          : all errors to stderr\n" );
  fprintf( stderr, "  -c file     : load alternate configfile\n" );
  fprintf( stderr, "  -t time     : specify startuptime on format: yyyy-mm-ddTHH:MM:SS\n" );
#if 0
  fprintf( stderr, "  -u          : update .ufdb files from .diff files\n" );
  fprintf( stderr, "  -C file|all : create new .db files from urls/domain files\n" );
  fprintf( stderr, "                specified in \"file\".\n" );
#endif
  fprintf( stderr, "  -T          : test mode: log unwanted URLs but do not block them\n" );
  fprintf( stderr, "  -r          : log the redirections\n" );

  exit( 1 );
}


int ufdbguard_main( int argc, char ** argv, char ** envp )
{
  int    ch;
  long   numreq = 0;
  struct SquidInfo squidInfo;
  struct Source * src;
  struct Acl * acl;
  struct timeval start_time, ready_time, stop_time;
  char * redirect;
  char * configFile = NULL;
  time_t t;
  char * vbuf;
  struct tms timer;
  UFDBthreadAdmin * admin;
  char   niso_str[22];
  char   buf[UFDB_MAX_URL_LENGTH];
  char   tmp[UFDB_MAX_URL_LENGTH];

  strcpy( progname, "ufdbGuard" );

  UFDBtimerInit( &timer );
  admin = UFDBallocThreadAdmin();

  /* instead of using fflush(), set linebuffer mode for stdout */
  vbuf = ufdbMalloc( 4096 );
  (void) setvbuf( stdout, vbuf, _IOLBF, 4096 );

  gettimeofday( &start_time, NULL );
  globalPid = getpid();
  while ((ch = getopt(argc, argv, "Phdrt:c:vCu")) > 0)
  {
    switch (ch) {
    case 'd':
      UFDBglobalDebug = 1;
      break;
    case 'r':
      globalDebugRedirect = 1;
      break;
    case 'c':
      configFile = optarg;
      break;
    case 'C':
    case 'u':		
      fprintf( stderr, "ufdbGuard: option '-%c' is not implemented. Use ufdbGenTable instead.\n", ch );
      exit( 1 );
      break;
    case 'v':
      fprintf( stderr, "ufdbGuard: %s\n", VERSION );
      exit( 0 );
      break;
    case 't':
      if ((t = iso2sec(optarg)) == -1) {
	fprintf( stderr, "-t dateformat error, should be yyyy-mm-ddTHH:MM:SS\n" );
	exit(0);
      }
      if (t < 0) {
	fprintf( stderr, "-t date have to after 1970-01-01T01:00:00\n" );
	exit(0);
      }
      niso( t, niso_str );
      ufdbLogError( "ufdbGuard emulating date %s", niso_str );
      globalDebugTimeDelta = t - start_time.tv_sec;
      start_time.tv_sec = start_time.tv_sec + globalDebugTimeDelta;
      break;
    case 'T':
      ufdbLogError( "-T option found.  Going into test mode." );
      noRedirects = 1;		/* Test mode means noRedirects */
      break;
    case 'P':			/* undocumented -P option for development purposes */
      parseOnly = 1;
      break;
    case '?':
    case 'h':
    default:
      usage();
    }
  }

  globalArgv = argv;
  globalEnvp = envp;
  ufdbSetGlobalErrorLogFile();
  sgReadConfig( configFile );
  /* ufdbSetGlobalErrorLogFile(); */
  ufdbLogMessage( "ufdbGuard " VERSION " started at %d.%03d", 
                  start_time.tv_sec, start_time.tv_usec/1000 );
  if (UFDBglobalDebug)
     ufdbLogMessage( "debug output is enabled." );

  ufdbSetThreadCPUaffinity( 0 );
  UFDBinitHTTPSchecker();

  if (globalUpdate || globalCreateDb != NULL)
  {
    ufdbLogError( "db update done" );
    gettimeofday( &stop_time, NULL );
    stop_time.tv_sec = stop_time.tv_sec + globalDebugTimeDelta;
    ufdbLogError( "ufdbGuard stopped at %d.%03d",
	          stop_time.tv_sec, stop_time.tv_usec/1000 );
    exit( 0 );
  }

  sgTimeElementSortEvents();
  ufdbHandleAlarmForTimeEvents( UFDB_PARAM_INIT );

  ufdbSetSignalHandler( SIGHUP,  sgHandlerSigHUP );
  ufdbSetSignalHandler( SIGPIPE, SIG_IGN );

  ufdbSetSignalHandler( SIGINT,  sgSignalHandler );
  ufdbSetSignalHandler( SIGTERM, sgSignalHandler );
#if 0
  ufdbSetSignalHandler( SIGQUIT, sgSignalHandler );
  ufdbSetSignalHandler( SIGILL,  sgSignalHandler );
  ufdbSetSignalHandler( SIGSEGV, sgSignalHandler );
#endif

  UFDBlogConfig();

  gettimeofday( &ready_time, NULL );
  ready_time.tv_sec = ready_time.tv_sec + globalDebugTimeDelta;
  ufdbLogError( "ufdbGuard ready for requests at %d.%03d",
	        ready_time.tv_sec, ready_time.tv_usec/1000 );

#if 1
  UFDBtimerStop( &timer );
  UFDBtimerPrint( &timer, "time to initialize" );
  UFDBtimerInit( &timer );
#endif

  UFDBglobalReconfig = 0;

  while (1)
  {
    while (fgets(buf, UFDB_MAX_URL_LENGTH, stdin) != NULL)	   /********** MAIN LOOP ************/
    {
      buf[UFDB_MAX_URL_LENGTH-1] = '\0';   		/* force null termination */
      if (sig_hup)
      {
	sgReloadConfig();
	sig_hup = 0;
      }
      if (sig_other)
        break;
      numreq++;

      if (UFDBglobalFatalError || failsafe_mode) {
	putchar( '\n' );		/* OK */
	FLUSH( stdout );
	continue;
      }

#if 0
      fprintf( stderr, ">>> %s\n", buf );
#endif

      if (parseLine(admin, buf, &squidInfo) != 1) 	/* put input line in struct squidInfo */
      {
	ufdbLogError( "error parsing input line: %s", buf );
	putchar( '\n' ); 		/* OK */
      }
      else
      {
	for (src = Source; ; )
	{
	  int access;

	  strcpy( tmp, squidInfo.src );		/* TODO: find out if strcpy() is necessary... */
	  src = sgFindSource( src, tmp, &squidInfo );
	  acl = sgAclCheckSource( src );
	  if (parseOnly)
	  {
	    putchar( '\n' );		/* OK */
	    break;
	  }
	  redirect = tmp;
	  access = UFDBCheckACLaccess( src, acl, &squidInfo, redirect );
	  if (access == UFDB_ACL_ACCESS_DUNNO  ||  access == UFDB_ACL_ACCESS_ALLOW)
	  {
	    /* src not found */
	    if (src == NULL || src->cont_search == 0) 
	    {
	       if (UFDBglobalLogAllRequests)
	       {
		  char * category;
		  if (squidInfo.aclpass == NULL)
		     category = "unknown";
		  else if (squidInfo.aclpass->name == NULL)
		     category = "unknown";
		  else
		     category = squidInfo.aclpass->name;

		  if (squidInfo.ident[0] == '\0')
		  {
		     squidInfo.ident[0] = '-';
		     squidInfo.ident[1] = '\0';
		  }
		  ufdbLogMessage( "PASS  %-8s %-15s %-10s %-9s %s",
				  squidInfo.ident, squidInfo.src, acl->name, category,
				  squidInfo.url2display );
	       }

	      putchar( '\n' );		/* OK */
	      break;
	    }
	    else
	      if (src->next != NULL) {
		src = src->next;
		continue;
	      }
              else
              {
		  if (UFDBglobalLogAllRequests)
		  {
		     char * category;
		     if (squidInfo.aclpass == NULL)
			category = "unknown";
		     else if (squidInfo.aclpass->name == NULL)
			category = "unknown";
		     else
			category = squidInfo.aclpass->name;

		     if (squidInfo.ident[0] == '\0')
		     {
			squidInfo.ident[0] = '-';
			squidInfo.ident[1] = '\0';
		     }
		    ufdbLogMessage( "PASS  %-8s %-15s %-10s %-9s %s",
				    squidInfo.ident, squidInfo.src, acl->name, category,
				    squidInfo.url2display );
	        }

		putchar( '\n' );  	/* OK */
		break;
	      }
	  }
	  else
	  {
	    /* src found, so block the URL and send the redirect string */
	    if (squidInfo.srcDomain[0] == '\0') {
	      squidInfo.srcDomain[0] = '-';
	      squidInfo.srcDomain[1] = '\0';
	    }

	    if (squidInfo.ident[0] == '\0') {
	      squidInfo.ident[0] = '-';
	      squidInfo.ident[1] = '\0';
	    }

            if (globalDebugRedirect)
	    {
	       ufdbLogMessage( "REDIRECT %s %s/%s %s %s %s\n", redirect, squidInfo.src,
			       squidInfo.srcDomain, squidInfo.ident, "GET", squidInfo.urlgroup );
	    }

	    if (noRedirects)
		putchar( '\n' );  	/* fake OK */
	    else
	    {
	       char urlgrp[64];
	       char * category;

	       if (squidInfo.aclpass == NULL)
		  category = "unknown";
	       else if (squidInfo.aclpass->name == NULL)
		  category = "unknown";
	       else
		  category = squidInfo.aclpass->name;

	       if (squidInfo.urlgroup[0] == '#')
	          urlgrp[0] = '\0';
	       else
	       {
		  urlgrp[0] = ' ';
	          strncpy( &urlgrp[1], squidInfo.urlgroup, 62 );
		  urlgrp[63] = '\0';
	       }
	       printf( "%s %s/%s %s %s%s\n", redirect, squidInfo.src,
			squidInfo.srcDomain, squidInfo.ident, "GET", urlgrp );

	       if (UFDBglobalLogAllRequests)
	       {
		    ufdbLogMessage( "BLOCK %-8s %-15s %-10s %-9s %s",
				    squidInfo.ident, squidInfo.src, acl->name, category,
				    squidInfo.url2display );
	       }
	    }
	    break;
	  }
	} /*for (;;)*/
        UFDBfreeRevURL( admin, squidInfo.revUrl );
      }
      FLUSH( stdout );

      if (sig_hup)
      {
        sig_hup = 0;
        sgReloadConfig();
      }
      if (sig_other)
        break;
    } /* while fgets() */

    FLUSH( stdout );

    if (sig_other)
    {
      ufdbLogError( "signal %d received. exiting...", sig_other );
      exit( 2 );
    }

#if 1
    UFDBtimerStop( &timer );
    ufdbLogError( "processed %ld requests", numreq );
    UFDBtimerPrint( &timer, "time used to handle requests" );
#endif

    if (feof(stdin))
      ufdbLogError( "EOF on input stream" );

    fclose( stdin );
    fclose( stdout );
    ufdbFree( vbuf );
    ufdbFree( admin );
#if defined(UFDB_FREE_MEMORY)
    ufdbFreeAllMemory();
#endif

#if !HAVE_SIGACTION
#if HAVE_SIGNAL
    if (errno != EINTR) 
    {
      gettimeofday( &stop_time, NULL );
      stop_time.tv_sec = stop_time.tv_sec + globalDebugTimeDelta;
      ufdbLogError( "ufdbGuard stopped at %d.%03d",
	            stop_time.tv_sec, stop_time.tv_usec/1000 );
      exit(2);
    }
#endif
#else 
    gettimeofday( &stop_time, NULL );
    stop_time.tv_sec = stop_time.tv_sec + globalDebugTimeDelta;
    ufdbLogError( "ufdbGuard stopped at %d.%03d",
	          stop_time.tv_sec, stop_time.tv_usec/1000 );
    exit(0);
#endif
  } /* while (1) */

  /*NOTREACHED*/
  exit( 0 );
}


void sgReloadConfig( void )
{
  sig_hup = 0;
  ufdbLogError( "got sigHUP reload config" );

  /* closing global logfile */
  fclose( globalErrorLog );
  globalErrorLog = NULL;

  execve( *globalArgv, globalArgv, globalEnvp );
  fprintf( stderr, "error execve: %d\n", errno );
}


