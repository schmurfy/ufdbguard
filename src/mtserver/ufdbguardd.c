/* 
 * ufdbguardd.c - URLfilterDB
 *
 * ufdbGuard is copyrighted (C) 2005-2011 by URLfilterDB with all rights reserved.
 * 
 * Parts of the ufdbGuard daemon are based on squidGuard.
 *
 * Multithreaded ufdbGuard daemon
 *
 * $Id: ufdbguardd.c,v 1.90 2011/06/22 18:00:58 root Exp root $
 */

#include "ufdb.h"
#include "sg.h"
#include "ufdbchkport.h"
#include "httpsQueue.h"
#include "httpserver.h"

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <ctype.h>
#include <signal.h>
#include <libgen.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/tcp.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <sys/utsname.h>
#ifdef HAVE_SYS_SYSCALL_H
#include <sys/syscall.h>
#endif
#ifdef __linux__
#include <linux/unistd.h>
#endif
#include <sys/socket.h>
#if HAVE_UNIX_SOCKETS
#include <sys/un.h>
#endif
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>

#ifdef __linux__
#include <execinfo.h>
#endif

#ifdef _POSIX_PRIORITY_SCHEDULING
#include <sched.h>
#endif

#define FULL 1
#define EMPTY 0

pthread_mutex_t sighup_mutex = UFDB_STATIC_MUTEX_INIT;
#ifdef UFDB_HAVE_NATIVE_RWLOCK_MUTEX
pthread_rwlock_t TheDatabaseLock;
#endif

#define UFDB_MAX_WORKERS	UFDB_MAX_THREADS
static int     n_workers = 65;

static int     my_argc;
static char ** my_argv;
static char ** my_envp;
static char *  configFile = NULL;
static struct timeval  start_time;

#define UFDBGUARDD_PID_FILE	"/var/tmp/ufdbguardd.pid"
char * globalPidFile = UFDBGUARDD_PID_FILE;

extern int globalPid;

extern int globalDebugRedirect;
extern int globalDebugTimeDelta;
extern int sig_other;
extern char ** globalArgv;
extern char ** globalEnvp;

extern int httpsConnectionCacheSize;

static int        portNumCmdLine = -1;

static int        runAsDaemon = 1;
static int        testMode = 0;
static int        parseOnly = 0;
static pthread_t  threadedSignalHandler;
static pthread_t  sockethandler;
static pthread_t  workers[UFDB_MAX_WORKERS];
static pthread_t  httpsthr[UFDB_NUM_HTTPS_VERIFIERS];
static pid_t      httpDaemonPid = 0;
static int        allowedToRemovePidFile = 0;

volatile unsigned long UFDBlookupCounter = 0;
volatile unsigned long UFDBlookupHttpsCounter = 0;
volatile unsigned long UFDBblockCounter = 0;
volatile unsigned long UFDBtestBlockCounter = 0;
volatile unsigned long UFDBuncategorisedCounter = 0;
volatile unsigned long UFDBsafesearchCounter = 0;
volatile unsigned long UFDBuploadSeqNo = 0;

static void uploadStatistics( char * reason );
static void * worker_main( void * ptr );

#ifdef UFDB_DEBUG
extern int yydebug;
#endif


static void usage( char option )
{
   if (option != '\0')
      fprintf( stderr, "unknown option '-%c'\n", option );

   fprintf( stderr, "usage: ufdbguardd [-A] [-d] [-v] [-r] [-R] [-T] [-h] [-w 32-129] [-p port] [-c <configfile>]\n" );
   fprintf( stderr, "-v      print version\n" );
   fprintf( stderr, "-c FN   use configuration file FN\n" );
   fprintf( stderr, "-T      test mode; do not block, only log which URLs might be blocked\n" );
   fprintf( stderr, "-w N    use N worker threads (default: %d)\n", n_workers );
   fprintf( stderr, "-p N    use TCP port N to listen on\n" );
   fprintf( stderr, "-U user run with privileges of \"user\"\n" );
   fprintf( stderr, "-d      extra debug information in log file\n" );
   fprintf( stderr, "-R      debug RE matching\n" );
   fprintf( stderr, "-r      log all redirections\n" );
   fprintf( stderr, "-N      do not analyse uncategorised URLs (not recommended)\n" );
   fprintf( stderr, "-h      print this help text\n" );

   if (option != '\0')
      exit( 1 );
   else
      exit( 0 );
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
      allowedToRemovePidFile = 1;
   }
}


static void removePidFile( void )
{
   FILE * fp;
#if HAVE_UNIX_SOCKETS
   char   unixsocket[64];
#endif

   /* When the initialisation of the socket failed there is most likely
    * already an ufdbguardd daemon running and we may not remove
    * the pid file nor kill any ufdbhttpd daemon.
    */
   if (!allowedToRemovePidFile)
      return;

   allowedToRemovePidFile = 0;

#if HAVE_UNIX_SOCKETS
   sprintf( unixsocket, "/tmp/ufdbguardd-%05d", UFDBglobalPortNum );
   if (unlink( unixsocket ) < 0  &&  errno != ENOENT)
      ufdbLogError( "cannot remove socket %s: %s", unixsocket, strerror(errno) );
#endif

   (void) unlink( globalPidFile );

   if (UFDBglobalHttpdPort > 0)
   {
      fp = fopen( UFDBHTTPD_PID_FILE, "r" );
      if (fp != NULL)
      {
         if (fscanf( fp, "%d", &httpDaemonPid ) == 1)
	 {
            (void) unlink( UFDBHTTPD_PID_FILE );
	    ufdbLogMessage( "sending TERM signal to the HTTP daemon (pid=%d)", httpDaemonPid );
	    if (kill( httpDaemonPid, SIGTERM ) < 0)
	       ufdbLogError( "cannot kill pid %d: %s", httpDaemonPid, strerror(errno) );
	    /* TODO: find out why ufdbhttpd does not act on the TERM signal... */
	    usleep( 100000 );
	    (void) kill( httpDaemonPid, SIGKILL );
	 }
         fclose( fp );
      }
   }
}


static void startHttpDaemon( void )
{
   int    i;
   char * argv[24];
   struct stat statbuf;
   char   portStr[16];
   char   httpdbin[1024];

   sprintf( httpdbin, "%s/%s", DEFAULT_BINDIR, "ufdbhttpd" );
   if (stat( httpdbin, &statbuf ) < 0)
   {
      ufdbLogError( "cannot find the HTTP daemon executable (%s): %s  *****", httpdbin, strerror(errno) );
      return;
   }

   if (UFDBglobalUserName[0] != '\0')
   {
      /* go back to user root to allow ufdbhttpd to use privileged ports */
      if (seteuid( 0 ) != 0)	  
         ufdbLogError( "pre-fork seteuid(0) failed: %s", strerror(errno) );
      else if (UFDBglobalDebug)
         ufdbLogMessage( "pre-fork: seteuid(0) was OK" );
   }

   httpDaemonPid = fork();
   if (httpDaemonPid == 0)	/* child */
   {
      UFDBcloseFilesNoLog();
      i = 0;
      argv[i++] = httpdbin;
      if (UFDBglobalDebug || UFDBglobalDebugHttpd)
         argv[i++] = "-d";
      argv[i++] = "-p";
      sprintf( portStr, "%d", UFDBglobalHttpdPort );
      argv[i++] = portStr;
      argv[i++] = "-l";
      if (UFDBglobalLogDir == NULL)
	 argv[i++] = DEFAULT_LOGDIR;
      else
	 argv[i++] = UFDBglobalLogDir;
      argv[i++] = "-I";
      if (UFDBglobalHttpdImagesDirectory[0] == '\0')
	 strcpy( UFDBglobalHttpdImagesDirectory, DEFAULT_IMAGESDIR );
      argv[i++] = UFDBglobalHttpdImagesDirectory;
      argv[i++] = "-i";
      argv[i++] = UFDBglobalHttpdInterface;
      if (UFDBglobalUserName[0] != '\0')
      {
	 argv[i++] = "-U";
	 argv[i++] = UFDBglobalUserName;
	 if (UFDBglobalHttpdPort < 1024)
	 {
	    /* go back to user root to allow ufdbhttpd to use privileged ports */
	    if (setreuid( 0, 0 ) != 0)	  
	    {
	       ufdbLogError( "Cannot change userid to root to start ufdbhttpd.  "
			     "Privileged ports (< 1024) cannot be used. *****" );
	    }
	 }
      }
      argv[i] = NULL;
      ufdbLogMessage( "starting HTTP daemon ..." );
      execv( httpdbin, argv );
      ufdbLogError( "failed starting http daemon: execv failed: %s", strerror(errno) );
      _exit( 2 );
   }

   if (UFDBglobalUserName[0] != '\0')
      UFDBdropPrivileges( UFDBglobalUserName );

   if (httpDaemonPid < 0)
   {
      ufdbLogError( "cannot start HTTP daemon: fork failed: %s", strerror(errno) );
      httpDaemonPid = 0;
   }
   else
   {
      int        status;
      pid_t      pid;

      ufdbLogMessage( "ufdbhttpd started.  port=%d interface=%s images=%s",
                      UFDBglobalHttpdPort, UFDBglobalHttpdInterface, UFDBglobalHttpdImagesDirectory );
      usleep( 300000 );
      while ((pid = waitpid( -1, &status, WNOHANG )) > 0)
	 ;
   }
}


