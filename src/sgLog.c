/*
 * ufdbGuard is copyrighted (C) 2005-2010 by URLfilterDB with all rights reserved.
 *
 * Parts of the ufdbGuard daemon are based on squidGuard.
 * squidGuard is copyrighted (C) 1998 by
 * ElTele Øst AS, Oslo, Norway, with all rights reserved.
 *
 * $Id: sgLog.c,v 1.56 2011/05/20 14:24:52 root Exp root $
 */


/* This module is well tested and stable for a long time AND
 * needs maximum performance, so we remove _FORTIFY_SOURCE.
 */
#undef _FORTIFY_SOURCE

#include "ufdb.h"
#include "ufdblib.h"
#include "sg.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/utsname.h>
#include <fcntl.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>
#include <pthread.h>
#include <syslog.h>

extern int    globalFatalError; 
extern int    globalPid;      		/* from main.c */
extern char * UFDBglobalLogDir;   
extern FILE * globalErrorLog;
extern int    globalDebugTimeDelta;

static pthread_mutex_t log_mutex = UFDB_STATIC_MUTEX_INIT;
static int    globalLogging = 1;

static int    rotationError = 0;
static int    UFDBforceLogRotation = 0;
static char   UFDBlogFilename[1024];
static unsigned long logfilesize = 0;


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
   if (rename( filename, newfile ) != 0)
   {
      /* OOPS, rename did not work. maybe no space or a permission problem
       * we need to do damage control: remove the original file.
       */
      if (unlink( filename ) != 0)
      {
         /* it is getting worse... */
	 if (truncate( filename, 0 ))
	    ;
      }
      if (++rotationError < 5)
	 syslog( LOG_ALERT, "ufdbguardd cannot rotate its log files !!  Check permissions and space." );
   }
}


void ufdbGlobalSetLogging( int logging )
{
   globalLogging = logging;
}


void UFDBrotateLogfile( void )
{
   UFDBforceLogRotation = 1;
}


void ufdbSetGlobalErrorLogFile( void )
{
   struct stat s;

   if (!globalLogging)
      return;

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
	 fprintf( stderr, "%s: can't write to logfile %s; %s", progname, UFDBlogFilename, strerror(errno) );
   }

   if (globalErrorLog == NULL)
      logfilesize = 1;
   else
      logfilesize = ftell( globalErrorLog );
}


void niso( 
   time_t    t, 
   char *    buf )
{
   struct tm lc;

   if (t == 0)
      t = time(NULL) + globalDebugTimeDelta;
   localtime_r( &t, &lc );
   sprintf( buf, "%04d-%02d-%02d %02d:%02d:%02d", 
            lc.tm_year + 1900, lc.tm_mon + 1, lc.tm_mday, lc.tm_hour, lc.tm_min, lc.tm_sec );
}


static void ufdbLog( 
   char *    msg  )
{
   char *    nl;
   int       multiline;
   char      date[22];
   char      logmsg[UFDB_MAX_URL_LENGTH+128];
   unsigned long  logmsglen;
#ifdef UFDB_DEBUG_LOGGING
   int       newlog = 0;
#endif

   if (!globalLogging)
      return;

   niso( 0, date );

   multiline = 0;
   logmsg[0] = '\0';
   logmsglen = 0;
   while ((nl = strchr( msg, '\n' )) != NULL)
   {
      *nl = '\0';
      logmsglen += (unsigned long) sprintf( logmsg+logmsglen, "%s [%d] %s%s\n", date, globalPid, multiline?"   ":"", msg );
      msg = nl + 1;
      multiline = 1;
   }
   logmsglen += (unsigned long) sprintf( logmsg+logmsglen, "%s [%d] %s%s\n", date, globalPid, multiline?"   ":"", msg );

   pthread_mutex_lock( &log_mutex );			/* LOG_MUTEX *******************/

   fputs( logmsg, stderr );
   fflush( stderr );

   pthread_mutex_unlock( &log_mutex );			/* LOG_MUTEX *******************/

#ifdef UFDB_DEBUG_LOGGING
   if (newlog)
      ufdbLogMessage( "logfilesize = %lu", logfilesize );	/* recursive! */
#endif
}


