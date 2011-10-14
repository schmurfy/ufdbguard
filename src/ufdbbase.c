/*
 * ufdbbase.c - URLfilterDB
 *
 * ufdbGuard is copyrighted (C) 2005-2010 by URLfilterDB with all rights reserved.
 *
 * Parts of ufdbGuard are based on squidGuard.
 * This module is NOT based on squidGuard.
 * 
 * RCS $Id: ufdbbase.c,v 1.56 2011/06/22 17:58:34 root Exp root $
 */

#include "ufdb.h"
#include "ufdblib.h"

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include <signal.h>
#include <errno.h>
#include <pthread.h>
#if HAVE_SYS_SYSCALL_H
#include <sys/syscall.h>
#endif
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <pwd.h>

#if HAVE_SETRESUID
int setresuid( uid_t ruid, uid_t euid, uid_t suid );
#endif

volatile int    UFDBglobalStatus = UFDB_API_STATUS_VIRGIN;
volatile int    UFDBglobalDatabaseStatus = UFDB_API_STATUS_DATABASE_OK;
volatile int    UFDBglobalReconfig = 1;
time_t UFDBglobalDatabaseLoadTime = 0;
int    UFDBglobalFatalError = 0;
char   UFDBglobalLicenseStatus[512] = "unknown";
char * UFDBglobalConfigFile = DEFAULT_CONFIGFILE;
int    UFDBglobalDebug = 0;
int    UFDBglobalDebugSkype = 0;
int    UFDBglobalDebugGtalk = 0;
int    UFDBglobalDebugYahooMsg = 0;
int    UFDBglobalDebugAim = 0;
int    UFDBglobalUnknownProtocolOverHttps = 1;
int    UFDBglobalDebugHttpd = 0;
int    UFDBglobalDebugRegexp = 0;
int    UFDBglobalExpressionOptimisation = 1;
char   UFDBglobalUserName[31+1] = "";
char * UFDBglobalLogDir = NULL;
char * UFDBglobalEmailServer = NULL;
char * UFDBglobalAdminEmail = NULL;
char * UFDBglobalExternalStatusCommand = NULL;
int    UFDBglobalLogBlock = 0;
int    UFDBglobalLogAllRequests = 0;
int    UFDBglobalURLlookupResultDBreload = UFDB_ALLOW;
int    UFDBglobalURLlookupResultFatalError = UFDB_ALLOW;
int    UFDBglobalUploadStats = 1;
int    UFDBglobalAnalyseUncategorisedURLs = 1;
int    UFDBglobalPortNum = UFDB_DAEMON_PORT;
char   UFDBglobalInterface[256] = "all";
char   UFDBglobalCAcertsFile[1024] = "";
char   UFDBglobalFatalErrorRedirect[1024] = "http://cgibin.urlfilterdb.com/cgi-bin/URLblocked.cgi?" 
		"category=fatal-error";
char   UFDBglobalLoadingDatabaseRedirect[1024] = "http://cgibin.urlfilterdb.com/cgi-bin/URLblocked.cgi?" 
		"category=loading-database";
int    UFDBglobalSafeSearch = 1;
int    UFDBglobalShowURLdetails = 0;
int    UFDBglobalTunnelCheckMethod = UFDB_API_HTTPS_CHECK_OFF;
int    UFDBglobalHttpsWithHostname = 0;
int    UFDBglobalHttpsOfficialCertificate = 0;
int    UFDBglobalSkypeOverHttps = 0;
int    UFDBglobalGtalkOverHttps = 0;
int    UFDBglobalYahooMsgOverHttps = 0;
int    UFDBglobalAimOverHttps = 0;
int    UFDBglobalHttpsNoSSLv2 = 0;
int    UFDBglobalHttpdPort = 0;
char   UFDBglobalHttpdInterface[256] = "all";
char   UFDBglobalHttpdImagesDirectory[256] = ".";
struct UFDBmemTable UFDBglobalCheckedDB;
struct ufdbRegExp * UFDBglobalCheckedExpressions = NULL;
unsigned long UFDBglobalMaxLogfileSize = 200 * 1024 * 1024;	/* 200 MB */
volatile unsigned long UFDBglobalTunnelCounter = 0;

volatile unsigned long UFDB_API_num_url_lookups = 0;
volatile unsigned long UFDB_API_num_url_matches = 0;
volatile unsigned long UFDB_API_num_safesearch = 0;
volatile unsigned long UFDB_API_upload_seqno = 0;


struct ufdbSetting * lastSetting = NULL;
struct ufdbSetting * Setting = NULL;

struct Source * lastSource = NULL;
struct Source * Source = NULL;
struct Source * saveSource = NULL;

struct Category * lastDest = NULL;
struct Category * Dest = NULL;

struct sgRewrite * lastRewrite = NULL;
struct sgRewrite * Rewrite = NULL;
struct ufdbRegExp *  lastRewriteRegExec = NULL;

struct Time * lastTime = NULL;
struct Time * Time = NULL;

FILE * globalErrorLog = NULL;
struct LogFile * globalLogFile = NULL;

struct LogFileStat * lastLogFileStat;
struct LogFileStat * LogFileStat;

struct TimeElement *lastTimeElement = NULL;
struct TimeElement *TimeElement = NULL;

struct Acl * lastAcl = NULL;
struct Acl * defaultAcl = NULL;
struct Acl * Acl = NULL;
struct AclDest * lastAclDest = NULL;

struct ufdbRegExp * lastRegExpDest = NULL;

