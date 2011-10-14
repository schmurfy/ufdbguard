/*
 * ufdbhttpd.c - URLfilterDB
 *
 * Copyrighted (C) 2007-2010 by URLfilterDB with all rights reserved.
 *
 * Parts of the ufdbGuard daemon are based on squidGuard.
 * This module is NOT based on squidGuard.
 *
 * Extremely simplified http daemon.
 * This HTTP daemon is for the ONLY purpose of serving http requests like
 * http://HOST:port/cgi-bin/URLblocked.cgi
 * 
 */

#include "ufdb.h"
#include "ufdb.h"
#include "ufdblib.h"
#include "httpserver.h"

#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <libgen.h>
#include <stdarg.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>


int    runAsDaemon = 1;

char   interface[256] = "all";
char   imagesDirectory[256] = "";
int    portnumber = 0;

char * globalPidFile = UFDBHTTPD_PID_FILE;
int    UFDBforceLogRotation = 0;
static char   UFDBlogFilename[1024];
static size_t logfilesize = 0;


static void usage( void )
{
   fprintf( stderr, "usage: ufdbhttpd [-d] [-U <user>] -p <port> -I <images-directory> -l <log-directory> [-i <interface>]\n" );
   exit( 1 );
}


/* Rotate log files.
 * There is a race condition when multiple instances try to rotate.
 * Therefore acquire a lock and sleep for 1 second if the lock fails.
 */
static void RotateLogfile( char * filename )
{
   int         i;
   char        oldfile[1024];
   char        newfile[1024];

   /* rotate the log file:
    * file.log.7  ->  file.log.8
    * file.log.6  ->  file.log.7
    * file.log.5  ->  file.log.6
    * file.log.4  ->  file.log.5
    * file.log.3  ->  file.log.4
    * file.log.2  ->  file.log.3
    * file.log.1  ->  file.log.2
    * file.log    ->  file.log.1
    */

   for (i = 8; i > 1; i--)
   {
      sprintf( newfile, "%s.%d", filename, i );
      sprintf( oldfile, "%s.%d", filename, i-1 );
      rename( oldfile, newfile );
   }

   sprintf( newfile, "%s.%d", filename, 1 );
   rename( filename, newfile );
}


void ufdbGlobalSetLogging( int logging )
{
}


void UFDBrotateLogfile( void )
{
   UFDBforceLogRotation = 1;
}


void ufdbSetGlobalErrorLogFile( void )
{
   struct stat s;

   if (globalErrorLog != NULL)
   {
      fclose( globalErrorLog );
      globalErrorLog = NULL;
   }

   if (UFDBglobalLogDir == NULL)
      strcpy( UFDBlogFilename, DEFAULT_LOGDIR );
   else
      strcpy( UFDBlogFilename, UFDBglobalLogDir );
   strcat( UFDBlogFilename, "/" );

   if (progname != NULL  &&  progname[0] != '\0')
   {
      strcat( UFDBlogFilename, progname );
      strcat( UFDBlogFilename, ".log" );
   }
   else
      strcat( UFDBlogFilename, DEFAULT_LOGFILE );

   if (stat(UFDBlogFilename,&s) == 0)
   {
      if (s.st_size > UFDBglobalMaxLogfileSize)
	 RotateLogfile( UFDBlogFilename );
   }

   globalErrorLog = fopen( UFDBlogFilename, "a" );
   if (globalErrorLog == NULL)
   {
      ufdbLogError( "%s: can't write to logfile %s; %s (uid=%d,euid=%d)", 
      	 	    progname, UFDBlogFilename, strerror(errno), getuid(), geteuid() );
      /*
       * We *want* a logfile, try an other directory...
       */
      strcpy( UFDBlogFilename, "/tmp/" );
      strcat( UFDBlogFilename, progname );
      strcat( UFDBlogFilename, ".log" );
      globalErrorLog = fopen( UFDBlogFilename, "a" );
      if (globalErrorLog == NULL)
	 ufdbLogError( "%s: can't write to logfile %s; %s", progname, UFDBlogFilename, strerror(errno) );
   }

   if (globalErrorLog == NULL)
      logfilesize = 0;
   else
      logfilesize = ftell( globalErrorLog );
}