static void adjustNumberOfThreads( void )
{
   int              i;
   pthread_attr_t   attr;

   /* IF the check_https_tunnel_option is aggressive we need more worker threads */
   /* Note that UFDB_MAX_THREADS is 129 so with the -w option we can have 129 worker threads */
   if (UFDBgetTunnelCheckMethod() == UFDB_API_HTTPS_CHECK_AGGRESSIVE  &&  n_workers < 129)
   {
      /* create more worker threads */
      i = n_workers;
      n_workers = 129;
      pthread_attr_init( &attr );
      pthread_attr_setscope( &attr, PTHREAD_SCOPE_SYSTEM );
      pthread_attr_setstacksize( &attr, 248 * 1024 );
#if HAVE_PTHREAD_ATTR_SETGUARDSIZE
      pthread_attr_setguardsize( &attr, 8 * 1024 );
#endif
      for (; i < n_workers; i++)
      {
	 pthread_create( &workers[i], &attr, worker_main, (void *) ((long) i) );
#ifdef _POSIX_PRIORITY_SCHEDULING
	 sched_yield();
#else
	 usleep( 10000 );
#endif
      }
      ufdbLogMessage( "#threads is increased to %d because check-proxy-tunnels is set to \"aggressive\"", n_workers );
   }
}


static void Usr1SignalCaught( int sig )
{
   FILE * fp;
   int    oldStatus;

   ufdbLogMessage( "USR1 signal received for logfile rotation" );
   oldStatus = UFDBchangeStatus( UFDB_API_STATUS_ROLLING_LOGFILE );
   UFDBrotateLogfile();
   if (UFDBglobalHttpdPort > 0)
   {
      fp = fopen( UFDBHTTPD_PID_FILE, "r" );
      if (fp != NULL)
      {
	 if (1 != fscanf( fp, "%d", &httpDaemonPid ))
	    httpDaemonPid = -1;
	 fclose( fp );
      }
      else
	 httpDaemonPid = -1;

      if (httpDaemonPid > 0)
      {
	 int retval;

	 ufdbLogMessage( "sending USR1 signal to ufdbhttpd (pid=%d)", httpDaemonPid );
	 retval = kill( httpDaemonPid, SIGUSR1 );
	 if (retval != 0)
	    ufdbLogError( "cannot send USR1 signal to httpd daemon: %s", strerror( errno ) );
      }
   }
   ufdbLogMessage( "USR1 signal received for logfile rotation" );
   UFDBchangeStatus( oldStatus );
}


static void Usr2SignalCaught( int sig )
{
   int    oldStatus;

   ufdbLogMessage( "USR2 signal received to trigger monitoring commands" );
   oldStatus = UFDBchangeStatus( UFDB_API_STATUS_UPDATE );
   UFDBchangeStatus( oldStatus );
   ufdbLogMessage( "status update done" );
}


/*
 * OpenBSD is very picky with its implementation of signals and
 * does not allow violation of the POSIX standard that says that
 * a signal handler may NOT raise a signal, so calling pthread_kill
 * is not allowed.  
 * The signal handlers below do nothing.  They are installed to
 * tell the OS to send the signals to the process and the special
 * signal interception threads will handle the signals asynchroneously.
 *
 * These signal handlers can do nothing, not even call 
 * ufdbLogMessage because it uses a mutex.
 */
static void catchHUPSignal( int signal )
{
}


static void catchUSR1Signal( int signal )
{
}


static void catchUSR2Signal( int signal )
{
}


static void catchChildSignal( int signal )
{
   /* For some reason, on OpenBSD the SIGCHLD does not get through
    * to the signal_handler_thread() so we must do a waitpid() here.
    */
#if defined(__OpenBSD__) || defined(__NetBSD__)  || defined(__FreeBSD__)
   int        status;
   pid_t      pid;

   while ((pid = waitpid( -1, &status, WNOHANG )) > 0)
      ;
#endif
}


void catchAbortSignal( int signum )
{
   UFDBglobalReconfig = 1;
   (void) alarm( 0 );
   ufdbLogMessage( "ABORT signal was sent. exiting..." );

   removePidFile();
   UFDBchangeStatus( UFDB_API_STATUS_TERMINATED );
   exit( 3 );
}


void catchTermSignal( int signum )
{
   /* do nothing. sigwait() in a thread will deal with the signal */
}


static void TermSignalCaught( int signum )
{
   UFDBglobalReconfig = 1;
   (void) alarm( 0 );
   ufdbLogMessage( "received %s signal", signum == SIGINT ? "INT" : "TERM" );
   uploadStatistics( signum == SIGINT ? "INT" : "TERM" );

   /* close all connections with clients */
   UFDBcloseFilesNoLog();

   UFDBchangeStatus( UFDB_API_STATUS_TERMINATED );

#if defined(UFDB_FREE_MEMORY)
   sleep( 3 );
   ufdbLogMessage( "freeing all memory ..." );
   ufdbFreeAllMemory();
   ufdbLogMessage( "done freeing memory." );
#endif

   usleep( 100000 );
   pthread_cancel( sockethandler );

   sleep( 15 );		/* we are the signal_handler_thread and prefer that the main thread does all the cleanup */
   removePidFile();
   ufdbLogMessage( "signal_handler_thread: bye bye" );
   exit( 0 );
}