char ** globalArgv;
char ** globalEnvp;
int globalDebugTimeDelta = 0;
int globalDebugRedirect = 0;
int globalPid = 0;
int globalUpdate = 0;
char * globalCreateDb = NULL;
int failsafe_mode = 0;
int sig_hup = 0;
int sig_other = 0;
int httpsConnectionCacheSize = UFDB_DEF_HTTPS_CONN_CACHE_SIZE;
char progname[80];

static long          num_cpus;
static unsigned long cpu_mask = 0UL;
#define UFDB_MAX_CPUS 32

#ifdef UFDB_DEBUG
pthread_mutex_t ufdb_malloc_mutex = UFDB_STATIC_MUTEX_INIT;
#endif
pthread_mutex_t ufdb_regexec_mutex = UFDB_STATIC_MUTEX_INIT;


/*
 *  Some libraries use malloc() ...
 *  But they do not use a mutex, so when these libraries are called,
 *  we must use a mutex around the calls to the libraries for those
 *  platforms where malloc() is not threadsafe.
 */
void __inline__ ufdbGetMallocMutex( char * fn )
{
#ifdef UFDB_DEBUG
   int res;
#endif

#ifdef UFDB_MALLOC_IS_THREADSAFE
   return;
#endif

#ifdef UFDB_DEBUG
   res = pthread_mutex_lock( &ufdb_malloc_mutex );
   if (res != 0)
      ufdbLogError( "ufdbGetMallocMutex: mutex_lock failed with code %d in %s", res, fn );
#endif
}


void __inline__ ufdbReleaseMallocMutex( char * fn )
{
#ifdef UFDB_DEBUG
   int res;
#endif

#ifdef UFDB_MALLOC_IS_THREADSAFE
   return;
#endif

#ifdef UFDB_DEBUG
   res = pthread_mutex_unlock( &ufdb_malloc_mutex );
   if (res != 0)
      ufdbLogError( "ufdbReleaseMallocMutex: mutex_unlock failed with code %d in %s", res, fn );
#endif
}


void * ufdbMalloc( size_t elsize )
{
   void * p;

   p = (void *) malloc( elsize );
   if (p == NULL  &&  elsize > 0)
   {
      /* TODO: getrlimit */
      ufdbLogFatalError( "cannot allocate %ld bytes memory", elsize );
   }

#if defined(UFDB_DEBUG)
   ufdbLogMessage( "      ufdbMalloc %d  -> %08x", elsize, p );
#endif

   return p;
}


void * ufdbCalloc( size_t n, size_t num )
{
   void * p;

   p = (void *) calloc( n, num );
   if (p == NULL  &&  n > 0)
   {
      /* TODO: getrlimit */
      ufdbLogFatalError( "cannot allocate %ld bytes memory", n*num );
   }

#if defined(UFDB_DEBUG)
   ufdbLogMessage( "      ufdbCalloc %d %d  = %d  -> %08x", n, num, n*num, p );
#endif

   return p;
}



void * ufdbRealloc( void * ptr, size_t elsize )
{
   void * p;

   p = (void *) realloc( ptr, elsize );
   if (p == NULL  &&  elsize > 0)
   {
      /* TODO: getrlimit */
      ufdbLogFatalError( "cannot allocate %ld bytes memory", elsize );
   }

#if defined(UFDB_DEBUG)
   ufdbLogMessage( "      ufdbRealloc 0x%08x %d  ->  %08x", ptr, elsize, p );
#endif

   return p;
}


void ufdbFree( void * ptr )
{
#if defined(UFDB_DEBUG)
   ufdbLogMessage( "      ufdbFree 0x%08x", ptr );
#endif

   if (ptr != NULL)
   {
      free( ptr );
   }
}


char * ufdbStrdup( const char * s )
{
   int size;

#if defined(UFDB_DEBUG)
   ufdbLogMessage( "   ufdbStrdup %-60.60s", s );
#endif

   size = strlen( s ) + 1;
   return strcpy( ufdbMalloc(size), s );
}


int UFDBregcomp( regex_t * preg, const char * regex, int cflags )
{
   int retval;

   retval = regcomp( preg, regex, cflags );

   return retval;
}


void UFDBregfree( regex_t * preg )
{
   regfree( preg );
}


#if defined(__GLIBC__)
#if (__GLIBC__ > 2) || (__GLIBC__ == 2  &&  __GLIBC_MINOR__ >= 4)
#define NEED_REGEXEC_MUTEX 0
#else
#define NEED_REGEXEC_MUTEX 1
#endif
#else
#define NEED_REGEXEC_MUTEX 0
#endif

__inline__ int UFDBregexec( const regex_t * preg, const char * string, size_t nmatch, regmatch_t pmatch[], int eflags )
{
   int retval;

#if NEED_REGEXEC_MUTEX
   pthread_mutex_lock( &ufdb_regexec_mutex );
#endif

   retval = regexec( preg, string, nmatch, pmatch, eflags );

#if NEED_REGEXEC_MUTEX
   pthread_mutex_unlock( &ufdb_regexec_mutex );
#endif

   return retval;
}


/*
 *  Initialise a database category (open and/or create a .ufdb file) 
 */
