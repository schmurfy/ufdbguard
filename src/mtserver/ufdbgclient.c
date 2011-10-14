/* 
 * ufdbgclient.c - URLfilterDB
 *
 * ufdbGuard is copyrighted (C) 2005-2011 by URLfilterDB with all rights reserved.
 *
 * Parts of the ufdbGuard daemon are based on squidGuard.
 * This module is NOT based on squidGuard.
 *
 * Light-weight process that connects to the ufdbguardd daemon to perform a URL verification.
 *
 * $Id: ufdbgclient.c,v 1.40 2011/05/04 21:46:39 root Exp root $
 */


#include "sg.h"

#include <stdio.h>
#include <time.h>
#include <libgen.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <netdb.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#if HAVE_UNIX_SOCKETS
#include <sys/un.h>
#endif

extern int    globalPid;
static int    debugRedirect;

int    noRedirects = 0;
char   serverName[256];
int    portNum;
int    squidConcurrentInput = 0;
int    connectionErrorBehaviour = UFDB_ALLOW;
char   connectErrorRedirect[1024] = 
		"http://cgibin.urlfilterdb.com/cgi-bin/URLblocked.cgi?"
		"category=fatal-error";
int    timeout = 12;
int    be_quiet = 0;
int    httpsChecks = UFDB_API_HTTPS_CHECK_OFF;

void UFDBsetTunnelCheckMethod( int method )
{
   ;
}

/*
 * we use some code that is shared between the multithreaded ufdbguardd and
 * our single-threaded app.
 * Use pthread stubs to satisfy the linker without using the pthread overhead.
 */
int pthread_mutex_lock( void * dummy )
{
   return 0;
}

int pthread_mutex_trylock( void * dummy )
{
   return 0;
}

int pthread_mutex_unlock( void * dummy )
{
   return 0;
}

int UFDBcheckForHTTPStunnel( char * hostname, int portnumber, int flags )
{
   return UFDB_API_ERR_NULL;
}


int connect_to_server( void )
{
   int                 s;
   int                 protocol;
   int                 retval;
   struct hostent *    server;
   struct sockaddr_in  addr_in;
   struct linger       linger;

   errno = 0;

   /* 
    * check the server name.
    */
   server = gethostbyname( serverName );
   if (server == NULL)
   {
      char *  c; 			/* see if we have a dotted IP address */
      struct in_addr server_addr;

      c = serverName;
      while (*c != '\0')
      {
         if (*c != '.' && (*c < '0' || *c > '9'))
	 {
	    fprintf( stderr, "%s: hostname lookup failed for server '%s'\n", progname, serverName );
	    exit( 1 );
	 }
	 c++;
      }

      /* the name looks like a dotted IP address */
      server_addr.s_addr = inet_addr( serverName );
      server = gethostbyaddr( (const char *) &server_addr, sizeof(struct in_addr), AF_INET );
      if (server == NULL)
      {
         fprintf( stderr, "%s: IP lookup failed for server '%s'\n", progname, serverName );
	 exit( 1 );
      }
   }

   /*
    * create the socket to connect to the daemon.
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
      ufdbLogError( "cannot create socket: %s", strerror(errno) );
      exit( 1 );
   }

#if HAVE_UNIX_SOCKETS
   if (protocol == AF_UNIX)
   {
      struct sockaddr_un  addr_un;

      addr_un.sun_family = AF_UNIX;
      sprintf( addr_un.sun_path, "/tmp/ufdbguardd-%05d", portNum );
      retval = connect( s, (struct sockaddr *) &addr_un, sizeof(addr_un) );
   }
   else
#endif
   {
      addr_in.sin_family = AF_INET;
      memcpy( (void *) &addr_in.sin_addr, (void *) server->h_addr_list[0], sizeof(addr_in.sin_addr) );  
      addr_in.sin_port = htons( portNum );
      retval = connect( s, (struct sockaddr *) &addr_in, sizeof(addr_in) );
   }

   if (retval < 0)
   {
      /* be silent about the connection error */
      close( s );
      s = -1;
   }
   else
   {
      int            sock_parm;
      struct timeval tv;

      linger.l_onoff = 0;
      linger.l_linger = 1;
      setsockopt( s, SOL_SOCKET, SO_LINGER, (void *) &linger, sizeof(linger) );
      sock_parm = 1;
      setsockopt( s, SOL_SOCKET, SO_KEEPALIVE, (void *) &sock_parm, sizeof(sock_parm) );

      /*
       * Allow server-side addresses to be reused (don't have to wait for timeout).
       * Turn off NAGLE.
       */
      if (protocol == AF_INET)
      {
	 sock_parm = 1;
	 setsockopt( s, IPPROTO_TCP, TCP_NODELAY, (void *) &sock_parm, sizeof(sock_parm) );
      }

      /*
       * TCP buffers can be quite large.
       * Optimize the window size by setting it to a relatively small fixed size on the 
       * client and the server.
       */
      if (protocol == AF_UNIX)
      {
	 sock_parm = 16384;
	 setsockopt( s, SOL_SOCKET, SO_SNDBUF, (void *) &sock_parm, sizeof(sock_parm) );
	 sock_parm = 16384;
	 setsockopt( s, SOL_SOCKET, SO_RCVBUF, (void *) &sock_parm, sizeof(sock_parm) );
      }

      /*
       *  Prevent long blocking on communication with the daemon.
       */
      tv.tv_sec = timeout;	/* 'timeout' seconds timeout for reads */
      tv.tv_usec = 0;
      setsockopt( s, SOL_SOCKET, SO_RCVTIMEO, (void *) &tv, sizeof(tv) );

      tv.tv_sec = 3;		/* 3 second timeout for writes */
      tv.tv_usec = 0;
      setsockopt( s, SOL_SOCKET, SO_SNDTIMEO, (void *) &tv, sizeof(tv) );
   }

   return s;
}


