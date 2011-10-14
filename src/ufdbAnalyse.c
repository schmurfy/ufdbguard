/*
 * ufdbanalyse.c - URLfilterDB
 *
 * ufdbGuard is copyrighted (C) 2005-2010 by URLfilterDB with all rights reserved.
 *
 * Parts of the ufdbGuard daemon are based on squidGuard.
 * This module is NOT based on squidGuard.
 *
 * read a Squid log file and produce a report with a table showing
 * percentages for categories.
 *
 * This program is meant to be used as a tool to find out what types
 * of websites are visited.
 *
 * RCS: $Id: ufdbAnalyse.c,v 1.19 2011/06/09 02:29:12 root Exp root $
 */

#include "ufdb.h"
#include "ufdblib.h"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>


#if 0
void ufdbFree( void * ptr )
{
   if (ptr != NULL)
      free( ptr );
}

void * ufdbMalloc( size_t n )
{
   char * mem;
   
   mem = malloc( n );
   return mem;
}

void * ufdbRealloc( void * ptr, size_t n )
{
   char * mem;
   
   mem = realloc( ptr, n );
   return mem;
}

void * ufdbCalloc( size_t n, size_t num )
{
   char * mem;

   mem = calloc( n, num );
   return mem;
}

void ufdbGetMallocMutex( char * fn )
{
}

void ufdbReleaseMallocMutex( char * fn )
{
}

char * ufdbStrdup( const char * s )
{
   int size = strlen( s ) + 1;
   return strcpy( ufdbMalloc(size), s );
}
#endif

void ufdbLogMessage( char * line, ... )
{
   fprintf( stderr, "%s\n:", line );
}


void ufdbLogError( char * line, ... )
{
   fprintf( stderr, "%s\n:", line );
}


void ufdbLogFatalError( char * line, ... )
{
   fprintf( stderr, "%s\n:", line );
}


void ufdbSetGlobalErrorLogFile( void )
{
}


int pthread_mutex_lock( void * mutex )
{
   return 0;
}

int pthread_mutex_unlock( void * mutex )
{
   return 0;
}


static void usage( void )
{
   fprintf( stderr, "usage: ufdbAnalyse [-a] -l <squid-log-file> -d <domainname> -e <email-address> -n <full-name>\n" );
   fprintf( stderr, "flags: -a  logfile has format of Apache instead of Squid\n" );
   fprintf( stderr, "       -l  Squid log file (parameter in squid.conf: cache_access_log)\n" );
   fprintf( stderr, "       -d  your domainname, e.g. example.com\n" );
   fprintf( stderr, "       -e  your email address, e.g. joe@example.com\n" );
   fprintf( stderr, "       -n  your full name\n" );
   exit( 1 );
}


static void * myrealloc( void * buffer, size_t newsize )
{
#define MY_CHUNK (64*1024)

   return realloc( buffer, newsize + (MY_CHUNK - (newsize+MY_CHUNK) % MY_CHUNK) );
}


static int parseApacheLog( char * line, char ** code, char ** nbytes, char ** url )
{
   char * p;
   char * slash;
   char * u;

   /* a line in a Apache log file looks like this:
    * 203.199.204.12 - - [30/Nov/2007:11:27:21 +0100]  "GET /smallcross.gif HTTP/1.0"  200  61
    *                    "http://74.86.20.90/livescore/data/crbz-31739122ndtv06/2007/2007_IND_PAK/IND_PAK_NOV30_DEC04/gen_scag.html" 
    *                    "Mozilla/4.0 (compatible; MSIE 6.0; Windows NT 5.1; SV1; FDM; InfoPath.1; .NET CLR 2.0.50727)"
    */

   p = strchr( line, '"' );
   if (p == NULL)
      return 0;

   p = strtok( p, " \t" );		/* HTTP command */
   if (p == NULL)
      return 0;

   u = strtok( NULL, " \t" );		/* URL */
   if (u == NULL)
      return 0;

   /* strip the URL for privacy; remove username/password and query terms */
   p = strchr( u, '?' );
   if (p != NULL)
      *p = '\0';
   p = strchr( u, ';' );
   if (p != NULL)
      *p = '\0';
   p = strchr( u, '&' );
   if (p != NULL)
      *p = '\0';
   p = strstr( u, "://" );
   if (p != NULL)
      u = p + 3;
   p = strchr( u, '@' );
   if (p != NULL)
   {
      slash = strchr( u, '/' );
      if (slash == NULL  ||  p < slash)
	 u = p + 1;
   }
   *url = u;

   p = strtok( p, " \t" );		/* protocol */
   if (p == NULL)
      return 0;

   *code = strtok( p, " \t" );		/* HTTP return code */
   if (*code == NULL)
      return 0;

   /* skip the rest of the line */

   return 1;
}