int UFDBloadDatabase( 
   struct UFDBmemTable * mtable, 
   char *                file    )
{
   int                   n;
   int                   in;
   struct stat           fileStat;
   char                  f[1024];

   if (file == NULL)
   {
      return UFDB_API_ERR_NULL;
   }

#if 0
   fprintf( stderr, "   UFDBloadDatabase( %s )\n", file );
#endif

   strcpy( f, file );
   in = open( f, O_RDONLY );
   if (in < 0)
   {
      strcat( f, UFDBfileSuffix );
      in = open( f, O_RDONLY );
   }

   if (in < 0)
   {
      return UFDB_API_ERR_NOFILE;
   }

   if (fstat(in,&fileStat) < 0)
   {
      close( in );
      return UFDB_API_ERR_NOFILE;
   }

   mtable->table.tag = (unsigned char *) "UNIVERSE";
   mtable->table.nNextLevels = 0;
   mtable->table.nextLevel = NULL;
   mtable->mem = ufdbMalloc( fileStat.st_size + 1 );
   mtable->tableSize = fileStat.st_size - sizeof(struct UFDBfileHeader);
   n = read( in, mtable->mem, fileStat.st_size );
   close( in );
   if (n != fileStat.st_size)
   {
      return UFDB_API_ERR_READ;
   }

   n = UFDBparseTableHeader( mtable );
#if 0
   fprintf( stderr, "   UFDBparseTableHeader returns %d\n", n );
#endif
   if (n != UFDB_API_OK)
      return n;
   UFDBparseTable( mtable );

   return UFDB_API_OK;
}



struct ufdbRegExp * ufdbNewPatternBuffer( 
  char * pattern, 
  int    flags )
{
  struct ufdbRegExp * re;

  re = (struct ufdbRegExp *) ufdbMalloc( sizeof(struct ufdbRegExp) );

  re->pattern = ufdbStrdup( pattern );
  re->substitute = NULL;
  re->compiled = (regex_t *) ufdbCalloc( 1, sizeof(regex_t) );
  re->error = UFDBregcomp( re->compiled, pattern, flags );
  re->flags = flags;
  re->global = 0;
  re->httpcode = NULL;
  re->next = NULL;

  return re;
}


/* 
 *  optimise list of RE1 ... REn into one RE with (RE1)| ... |(REn) 
 */
struct ufdbRegExp * UFDBoptimizeExprList( 
   char *              reSource,
   struct ufdbRegExp * reList )
{
   struct ufdbRegExp * re;
   char * newpattern;
   int    n;
   int    totalStrlen;

   n = 0;
   totalStrlen = 0;

   for (re = reList;  re != NULL;  re = re->next)
   {
      if (re->error == 0)
      {
         n++;
	 totalStrlen += strlen( re->pattern );
      }
   }

   if (n > 1)
   {
      newpattern = ufdbMalloc( totalStrlen + 3*n + 1 );
      newpattern[0] = '\0';
      for (re = reList;  re != NULL;  re = re->next)
      {
         if (re->error == 0)
	 {
	    strcat( newpattern, "(" );
	    strcat( newpattern, re->pattern );
	    if (re->next == NULL)
	       strcat( newpattern, ")" );
	    else
	       strcat( newpattern, ")|" );
	 }
      }
      re = ufdbNewPatternBuffer( newpattern, reList->flags );
      if (re->error) 
      {
         ufdbLogMessage( "unable to optimise the list of expressions" );
	 ufdbFreeRegExprList( re );
	 return reList;
      }

      ufdbLogMessage( "the expressions of %s have been optimised", reSource );
      if (UFDBglobalDebugRegexp)
         ufdbLogMessage( "optimised expression: %s", newpattern );

      ufdbFreeRegExprList( reList );
      return re;
   }

   return reList;
}


/*
 * initialize an expression list (read them from a file and do the regexp compilation)
 */
int UFDBloadExpressions( 
   struct ufdbRegExp ** exprList,
   char *               file  )
{
   FILE *               fin;
   char *               eoln;
   struct ufdbRegExp *  re;
   struct ufdbRegExp *  last;
   int                  retCode;
   char                 line[1024];

   if (exprList == NULL)
      return UFDB_API_ERR_NULL;
   *exprList = NULL;

   if (file == NULL)
      return UFDB_API_ERR_NULL;

   fin = fopen( file, "r" );
   if (fin == NULL)
      return UFDB_API_ERR_NOFILE;

   retCode = UFDB_API_OK;
   last = NULL;
   re = NULL;

   while (fgets( line, sizeof(line), fin ) != NULL)
   {
      if (line[0] == '#')         /* skip comments */
         continue;

      eoln = strchr( line, '\n' );
      if (eoln == NULL  ||  eoln == &line[0])
         continue;	/* skip empty lines and lines without a newline */
      else
      {
         if (*(eoln-1) == '\r')
	    eoln--;
      }
      *eoln = '\0';	/* get rid of the newline */

      re = ufdbNewPatternBuffer( line, REG_EXTENDED | REG_NOSUB | REG_ICASE );
      if (re->error)
      {
#if 0
	 printf( "error compiling %s: \"%s\"\n", file, line );
#endif
         retCode = UFDB_API_ERR_EXPR;
      }

      re->next = last;
      last = re;
   }

   *exprList = UFDBoptimizeExprList( file, re );
   fclose( fin );

   return retCode;
}


/*
 * match a URL with a compiled RE.
 * return 0 if no match, 1 if there is a match.
 */