static void process_cmd( 
   int    fd, 
   char * url, 
   char * ip, 
   char * name )
{
   int    len;
   int    nbytes;
   char   line[UFDB_MAX_URL_LENGTH+256];

   if (strlen(url) > UFDB_MAX_URL_LENGTH-2)
      url[UFDB_MAX_URL_LENGTH-2] = '\0';

   if (strstr( url, "://" ) == NULL  &&  strstr( url, ":443" ) == NULL)
      sprintf( line, "http://%s %s/- %s GET\n", url, ip, name );
   else
      sprintf( line, "%s %s/- %s GET\n", url, ip, name );

   len = strlen( line );
   nbytes = write( fd, line, len );
   if (nbytes != len)
   {
      ufdbLogError( "line length is %d but wrote %d bytes.", len, nbytes );
      exit( 1 );
   }

   nbytes = read( fd, line, UFDB_MAX_URL_LENGTH );
   if (nbytes > UFDB_MAX_URL_LENGTH-1)
      nbytes = UFDB_MAX_URL_LENGTH - 1;
   if (nbytes < 0)
   {
      ufdbLogFatalError( "read from daemon failed: %s", strerror(errno) );
      if (fwrite( "\n", 1, 1, stdout ))
         ;
   }
   else
   {
      line[nbytes] = '\0';            /* force null termination */
      if (fwrite( line, nbytes, 1, stdout ))
         ;
   }
}