void ufdbLogError( char * format, ... )
{
   va_list ap;
   char    msg[UFDB_MAX_URL_LENGTH];

   if (!globalLogging)
      return;

   va_start( ap, format );
   if (vsnprintf( msg, UFDB_MAX_URL_LENGTH-32, format, ap ) > UFDB_MAX_URL_LENGTH-34)
   {
      msg[UFDB_MAX_URL_LENGTH-34] = ' ';
      msg[UFDB_MAX_URL_LENGTH-33] = '.';
      msg[UFDB_MAX_URL_LENGTH-32] = '.';
      msg[UFDB_MAX_URL_LENGTH-31] = '.';
      msg[UFDB_MAX_URL_LENGTH-30] = '\0';
   }
   va_end( ap );

   ufdbLog( msg );
}


void ufdbLogMessage( char * format, ... )
{
   va_list ap;
   char    msg[UFDB_MAX_URL_LENGTH];

   if (!globalLogging)
      return;

   va_start( ap, format );
   if (vsnprintf( msg, UFDB_MAX_URL_LENGTH-32, format, ap ) > UFDB_MAX_URL_LENGTH-34)
   {
      msg[UFDB_MAX_URL_LENGTH-34] = ' ';
      msg[UFDB_MAX_URL_LENGTH-33] = '.';
      msg[UFDB_MAX_URL_LENGTH-32] = '.';
      msg[UFDB_MAX_URL_LENGTH-31] = '.';
      msg[UFDB_MAX_URL_LENGTH-30] = '\0';
   }
   va_end( ap );

   ufdbLog( msg );
}


void ufdbLogFatalError( char * format, ... )
{
   va_list ap;
   char    msg[UFDB_MAX_URL_LENGTH];
   char    logmsg[UFDB_MAX_URL_LENGTH];

   va_start( ap, format );
   if (vsnprintf( msg, UFDB_MAX_URL_LENGTH-48, format, ap ) > UFDB_MAX_URL_LENGTH-50)
   {
      msg[UFDB_MAX_URL_LENGTH-50] = ' ';
      msg[UFDB_MAX_URL_LENGTH-49] = '.';
      msg[UFDB_MAX_URL_LENGTH-48] = '.';
      msg[UFDB_MAX_URL_LENGTH-47] = '.';
      msg[UFDB_MAX_URL_LENGTH-46] = '\0';
   }
   va_end( ap );

   sprintf( logmsg, "*FATAL* %s  *****", msg );
   ufdbLog( logmsg );

   if (fileno(stderr) >= 0)
      fprintf( stderr, "%s\n", logmsg );

   UFDBglobalFatalError = 1;
}


char * ip2str( 
   char * buffer, 
   int    i )
{
   sprintf( buffer, "%d.%d.%d.%d",  
   	    (i>>24)&0xff, (i>>16)&0xff, (i>>8)&0xff, (i)&0xff  );
   return buffer;
}


int mask2cidr( unsigned long mask )
{
   int cidr;

   cidr = 32;
   while (cidr > 0)
   {
      if (mask & 1)
         break;
      mask >>= 1;
      cidr--;
   }

   return cidr;
}


void UFDBlogIP( struct Ip * ip )
{
   char        ipbuffer[16];
   char        ipbuffer2[16];
   struct Ip * nextip;

   if (ip == NULL)
      return;

   for (;  ip != NULL;  ip = ip->next)
   {
      switch (ip->type) 
      {
      case SG_IPTYPE_HOST:
	 ufdbLogMessage( "   ip %d.%d.%d.%d", 
	    	(ip->net>>24)&0xff, (ip->net>>16)&0xff, (ip->net>>8)&0xff, (ip->net)&0xff );
         break;
      case SG_IPTYPE_RANGE:
	 nextip = ip->next;
	 (void) ip2str( ipbuffer, ip->net );
	 ufdbLogMessage( "   ip %s - %s", ipbuffer, ip2str(ipbuffer2,ip->mask) );
         break;
      case SG_IPTYPE_CIDR:
	 nextip = ip->next;
	 ufdbLogMessage( "   ip %d.%d.%d.%d/%d",    
	        (ip->net>>24)&0xff, (ip->net>>16)&0xff, (ip->net>>8)&0xff, (ip->net)&0xff,
		mask2cidr(ip->mask) );
	 break;
      case SG_IPTYPE_CLASS:
	 nextip = ip->next;
	 ufdbLogMessage( "   ip %d.%d.%d.%d/%s", 
	    	(ip->net>>24)&0xff, (ip->net>>16)&0xff, (ip->net>>8)&0xff, (ip->net)&0xff, 
		ip2str(ipbuffer2,ip->mask) );
         break;
      default:
	 break;
      }
   }
}