int ufdbRegExpMatch(   
   struct ufdbRegExp * regexp, 
   char *              str )
{
   struct ufdbRegExp * rp;
   int                 error;

   if (UFDBglobalDebugRegexp)
      ufdbLogMessage( "   ufdbRegExpMatch \"%s\"", str );

   for (rp = regexp;  rp != NULL;  rp = rp->next)
   {
      if (UFDBglobalDebugRegexp)
	 ufdbLogMessage( "   ufdbRegExpMatch  %s  %s", str, rp->pattern );
      if (rp->error)
         continue;
      error = UFDBregexec( rp->compiled, str, 0, 0, 0 );
      if (error == 0) 	/* match */
      {
	 if (UFDBglobalDebugRegexp)
	    ufdbLogMessage( "   RE match:  %s  %s", rp->pattern, str );
         return UFDB_API_MATCH;
      }
      if (error != REG_NOMATCH) 
      {
	 if (UFDBglobalDebugRegexp)
	    ufdbLogMessage( "   RE error %d:  %s  %s", error, rp->pattern, str );
         return UFDB_API_ERR_EXPR;
      }
   }

   return 0;
}


void ufdbFreeRegExprList( struct ufdbRegExp * re )
{
   if (re == NULL)
      return;

   ufdbFreeRegExprList( re->next );

   ufdbFree( re->pattern );
   UFDBregfree( re->compiled );
   ufdbFree( re->compiled );
   ufdbFree( re->substitute );
   ufdbFree( re );
}


void ufdbResetCPUs( void )
{
   cpu_mask = 0UL;
}


/*
 * Linux support binding threads to a set of CPUs which prevents cache contention.
 * Freebsd has no support, but the 5.x series is recommended for best multithreaded performance.
 * On Solaris it is recommended to start ufdbguardd in a processor set.
 */
int ufdbSetThreadCPUaffinity( int thread_num )
{
#if defined(__linux__) && defined(SYS_sched_setaffinity)
   int retval;

   if (cpu_mask != 0UL)
   {
      retval = syscall( SYS_sched_setaffinity, 0, 4, &cpu_mask );
      if (retval < 0)
         return UFDB_API_ERR_ERRNO;
   }
#endif

   return UFDB_API_OK;
}


/*
 * Bind my processes and threads to a set of CPUs.
 * This increases the cache efficiency for all programs.
 */
int ufdbSetCPU( 
   char * CPUspec )		/* comma-separated CPU numbers (starting with CPU 0) */
{
   int    cpu;

#if defined(_SC_NPROCESSORS_ONLN)
   num_cpus = sysconf( _SC_NPROCESSORS_ONLN );
   /* printf( "this system has %ld CPUs\n", num_cpus ); */

#elif defined(__linux__) && defined(SYS_sched_getaffinity)
   /* sched_getaffinity() is buggy on linux 2.4.x so we use syscall() instead */
   cpu = syscall( SYS_sched_getaffinity, getpid(), 4, &cpu_mask );
   /* printf( "sched_getaffinity returned %d %08lx\n", cpu, cpu_mask ); */
   if (cpu >= 0)
   {
      num_cpus = 0;
      for (cpu = 0; cpu < UFDB_MAX_CPUS; cpu++)
	 if (cpu_mask & (1 << cpu))
	    num_cpus++;
      /* printf( "   found %d CPUs in the cpu mask\n", num_cpus ); */
   }
   else
#else
      num_cpus = UFDB_MAX_CPUS;
#endif

   cpu_mask = 0;
   while (*CPUspec)
   {
      if (sscanf(CPUspec,"%d",&cpu) == 1)
      {
	 if (cpu >= 0 && cpu < num_cpus)
	 {
	    cpu_mask |= (1 << cpu);
	 }
	 else
	    return UFDB_API_ERR_RANGE;
      }
      else
         return UFDB_API_ERR_RANGE;

      /* find the next CPU number */
      while (isdigit( (int) *CPUspec))
         CPUspec++;
      while (*CPUspec == ' '  ||  *CPUspec == ',')
         CPUspec++;
   }

   return UFDB_API_OK;
}


/*
 * A bitmap with IP addresses of clients is used to count the number of clients.
 * To keep the bitmap small (2 MB) the first octet of the IP address is ignored.
 *
 * This code assumes that there are 8 bits per byte.
 */

#define BITMAPLENGTH (256U * 256U * 256U / (sizeof(unsigned int) * 8U))
static unsigned int IPbitmap[BITMAPLENGTH];

/* We can receive from Squid an IP address (most common) or a FQDN.
 * In case that we receive a FQDN, we calculate a hash and use this 
 * as a psuedo IP address.
 */
void UFDBregisterCountedIP( const char * address )
{
   unsigned char * a;
   unsigned int    i, o1, o2, o3;
   unsigned int    nshift;

   a = (unsigned char *) address;

   /* extremely simple way of looking if the parameter is an IP address... */
   if (*a >= '0' && *a <= '9')
   {
      /* first skip the first octect */
      while (*a != '.'  && *a != '\0')
	 a++;
      if (*a == '.') a++;

      o1 = 0;
      while (*a >= '0' && *a <= '9' && *a != '\0')
      {
	 o1 = o1 * 10U + (*a - '0');
	 a++;
      }
      if (*a == '.') a++;

      o2 = 0;
      while (*a >= '0' && *a <= '9' && *a != '\0')
      {
	 o2 = o2 * 10U + (*a - '0');
	 a++;
      }
      if (*a == '.') a++;

      o3 = 0;
      while (*a >= '0' && *a <= '9' && *a != '\0')
      {
	 o3 = o3 * 10U + (*a - '0');
	 a++;
      }
      o1 = (o1 << 16) + (o2 << 8) + o3;
   }
   else 	/* no IP a, calculate a hash */
   {
      o1 = 104729U;
      while (*a != '\0')
      {
         o1 = o1 * 17U + (*a - ' ');
	 a++;
      }
      o1 = o1 % (256U * 256U * 256U);
   }

   i = o1 / (sizeof(unsigned int) * 8U);
   /* if we got a non-existent IP address, we might go out of bounds... */
   if (i >= BITMAPLENGTH)
      i = BITMAPLENGTH - 1;

   nshift = o1 - i * (sizeof(unsigned int) * 8U);
   /* 
    * To be thread-safe we should use a semaphore here.
    * But since this counter is not critical and a lost bit-set 
    * will probably be covered by another call to this function,
    * we choose for superfast code and skip the use of a semaphore.
    */
   IPbitmap[i] |= (1 << nshift);
}