static int parseSquidLog( char * line, char ** code, char ** nbytes, char ** url )
{
   char * p;
   char * slash;
   char * u;

   /* a line in a Squid log file looks like this:
    * 1195621581.452     74 10.1.1.2 TCP_MISS/200 1169 GET http://example.com - DIRECT/194.46.8.130 text/html
    */
   if (strtok( line, " \t" ) == NULL) 	/* time */
      return 0;

   if (strtok( NULL, " \t" ) == NULL)	/* dummy */
      return 0;

   if (strtok( NULL, " \t" ) == NULL)	/* IP */
      return 0;

   *code = strtok( NULL, " \t" );		/* Squid code / HTTP code */
   if (*code == NULL)
      return 0;
   p = strchr( *code, '/' );
   if (p != NULL)
      *code = p + 1;

   *nbytes = strtok( NULL, " \t" );		/* #bytes */
   if (*nbytes == NULL)
      return 0;

   if (strtok( NULL, " \t" ) == NULL)	/* HTTP command */
      return 0;

   u = strtok( NULL, " \t" );		/* URL */
   if (u == NULL)
      return 0;
   /* ignore the rest of the input line */

   /* strip the URL for privacy; remove username/password and query terms */
   p = strchr( u, '?' );
   if (p != NULL)
      *p = '\0';
   p = strchr( u, '&' );
   if (p != NULL)
      *p = '\0';
   p = strstr( u, "://" );
   if (p != NULL)
      u = p + 3;
   p = strchr( u, '@' );
   if (p != NULL)
   {
      slash = strchr( u, '/' );
      if (slash == NULL  ||  p < slash)
	 u = p + 1;
   }
   *url = u;

   return 1;
}


static void sendLogfile( int serverSocket, FILE * logfile, char * email, int apachelog )
{
   int    errors;
   int    n;
   long   nbytes_sent;
   int    nlines_sent;
   char * code;
   char * nbytes;
   char * url;
   char * buffer;
   char * oldbuffer;
   char   linebuf[4192];
   char   output[4192];

   errors = 0;
   nbytes = 0;
   nbytes_sent = 0;
   nlines_sent = 0;
   buffer = malloc( MY_CHUNK );

   code = NULL;
   url = NULL;
   while (fgets( linebuf, 4192, logfile ) != NULL)
   {
      linebuf[4191] = '\0';

      if (apachelog)
         n = parseApacheLog( linebuf, &code, &nbytes, &url );
      else
         n = parseSquidLog( linebuf, &code, &nbytes, &url );
      if (n == 0)
      {
         errors++;
	 continue;
      }

      n = sprintf( output, "%s %s %s\n", code, nbytes, url );
      /* we assume that realloc is really fast... */
      oldbuffer = buffer;
      buffer = myrealloc( buffer, nbytes_sent+n+1 );
      if (buffer == NULL)
      {
         fprintf( stderr, "Oops: cannot allocate more than %ld KB... truncating input to %d lines...\n", 
	          nbytes_sent/1024, nlines_sent );
	 buffer = oldbuffer;
	 break;
      }
      nlines_sent++;
      if (nlines_sent % 20000 == 0)		/* send maximum 20000 lines */
      {
         putchar( '.' );
	 fflush( stdout );
      }
      strcpy( &buffer[nbytes_sent], output );
      nbytes_sent += (long) n;
      if (nbytes_sent > 100 * 1024 * 1024L)	/* send maximum 100 MB */
      {
         fprintf( stderr, "\n" );
         fprintf( stderr, "Over 100 MB (%d URLs) will be sent to be analysed.\n", nlines_sent );
         fprintf( stderr, "To save bandwidth and processing power, the rest of the input file is ignored.\n" );
	 break;
      }
   }
   printf( "\n" );

   if (errors > 2)
   {
      fprintf( stderr, "There were %d parse errors.\n", errors );
      fprintf( stderr, "Are you sure that a Squid log file (e.g. access.log) is specified ?\n" );
      fprintf( stderr, "No analysis can be performed.\n" );
      strcpy( output, "Content-Length: 0\r\n\r\n" );
      if (write( serverSocket, output, strlen(output) ))
         ;
      return;
   }

   if (nlines_sent < 50)
   {
      fprintf( stderr, "The Squid logfile has only %d URLs which is not sufficient for an analysis.\n", nlines_sent );
      fprintf( stderr, "No analysis can be performed.\n" );
      strcpy( output, "Content-Length: 0\r\n\r\n" );
      if (write( serverSocket, output, strlen(output) ))
         ;
      return;
   }

   /* send the last line of the header: content length and then the content */
   n = sprintf( output, "Content-Length: %ld\r\n"
                    "\r\n",
		    nbytes_sent );

   if (write( serverSocket, output, n ) != n)
   {
      fprintf( stderr, "\n" );
      fprintf( stderr, "cannot write Content-Length to server: %s\n", strerror(errno) );
      return;
   }

   printf( "I am going to upload %d URLs (%d KB) ...\n", nlines_sent, (int) (nbytes_sent/1024) );

   errno = 0;
   if (write( serverSocket, buffer, nbytes_sent ) != nbytes_sent)
   {
      fprintf( stderr, "cannot write to server: %s\n", strerror(errno) );
      return;
   }

   printf( "The upload is finished.\n" );
}