static void HupSignalCaught( int sig )
{
   unsigned int sleep_period;
   time_t       t;
   struct tm    thetime;
   int          status;
   pid_t        pid;
   FILE *       fp;
#ifdef UFDB_HAVE_NATIVE_RWLOCK_MUTEX
   int             ret;
   struct timespec DatabaseLockTimeout;
#endif

   if (pthread_mutex_trylock( &sighup_mutex ) != 0)
   {
      ufdbLogMessage( "HUP signal is already being processed" );
      return;
   }
   ufdbLogMessage( "HUP signal received to reload the configuration and database" );

   UFDBglobalReconfig = 1;
   alarm( 0 );

   UFDBchangeStatus( UFDB_API_STATUS_RELOADING );

   uploadStatistics( "HUP" );

   ufdbLogMessage( "statistics: %lu URL lookups (%lu https). %lu URLs blocked. "
		   "%lu tunnels detected. %lu safe searches. %lu clients.", 
		   UFDBlookupCounter, 
		   UFDBlookupHttpsCounter,
		   UFDBblockCounter + UFDBtestBlockCounter,
		   UFDBglobalTunnelCounter,
		   UFDBsafesearchCounter,
		   UFDBgetNumberOfRegisteredIPs() );

   /* To prevent counting 'old' clients: 
    * if today is Wednesday, we reset the IP counters.
    */
   t = time( NULL );
   localtime_r( &t, &thetime );
   if (thetime.tm_wday == 3)
   {
      UFDBinitializeIPcounters();
   }

   sleep( 3 );
   /* after this short sleep, it is very unlikely that ufdbguardd still
    * sends redirects, so it effectively means that ufdbhttpd can be restarted safely.
    */
   httpDaemonPid = -1;
   if (UFDBglobalHttpdPort > 0)
   {
      fp = fopen( UFDBHTTPD_PID_FILE, "r" );
      if (fp != NULL)
      {
	 httpDaemonPid = -1;
	 if (1 != fscanf( fp, "%d", &httpDaemonPid ))
	    ufdbLogError( "file %s does not have a process id", UFDBHTTPD_PID_FILE );
	 fclose( fp );
      }
      else
      {
	 ufdbLogError( "cannot open file %s: %s", UFDBHTTPD_PID_FILE, strerror(errno) );
	 httpDaemonPid = -1;
      }

      if (httpDaemonPid > 0)
      {
	 int retval;

	 ufdbLogMessage( "killing ufdbhttpd (pid=%d)", httpDaemonPid );
	 retval = kill( httpDaemonPid, SIGTERM );
	 if (retval != 0)
	    ufdbLogError( "cannot send TERM signal to the HTTP daemon (pid=%d): %s", httpDaemonPid, strerror( errno ) );
      }
   }

   sleep( 1 );
   /* be sure that the daemon is killed. */
   if (UFDBglobalHttpdPort > 0   &&  httpDaemonPid > 0)
      (void) kill( httpDaemonPid, SIGKILL );

   usleep( 500000 );
   while ((pid = waitpid( -1, &status, WNOHANG )) > 0)
      ;

   if (UFDBglobalHttpdPort > 0  &&  httpDaemonPid > 0)
   {
      if (kill( httpDaemonPid, 0 ) < 0)
	 ufdbLogMessage( "HTTP daemon (pid=%d) terminated to be restarted (OK)", httpDaemonPid );
      else
	 ufdbLogError( "HTTP daemon did not terminate: %s", strerror(errno) );
   }

#ifdef UFDB_HAVE_NATIVE_RWLOCK_MUTEX
   /* 
    * Synchronise with all worker threads.
    * UFDBglobalReconfig is set for the last 3+1+1 seconds so the worker threads have 
    * most likely no read-locks any more.  Try to get a write lock within 0.1 second
    * and if this fails, we will sleep 20 seconds more and acquire the rwlock with
    * pthread_rwlock_wrlock().
    *
    * We use a 0.1-second timeout since we do not want to block readers (workers)
    * too long because the flow of messages back to Squid may not be interrupted too long.
    * Note: 1,000,000,000 nanoseconds make 1 second.
    */
   DatabaseLockTimeout.tv_sec = 0;
   DatabaseLockTimeout.tv_nsec = 100000000;
   ret = pthread_rwlock_timedwrlock( &TheDatabaseLock, &DatabaseLockTimeout );
   if (ret != 0)
   {
      if (ret == ETIMEDOUT)
      {
	 ufdbLogMessage( "sleeping 20 seconds before acquiring the write lock" );
	 sleep( 20 );
      }
      else 
	 ufdbLogError( "pthread_rwlock_timedwrlock failed with code %d", ret );
      /* 
       * Wait indefinitely for a write lock ...
       * THIS *MAY* DISTURB USERS since Squid is not getting answers back from some workers.
       */
      ret = pthread_rwlock_wrlock( &TheDatabaseLock );
#ifdef UFDB_DEBUG
      if (ret != 0)
	 ufdbLogError( "pthread_rwlock_wrlock failed with code %d", ret );
#endif
   }
   (void) pthread_rwlock_unlock( &TheDatabaseLock );

#else
   /* sleep() is a very crude way to allow worker threads to finish what
    * they were doing, before we remove the database from memory,
    * but it allows us not to use mutexes for a many-reader-one-writer problem.
    */
   sleep_period = n_workers / 2;
   if (sleep_period < 12)
      sleep_period = 12;
   if (sleep_period > 20)
      sleep_period = 20;
   sleep_period -= 3+1+1;		/* we already slept a few seconds */
   ufdbLogMessage( "sleeping %d seconds to allow worker threads to finish current work", 
		   sleep_period );
   sleep( sleep_period );		
#endif

   /* TODO: reload CA certificates */
   /* TODO: clear security cache */

   ufdbLogMessage( "releasing database from memory ..." );
   ufdbFreeAllMemory();
   ufdbLogMessage( "reading configuration file and database ..." );
   UFDBglobalFatalError = 0;
   UFDBglobalHttpdPort = 0;
   httpDaemonPid = 0;
   sgReadConfig( configFile );
   if (UFDBglobalFatalError)
   {
      ufdbLogMessage( "A FATAL ERROR OCCURRED: ALL REQUESTS ARE ANSWERED WITH \"OK\" (see previous lines)  *****" );
   }

   UFDBlogConfig();

   if (UFDBglobalHttpdPort > 0)
      startHttpDaemon();

   adjustNumberOfThreads();

   UFDBlookupCounter = 0;
   UFDBlookupHttpsCounter = 0;
   UFDBblockCounter = 0;
   UFDBtestBlockCounter = 0;
   UFDBuncategorisedCounter = 0;
   UFDBglobalTunnelCounter = 0;
   UFDBsafesearchCounter = 0;
   UFDBuploadSeqNo++;

   ufdbLogMessage( "the new configuration and database are loaded for ufdbguardd " VERSION );
   UFDBglobalReconfig = 0;
   if (UFDBglobalFatalError)
   {
      UFDBchangeStatus( UFDB_API_STATUS_FATAL_ERROR );
   }
   else
   {
      UFDBchangeStatus( UFDB_API_STATUS_RELOAD_OK );
      ufdbHandleAlarmForTimeEvents( UFDB_PARAM_INIT );
   }

   pthread_mutex_unlock( &sighup_mutex );
}


/* 
 * Communication between the worker and other threads is done via
 * RW-locks and data is passed via a queue.
 *
 * Since this queue is a FIFO we could use a tail queue. But we don't
 * like the malloc() overhead, so therefore we use a static array.
 */

#define UFDB_MAX_FD_QUEUE   UFDB_MAX_THREADS


static struct {
   int    fd;
} q[UFDB_MAX_FD_QUEUE];

volatile static int    ihead = 0;                    /* array index for the head */
volatile static int    itail = 0;                    /* array index for the tail */
volatile static int    n_queued = 0;
static pthread_mutex_t fdq_lock = UFDB_STATIC_MUTEX_INIT;
static pthread_cond_t  empty = PTHREAD_COND_INITIALIZER;


/*
 * enqueue a new fd.
 */
static void putFdQueue( int fd )
{
   int ret;

   ret = pthread_mutex_lock( &fdq_lock );
#ifdef UFDB_DEBUG
   if (ret != 0)
      ufdbLogError( "putFdQueue: mutex_lock failed with code %d *****", ret );
#endif

   if (n_queued < UFDB_MAX_FD_QUEUE)
   {
      /*
       * insert it at the tail
       */
      q[itail].fd = fd;

      itail = (itail + 1) % UFDB_MAX_FD_QUEUE;
      n_queued++;

      /*
       * leave the critical section
       */
      ret = pthread_mutex_unlock( &fdq_lock );
#ifdef UFDB_DEBUG
      if (ret != 0)
	 ufdbLogError( "putFdQueue: mutex_unlock failed with code %d *****", ret );
#endif

      /*
       * signal anyone who is waiting
       */
      pthread_cond_broadcast( &empty );
   } 
   else    
   {
      ufdbLogFatalError( "FATAL ERROR: connection queue is full.  *****\n"
                         "There are too many ufdbgclient processes !!\n"
                         "the maximum is %d when the -w option is used\n"
      			 "closing fd %d", 
			 UFDB_MAX_FD_QUEUE, fd );
      /* there is no other option than to close the connection */
      /* TO-DO: maybe: close all queued fd's ? */
      close( fd );
   }
}


/*
 * get a fd where there is new content.
 */
static void getFdQueue( int * fd )
{
   int ret;

allover:
   ret = pthread_mutex_lock( &fdq_lock );
#ifdef UFDB_DEBUG
   if (ret != 0)
      ufdbLogError( "getFdQueue: mutex_lock failed with code %d *****", ret );
#endif

   while (1)
   {
      /*
       * if there are jobs in the queue
       */
      if (n_queued > 0)
      {
         n_queued--;
         /*
          * get the one at the head
          */
         *fd = q[ihead].fd;
         ihead = (ihead + 1) % UFDB_MAX_FD_QUEUE;

         ret = pthread_mutex_unlock( &fdq_lock );
#ifdef UFDB_DEBUG
	 if (ret != 0)
	    ufdbLogError( "getFdQueue: mutex_unlock failed with code %d *****", ret );
#endif

	 return;
      } 
      else   /* otherwise wait until there are fds available */ 
      {
         pthread_cond_wait( &empty, &fdq_lock );
         /*
          * when I'm here, I've been signaled because there
          * are jobs in the queue.  Go try and get one.
          */
	 ret = pthread_mutex_unlock( &fdq_lock );
#ifdef UFDB_DEBUG
	 if (ret != 0)
	    ufdbLogError( "getFdQueue: mutex_unlock failed with code %d *****", ret );
#endif
	 usleep( ((unsigned long) pthread_self()) % 821 );
	 goto allover;
      }
   }
}