static void process_input( 
   int    server_socket  )
{
   int    nbytes;
   int    cid_prefix_len;
   int    len;
   int    fd_stdout = fileno( stdout );
   int    num_waivers;
   time_t t0;
   time_t t1;
   char * squidConcurrentInputID;
   char * request;
   char * result;
   char   requestBuffer[UFDB_HTTP_1_1_MAX_URI_LENGTH+32];
   char   resultBuffer[UFDB_MAX_URL_LENGTH+32];

   /*
    * loop to process queries on stdin.
    * If we cannot connect to the server, we continue looping:
    * - try to connect again to the server socket
    * - if failure, send an "OK" back to the requestor.
    */
   num_waivers = 6;
   if (server_socket < 0)
   {
      ufdbLogFatalError( "cannot connect to ufdbguardd daemon socket: %s\n"
                         "Check if ufdbguardd is running.", 
                         strerror(errno) );
      num_waivers = 1;
   }
   
   while (UFDBfgets(requestBuffer, UFDB_HTTP_1_1_MAX_URI_LENGTH+31, stdin) != NULL)	   /* read request */
   {
      if (UFDBglobalDebug)
	 ufdbLogMessage( "URL request: %s", requestBuffer );

      if (server_socket < 0)
      {
	 num_waivers--;
	 if (num_waivers < 0)
	 {
	    num_waivers = 6;
	    server_socket = connect_to_server();		/* retry to connect after 6 lookups */
	    if (server_socket >= 0)
	    {
	       ufdbLogMessage( "communication with daemon re-established" );
	    }
	 }
      }

      if (squidConcurrentInput)
      {
         squidConcurrentInputID = requestBuffer;
	 request = strchr( requestBuffer, ' ' );
	 if (request == NULL)
	 {
	    ufdbLogError( "invalid input: cannot find space separator on input line.  Verify use of -C option." );
	    squidConcurrentInputID = "";
	    request = requestBuffer;
	 }
	 else
	 {
	    *request = '\0';
	    request++;
	 }
      }
      else
      {
	 squidConcurrentInputID = "";
         request = requestBuffer;
      }

      if (server_socket < 0)		/* daemon is dead, connectionErrorBehaviour is leading */
      {
	 if (connectionErrorBehaviour == UFDB_ALLOW)
	    len = sprintf( resultBuffer, "%s\n", squidConcurrentInputID );
	 else if (*squidConcurrentInputID != '\0')
	    len = sprintf( resultBuffer, "%s %s\n", squidConcurrentInputID, connectErrorRedirect );
	 else
	    len = sprintf( resultBuffer, "%s\n", connectErrorRedirect );
	 if (write( fd_stdout, resultBuffer, len ) != len)
	 {
	    ufdbLogError( "lost communication with parent (write): %s\ncheck if squid is running.",
	    		  strerror(errno) );
	    break;
	 }
	 continue;
      }

      len = strlen( request );
      if (len >= UFDB_MAX_URL_LENGTH)		/* truncate URI to UFDB_MAX_URL_LENGTH bytes */
      {
         len = UFDB_MAX_URL_LENGTH;
	 request[len] = '\n';
	 request[len+1] = '\0';
      }

      /* forward the request to ufdbguardd */
      errno = 0;
      nbytes = write( server_socket, request, len );

      if (nbytes <= 0)  
      {								/* server socket was closed */
	 if (nbytes == 0)
	    errno = EINVAL;
         close( server_socket );
	 server_socket = -1;
	 num_waivers = 6;
	 ufdbLogError( "communication with ufdbguardd daemon lost (write): %s  ******\n"
	 	       "Check if ufdbguardd is running.\n"
		       "all verifications will be rated \"OK\"",
	 	       strerror(errno) );
	 if (connectionErrorBehaviour == UFDB_ALLOW)
	    len = sprintf( resultBuffer, "%s\n", squidConcurrentInputID );
	 else if (*squidConcurrentInputID != '\0')
	    len = sprintf( resultBuffer, "%s %s\n", squidConcurrentInputID, connectErrorRedirect );
	 else
	    len = sprintf( resultBuffer, "%s\n", squidConcurrentInputID );
	 if (write( fd_stdout, resultBuffer, len ) != len)
	 {
	    ufdbLogError( "lost communication with parent (write): %s\ncheck if squid is running.",
	    		  strerror(errno) );
	    break;
	 }
	 continue;
      }

      if (nbytes != len)
      {
         close( server_socket );
	 server_socket = -1;
	 num_waivers = 6;
	 ufdbLogError( "line length is %d but wrote %d bytes to ufdbguardd (%s).\n"
		       "all verifications will be rated \"OK\"",
	               len, nbytes, strerror(errno) );
	 if (connectionErrorBehaviour == UFDB_ALLOW)
	    len = sprintf( resultBuffer, "%s\n", squidConcurrentInputID );
	 else if (*squidConcurrentInputID != '\0')
	    len = sprintf( resultBuffer, "%s %s\n", squidConcurrentInputID, connectErrorRedirect );
	 else
	    len = sprintf( resultBuffer, "%s\n", squidConcurrentInputID );
	 if (write( fd_stdout, resultBuffer, len ) != len)
	 {
	    ufdbLogError( "lost communication with parent (write): %s\ncheck if squid is running.",
	    		  strerror(errno) );
	    break;
	 }
	 continue;
      }

      /* read the result that is sent by ufdbguardd */

      if (squidConcurrentInput)
      {
	 char * id;
	 for (result = resultBuffer, id = squidConcurrentInputID;  *id != '\0';  result++, id++)
	    *result = *id;
	 *result++ = ' ';
	 cid_prefix_len = result - resultBuffer;
      }
      else
      {
         result = resultBuffer;
	 cid_prefix_len = 0;
      }

      errno = 0;
      t0 = time( NULL );
      nbytes = read( server_socket, result, UFDB_MAX_URL_LENGTH );
      t1 = time( NULL );
      if ((t1 - t0) > 2)
      {
         ufdbLogMessage( "server response is slow: nbytes=%d numsec=%d error=%s", 
	                 nbytes, (t1 - t0), strerror(errno) );
      }

      if (nbytes <= 0)
      {								/* server socket was closed */
	 if (nbytes == 0)
	    errno = EINVAL;
         close( server_socket );
	 server_socket = -1;
	 num_waivers = 6;
	 ufdbLogError( "communication with ufdbguardd daemon lost (read): %s\n"
	 	       "check if ufdbguardd is running.\n"
		       "all verifications will be rated \"OK\".",
	 	       strerror(errno) );
	 if (connectionErrorBehaviour == UFDB_ALLOW)
	    len = sprintf( resultBuffer, "%s\n", squidConcurrentInputID );
	 else if (*squidConcurrentInputID != '\0')
	    len = sprintf( resultBuffer, "%s %s\n", squidConcurrentInputID, connectErrorRedirect );
	 else
	    len = sprintf( resultBuffer, "%s\n", squidConcurrentInputID );
	 if (write( fd_stdout, resultBuffer, len ) != len)
	 {
	    ufdbLogError( "lost communication with parent (write): %s  *****\ncheck if squid is running.",
	    		  strerror(errno) );
	    break;
	 }
	 continue;
      }

      if (nbytes > UFDB_MAX_URL_LENGTH - 17)
	 nbytes = UFDB_MAX_URL_LENGTH - 17;
      result[nbytes] = '\0';            	

      if (debugRedirect)
      {
	 if (nbytes > 1)
	    ufdbLogMessage( "   REDIRECT  -->%s   -->%s", request, resultBuffer );
      }

      if (noRedirects)					/* test mode */
      {
	 len = sprintf( resultBuffer, "%s\n", squidConcurrentInputID );
	 if (write( fd_stdout, resultBuffer, len ) != len)
	 {
	    ufdbLogError( "lost communication with parent (write): %s  *****\ncheck if squid is running.",
	    		  strerror(errno) );
	    break;
	 }
      }
      else
      {
	 if (squidConcurrentInput)
	    nbytes += cid_prefix_len;
	 if (write( fd_stdout, resultBuffer, nbytes ) != nbytes)	/* forward result */
	 {
	    ufdbLogError( "lost communication with parent (write): %s  ***\ncheck if squid is running.",
	    		  strerror(errno) );
	    break;
	 }
      }

      if ((t1 - t0) >= 4)		/* !&*@^#$ why is the server slow ? */
      {
         close( server_socket );
	 server_socket = -1;
	 num_waivers = 0;
      }
   }

   if (UFDBglobalDebug)
      ufdbLogMessage( "end of input" );
}