static char * TLDs[] = 
{
   "ac", "ad", "ae", "aero", "af", "ag", "ai", "al",
   "am", "an", "ao", "aq", "ar", "arpa", "as", "asia",
   "at", "au", "aw", "ax", "az", "ba", "bb", "bd",
   "be", "bf", "bg", "bh", "bi", "biz", "bj", "bm",
   "bn", "bo", "br", "bs", "bt", "bv", "bw", "by",
   "bz", "ca", "cat", "cc", "cd", "cf", "cg", "ch",
   "ci", "ck", "cl", "cm", "cn", "co", "com", "coop",
   "cr", "cu", "cv", "cx", "cy", "cz", "de", "dj",
   "dk", "dm", "do", "dz", "ec", "edu", "ee", "eg",
   "er", "es", "et", "eu", "fi", "fj", "fk", "fm",
   "fo", "fr", "ga", "gb", "gd", "ge", "gf", "gg",
   "gh", "gi", "gl", "gm", "gn", "gov", "gp", "gq",
   "gr", "gs", "gt", "gu", "gw", "gy", "hk", "hm",
   "hn", "hr", "ht", "hu", "id", "ie", "il", "im",
   "in", "info", "int", "io", "iq", "ir", "is", "it",
   "je", "jm", "jo", "jobs", "jp", "ke", "kg", "kh",
   "ki", "km", "kn", "kp", "kr", "kw", "ky", "kz",
   "la", "lb", "lc", "li", "lk", "lr", "ls", "lt",
   "lu", "lv", "ly", "ma", "mc", "md", "me", "mg",
   "mh", "mil", "mk", "ml", "mm", "mn", "mo", "mobi",
   "mp", "mq", "mr", "ms", "mt", "mu", "museum", "mv",
   "mw", "mx", "my", "mz", "na", "name", "nc", "ne",
   "net", "nf", "ng", "ni", "nl", "no", "np", "nr",
   "nu", "nz", "om", "org", "pa", "pe", "pf", "pg",
   "ph", "pk", "pl", "pm", "pn", "pr", "pro", "ps",
   "pt", "pw", "py", "qa", "re", "ro", "rs", "ru",
   "rw", "sa", "sb", "sc", "sd", "se", "sg", "sh",
   "si", "sj", "sk", "sl", "sm", "sn", "so", "sr",
   "st", "su", "sv", "sy", "sz", "tc", "td", "tel",
   "tf", "tg", "th", "tj", "tk", "tl", "tm", "tn",
   "to", "tp", "tr", "travel", "tt", "tv", "tw", "tz",
   "ua", "ug", "uk", "us", "uy", "uz", "va", "vc",
   "ve", "vg", "vi", "vn", "vu", "wf", "ws", "xxx", "ye",
   "yt", "za", "zm", "zw", NULL
};