static void daemon_accept_connections( 
   int            s, 
   int            protocol )
{
   int            n;
   int            newfd;
   fd_set         fds;
   struct timeval tv;

   /*
    * Allow that this thread can be cancelled without delays at any time.
    */
   pthread_setcancelstate( PTHREAD_CANCEL_ENABLE, NULL );
   pthread_setcanceltype( PTHREAD_CANCEL_ASYNCHRONOUS, NULL );

   while (!sig_other)
   {
      FD_ZERO( &fds );
      FD_SET( s, &fds );
      errno = 0;
      n = select( s+1, &fds, (fd_set *) NULL, (fd_set *) NULL, (struct timeval *) NULL );
      if (n > 0) 
      {
	 newfd = accept( s, NULL, NULL );
	 if (newfd < 0)
	 {
	    if (errno == EINTR)
	    {
	       continue;
	    }
	    ufdbLogError( "SRV: accept on socket failed: %s", strerror(errno) );
	    return;
	 }

	 if (protocol == AF_INET)
	 {
	    int sock_parm;

	    sock_parm = 1;
	    setsockopt( newfd, IPPROTO_TCP, TCP_NODELAY, (void *) &sock_parm, sizeof(sock_parm) );
	    sock_parm = 1;
	    setsockopt( newfd, SOL_SOCKET, SO_KEEPALIVE, (void *) &sock_parm, sizeof(sock_parm) );
	    sock_parm = 16384;
	    setsockopt( newfd, SOL_SOCKET, SO_SNDBUF, (void *) &sock_parm, sizeof(sock_parm) );
	    sock_parm = 16384;
	    setsockopt( newfd, SOL_SOCKET, SO_RCVBUF, (void *) &sock_parm, sizeof(sock_parm) );
	 }

	 /*
	  *  In case of troubles, the timeout prevents that a thread will block for ever.
	  */
#if 0
	 tv.tv_sec = 16 * 60;
	 tv.tv_usec = 0;
	 setsockopt( newfd, SOL_SOCKET, SO_RCVTIMEO, (void *) &tv, sizeof(tv) );
#endif
	 tv.tv_sec = 5;			/* 5 second send timeout */
	 tv.tv_usec = 0;
	 setsockopt( newfd, SOL_SOCKET, SO_SNDTIMEO, (void *) &tv, sizeof(tv) );

	 if (UFDBglobalDebug)
	    ufdbLogMessage( "SRV: accept new fd %2d", newfd );

	 putFdQueue( newfd );
      }
   }
}


void * socket_handler_thread( void * ptr )
{
   int                 protocol;
   sigset_t            sigset;
   int                 i;
   int                 s;
   int                 sock_parm;
   int                 retval;
   long                num_cpus;
   struct utsname      sysinfo;
   pthread_attr_t      attr;
   struct sockaddr_in  addr_in;
#if HAVE_UNIX_SOCKETS
   struct sockaddr_un  addr_un;
#endif

   UFDBglobalReconfig = 1;

   /* Most signals must be blocked.
    * This is a requirement to use sigwait() in a thread.
    */
   sigemptyset( &sigset );
   sigaddset( &sigset, SIGHUP );
   sigaddset( &sigset, SIGUSR1 );
   sigaddset( &sigset, SIGUSR2 );
   sigaddset( &sigset, SIGCHLD );
   sigaddset( &sigset, SIGTERM );
   sigaddset( &sigset, SIGINT );
   sigaddset( &sigset, SIGALRM );
   pthread_sigmask( SIG_BLOCK, &sigset, NULL );

   gettimeofday( &start_time, NULL );

   globalArgv = my_argv;
   globalEnvp = my_envp;
   ufdbSetGlobalErrorLogFile();
   ufdbResetUnknownURLs();
   UFDBglobalFatalError = 0;
   sgReadConfig( configFile );
   /* ufdbSetGlobalErrorLogFile(); */
   UFDBlogConfig();

   adjustNumberOfThreads();

   UFDBinitHTTPSchecker();
#ifdef _POSIX_PRIORITY_SCHEDULING
   sched_yield();
#else
   usleep( 100000 );
#endif
   pthread_attr_init( &attr );
   pthread_attr_setscope( &attr, PTHREAD_SCOPE_SYSTEM );
   pthread_attr_setstacksize( &attr, 248 * 1024 );
#if HAVE_PTHREAD_ATTR_SETGUARDSIZE
   pthread_attr_setguardsize( &attr, 8 * 1024 );
#endif
   for (i = 0; i < UFDB_NUM_HTTPS_VERIFIERS; i++)
   {
      pthread_create( &httpsthr[i], &attr, UFDBhttpsTunnelVerifier, (void *) ((long) i) );
#ifdef _POSIX_PRIORITY_SCHEDULING
      sched_yield();
#else
      usleep( 10000 );
#endif
   }
   usleep( 30000 );

   if (testMode)
      ufdbLogMessage( "-T option is used.  In test mode no URLs are ever blocked." );
   if (UFDBglobalFatalError)
   {
      ufdbLogError( "A FATAL ERROR OCCURRED: ALL REQUESTS ARE ANSWERED WITH \"OK\" (see previous lines for more information)  *****" );
      UFDBchangeStatus( UFDB_API_STATUS_FATAL_ERROR );
   }
   else
   {
      UFDBchangeStatus( UFDB_API_STATUS_STARTED_OK );
      ufdbHandleAlarmForTimeEvents( UFDB_PARAM_INIT );
   }

   /*
    * Create the daemon socket that the it accepts connections on.
    * if available, first try a UNIX socket and then an IP socket.
    */
#if HAVE_UNIX_SOCKETS
   protocol = AF_UNIX;
   s = socket( protocol, SOCK_STREAM, 0 );
   if (s < 0)
   {
      protocol = AF_INET;
      s = socket( protocol, SOCK_STREAM, 0 );
   }
#else
   protocol = AF_INET;
   s = socket( protocol, SOCK_STREAM, 0 );
#endif

   if (s < 0)
   {
      ufdbLogError( "cannot create daemon socket: %s", strerror(errno) );
      pthread_exit( (void *) 0 ); 
   }

   /* 
    * The port number can be specified in the config file (UFDBglobalPortNum is set by the parser)
    * or a command line parameter (portNumCmdLine is set in main).
    * The command line setting has preference.
    */
   if (portNumCmdLine >= 0)
      UFDBglobalPortNum = portNumCmdLine;

#if HAVE_UNIX_SOCKETS
   if (protocol == AF_UNIX)
   {
      sprintf( addr_un.sun_path, "/tmp/ufdbguardd-%05d", UFDBglobalPortNum );
#if 0
      if (unlink( addr_un.sun_path ) < 0  &&  errno != ENOENT)
         ufdbLogError( "cannot remove socket %s: %s", addr_un.sun_path, strerror(errno) );
#endif
      addr_un.sun_family = AF_UNIX;
      retval = bind( s, (struct sockaddr *) &addr_un, sizeof(addr_un) );
      /* allow anybody to connect to the socket */
      if (retval >= 0)
         chmod( addr_un.sun_path, S_IRUSR|S_IWUSR| S_IRGRP|S_IWGRP| S_IROTH|S_IWOTH );
   }
   else
#endif
   {
      /*
       * Allow server-side addresses to be reused (don't have to wait for timeout).
       */
      sock_parm = 1;
      setsockopt( s, SOL_SOCKET, SO_REUSEADDR, (void *) &sock_parm, sizeof(sock_parm) );

      addr_in.sin_family = AF_INET;
      addr_in.sin_port = htons( UFDBglobalPortNum );
      if (strcmp( UFDBglobalInterface, "all" ) == 0)
	 addr_in.sin_addr.s_addr = htonl( INADDR_ANY );
      else
      {
	 struct in_addr iaddr;
	 if (inet_aton( UFDBglobalInterface, &iaddr ) == 0)
	 {
	    addr_in.sin_addr.s_addr = htonl( INADDR_ANY );
	    ufdbLogError( "interface parameter '%s' is invalid. I will listen on port %d on ALL interfaces.",
			  UFDBglobalInterface, UFDBglobalPortNum );
	 }
	 else
	    addr_in.sin_addr.s_addr = iaddr.s_addr;
      }
      retval = bind( s, (struct sockaddr *) &addr_in, sizeof(addr_in) );
   }

   if (retval < 0)
   {
      ufdbLogFatalError( "cannot bind daemon socket: %s (protocol=%s)", strerror(errno), 
                         protocol==AF_INET?"IP":"UNIX" );
      fprintf( stderr, "cannot bind daemon socket: %s (protocol=%s) *****\n", strerror(errno), 
               protocol==AF_INET?"IP":"UNIX" );
      ufdbLogMessage( "check for and kill old daemon processes" );
      fprintf( stderr, "check for and kill old daemon processes\n" );
#if HAVE_UNIX_SOCKETS
      ufdbLogMessage( "and remove UNIX socket file \"%s\"", addr_un.sun_path );
      fprintf( stderr, "and remove UNIX socket file \"%s\"\n", addr_un.sun_path );
#endif
      close( s );
      UFDBchangeStatus( UFDB_API_STATUS_FATAL_ERROR );
      pthread_exit( (void *) 0 );
   }
   
   /*
    * According to comment in the Apache httpd source code, these socket
    * options should only be set after a successful bind....
    */
   sock_parm = 1;
   setsockopt( s, SOL_SOCKET, SO_KEEPALIVE, (void *) &sock_parm, sizeof(sock_parm) );

   if (protocol == AF_INET)
   {
      sock_parm = 1;
      setsockopt( s, IPPROTO_TCP, TCP_NODELAY, (void *) &sock_parm, sizeof(sock_parm) );
   }

   if (listen( s, UFDB_MAX_WORKERS ) < 0)
   {
      ufdbLogError( "cannot listen on daemon socket: %s", strerror(errno) );
      fprintf( stderr, "cannot listen on daemon socket: %s\n", strerror(errno) );
      close( s );
      pthread_exit( (void *) 0 );
   }

   if (protocol == AF_INET)
      ufdbLogMessage( "listening on interface %s port %d", UFDBglobalInterface, UFDBglobalPortNum );
#if HAVE_UNIX_SOCKETS
   else
      ufdbLogMessage( "listening on UNIX socket %s", addr_un.sun_path );
#endif

   /* Now is the right moment to write out main PID to the pid file
    * that is used by /etc/init.d/ufdb to send signals to ufdbguardd.
    */
   writePidFile();
   atexit( removePidFile );

#ifdef _POSIX_PRIORITY_SCHEDULING
   ufdbLogMessage( "processor yielding is enabled" );
#endif
   ufdbGetSysInfo( &sysinfo );
   num_cpus = ufdbGetNumCPUs();
   ufdbLogMessage( "system: %s %s %s %s on %ld CPU%s", sysinfo.machine, sysinfo.sysname, sysinfo.release, 
                   sysinfo.nodename, num_cpus, num_cpus>1?"s":"" );
   ufdbLogMessage( "ufdbguardd " VERSION " started with %d URL verification threads and %d SSL verification threads", 
                   n_workers, UFDB_NUM_HTTPS_VERIFIERS );
   if (UFDBglobalHttpdPort > 0)
      startHttpDaemon();

   UFDBglobalReconfig = 0;

   daemon_accept_connections( s, protocol );

   removePidFile();
   pthread_exit( NULL );

   return NULL;
}