static void niso( 
  time_t t, 
  char * buf )
{
  time_t    tp;
  struct tm lc;

  if (t == 0)
    tp = time(NULL) + globalDebugTimeDelta;
  else
    tp = t;
  localtime_r( &tp, &lc );
  sprintf( buf, "%04d-%02d-%02d %02d:%02d:%02d", 
          lc.tm_year + 1900, lc.tm_mon + 1,
	  lc.tm_mday, lc.tm_hour, lc.tm_min, lc.tm_sec );
}



static void ufdbLog( 
   char *    msg  )
{
   char *    nl;
   int       multiline;
   char      date[22];
   char      logmsg[UFDB_MAX_URL_LENGTH+128];
   int       logmsglen;

   niso( 0, date );

   multiline = 0;
   logmsg[0] = '\0';
   logmsglen = 0;
   while ((nl = strchr( msg, '\n' )) != NULL)
   {
      *nl = '\0';
      logmsglen += sprintf( logmsg+logmsglen, "%s [%d] %s%s\n", date, globalPid, multiline?"   ":"", msg );
      msg = nl + 1;
      multiline = 1;
   }
   logmsglen += sprintf( logmsg+logmsglen, "%s [%d] %s%s\n", date, globalPid, multiline?"   ":"", msg );

   if (UFDBforceLogRotation)
   {
      UFDBforceLogRotation = 0;
      if (globalErrorLog != NULL)  
      {
	 RotateLogfile( UFDBlogFilename );
	 ufdbSetGlobalErrorLogFile();
      }
   }

   if (globalErrorLog == NULL) 
   {
     fputs( logmsg, stderr );
     fflush( stderr );
   }
   else
   {
      if (logfilesize == 0)
	 logfilesize = ftell( globalErrorLog );

      /* write is *much* faster than fputs */
      if (write( fileno(globalErrorLog), logmsg, (size_t) logmsglen ) > 0)
	 logfilesize += logmsglen;

      if (logfilesize > UFDBglobalMaxLogfileSize)
      {
	 RotateLogfile( UFDBlogFilename );
	 ufdbSetGlobalErrorLogFile();
	 logfilesize = 0;
      }
   }
}


void ufdbLogError( char * format, ... )
{
   va_list ap;
   char    msg[UFDB_MAX_URL_LENGTH];

   va_start( ap, format );
   vsnprintf( msg, UFDB_MAX_URL_LENGTH-32, format, ap );
   msg[UFDB_MAX_URL_LENGTH-32] = '\0';
   va_end( ap );

   ufdbLog( msg );
}


void ufdbLogMessage( char * format, ... )
{
   va_list ap;
   char    msg[UFDB_MAX_URL_LENGTH];

   va_start( ap, format );
   vsnprintf( msg, UFDB_MAX_URL_LENGTH-32, format, ap );
   msg[UFDB_MAX_URL_LENGTH-32] = '\0';
   va_end( ap );

   ufdbLog( msg );
}


void ufdbLogFatalError( char * format, ... )
{
   va_list ap;
   char    msg[UFDB_MAX_URL_LENGTH];
   char    logmsg[UFDB_MAX_URL_LENGTH];

   va_start( ap, format );
   vsnprintf( msg, UFDB_MAX_URL_LENGTH-48, format, ap );
   msg[UFDB_MAX_URL_LENGTH-48] = '\0';
   va_end( ap );

   sprintf( logmsg, "*FATAL* %s  *****", msg );
   ufdbLog( logmsg );

   UFDBglobalFatalError = 1;
}


static void writePidFile( void )
{
   FILE * fp;

   (void) unlink( globalPidFile );
   fp = fopen( globalPidFile, "w" );
   if (fp == NULL)
      ufdbLogError( "cannot write to PID file %s", globalPidFile );
   else
   {
      fprintf( fp, "%d\n", globalPid );
      fclose( fp );
      if (UFDBglobalDebugHttpd  || UFDBglobalDebug)
         ufdbLogMessage( "wrote my pid (%d) to %s", globalPid, globalPidFile );
   }
}


void removePidFile( void )
{
   (void) unlink( globalPidFile );
}


static void TermHandler( int sig )
{
   ufdbLogMessage( "signal TERM received. " );
   removePidFile();
   exit( 0 );
}