static int validTLD( 
   char * line )
{
   char * tld;
   char ** t;

   tld = strrchr( line, '.' );
   if (tld == NULL)
      return 0;

   tld++;
   for (t = TLDs; *t != NULL; t++)
   {
      if (strcasecmp( *t, tld ) == 0)
         return 1;
   }
   return 0;
}


int main( int argc, char * argv[] )
{
   char   opt;
   int    n;
   int    s;
   FILE * fp;
   int    apachelog;
   char   answer[32];
   char   domain[128];
   char   email[128];
   char   fullname[128];
   char   logfile[1024];
   char   header[2048];

   logfile[0] = email[0] = domain[0] = fullname[0] = '\0';
   apachelog = 0;
   while ((opt = getopt( argc, argv, "?al:d:e:n:" )) > 0)
   {
      switch (opt)
      {
      case 'a':
      	 apachelog = 1;
	 break;
      case 'd':
         strcpy( domain, optarg );
	 break;
      case 'e':
         strcpy( email, optarg );
	 break;
      case 'n':
         strcpy( fullname, optarg );
	 break;
      case 'l':
         strcpy( logfile, optarg );
	 break;
      case '?':
      default:
	 usage();
	 break;
      }
   }

   if (fullname[0] == '\0' || logfile[0] == '\0' || email[0] == '\0' || domain[0] == '\0')
      usage();

   if (strchr( email, '@' ) == NULL  ||
       strchr( email, '.' ) == NULL  ||
       strstr( email, "example.com" ) != NULL  ||
       !validTLD(email))
   {
      fprintf( stderr, "The report is emailed to you so you must supply a correct and valid email address\n" );
      fprintf( stderr, "No data is uploaded.  No report will be emailed.\n" );
      exit( 1 );
   }

   if (!isatty(fileno(stdin)))
   {
      fprintf( stderr, "ufdbAnalyse requires that standard input is a tty.\n" );
      fprintf( stderr, "Do not use pipes or file redirection.\n" );
      exit( 1 );
   }

   fp = fopen( logfile, "r" );
   if (fp == NULL)
   {
      fprintf( stderr, "cannot read Squid logfile \"%s\"\n", logfile );
      exit( 1 );
   }

   printf( "Please do not upload more than 1 file and wait for the results.\n" );
   printf( "The results will be sent by the support desk via email to %s\n", email );
   printf( "Type \"yes\" to analyse %s ", logfile );
   answer[0] = '\0';
   if (fgets( answer, 30, stdin ) == NULL)
      exit( 1 );
   if (strncmp( answer, "yes", 3 ) != 0)
   {
      fprintf( stderr, "Answer \"yes\" to upload the file.  The upload is aborted.\n" );
      exit( 0 );
   }

   s = UFDBopenSocket( UFDB_UPLOAD_UNCATEGORISED_URLS_WEBSITE, 80 );
   if (s < 0)
   {
      fprintf( stderr, "cannot open communication socket with http://%s  %s\n", 
               UFDB_UPLOAD_UNCATEGORISED_URLS_WEBSITE, strerror(errno) );
      exit( 1 );
   }

   sprintf( header, "POST " UFDB_UPLOAD_ANALYSE_SQUID_LOG_CGI " HTTP/1.1\r\n"
		    "Host: " UFDB_UPLOAD_UNCATEGORISED_URLS_WEBSITE "\r\n"
   	 	    "User-Agent: ufdbAnalyse-" VERSION "\r\n"
		    "Content-Type: text/plain\r\n"
		    "Connection: close\r\n"
		    "X-mydomain: %s\r\n"
		    "X-myemail: %s\r\n"
		    "X-fullname: %s\r\n",
		    domain, email, fullname );
   n = strlen( header );
   if (write( s, header, n  ) != n )
   {
      fprintf( stderr, "cannot write header to http://%s\n", UFDB_UPLOAD_UNCATEGORISED_URLS_WEBSITE );
      exit( 1 );
   }

#if 0
   printf( "header sent to %s\n", UFDB_UPLOAD_UNCATEGORISED_URLS_WEBSITE );
   printf( "%s", header );
#endif

   /* TODO: also send local always-allow and always-block categories */
   sendLogfile( s, fp, email, apachelog );

   close( s );
   fclose( fp );

   exit( 0 );
   /*NOTREACHED*/
   return 0;
}