/*
 * This thread waits and handles the following signals:
 *    HUP, CHLD, USR1, USR2, TERM, INT and ALRM.
 */
void * signal_handler_thread( void * ptr )
{
   sigset_t        sigset;
   int             sig;

   ufdbSetThreadCPUaffinity( 0 );

   /* The HUP and other signals must be blocked.
    * This is a requirement to use sigwait() in a thread.
    */
   sigemptyset( &sigset );
   sigaddset( &sigset, SIGHUP );
   sigaddset( &sigset, SIGUSR1 );
   sigaddset( &sigset, SIGUSR2 );
   sigaddset( &sigset, SIGCHLD );
   sigaddset( &sigset, SIGTERM );
   sigaddset( &sigset, SIGINT );
   sigaddset( &sigset, SIGALRM );
   pthread_sigmask( SIG_BLOCK, &sigset, NULL );

   while (1)
   {
      int        status;
      pid_t      pid;

      while ((pid = waitpid( -1, &status, WNOHANG )) > 0)
	 ;

      sig = 0;
      if (0 != sigwait( &sigset, &sig ))
      {
         ufdbLogError( "signal_handler_thread: sigwait() failed with %s", strerror(errno) );
	 continue;
      }

      switch (sig)
      {
      case SIGCHLD:   
         break;
      case SIGTERM:
      case SIGINT:
         TermSignalCaught( sig );
	 break;
      case SIGHUP:
	 HupSignalCaught( sig );
	 break;
      case SIGUSR1:
	 Usr1SignalCaught( sig );
         break;
      case SIGUSR2:
	 Usr2SignalCaught( sig );
         break;
      case SIGALRM:
	 ufdbHandleAlarmForTimeEvents( UFDB_PARAM_ALARM );
         break;
      default:
         ufdbLogError( "unexpected signal %d received", sig );
      }
   }

   return NULL;
}


extern struct Category * Dest;


/*
 * A pseudo-random sample of URLs is checked against all known categories
 * and if it appears to be uncategorised, it is buffered to be sent
 * to URLfilterDB at a later date (when SIGHUP is received).
 *
 * return 0 iff the URL is uncategorised, 1 otherwise.
 */
int ufdbVerifyURLallCats( 
   UFDBrevURL *          revURL,
   char *                URL )
{
   struct Category *     cat;
   struct UFDBmemTable * mt;

   if (UFDBglobalReconfig)
      return 1;

   if (UFDBglobalCheckedDB.table.nNextLevels > 0)
   {
      if (UFDBlookupRevUrl( &(UFDBglobalCheckedDB.table.nextLevel[0]), revURL ))
         return 1;
   }
   if (UFDBglobalCheckedExpressions != NULL)
   {
      if (ufdbRegExpMatch( UFDBglobalCheckedExpressions, URL ) == UFDB_API_MATCH)
         return 1;
   }

   for (cat = Dest;  cat != NULL;  cat = cat->next)
   {
      if (UFDBglobalReconfig)
         return 1;
      if (cat->domainlistDb != NULL)
      {
	 mt = (struct UFDBmemTable *) cat->domainlistDb->dbcp;
	 if (UFDBlookupRevUrl( &(mt->table.nextLevel[0]), revURL ))
	    return 1;
      }
      if (cat->regExp != NULL)
      {
	 if (ufdbRegExpMatch( cat->regExp, URL ) == UFDB_API_MATCH)
	    return 1;
      }
   }

   return 0;
}


static volatile int sample = 1;


static char * _blockReason(
   int reason )
{
   switch (reason)
   { 
   case UFDB_API_BLOCK_NONE:          return "PASS";
   case UFDB_API_BLOCK_ACL:           return "BLOCK";
   case UFDB_API_BLOCK_HTTPS_OPTION:  return "BLOCK";
   case UFDB_API_BLOCK_SKYPE:         return "BLOCK";
   case UFDB_API_BLOCK_SAFESARCH:     return "REDIR";
   case UFDB_API_BLOCK_LOADING_DB:    return "BLOCK-LD";
   case UFDB_API_BLOCK_FATAL_ERROR:   return "BLOCK-FATAL";
   }
   return "BLOCK";
}


/*
 * main of the worker threads.
 */