void UFDBinitializeIPcounters( void )
{
   int i;

   for (i = 0; i < BITMAPLENGTH; i++)
      IPbitmap[i] = 0;
}


unsigned long UFDBgetNumberOfRegisteredIPs( void )
{
   unsigned long n;
   unsigned int  v;
   int           i;

   n = 0;
   for (i = 0; i < BITMAPLENGTH; i++)
   {
      v = IPbitmap[i];
      while (v != 0)
      {
         if (v & 1)
	    n++;
	 v >>= 1;
      }
   }

   return n;
}


void ufdbSetSignalHandler( int signal, void (*handler)(int) )
{
#if HAVE_SIGACTION
   struct sigaction act;

#ifndef SA_NODEFER
#define SA_NODEFER 0
#endif

#ifndef SA_NOCLDSTOP
#define SA_NOCLDSTOP 0
#endif

   act.sa_handler = handler;
   act.sa_flags = SA_RESTART;
   if (signal == SIGCHLD)
      act.sa_flags |= SA_NOCLDSTOP;
   if (signal == SIGALRM)
      act.sa_flags |= SA_NODEFER;
   sigemptyset( &act.sa_mask );
   sigaction( signal, &act, NULL );

#else

#if HAVE_SIGNAL
   signal( signal, handler );
#else
   ufdbLogError( "ufdbSetSignalHandler: cannot set handler for signal %d", signal );
#endif

#endif
}


static pid_t   ruid = -1;
static pid_t   euid;


void UFDBdropPrivileges( 
   const char *    username )
{
   struct passwd * fieldptrs;
#if HAVE_GETPWNAM_R
   struct passwd   pws;
   char            pws_fields[1024];
#endif

   if (ruid == -1)
   {
      ruid = getuid();
      euid = geteuid();

      /* umask 137: no x for user, no wx for group, no rwx for others */
      umask( S_IXUSR | S_IWGRP|S_IXGRP | S_IROTH|S_IWOTH|S_IXOTH );
   }

   if (username == NULL  ||  username[0] == '\0')
      return;

#if HAVE_GETPWNAM_R
   if (getpwnam_r( username, &pws, pws_fields, sizeof(pws_fields)-1, &fieldptrs ) != 0)
#else
   fieldptrs = getpwnam( username );
   if (fieldptrs == NULL)
#endif
   {
      ufdbLogError( "Cannot get info on user '%s' so cannot change to this user  *****\n"
      		    "User '%s' probably does not exist.", 
		    username, username );
      return;
   }

   /* 
    * We have already written to the log file which may have been created as user root.
    * and need to change the ownership of the log file to be able to continue logging...
    */
   if (globalErrorLog != NULL)
   {
      if (fchown( fileno(globalErrorLog), fieldptrs->pw_uid, -1 ))
         ;
   }

   if (UFDBglobalDebug)
      ufdbLogMessage( "going to call seteuid(%d) and setegid(%d) to set euid to '%s'", 
                      fieldptrs->pw_uid, fieldptrs->pw_gid, username );

#if HAVE_SETRESUID
   if (setresuid( fieldptrs->pw_uid, fieldptrs->pw_uid, 0 ) != 0)
#else
   if (seteuid( fieldptrs->pw_uid ) != 0)
#endif
   {
      ufdbLogError( "Cannot drop privileges and become user '%s': %s  *****", username, strerror(errno) );
      if (geteuid() != 0)
	 ufdbLogError( "I am not root so cannot drop/change privileges to user '%s'", username );

      return;
   }
   else if (UFDBglobalDebug || UFDBglobalDebugHttpd)
      ufdbLogMessage( "dropped privileges and became user '%s'", username );

   (void) setegid( fieldptrs->pw_gid );

#if 0 && defined(__linux__)
   if (UFDBglobalDebug || UFDBglobalDebugHttpd)
   {
      pid_t rid, eid, sid;
      (void) getresuid( &rid, &eid, &sid );
      ufdbLogMessage( "Privileges are dropped: now running as user '%s'  %d %d %d", username, rid, eid, sid );
   }
#else
   if (UFDBglobalDebug || UFDBglobalDebugHttpd)
      ufdbLogMessage( "Privileges are dropped: now running as user '%s'", username );
#endif
}