static void client_usage( void )
{
   fprintf( stderr, "usage: ufdbgclient [-d] [-v] [-r] [-C] [-T] [-t timeout] [-S server] [-p port] [-l logdir] [<URL> <IP> <user>]\n" );
   fprintf( stderr, "Options:\n" );
   fprintf( stderr, "  -d          : debug\n" );
   fprintf( stderr, "  -v          : show version number\n" );
   fprintf( stderr, "  -r          : log the redirections (performance impact)\n" );
#if 0
   fprintf( stderr, "  -N          : no log file\n" );
#endif
   fprintf( stderr, "  -C          : use Squid concurrency parameter (url_rewrite_concurrency)\n" );
   fprintf( stderr, "  -e policy   : policy on connection error with ufdbguardd: URL lookup result is 'deny' or 'allow'\n" );
   fprintf( stderr, "  -E URL      : redirection URL for denied access on connention error\n" );
   fprintf( stderr, "  -T          : test mode: log unwanted URLs but do not block them\n" );
   fprintf( stderr, "  -t numsec   : timeout waiting for daemon (default: %d seconds)\n", timeout );
   fprintf( stderr, "  -S server   : specify server name (default is %s)\n", "localhost" );
   fprintf( stderr, "  -p portnum  : override communication port (default is %d)\n", UFDB_DAEMON_PORT );
   fprintf( stderr, "  -l logdir   : log directory (default is %s)\n", DEFAULT_LOGDIR );
   fprintf( stderr, "  -q          : quiet mode\n" );
   fprintf( stderr, "  <URL>       : e.g. www.urlfilterdb.com\n" );
   fprintf( stderr, "  <IP>        : e.g. 10.1.1.1\n" );
   fprintf( stderr, "  <user>      : e.g. johndoe or '-'\n" );
}


/* 
 * we do NOT want to link with the bz2 library so we have some stubs.
 */
void BZ2_bzBuffToBuffDecompress()
{ }