static void mySignalHandler( int sig )
{
   char  sigDesc[16];

   switch (sig)
   {
      case SIGHUP:   strcpy( sigDesc, "HUP" );   break;
      case SIGINT:   strcpy( sigDesc, "INT" );   break;
      case SIGTERM:  strcpy( sigDesc, "TERM" );  break;
      case SIGILL:   strcpy( sigDesc, "ILL" );   break;
      case SIGSEGV:  strcpy( sigDesc, "SEGV" );  break;
      default:       sprintf( sigDesc, "%d", sig );  break;
   }
   ufdbLogMessage( "signal %s received. exiting...", sigDesc );

   removePidFile();
   exit( 0 );
}


static void USR1signalReceived( int sig )
{
   ufdbLogMessage( "USR1 signal received for logfile rotation" );
   UFDBrotateLogfile();
   ufdbLogMessage( "USR1 signal received for logfile rotation" );
}


static void closeAllFiles( void )
{
   int fd;

   for (fd = 3; fd < 150; fd++)
   {
      close( fd );
   }
}


int main( int argc, char ** argv )
{
   int    ch;

   strcpy( progname, "ufdbhttpd" );
   globalPid = getpid();
   globalErrorLog = NULL;

   closeAllFiles();
   ufdbSetGlobalErrorLogFile();

   while ((ch = getopt(argc, argv, "Ddhi:p:I:l:L:U:V")) > 0)
   {
      switch (ch) {
      case 'D':				/* undocumented -D option: do not daemonize */
      	 runAsDaemon = 0;
	 break;
      case 'd':
	 UFDBglobalDebug = 1;
      	 runAsDaemon = 0;
	 break;
      case 'i':
	 strcpy( interface, optarg );
	 break;
      case 'p':
	 portnumber = atoi( optarg );
	 break;
      case 'I':
	 strcpy( imagesDirectory, optarg );
	 break;
      case 'l':
	 UFDBglobalLogDir = optarg;
	 break;
      case 'L':				/* undocumented -L option for alternate PID file */
	 globalPidFile = optarg;
         break;
      case 'U':
	 if (strlen(optarg) <= 31)
	    strcpy( UFDBglobalUserName, optarg );
	 else
	    ufdbLogFatalError( "username supplied with -U option is too long" );
         break;
      case 'V':
         UFDBglobalDebugHttpd = 1;
	 break;
      case '?':
      case 'h':
      default:
	 usage();
      }
   }

   if (UFDBglobalLogDir == NULL ||
       imagesDirectory[0] == '\0'  ||
       portnumber <= 0)
   {
      ufdbLogFatalError( "%s started with incorrect parameters. aborting...", progname );
      usage();
      exit( 1 );
   }

   if (runAsDaemon)
   {
      pid_t pid;
      if ((pid = fork()) != 0)		/* the parent exits */
      {
	 exit( 0 );        
      }
      if (setsid() < 0)
         ufdbLogFatalError( "cannot create daemon session: %s", strerror(errno) );
   }
   globalPid = getpid();

   ufdbSetSignalHandler( SIGPIPE, SIG_IGN );
   ufdbSetSignalHandler( SIGUSR1, USR1signalReceived );
   ufdbSetSignalHandler( SIGHUP,  mySignalHandler );
   ufdbSetSignalHandler( SIGINT,  mySignalHandler );
   ufdbSetSignalHandler( SIGTERM, TermHandler );
   
   writePidFile();
   atexit( removePidFile );
   ufdbLogMessage( "%s " VERSION " started\ninterface=%s  port=%d  images=%s",
                   progname, interface, portnumber, imagesDirectory );
   if (UFDBglobalDebug)
      ufdbLogMessage( "debug option is ON" );
   if (UFDBglobalDebugHttpd)
      ufdbLogMessage( "HTTP debug option is ON" );
   if (UFDBglobalUserName[0] != '\0')
      ufdbLogMessage( "dropping privileges to user %s", UFDBglobalUserName );

   ufdbSimulateHttpServer( interface, portnumber, UFDBglobalUserName, imagesDirectory, 0 );

   exit( 0 );
   return 0;  /* make compiler happy */
}