static void * worker_main( void * ptr )
{
   sigset_t                sigset;
   UFDBthreadAdmin *       admin;
   int                     tnum;
   int                     fd;
   int                     nbytes;
   unsigned long           num_reqs;
   struct Source *         src;
   struct Acl *            acl;
   char * 	 	   redirect;
   struct SquidInfo        squidInfo;
   char *                  line;
   char *                  answer;
   char *                  tmp;
#ifdef UFDB_HAVE_NATIVE_RWLOCK_MUTEX
   int                     ret;
#endif

   extern struct Source *  Source;

   tnum = (int) ((long) ptr);

   /* The HUP signal must be blocked.
    * This is a requirement to use sigwait() in a thread.
    */
   sigemptyset( &sigset );
   sigaddset( &sigset, SIGHUP );
   sigaddset( &sigset, SIGUSR1 );
   sigaddset( &sigset, SIGUSR2 );
   sigaddset( &sigset, SIGCHLD );
   sigaddset( &sigset, SIGTERM );
   sigaddset( &sigset, SIGINT );
   sigaddset( &sigset, SIGALRM );
   pthread_sigmask( SIG_BLOCK, &sigset, NULL );

   ufdbSetThreadCPUaffinity( tnum );

   admin = UFDBallocThreadAdmin();

   line = ufdbCalloc( 1, UFDB_MAX_URL_LENGTH );
   answer = ufdbCalloc( 1, UFDB_MAX_URL_LENGTH );
   tmp = ufdbCalloc( 1, UFDB_MAX_URL_LENGTH );

   while (1)
   {
      pthread_testcancel();
      getFdQueue( &fd );
      num_reqs = 0UL;
      if (UFDBglobalDebug)
	 ufdbLogMessage( "W%02d: open fd %2d", tnum, fd );

      while ((nbytes = read( fd, line, UFDB_MAX_URL_LENGTH-2 )) > 0)
      {
	 int access;

	 pthread_testcancel();

         line[nbytes] = '\0';
         num_reqs++;
	 UFDBlookupCounter++;	/* no mutex: faster and we do not care about a few increments less */

         if (UFDBglobalDebug > 1)
	    ufdbLogMessage( "W%02d: line=%s", tnum, line );

	 if (strncmp( line, "http://qdaemonstatus.urlfilterdb.com", 36 ) == 0)
	 {
	    int nc;
	    nc = sprintf( tmp, "http://cgibin.urlfilterdb.com/cgi-bin/URLblocked.cgi?reconfig=%d\n", UFDBglobalReconfig );
	    if (write( fd, tmp, nc ) < 0)
	    {
	       ufdbLogError( "W%02d: write failed: fd=%d %s", tnum, fd, strerror(errno) );
	       goto write_error;
	    }
	    continue;
	 }

	 /* The strstr() is to prevent loops with redirect 302:... ads and other URLs
	  * that are matched by regular expressions 
	  */
	 if (strstr( line, "/URLblocked.cgi" ) != NULL  ||
	     strncmp( line, "http://cgibin.urlfilterdb.com/", 30 ) == 0)
	 {
	    if (write( fd, "\n", 1 ) < 0)
	    {
	       ufdbLogError( "W%02d: write failed: fd=%d %s", tnum, fd, strerror(errno) );
	       goto write_error;
	    }
	    continue;
	 }

	 squidInfo.revUrl = NULL;
	 if (parseLine(admin, line, &squidInfo) != 1) 		/* break down input line into struct squidInfo */
	 {
	    line[400] = '\0';
	    for (tmp = line; *tmp != '\0'; tmp++)
	       if (!isprint(*tmp))
	          *tmp = '_';
	    ufdbLogError( "W%02d: error parsing input line: %s", tnum, line );
	    if (write( fd, "\n", 1 ) < 0)
	    {
	       ufdbLogError( "W%02d: write failed: fd=%d %s", tnum, fd, strerror(errno) );
	       UFDBfreeRevURL( admin, squidInfo.revUrl );
	       goto write_error;
	    }
	    continue;
	 }

	 if (squidInfo.srcDomain[0] == '\0') 
	 {
	    squidInfo.srcDomain[0] = '-';
	    squidInfo.srcDomain[1] = '\0';
	 }

	 if (squidInfo.ident[0] == '\0') 
	 {
	    squidInfo.ident[0] = '-';
	    squidInfo.ident[1] = '\0';
	 }

	 if (strcmp( squidInfo.protocol, "https" ) == 0)
	    UFDBlookupHttpsCounter++;
	 UFDBregisterCountedIP( squidInfo.src );

#ifdef UFDB_HAVE_NATIVE_RWLOCK_MUTEX
	 ret = pthread_rwlock_rdlock( &TheDatabaseLock );
#ifdef UFDB_DEBUG
	 if (ret != 0)
	    ufdbLogError( "worker_main: pthread_rwlock_rdlock failed with code %d", ret );
#endif
#endif

	 for (src = Source; ; )
	 {
	    char * category;
	    char * ACLident;

	    strcpy( tmp, squidInfo.src );	/* TODO: find out if strcpy() is necessary... */
	    src = sgFindSource( src, tmp, &squidInfo );
	    acl = sgAclCheckSource( src );

	    if (parseOnly)
	    {
	       if (write( fd, "\n", 1 ) < 0)
	       {
		  ufdbLogError( "W%02d: write failed: fd=%d %s", tnum, fd, strerror(errno) );
		  UFDBfreeRevURL( admin, squidInfo.revUrl );
		  goto write_error;
	       }
	       break;
	    }

	    redirect = tmp;
	    access = UFDBCheckACLaccess( src, acl, &squidInfo, redirect );		/* DUNNO/ALLOW/BLOCK */

	    if (globalDebugRedirect)
	    {
		ufdbLogMessage( "W%02d:   REDIRECT %s %s/%s %s %s %s", tnum,
				redirect, squidInfo.src, squidInfo.srcDomain, 
				squidInfo.ident, "GET", squidInfo.urlgroup );
	    }

	    if (squidInfo.blockReason == UFDB_API_BLOCK_FATAL_ERROR)
	    {
	       ACLident = "config";
	       category = "fatal-error";
	    }
	    else if (squidInfo.blockReason == UFDB_API_BLOCK_LOADING_DB)
	    {
	       ACLident = "config";
	       category = "loading-database";
	    }
	    else 
	    {
	       if (acl == NULL)
		  ACLident = "acl-error";
	       else
		  ACLident = acl->name;

	       if (UFDBglobalFatalError)
		  category = "fatal-error";
	       else if (UFDBglobalReconfig)
		  category = "loading-database";
	       if (squidInfo.aclpass == NULL  ||  squidInfo.aclpass->name == NULL)
		  category = "error";
	       else 
		  category = squidInfo.aclpass->name;
	    }

	    if (access == UFDB_ACL_ACCESS_DUNNO  ||  access == UFDB_ACL_ACCESS_ALLOW)
	    {
	       if (src == NULL || src->cont_search == 0) 		/* sources are exhausted */
	       {
								     /* global SafeSearch */
		  if (access == UFDB_ACL_ACCESS_DUNNO  &&
		      UFDBglobalSafeSearch  &&
		      UFDBaddSafeSearch( squidInfo.domain, squidInfo.surl, squidInfo.orig ) 
			 == UFDB_API_MODIFIED_FOR_SAFESEARCH)
		  {
		     int len;

		     UFDBsafesearchCounter++;
		     if (UFDBglobalLogAllRequests || UFDBglobalLogBlock || globalDebugRedirect)
		     {
			ufdbLogMessage( "REDIR %-8s %-15s %-10s %-9s %s %s", 
					squidInfo.ident, squidInfo.src, ACLident,
					"SAFESEARCH", squidInfo.url2display, squidInfo.urlgroup );
		     }

		     /* send a REDIRECT to Squid */
		     len = strlen( squidInfo.orig );
		     squidInfo.orig[len] = '\n';
		     len++;
		     squidInfo.orig[len] = '\0';
		     if (write( fd, squidInfo.orig, len ) < 0)
		     {
			ufdbLogError( "W%02d: write failed: fd=%d %s", tnum, fd, strerror(errno) );
			UFDBfreeRevURL( admin, squidInfo.revUrl );
			goto write_error;
		     }
		     break;
		  }

		  if (write( fd, "\n", 1 ) < 0)
		  {
		     ufdbLogError( "W%02d: write failed: fd=%d %s", tnum, fd, strerror(errno) );
		     UFDBfreeRevURL( admin, squidInfo.revUrl );
		     goto write_error;
		  }

		  if (UFDBglobalLogAllRequests)
		  {
		     ufdbLogMessage( "PASS  %-8s %-15s %-10s %-9s %s %s",
				     squidInfo.ident, squidInfo.src, ACLident, category, 
				     squidInfo.url2display, squidInfo.urlgroup );
		  }

		  if (UFDBglobalUploadStats && !UFDBglobalReconfig)
		  {
		     sample++;
		     if (sample % 11 == 0)	/* approx. 9% is sampled */
		     {
			if ((squidInfo.domain[0] <= '9'  &&  squidInfo.domain[0] >= '0')  &&
			    ( (strncmp( squidInfo.domain, "10.", 3 )==0) ||
			      (strncmp( squidInfo.domain, "127.", 4 )==0) ||
			      (strncmp( squidInfo.domain, "192.168.", 8 )==0) ))
			{
			   /* do NOT check 10.*, 127.* and 192.168.*  with uncategorised domains */
			}
			else
			{
			   if (!ufdbVerifyURLallCats( squidInfo.revUrl, squidInfo.url2display ))
			   {
			      ufdbRegisterUnknownURL( squidInfo.domain, squidInfo.port );
			      UFDBuncategorisedCounter++;
			   }
			}
		     }
		  }

		  break;
	       }

	       /* source != NULL */

	       if (access == UFDB_ACL_ACCESS_DUNNO  &&  src->next != NULL) 
	       {
		  src = src->next;
		  continue;
	       }
	       else
	       {
		  if (write( fd, "\n", 1 ) < 0)
		  {
		     ufdbLogError( "W%02d: write failed: fd=%d %s", tnum, fd, strerror(errno) );
		     UFDBfreeRevURL( admin, squidInfo.revUrl );
		     goto write_error;
		  }
		  break;
	       }
	    }
	    else			/* access == UFDB_ACL_ACCESS_BLOCK */
	    {
	       if (testMode)
	       {
		  ufdbLogMessage( "TEST-%-5s %s %s %s %s %s %s",
				  _blockReason(squidInfo.blockReason),
				  squidInfo.ident, squidInfo.src, ACLident,
				  category, squidInfo.url2display, squidInfo.urlgroup );
		  if (write( fd, "\n", 1 ) < 0)	/* send back an "OK" message */
		  {
		     ufdbLogError( "W%02d: write failed: fd=%d %s", tnum, fd, strerror(errno) );
		     UFDBfreeRevURL( admin, squidInfo.revUrl );
		     goto write_error;
		  }
		  UFDBtestBlockCounter++;
	       }
	       else
	       {
		  char urlgrp[64];

		  if (squidInfo.urlgroup[0] == '#')
		     urlgrp[0] = '\0';
		  else
		  {
		     urlgrp[0] = ' ';
		     strncpy( &urlgrp[1], squidInfo.urlgroup, 62 );
		     urlgrp[63] = '\0';
		  }

		  if (UFDBglobalLogBlock || UFDBglobalLogAllRequests)
		     ufdbLogMessage( "%-5s %-8s %-15s %-10s %-9s %s %s",
				     _blockReason(squidInfo.blockReason),
				     squidInfo.ident, squidInfo.src, ACLident,
				     category, squidInfo.url2display, squidInfo.urlgroup );
		  sprintf( answer, "%s %s/%s %s %s%s\n", redirect, squidInfo.src,
			   squidInfo.srcDomain, squidInfo.ident, "GET", urlgrp );
		  if (write( fd, answer, strlen(answer) ) < 0)
		  {
		     ufdbLogError( "W%02d: write failed: fd=%d %s", tnum, fd, strerror(errno) );
		     UFDBfreeRevURL( admin, squidInfo.revUrl );
		     goto write_error;
		  }
		  UFDBblockCounter++;
	       }
	       break;
	    }
	 }
#ifdef UFDB_HAVE_NATIVE_RWLOCK_MUTEX
	 (void) pthread_rwlock_unlock( &TheDatabaseLock );
#endif
	 UFDBfreeRevURL( admin, squidInfo.revUrl );

#ifdef _POSIX_PRIORITY_SCHEDULING
	if (UFDBhttpsVerificationQueued() > 2)
	   sched_yield();	
#endif
	 if (sig_other)
	    break;
         pthread_testcancel();
      }
      if (nbytes < 0)
      {
         ufdbLogError( "W%02d: read failed: fd=%d %s", tnum, fd, strerror(errno) );
      }
write_error:

#ifdef UFDB_HAVE_NATIVE_RWLOCK_MUTEX
      (void) pthread_rwlock_unlock( &TheDatabaseLock );
#endif

      if (num_reqs > 1UL)
	 ufdbLogMessage( "W%02d: %lu URL verifications", tnum, num_reqs );

      if (UFDBglobalDebug)
	 ufdbLogMessage( "W%02d: close fd %2d", tnum, fd );
      close( fd );
      fd = -1;
   }

   /*NOTREACHED*/
   pthread_exit( NULL );
}