char * ufdbAPIstatusString( int api_code )
{
   switch (api_code)
   {
   case UFDB_API_OK:                       return "OK";
   case UFDB_API_ERR_NULL:                 return "ERR_NULL";
   case UFDB_API_ERR_NOFILE:               return "ERR_NOFILE";
   case UFDB_API_ERR_READ:                 return "ERR_READ";
   case UFDB_API_ERR_EXPR:                 return "ERR_EXPR";
   case UFDB_API_ERR_RANGE:                return "ERR_RANGE";
   case UFDB_API_ERR_ERRNO:                return "ERR_ERRNO";
   case UFDB_API_ERR_SOCKET:               return "ERR_SOCKET";
   case UFDB_API_ERR_NOMEM:                return "ERR_NOMEM";
   case UFDB_API_REQ_QUEUED:               return "REQ_QUEUED";
   case UFDB_API_ERR_TUNNEL:               return "ERR_TUNNEL";
   case UFDB_API_ERR_INVALID_CERT:         return "ERR_INVALID_CERT";
   case UFDB_API_ERR_IP_ADDRESS:           return "ERR_IP_ADDRESS";
   case UFDB_API_ERR_OLD_TABLE:            return "ERR_OLD_TABLE";
   case UFDB_API_ERR_INVALID_TABLE:        return "ERR_INVALID_TABLE";
   case UFDB_API_ERR_INVALID_KEY:          return "ERR_INVALID_KEY";
   case UFDB_API_ERR_IS_SKYPE:             return "ERR_IS_SKYPE";
   case UFDB_API_ERR_FULL:                 return "ERR_FULL";
   case UFDB_API_ERR_UNKNOWN_PROTOCOL:     return "ERR_UNKNOWN_PROTOCOL";
   case UFDB_API_ERR_IS_GTALK:             return "ERR_IS_GTALK";
   case UFDB_API_ERR_IS_YAHOOMSG:          return "ERR_IS_YAHOOMSG";
   case UFDB_API_ERR_IS_AIM:               return "ERR_IS_AIM";
   case UFDB_API_BEING_VERIFIED:           return "BEING_VERIFIED";
   case UFDB_API_MODIFIED_FOR_SAFESEARCH:  return "MODIFIED_FOR_SAFESEARCH";
   case -1:				   return "INTERNAL_ERROR_MINUS_ONE";
   }

   return "INTERNAL_ERROR_UNKNOWN_CODE";
}


char * ufdbDBstat2string( int status )
{
   switch (status)
   {
      case UFDB_API_STATUS_DATABASE_OK:       return "up to date";
      case UFDB_API_STATUS_DATABASE_OLD:      return "one or more tables are more than 4 days old.  Check cron job for ufdbUpdate.";
      case UFDB_API_STATUS_DATABASE_EXPIRED:  return "one or more tables are EXPIRED.  Check licenses and cron job for ufdbUpdate.";
   }
   return "internal-error";
}


char * ufdbStatus2string( int status )
{
   switch (status)
   {
      case UFDB_API_STATUS_VIRGIN:          return "virgin";
      case UFDB_API_STATUS_STARTED_OK:      return "started";
      case UFDB_API_STATUS_TERMINATED:      return "terminated";
      case UFDB_API_STATUS_RELOADING:       return "reloading";
      case UFDB_API_STATUS_RELOAD_OK:       return "reloaded";
      case UFDB_API_STATUS_FATAL_ERROR:     return "error";
      case UFDB_API_STATUS_ROLLING_LOGFILE: return "rolling-logfile";
      case UFDB_API_STATUS_UPDATE: 	    return "status-update";
   }
   return "internal-error";
}