void UFDBlogDB( 
   char *        text, 
   struct sgDb * db )
{
   struct UFDBmemDB * memdb = (struct UFDBmemDB *) db;

   while (memdb != NULL)
   {
      char * key;
      key = memdb->key;
      if (key == NULL)
         key = "NULL";
      ufdbLogMessage( "   %s \"%s\"", text, key );
      memdb = memdb->next;
   }
}


void UFDBlogACL( struct Acl * acl )
{
   struct AclDest * acldest;
   char             lbuf[4096];

   if (acl->within == UFDB_ACL_ELSE)
      ufdbLogMessage( "   else {" );
   else
   {
      lbuf[0] = '\0';
      if (acl->within == UFDB_ACL_WITHIN)
	 sprintf( lbuf, "within \"%s\" ", acl->time->name );
      else if (acl->within == UFDB_ACL_OUTSIDE)
	 sprintf( lbuf, "outside \"%s\" ", acl->time->name );
      ufdbLogMessage( "   \"%s\" %s{", acl->name, lbuf );
   }

   lbuf[0] = '\0';
   for (acldest = acl->pass;  acldest != NULL;  acldest = acldest->next)
   {
      switch (acldest->type)
      {
      case ACL_TYPE_TERMINATOR:
	 if (acldest->access)
	    strcat( lbuf, " all" );
	 else
	    strcat( lbuf, " none" );
         break;
      case ACL_TYPE_DEFAULT:
	 strcat( lbuf, " " );
	 if (!acldest->access)
	    strcat( lbuf, "!" );
	 strcat( lbuf, acldest->name );
         break;
      case ACL_TYPE_INADDR:
	 strcat( lbuf, " in-addr" );
         break;
      }
   }
   ufdbLogMessage( "      pass %s", lbuf );
   ufdbLogMessage( "   }" );
}


static char * code2tunnelcheck( 
   int code )
{
   switch (code) 
   {
      case UFDB_API_HTTPS_CHECK_OFF:   	        return "off";
      case UFDB_API_HTTPS_CHECK_QUEUE_CHECKS:   return "queue-checks";
      case UFDB_API_HTTPS_CHECK_AGGRESSIVE:     return "aggressive";
      case UFDB_API_HTTPS_LOG_ONLY:             return "log-only";
   }
   return "unknown";
}


static char * t2hhmm( int t0 )
{
   static char hhmm[8];

   sprintf( hhmm, "%02d:%02d", t0 / 60, t0 % 60 );
   return hhmm;
}