void ufdbCatchBadSignal( int signum )	/* TODO: gracefull exit on SIGSEGV */
{
#ifdef __linux__
   int       i;
   int       num;
   int       i_am_thread;
   pthread_t self;
   char      me[32];
#endif
   
   UFDBglobalReconfig = 1;		/* be extra careful and set UFDBglobalReconfig */
   alarm( 0 );

   /* ufdbguardd is a multithreaded application and other threads may also 
    * call UFDBchangeStatus() ...  We have seen the status being changed 
    * to ERROR and immediately followed by another status change to RELOADED.
    * To try to make it more robust we do a short sleep.
    */
   sleep( 1 );

   UFDBchangeStatus( UFDB_API_STATUS_FATAL_ERROR );

#ifdef __linux__
   if (strcmp( UFDB_GDB_PATH, "no" ) == 0)
   {
      ufdbLogMessage( "ufdb-pstack is not called since the gdb package was not installed" );
   }
   else
   {
      FILE * fp;
      char   buffer[512];
      char   cmd[512];

      sprintf( cmd, DEFAULT_BINDIR "/ufdb-pstack %d 2>&1", getpid() );
      fp = popen( cmd, "r" );
      while (fgets( buffer, 512, fp ) != NULL)
      {
	 char * nl;
	 nl = strchr( buffer, '\n' );
	 if (nl != NULL)
	    *nl = '\0';
         ufdbLogMessage( "pstack  %s", buffer );
      }
      pclose( fp );
   }

   self = pthread_self();

   /* find out which thread has a signal */
   i_am_thread = 0;
   sprintf( me, "pid %d", (int) getpid() );
   if (self == threadedSignalHandler)
   {
      strcpy( me, "thread signal-handler" );
      i_am_thread = 1;
   }
   else if (self == sockethandler)
   {
      strcpy( me, "thread socket-handler" );
      i_am_thread = 1;
   }
   else 
   {
      num = UFDB_NUM_HTTPS_VERIFIERS;
      for (i = 0; i < num; i++)
         if (self == httpsthr[i])
	 {
	    sprintf( me, "thread https-verifier-%02d", i );
	    i_am_thread = 1;
	    break;
	 }
      num = n_workers;
      for (i = 0; i < num; i++)
         if (self == workers[i])
	 {
	    sprintf( me, "thread worker-%02d", i );
	    i_am_thread = 1;
	    break;
	 }
   }

   ufdbLogMessage( "\"%s\" caught signal %d.", me, signum );
#endif

   removePidFile();
   pthread_cancel( sockethandler );

   sleep( 15 );

   UFDBchangeStatus( UFDB_API_STATUS_TERMINATED );
   ufdbLogMessage( "ufdbguardd aborts" );

   kill( globalPid, SIGABRT );

   sleep( 2 );
   exit( 2 );
}


extern char ** environ;