void ufdbSendEmailToAdmin( int newStatus )
{
   int    mx;
   int    n;
   struct timeval      tv;
   char   hostname[256+1];
   char   line[1024];

   if (UFDBglobalEmailServer == NULL  ||  UFDBglobalAdminEmail == NULL)
      return;

   gethostname( hostname, sizeof(hostname) );
   hostname[256] = '\0';

   mx = UFDBopenSocket( UFDBglobalEmailServer, 25 );
   if (mx < 0)
   {
      ufdbLogError( "cannot open connection to mail server \"%\" to inform status change: %s", 
                    UFDBglobalEmailServer, strerror(errno) );
      return;
   }

   /*
    *  Prevent that the read() takes ages.  Use an agressive timeout.
    */
   tv.tv_sec = 4;
   tv.tv_usec = 0;
   setsockopt( mx, SOL_SOCKET, SO_RCVTIMEO, (void *) &tv, sizeof(tv) );
   tv.tv_sec = 4;
   tv.tv_usec = 0;
   setsockopt( mx, SOL_SOCKET, SO_SNDTIMEO, (void *) &tv, sizeof(tv) );

   if (UFDBglobalDebug)
      ufdbLogMessage( "   mail: opened socket 25 to mail server %s", UFDBglobalEmailServer );

   do {
      n = read( mx, line, 1024 );
   } while (n < 0  &&  errno == EINTR);
   if (n < 4  ||  strncmp( line, "220", 3 ) != 0)
   {
      ufdbLogError( "ufdbSendEmailToAdmin: mail server %s did not send 220 welcome message: %s", 
      		    UFDBglobalEmailServer, strerror(errno) );
      close( mx );
      return;
   }
   if (UFDBglobalDebug)
      ufdbLogMessage( "   mail: opened socket 25 to mail server %s", UFDBglobalEmailServer );

   /* The welcome message may be long, so try to read it all.
    * There is a 2 second timeout so no worries if there is no more input.
    */
   tv.tv_sec = 2;
   tv.tv_usec = 0;
   setsockopt( mx, SOL_SOCKET, SO_RCVTIMEO, (void *) &tv, sizeof(tv) );
   do {
      n = read( mx, line, 1024 );
   } while (n < 0  &&  errno == EINTR);

   /*
    * From now on we wait maximum 20 seconds for responses of the mail server
    */
   tv.tv_sec = 20;
   tv.tv_usec = 0;
   setsockopt( mx, SOL_SOCKET, SO_RCVTIMEO, (void *) &tv, sizeof(tv) );
   
   sprintf( line, "HELO %s\r\n", hostname );
   n = write( mx, line, strlen(line) );
   if (n != strlen(line))
   {
      ufdbLogError( "ufdbSendEmailToAdmin: failed to send HELO to mail server %s: %s", 
                    UFDBglobalEmailServer, strerror(errno) );
      close( mx );
      return;
   }
   do {
      n = read( mx, line, 1024 );
   } while (n < 0  &&  errno == EINTR);
   if (n < 1)
   {
      ufdbLogError( "ufdbSendEmailToAdmin: mail server %s did not send reply to HELO: %s", 
      		    UFDBglobalEmailServer, strerror(errno) );
      close( mx );
      return;
   }

   if (UFDBglobalDebug)
      ufdbLogMessage( "   mail: done with HELO handshake to %s", UFDBglobalEmailServer );
   
   sprintf( line, "MAIL FROM:<%s>\r\n", UFDBglobalAdminEmail );
   n = write( mx, line, strlen(line) );
   if (n != strlen(line))
   {
      ufdbLogError( "ufdbSendEmailToAdmin: failed to send MAIL FROM to mail server %s: %s", 
                    UFDBglobalEmailServer, strerror(errno) );
      close( mx );
      return;
   }
   do {
      n = read( mx, line, 1024 );
   } while (n < 0  &&  errno == EINTR);
   if (n < 1)
   {
      ufdbLogError( "ufdbSendEmailToAdmin: mail server %s did not send reply to MAIL FROM: %s", 
      		    UFDBglobalEmailServer, strerror(errno) );
      close( mx );
      return;
   }
   if (strncmp( line, "25", 2 ) != 0)
   {
      char * newline;

      newline = strchr( line, '\r' );
      if (newline == NULL)
	 newline = strchr( line, '\n' );
      if (newline != NULL)
         *newline ='\0';
      ufdbLogError( "ufdbSendEmailToAdmin: mail server %s did not send reply with OK to MAIL FROM:\n%s", 
      		    UFDBglobalEmailServer, line );
      close( mx );
      return;
   }
   
   sprintf( line, "RCPT TO:<%s>\r\n", UFDBglobalAdminEmail );
   n = write( mx, line, strlen(line) );
   if (n != strlen(line))
   {
      ufdbLogError( "ufdbSendEmailToAdmin: failed to send RCPT TO to mail server %s: %s", 
                    UFDBglobalEmailServer, strerror(errno) );
      close( mx );
      return;
   }
   do {
      n = read( mx, line, 1024 );
   } while (n < 0  &&  errno == EINTR);
   if (n < 1)
   {
      ufdbLogError( "ufdbSendEmailToAdmin: mail server %s did not send reply to RCPT TO: %s", 
      		    UFDBglobalEmailServer, strerror(errno) );
      close( mx );
      return;
   }
   if (strncmp( line, "25", 2 ) != 0)
   {
      char * newline;

      newline = strchr( line, '\r' );
      if (newline == NULL)
	 newline = strchr( line, '\n' );
      if (newline != NULL)
         *newline ='\0';
      ufdbLogError( "ufdbSendEmailToAdmin: mail server %s did not send reply with OK to RCPT TO:\n%s", 
      		    UFDBglobalEmailServer, line );
      close( mx );
      return;
   }
   
   if (UFDBglobalDebug)
      ufdbLogMessage( "   mail: sending DATA to %s", UFDBglobalEmailServer );
   
   sprintf( line, "DATA\r\n" );
   n = write( mx, line, strlen(line) );
   if (n != strlen(line))
   {
      ufdbLogError( "ufdbSendEmailToAdmin: failed to send DATA to mail server %s: %s", 
                    UFDBglobalEmailServer, strerror(errno) );
      close( mx );
      return;
   }
   do {
      n = read( mx, line, 1024 );
   } while (n < 0  &&  errno == EINTR);
   if (n < 1)
   {
      ufdbLogError( "ufdbSendEmailToAdmin: mail server %s did not send reply to DATA: %s", 
      		    UFDBglobalEmailServer, strerror(errno) );
      close( mx );
      return;
   }
   if (strncmp( line, "354", 3 ) != 0)
   {
      char * newline;

      newline = strchr( line, '\r' );
      if (newline == NULL)
	 newline = strchr( line, '\n' );
      if (newline != NULL)
         *newline ='\0';
      ufdbLogError( "ufdbSendEmailToAdmin: mail server %s did not send reply with OK to DATA:\n%s", 
      		    UFDBglobalEmailServer, line );
      close( mx );
      return;
   }

   sprintf( line, "From: ufdbGuard daemon <%s>\r\n"
		  "To: URL filter administrator <%s>\r\n"
                  "Subject: ufdbGuard on %s has a new status: %s\r\n"
		  "X-Mailer: ufdbguardd " VERSION
		  "\r\n"
		  "ufdbGuard with pid %d on %s has a new status: %s\n"
		  "database status: %s\n"
		  "license status: %s\n"
		  "configuration file: %s\n"
		  "version: " VERSION "\n"
		  "\r\n.\r\n",
		  UFDBglobalAdminEmail,
		  UFDBglobalAdminEmail,
		  hostname, ufdbStatus2string(newStatus),
		  globalPid, hostname, ufdbStatus2string(newStatus),
		  ufdbDBstat2string(UFDBglobalDatabaseStatus),
		  UFDBglobalLicenseStatus,
		  UFDBglobalConfigFile
	  );
   n = write( mx, line, strlen(line) );
   if (n != strlen(line))
   {
      ufdbLogError( "ufdbSendEmailToAdmin: failed to send CONTENT to mail server %s: %s", 
                    UFDBglobalEmailServer, strerror(errno) );
      close( mx );
      return;
   }
   do {
      n = read( mx, line, 1024 );
   } while (n < 0  &&  errno == EINTR);
   if (n < 1)
   {
      ufdbLogError( "ufdbSendEmailToAdmin: mail server %s did not send reply to DATA END: %s", 
      		    UFDBglobalEmailServer, strerror(errno) );
      close( mx );
      return;
   }
   if (strncmp( line, "25", 2 ) != 0)
   {
      char * newline;

      newline = strchr( line, '\r' );
      if (newline == NULL)
	 newline = strchr( line, '\n' );
      if (newline != NULL)
         *newline ='\0';
      ufdbLogError( "ufdbSendEmailToAdmin: mail server %s did not send reply with OK to DATA END:\n%s", 
      		    UFDBglobalEmailServer, line );
      close( mx );
      return;
   }

   if (UFDBglobalDebug)
      ufdbLogMessage( "   mail: sending QUIT to %s", UFDBglobalEmailServer );
   
   sprintf( line, "QUIT\r\n" );
   n = write( mx, line, strlen(line) );
   if (n != strlen(line))
   {
      ufdbLogError( "ufdbSendEmailToAdmin: failed to send QUIT to mail server %s: %s", 
                    UFDBglobalEmailServer, strerror(errno) );
      close( mx );
      return;
   }
   n = read( mx, line, 1024 );
   /* ignore errors */
   close( mx );

   ufdbLogMessage( "sent email with status update to %s using server %s", UFDBglobalAdminEmail, UFDBglobalEmailServer );
}