void UFDBlogConfig( void )
{
   struct Category *     cat;
   struct Source *       src;
   struct Acl *          acl;
   struct Time *         t;
   struct TimeElement *  te;
   char *                param;
   struct utsname        sysinfo;

   ufdbGetSysInfo( &sysinfo );
   ufdbLogMessage( "configuration on host %s:", sysinfo.nodename );

   param = ufdbSettingGetValue( "dbhome" );
   if (param == NULL)
      ufdbLogMessage( "# default: dbhome %s", DEFAULT_DBHOME );
   else
      ufdbLogMessage( "dbhome \"%s\"", param );

   param = ufdbSettingGetValue( "logdir" );
   if (param == NULL)
      ufdbLogMessage( "# default: logdir %s", DEFAULT_LOGDIR );
   else
      ufdbLogMessage( "logdir \"%s\"", param );

   ufdbLogMessage( "port %d", UFDBglobalPortNum );
#if HAVE_UNIX_SOCKETS
   ufdbLogMessage( "# interface \"%s\"", UFDBglobalInterface );
#else
   ufdbLogMessage( "interface \"%s\"", UFDBglobalInterface );
#endif
   
   if (UFDBglobalEmailServer == NULL)
      ufdbLogMessage( "# default: email-server \"none\"" );
   else
      ufdbLogMessage( "email-server \"%s\"", UFDBglobalEmailServer );

   if (UFDBglobalAdminEmail == NULL)
      ufdbLogMessage( "# default: admin-email \"none\"" );
   else
      ufdbLogMessage( "admin-email \"%s\"", UFDBglobalAdminEmail );

   if (UFDBglobalExternalStatusCommand == NULL)
      ufdbLogMessage( "# default: external-status-command \"none\"" );
   else
      ufdbLogMessage( "external-status-command \"%s\"", UFDBglobalExternalStatusCommand );

   ufdbLogMessage( "logblock %s", UFDBglobalLogBlock ? "on" : "off" );
   if (UFDBglobalShowURLdetails)
      ufdbLogMessage( "ufdb-show-url-details on" );
   ufdbLogMessage( "logall %s", UFDBglobalLogAllRequests ? "on" : "off" );
   ufdbLogMessage( "ufdb-debug-filter %s", UFDBglobalDebug ? "on" : "off" );
   ufdbLogMessage( "ufdb-expression-debug %s", UFDBglobalDebugRegexp ? "on" : "off" );
   ufdbLogMessage( "max-logfile-size %lu", UFDBglobalMaxLogfileSize );
   if (UFDBglobalAnalyseUncategorisedURLs < 0)
      ufdbLogMessage( "# analyse-uncategorised-urls is overruled by -N option" );
   else
      ufdbLogMessage( "analyse-uncategorised-urls %s", UFDBglobalAnalyseUncategorisedURLs ? "on" : "off" );
   if (!UFDBglobalUploadStats || UFDBglobalDebug)
      ufdbLogMessage( "upload-stats %s", UFDBglobalUploadStats ? "on" : "off" );
   ufdbLogMessage( "check-proxy-tunnels %s", code2tunnelcheck(UFDBglobalTunnelCheckMethod) );
   ufdbLogMessage( "safe-search %s", UFDBglobalSafeSearch ? "on" : "off" );
   if (UFDBglobalHttpdPort <= 0)
      ufdbLogMessage( "# no http-server defined" );
   else
      ufdbLogMessage( "http-server { port = %d, interface = %s, images = \"%s\" }", 
                      UFDBglobalHttpdPort, UFDBglobalHttpdInterface, UFDBglobalHttpdImagesDirectory );

   ufdbLogMessage( "url-lookup-result-during-database-reload %s", 
                   UFDBglobalURLlookupResultDBreload==UFDB_ALLOW ? "allow" : "deny" );
   ufdbLogMessage( "redirect-loading-database \"%s\"", UFDBglobalLoadingDatabaseRedirect );
   ufdbLogMessage( "url-lookup-result-when-fatal-error %s", 
                   UFDBglobalURLlookupResultFatalError==UFDB_ALLOW ? "allow" : "deny" );
   ufdbLogMessage( "redirect-fatal-error \"%s\"", UFDBglobalFatalErrorRedirect );

   ufdbLogMessage( "" );

   for (t = Time;  t != NULL;  t = t->next)
   {
      ufdbLogMessage( "time \"%s\" {", t->name );
      for (te = t->element;  te != NULL;  te = te->next)
      {
	 char line[128];
	 line[0] = '\0';

	 if (te->wday)
	 {
	    if ((te->wday & 0x7F) == 0x7F) 
	       strcat( line, "*   " );
	    else
	    {
	       if (te->wday & 0x01) strcat( line, "sun " );
	       if (te->wday & 0x02) strcat( line, "mon " );
	       if (te->wday & 0x04) strcat( line, "tue " );
	       if (te->wday & 0x08) strcat( line, "wed " );
	       if (te->wday & 0x10) strcat( line, "thu " );
	       if (te->wday & 0x20) strcat( line, "fri " );
	       if (te->wday & 0x40) strcat( line, "sat " );
	    }
	    strcat( line, "  " );
	    strcat( line, t2hhmm(te->from) );
	    strcat( line, " - " );
	    strcat( line, t2hhmm(te->to) );
	    ufdbLogMessage( "   weekly  %s", line );
	 }
	 else if (te->fromdate)
	 {
	    char from_str[24];
	    char to_str[24];

	    niso( te->fromdate, from_str );
	    niso( te->todate, to_str );
	    ufdbLogMessage( "   date    %s  -  %s", from_str, to_str );
	 }
	 else /* y m d */
	 {
	    char nb[8];

	    if (te->y < 0)   
	       strcat( line, "*" );
	    else
	       strcat( line, (sprintf(nb,"%d",te->y),nb) );

	    strcat( line, "-" );

	    if (te->m < 0)   
	       strcat( line, "*" );
	    else
	       strcat( line, (sprintf(nb,"%02d",te->m),nb) );

	    strcat( line, "-" );

	    if (te->d < 0)   
	       strcat( line, "*" );
	    else
	       strcat( line, (sprintf(nb,"%02d",te->d),nb) );

	    strcat( line, "   " );

	    strcat( line, t2hhmm(te->from) );
	    strcat( line, " - " );
	    strcat( line, t2hhmm(te->to) );

	    ufdbLogMessage( "   date    %s", line );
	 }
      }
      ufdbLogMessage( "}" );
   }

   ufdbLogMessage( "" );

   for (cat = Dest;  cat != NULL;  cat = cat->next)
   {
      ufdbLogMessage( "category \"%s\" {", cat->name );
      if (cat->domainlist != NULL)
	 ufdbLogMessage( "   domainlist     \"%s\"", cat->domainlist );
      if (cat->expressionlist != NULL)
	 ufdbLogMessage( "   expressionlist \"%s\"", cat->expressionlist );
      if (strcmp( cat->name, "security" ) == 0  &&  cat->cse != NULL)
	 ufdbLogMessage( "   cacerts        \"%s\"", cat->cse );
      if (cat->redirect != NULL)
	 ufdbLogMessage( "   redirect       \"%s\"", cat->redirect );
      if (cat->options != 0)
      {
	 if (cat->options != UFDB_OPT_SAFE_SEARCH)
	 {
	    ufdbLogMessage( "   option         enforce-https-with-hostname %s", cat->options & UFDB_OPT_HTTPS_WITH_HOSTNAME ? "on" : "off" );
	    ufdbLogMessage( "   option         enforce-https-official-certificate %s", cat->options & UFDB_OPT_HTTPS_OFFICAL_CERTIFICATE ? "on" : "off" );
	    ufdbLogMessage( "   option         allow-skype-over-https %s", cat->options & UFDB_OPT_SKYPE_OVER_HTTPS ? "on" : "off" );
	    ufdbLogMessage( "   option         allow-gtalk-over-https %s", cat->options & UFDB_OPT_GTALK_OVER_HTTPS ? "on" : "off" );
	    ufdbLogMessage( "   option         allow-yahoomsg-over-https %s", cat->options & UFDB_OPT_YAHOOMSG_OVER_HTTPS ? "on" : "off" );
	    ufdbLogMessage( "   option         allow-aim-over-https %s", cat->options & UFDB_OPT_AIM_OVER_HTTPS ? "on" : "off" );
	    ufdbLogMessage( "   option         allow-unknown-protocol-over-https %s", cat->options & UFDB_OPT_UNKNOWN_PROTOCOL_OVER_HTTPS ? "on" : "off" );
	    ufdbLogMessage( "   option         https-prohibit-insecure-sslv2 %s", cat->options & UFDB_OPT_PROHIBIT_INSECURE_SSLV2 ? "on" : "off" );
	 }
         if (cat->options & UFDB_OPT_SAFE_SEARCH)
	    ufdbLogMessage( "   option         safe-search on" );
      }
      ufdbLogMessage( "}" );
   }

   ufdbLogMessage( "" );

   for (src = Source;  src != NULL;  src = src->next)
   {
      ufdbLogMessage( "source \"%s\" {", src->name );
      UFDBlogIP( src->ip );
      UFDBlogDB( "domain", src->domainDb );
      UFDBlogDB( "user", src->userDb );
      if (src->time != NULL)
	 ufdbLogMessage( "   time %s", "defined" );
      ufdbLogMessage( "}" );
   }

   ufdbLogMessage( "" );

   ufdbLogMessage( "acl {" );
   for (acl = Acl;  acl != NULL;  acl = acl->next)
   {
      UFDBlogACL( acl );
   }
   ufdbLogMessage( "}" );

   ufdbLogMessage( "" );

   ufdbLogMessage( "database status: %s", ufdbDBstat2string(UFDBglobalDatabaseStatus) );
   ufdbLogMessage( "license status: %s", UFDBglobalLicenseStatus );

   ufdbLogMessage( "" );
}


void UFDBcloseFilesNoLog( void )
{
   int i;
   int logfile;

   if (globalErrorLog == NULL)
      logfile = fileno( stderr );
   else
      logfile = fileno( globalErrorLog );

   for (i = 0;  i < 2*UFDB_MAX_THREADS + 32;  i++)
   {
      if (i != logfile)
	 (void) close( i );
   }
}