int main( 
   int            argc,
   char **        argv )
{
   int            i;
   pid_t          pid;
   sigset_t       sigset;
   pthread_attr_t attr;
   time_t 	  t;
   void *         dummy_ptr;
   char           niso_str[22];

   char ** envp = environ;

   strcpy( progname, "ufdbguardd" );
   UFDBinitializeIPcounters();

   /* TODO: unset LC variables; regex cannot deal with multibyte characters */

   while ((i = getopt(argc, argv, "ATNdDRPSh?rt:c:L:p:U:vw:")) != EOF)
   {
      switch (i) {
      case 'A':
	 fprintf( stderr, "-A option is obsolete\n");
	 break;
      case 'N':
         UFDBglobalAnalyseUncategorisedURLs = -1;
	 break;
      case 'D':
	 runAsDaemon = 0;	/* undocumented -D option to prevent running as daemon */
	 break;
      case 'd':
	 UFDBglobalDebug = 1;
#ifdef UFDB_DEBUG
	 yydebug = 1;
#endif
	 break;
      case 'c':
	 configFile = optarg;
	 break;
      case 'p':
      	 portNumCmdLine = atoi( optarg );
	 if (portNumCmdLine <= 0) {
	    fprintf( stderr, "port number must be > 0\n" );
	    exit( 1 );
	 }
	 break;
      case 'P':			/* undocumented -P option for development purposes */
	 parseOnly = 1;
	 break;
      case 'R':
         UFDBglobalDebugRegexp = 1;
	 break;
      case 'S':
         UFDBglobalUploadStats = 0;
	 break;
      case 'r':
	 globalDebugRedirect = 1;
	 break;
      case 't':			/* undocumented -t option for time-related testing */
	 if ((t = iso2sec(optarg)) == -1) {
	    fprintf( stderr, "-t dateformat error, should be yyyy-mm-ddTHH:MM:SS\n" );
	    exit( 1 );
	 }
	 if (t < 0) {
	    fprintf( stderr, "-t date have to after 1970-01-01T01:00:00\n" );
	    exit( 1 );
	 }
	 niso( t, niso_str );
	 ufdbLogMessage( "ufdbguardd emulating date %s", niso_str );
	 globalDebugTimeDelta = t - start_time.tv_sec;
	 start_time.tv_sec = start_time.tv_sec + globalDebugTimeDelta;
	 break;
      case 'T':
	 testMode = 1;		
	 break;
      case 'U':
	 if (strlen(optarg) <= 31)
	    strcpy( UFDBglobalUserName, optarg );
	 else
	    ufdbLogFatalError( "username supplied with -U option is too long" );
         break;
      case 'L':				/* undocumented -L option for alternate PID file */
	 globalPidFile = optarg;
         break;
      case 'v':
	 fprintf( stderr, "ufdbguardd: %s\n", VERSION );
	 exit( 0 );
	 break;
      case 'w':
         n_workers = atoi( optarg );
	 if (n_workers < 32)
	    n_workers = 32;
	 else if (n_workers > UFDB_MAX_WORKERS)
	 {
	    fprintf( stderr, "-w option exceeds the maximum.  The #threads is set to %d\n", UFDB_MAX_WORKERS );
	    n_workers = UFDB_MAX_WORKERS;
	 }
	 break;
      case '?':
      case 'h':
         usage( '\0' );
	 break;
      default:
         usage( i );
      }
   }

   UFDBdropPrivileges( UFDBglobalUserName );

   if (runAsDaemon)
   {
      if ((pid = fork()) != 0)
	 exit( 0 );        
#ifndef UFDB_DEBUG
      close( 0 );
      close( 1 );
#endif
      setsid();
   }
   globalPid = getpid();

   /*
    * Initialise signal handlers.
    * The HUP signal is dealt with on a per thread basis.
    */
#ifndef UFDB_CORE_DUMP_SEGV
   ufdbSetSignalHandler( SIGILL,  ufdbCatchBadSignal );
   ufdbSetSignalHandler( SIGBUS,  ufdbCatchBadSignal );
   ufdbSetSignalHandler( SIGSEGV, ufdbCatchBadSignal );
#endif

   ufdbSetSignalHandler( SIGPIPE, SIG_IGN );

   ufdbSetSignalHandler( SIGABRT, catchAbortSignal );

   ufdbSetSignalHandler( SIGTERM, catchTermSignal );
   ufdbSetSignalHandler( SIGINT,  catchTermSignal );

   ufdbSetSignalHandler( SIGHUP,  catchHUPSignal );
   ufdbSetSignalHandler( SIGUSR1, catchUSR1Signal );
   ufdbSetSignalHandler( SIGUSR2, catchUSR2Signal );

   ufdbSetSignalHandler( SIGCHLD, catchChildSignal );

   /* All signals must be blocked.
    * This is a requirement to use sigwait() in a thread.
    */
   sigemptyset( &sigset );
   sigaddset( &sigset, SIGHUP );
   sigaddset( &sigset, SIGUSR1 );
   sigaddset( &sigset, SIGUSR2 );
   sigaddset( &sigset, SIGCHLD );
   sigaddset( &sigset, SIGTERM );
   sigaddset( &sigset, SIGINT );
   sigaddset( &sigset, SIGALRM );
   pthread_sigmask( SIG_BLOCK, &sigset, NULL );

   my_argc = argc;
   my_argv = argv;
   my_envp = envp;

#ifdef UFDB_HAVE_NATIVE_RWLOCK_MUTEX
   pthread_rwlockattr_t mylock_attr;
   pthread_rwlockattr_init( &mylock_attr );
#ifdef defined(HAVE_PTHREAD_RWLOCK_PREFER_WRITER_NP) && defined(HAVE_PTHREAD_RWLOCK_PREFER_WRITER_NP)
   pthread_rwlockattr_setkind_np( &mylock_attr, PTHREAD_RWLOCK_PREFER_WRITER_NP );
#endif
   pthread_rwlock_init( &TheDatabaseLock, mylock_attr );
#endif

   /*
    * create the threads.
    */
   pthread_attr_init( &attr );
   pthread_attr_setscope( &attr, PTHREAD_SCOPE_SYSTEM );
   pthread_attr_setstacksize( &attr, 248 * 1024 );
#if HAVE_PTHREAD_ATTR_SETGUARDSIZE
   pthread_attr_setguardsize( &attr, 8 * 1024 );
#endif

   pthread_create( &threadedSignalHandler, &attr, signal_handler_thread, (void *) 0 );
#ifdef _POSIX_PRIORITY_SCHEDULING
   sched_yield();
#else
   usleep( 10000 );
#endif

   pthread_create( &sockethandler, &attr, socket_handler_thread, (void *) 0 );
#ifdef _POSIX_PRIORITY_SCHEDULING
   sched_yield();
#else
   usleep( 10000 );
#endif
   usleep( 30000 );

   if (UFDBglobalDebug)
      ufdbLogMessage( "using %d worker threads", n_workers );
   for (i = 0; i < n_workers; i++)
   {
      pthread_create( &workers[i], &attr, worker_main, (void *) ((long) i) );
#ifdef _POSIX_PRIORITY_SCHEDULING
      sched_yield();
#else
      usleep( 10000 );
#endif
   }

   /*
    * exit when the sockethandler thread terminates.
    */
   pthread_join( sockethandler, &dummy_ptr );

   /*
    * cleanup fast. cancel all other threads and wait a short while.
    */
   UFDBglobalReconfig = 1;
   alarm( 0 );
#ifdef UFDB_CLEANUP_CANCELS_ALL_THREADS
   for (i = 0; i < n_workers; i++)
      pthread_cancel( workers[i] );
   for (i = 0; i < UFDB_NUM_HTTPS_VERIFIERS; i++)
      pthread_cancel( httpsthr[i] );
   sleep( 1 );
#endif

   if (UFDBglobalDebug)
      sleep( 1 );

   removePidFile();
   ufdbLogMessage( "ufdbGuard daemon stopped" );
   UFDBchangeStatus( UFDB_API_STATUS_TERMINATED );

   exit( 0 );

   return 0;
}


static void uploadStatistics( 
   char * reason )
{
   char * URLs;
   char * message;
   int    length;
   int    s;
   int    nbytes;
   int    written;
   long   num_cpus;
   unsigned long  num_clients;
   struct utsname sysinfo;

   if (!UFDBglobalUploadStats)
      return;

   num_clients = UFDBgetNumberOfRegisteredIPs();
   if (UFDBglobalCheckedDB.mem == NULL)		/* not a client of URLfilterDB */
   {
      if (UFDBlookupCounter < 200)
	 return;
      if (num_clients < 2)
         return;
   }

   if (UFDBglobalDebug)
      ufdbLogMessage( "uploadStatistics (upload uncategorised URLs: %s)", 
                      UFDBglobalAnalyseUncategorisedURLs>0 ? "yes" : "no" );

   if (UFDBglobalAnalyseUncategorisedURLs > 0)
      URLs = ufdbGetUnknownURLs();
   else
      URLs = "example.com|";

   length = strlen( URLs );
   if (length <= 1)
   {
      URLs = "example.com|";
      length = strlen( URLs );
   }

   message = ufdbMalloc( 1500 + length );
   if (message == NULL)
   {
      ufdbResetUnknownURLs();
      return;
   }

   ufdbGetSysInfo( &sysinfo );
   num_cpus = ufdbGetNumCPUs();

   sprintf( message, 
	    "POST /cgi-bin/uncat.pl HTTP/1.1\r\n"
	    "Host: " UFDB_UPLOAD_UNCATEGORISED_URLS_WEBSITE "\r\n"
	    "User-Agent: ufdbGuardd-" VERSION "\r\n"
	    "Content-Type: text/plain\r\n"
	    "Content-Length: %d\r\n"
	    "Connection: close\r\n"
	    "X-HasURLfilterDB: %s\r\n"
	    "X-SiteInfo: %s %ld %s %s\r\n"
	    "X-NodeName: %s\r\n"
	    "X-NumClients: %lu\r\n"
	    "X-NumLookups: %lu\r\n"
	    "X-NumHttpsLookups: %lu\r\n"
	    "X-NumTunnelsDetected: %lu\r\n"
	    "X-NumTestBlock: %lu\r\n"
	    "X-NumBlock: %lu\r\n"
	    "X-NumSafeSearch: %lu\r\n"
	    "X-Uncategorised: %lu\r\n"
	    "X-UploadSeqNo: %lu\r\n"
	    "X-UploadReason: %s\r\n"
	    "\r\n"
	    "%s\r\n",
            length, 
	    (UFDBglobalCheckedDB.mem == NULL ? "no" : "yes"),
	    sysinfo.machine, num_cpus, sysinfo.sysname, sysinfo.release, 
	    sysinfo.nodename,
	    num_clients,
	    UFDBlookupCounter,
	    UFDBlookupHttpsCounter,
	    UFDBglobalTunnelCounter,
	    UFDBtestBlockCounter,
	    UFDBblockCounter,
	    UFDBsafesearchCounter,
	    UFDBuncategorisedCounter,
	    UFDBuploadSeqNo,
	    reason,
	    URLs );
   length = strlen( message );

   /* This function migth be called N times when a bad signal is received.
    * Let's try to execute the upload only once by resetting the unknown URLs quickly.
    */
   ufdbResetUnknownURLs();

   s = UFDBopenSocket( UFDB_UPLOAD_UNCATEGORISED_URLS_WEBSITE, 80 );
   if (s < 0)
   {
      ufdbLogError( "cannot upload statistics/uncategorised URLs  *****\n"
                    "cannot open a communication socket to %s:%d (%s)",
                    UFDB_UPLOAD_UNCATEGORISED_URLS_WEBSITE, 80, strerror(errno) );
      return;
   }

   written = 0;
   while (length > 0)
   {
      nbytes = write( s, message+written, length );
      if (nbytes == 0)
         break;
      if (nbytes < 0)
      {
         if (errno != EINTR)
	    break;
      }
      else
      {
         written += nbytes;
	 length -= nbytes;
      }
   }

   close( s );

   ufdbFree( message );

   if (UFDBglobalDebug  ||  UFDBglobalAnalyseUncategorisedURLs > 0)
      ufdbLogMessage( "uncategorised domains have been uploaded to URLfilterDB" );
}