static void ufdbExecuteExternalCommand( int status )
{
   pid_t pid;

   if (UFDBglobalExternalStatusCommand == NULL)
      return;

   ufdbLogMessage( "going to execute: %s -s %s -d %s -l %s -v %s",
                   UFDBglobalExternalStatusCommand, 
		   ufdbStatus2string(status), 
		   ufdbDBstat2string(UFDBglobalDatabaseStatus),
		   UFDBglobalLicenseStatus,
		   VERSION  );

   pid = fork();
   if (pid == 0)	/* we are the forked child */
   {
      int    i;
      char * argv[12];

      for (i = 0; i < 2*UFDB_MAX_THREADS + 32;  i++)
         close( i );
      i = 0;
      argv[i++] = UFDBglobalExternalStatusCommand;
      argv[i++] = "-s";
      argv[i++] = ufdbStatus2string( status );
      argv[i++] = "-d";
      argv[i++] = ufdbDBstat2string( UFDBglobalDatabaseStatus );
      argv[i++] = "-l";
      argv[i++] = UFDBglobalLicenseStatus;
      argv[i++] = "-v";
      argv[i++] = VERSION;
      argv[i] = NULL;
      execv( UFDBglobalExternalStatusCommand, argv );
      _exit( 2 );
   }

   if (pid < 0)
   {
      ufdbLogError( "fork failed: cannot execute external status command: %s", strerror(errno) );
      return;
   }

   /* we could do a waitpid() here but there is an other thread that does that */
}


int UFDBchangeStatus( int status )
{
   int    oldStatus;
   char * dbhome;
   FILE * fp;
   char   licStatFileName[1024];

   if (status == UFDBglobalStatus)
      return status;

   /* prevent changing the ERROR state into an OK state */
   if (UFDBglobalStatus == UFDB_API_STATUS_FATAL_ERROR)
   {
      if (status == UFDB_API_STATUS_RELOAD_OK)
         return UFDB_API_STATUS_FATAL_ERROR;
   }

   /* update the license status */
   dbhome = ufdbSettingGetValue( "dbhome" );
   if (dbhome == NULL)
      dbhome = DEFAULT_DBHOME;

   strcpy( licStatFileName, dbhome );
   strcat( licStatFileName, "/" );
   strcat( licStatFileName, UFDB_LICENSE_STATUS_FILENAME );
   fp = fopen( licStatFileName, "r" );
   if (fp != NULL)
   {
      char * newline;
      if (fgets( UFDBglobalLicenseStatus, sizeof(UFDBglobalLicenseStatus)-1, fp ))
         ;
      fclose( fp );
      if ((newline = strchr( UFDBglobalLicenseStatus, '\n' )) != NULL)
         *newline = '\0';
      if (UFDBglobalLicenseStatus[0] == '\0')
	 strcpy( UFDBglobalLicenseStatus, "unknown" );
   }
   else
      strcpy( UFDBglobalLicenseStatus, "unknown" );

#if 0
   ufdbLogMessage( "UFDBchangeStatus %d", status );
#endif

   ufdbSendEmailToAdmin( status );
   ufdbExecuteExternalCommand( status );

   oldStatus = UFDBglobalStatus;
   UFDBglobalStatus = status;

#if 0
   ufdbLogMessage( "UFDBchangeStatus done" );
#endif
   
   return oldStatus;
}