static int denyORallow( char * option )
{
#if 0
   fprintf( stderr, "denyORallow \"%s\"\n", option );
#endif

   if (strcmp( option, "allow" ) == 0)
      return UFDB_ALLOW;
   else if (strcmp( option, "deny" ) == 0  ||  strcmp( option, "block" ) == 0)
      return UFDB_DENY;

   fprintf( stderr, "-e option must be followed by 'allow' or 'deny'\n" );
   client_usage();
   exit( 1 );
   return UFDB_ALLOW;  /* make compiler happy */
}


int main( 
   int                 argc, 
   char **             argv )
{
   int                 s;
   struct stat         stbuf;
   struct tms          timer;
   struct timeval      start_time;

   globalPid = getpid();
   strcpy( progname, "ufdbgclient" );

   strcpy( serverName, "localhost" );
   portNum = UFDB_DAEMON_PORT;

   while ((s = getopt(argc, argv, "e:E:NChdrTvql:p:S:t:")) != EOF)
   {
      switch (s) {
      case 'd':
	 UFDBglobalDebug = 1;
	 break;
      case 'e':
	 connectionErrorBehaviour = denyORallow( optarg );	
         break;
      case 'E':
         strcpy( connectErrorRedirect, optarg );
	 break;
      case 'r':
	 debugRedirect = 1;
	 break;
      case 'l':
	 UFDBglobalLogDir = optarg;
	 if (stat( UFDBglobalLogDir, &stbuf ) != 0)
	 {
	    fprintf( stderr, "ufdbgclient: error in -l %s: %s\n", UFDBglobalLogDir, strerror(errno) );
	    exit( 1 );
	 }
	 if (!S_ISDIR(stbuf.st_mode))
	 {
	    fprintf( stderr, "ufdbgclient: %s is not a directory\n", UFDBglobalLogDir );
	    exit( 1 );
	 }
	 break;
      case 'N':
         ufdbGlobalSetLogging( 0 );
	 break;
      case 'C':
	 squidConcurrentInput = 1;
         break;
      case 'p':
         portNum = atoi( optarg );
	 if (portNum < 1)
	 {
	    fprintf( stderr, "ufdbgclient: port number must be > 0\n" );
	    exit( 1 );
	 }
	 break;
      case 'q':
	 be_quiet = 1;
         break;
      case 'v':
	 fprintf( stderr, "ufdbgclient: %s\n", VERSION );
	 exit( 0 );
	 break;
      case 'S':
         strcpy( serverName, optarg );
	 break;
      case 't':
	 timeout = atoi( optarg );
	 if (timeout < 2)
	    timeout = 2;
	 if (timeout > 200)
	    timeout = 200;
         break;
      case 'T':
	 fprintf( stderr, "-T option found.  Going into test mode.\n" );
	 noRedirects = 1;		/* Test mode means noRedirects */
	 break;
      case '?':
      case 'h':
      default:
         client_usage();
	 exit( 0 );
      }
   }

   ufdbSetGlobalErrorLogFile();
   signal( SIGPIPE, SIG_IGN );
   
   s = connect_to_server();

   /* 
    * process one query from the command line ?
    */
   if (optind < argc - 2)
   {
      if (s < 0)
      {
	 int  nbytes;
	 char line[1024];

	 if (connectionErrorBehaviour == UFDB_ALLOW)
	    strcpy( line, "\n" );
	 else
	    sprintf( line, "%s\n", connectErrorRedirect );
	 nbytes = strlen( line );
	 if (fwrite( line, nbytes, 1, stdout ))
	    ;
	 ufdbLogError( "cannot connect to port %d on server %s: %s", 
	               portNum, serverName, strerror(errno) );
	 exit( 1 );
      }
      process_cmd( s, argv[optind], argv[optind+1], argv[optind+2] );
      close( s );
      exit( 0 );
   }

   if (!be_quiet)
   {
      gettimeofday( &start_time, NULL );
      ufdbLogMessage( "ufdbgclient " VERSION " started" );
      UFDBtimerInit( &timer );
   }

   process_input( s );

   if (!be_quiet)
   {
      char cpuusagetxt[200];

      UFDBtimerStop( &timer );
      UFDBtimerPrintString( cpuusagetxt, &timer, "CPU times" );
      gettimeofday( &start_time, NULL );
      ufdbLogMessage( "ufdbgclient " VERSION " finished\n%s", cpuusagetxt );
   }

   close( s );

   exit( 0 );
}

