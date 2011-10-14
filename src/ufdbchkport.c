/*
 * ufdbchkport.c - URLfilterDB
 *
 * ufdbGuard is copyrighted (C) 2005-2011 by URLfilterDB with all rights reserved.
 *
 * Parts of the ufdbGuard daemon are based on squidGuard.
 * This module is NOT based on squidGuard.
 *
 * RCS $Id: ufdbchkport.c,v 1.49 2011/05/20 14:26:45 root Exp root $
 */

#include "ufdb.h"
#include "ufdblib.h"
#include "ufdbchkport.h"
#include "httpsQueue.h"

#include "ufdbHashtable.h"

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#ifdef _POSIX_PRIORITY_SCHEDULING
#include <sched.h>
#endif

#include <sys/select.h>

#include <openssl/ssl.h>
#include <openssl/engine.h>
#include <openssl/x509v3.h>
#include <openssl/err.h>


#undef UFDB_HTTPS_CACHE_DEBUG
#ifdef UFDB_DEBUG
#define UFDB_HTTPS_CACHE_DEBUG 1
#endif 

#if UFDB_DO_DEBUG || 0
#define DEBUG(x) fprintf x 
#else
#define DEBUG(x) 
#endif


static int num_static_ssl_locks = 0;
static pthread_mutex_t * crypto_mutexes = NULL;
static SSL_CTX * ssl_ctx = NULL;
static struct UFDBhashtable * myht = NULL;
static int ufdbCacertsLoaded = 0;

static int  UFDBverifyPortHasHTTPS( char * hostname, int portnumber, int flags );
static int  lookupHTTPScache( char * hostname, int portnumber, int keepLockForInsert );
static void updateHTTPScache( char * hostname, int portnumber, int status, int lockSetBySearch );
static void insertHTTPScache( char * hostname, int portnumber, int status, int lockSetBySearch );

#ifdef UFDB_LOAD_CERT_CHAIN
static int UFDBloadIntermediateCertificates( X509 * cert );
#endif

#if  0  ||  defined(UFDB_DEBUG)  ||  !defined(HAVE_THREADSAFE_OPENSSL_LIB)

static pthread_mutex_t https_mutex = UFDB_STATIC_MUTEX_INIT;

#define SSL_MUTEX_FN_INIT 	int _mutex_retval;
#define SSL_MUTEX_LOCK(fn) \
{                                                                \
   _mutex_retval = pthread_mutex_unlock( &https_mutex );         \
   if (_mutex_retval != 0)                                       \
      ufdbLogError( fn ": mutex_unlock failed with code %d", _mutex_retval );  \
}
#define SSL_MUTEX_UNLOCK(fn) \
{                                                                \
   _mutex_retval = pthread_mutex_unlock( &https_mutex );         \
   if (_mutex_retval != 0)                                       \
      ufdbLogError( fn ": mutex_unlock failed with code %d", _mutex_retval );  \
}

#else
#define SSL_MUTEX_FN_INIT
#define SSL_MUTEX_LOCK(fn)
#define SSL_MUTEX_UNLOCK(fn)
#endif


#define UFDB_DEBUG_PROBES 1


/*
 * Parsed OpenSSL version number.
 */
typedef struct {
    int     major;
    int     minor;
    int     micro;
    int     patch;
    int     status;
} TLS_VINFO;

static int ssl_connect( char * hostname, int portnumber, int fd, SSL ** ssl );
static int ssl_check_certificate( SSL * ssl, const char * hostname, int portnumber );
static int openssl_write( int fd, char * buf, int bufsize, SSL * ssl );
static int openssl_read( int fd, char * buf, int bufsize, SSL * ssl );
static void openssl_close( SSL * ssl );


static void print_errors( void )
{
   unsigned long sslerr;
   const char *  file;
   int           line;
   const char *  data;
   int           flags;
   int           header = 0;
   char          errstring[256];

   header = 0;
   while ((sslerr = ERR_get_error_line_data( &file, &line, &data, &flags )) != 0)
   {
      if (!header)
      {
	 ufdbLogError( "HTTPS/SSL/tunnel verification failed with OpenSSL error:" );
	 header = 1;
      }

      ERR_error_string_n( sslerr, errstring, sizeof(errstring) );
      ufdbLogMessage( "   SSL error: %s", errstring );
   }
}


int UFDBloadAllowedHTTPSsites( 
   char * filename )
{
   return UFDB_API_OK;
}


int UFDBread( 
   int	  fd, 
   char * buffer, 
   int    max )
{
   int    n;
   int    total;

   total = 0;
   while ((n = read( fd, buffer, max )) > 0)
   {
      total += n;
      buffer += n;
      max -= n;
   }
   if (total == 0)
      return n;
   return total;
}


static unsigned int hostname2hash( 
   void *       key )
{
   int          n;
   unsigned int hash;
   char *       hostname = (char *) key;

   hash = 0;
   n = 15;
   while (n > 0  &&  *hostname != '\0')
   {
      hash = (hash << 3) + ((unsigned int) (*hostname));
      hash ^= (hash << 14);
      hostname++;
      n--;
   }

   return hash;
}


static int mystrcmp( void * a, void * b )
{
   return strcmp( (char *) a, (char *) b ) == 0;
}


void UFDBsetTunnelCheckMethod( int method )
{
   UFDBglobalTunnelCheckMethod = method;
}


static void ufdb_pthread_locking_callback( 
   int          mode, 
   int          type, 
   const char * file, 
   int          line )
{
   int          ret;

#if 0
   if (UFDBglobalDebug)
      ufdbLogMessage( "      CRYPTO lock %6s %s%s %2d", 
                      (mode & CRYPTO_LOCK) ? "lock" : "unlock", 
		      (mode & CRYPTO_READ) ? "read" : "",
		      (mode & CRYPTO_WRITE) ? "write" : "",
		      type );
#endif

   if (type < 0  || type > num_static_ssl_locks)
   {
      ufdbLogMessage( "CRYPTO lock type %d out of range (max=%d)  *****", type, num_static_ssl_locks );
      return;
   }

   if (mode & CRYPTO_LOCK)
   {
      ret = pthread_mutex_lock( &crypto_mutexes[type] );
#ifdef UFDB_DEBUG
      if (ret != 0)
	 ufdbLogError( "ufdb_pthread_locking_callback: mutex_lock[%d] failed with code %d", type, ret );
#endif
   }
   else if (mode & CRYPTO_UNLOCK)
   {
      ret = pthread_mutex_unlock( &crypto_mutexes[type] );
#ifdef UFDB_DEBUG
      if (ret != 0)
	 ufdbLogError( "ufdb_pthread_locking_callback: mutex_unlock[%d] failed with code %d", type, ret );
#endif
   }
   else
      ufdbLogError( "ufdb_pthread_locking_callback: no LOCK|UNLOCK for type %d", type );
}


static unsigned long ufdb_pthread_id_callback( void )
{
   unsigned long id = (unsigned long) pthread_self();

   return id;
}


typedef struct CRYPTO_dynlock_value {
   pthread_mutex_t lock;
} CRYPTO_dynlock_value;


static CRYPTO_dynlock_value * ufdb_ssl_dyn_create(
   const char * file, 
   int          line )
{
   CRYPTO_dynlock_value * lp;

   lp = ufdbMalloc( sizeof(CRYPTO_dynlock_value) );
   pthread_mutex_init( &(lp->lock), NULL );
   return lp;
}


static void ufdb_ssl_dyn_lock(
   int                    mode, 
   CRYPTO_dynlock_value * lp, 
   const char *           file,
   int                    line )
{
   int                    ret;

#if 0
   if (UFDBglobalDebug)
   {
      ufdbLogMessage( "      ufdb_ssl_dyn_lock %6s %s%s", 
                      (mode & CRYPTO_LOCK) ? "lock" : "unlock", 
		      (mode & CRYPTO_READ) ? "read" : "",
		      (mode & CRYPTO_WRITE) ? "write" : "" );
   }
#endif

   if (mode & CRYPTO_LOCK)
   {
      ret = pthread_mutex_lock( &(lp->lock) );
#ifdef UFDB_DEBUG
      if (ret != 0)
	 ufdbLogError( "ufdb_ssl_dyn_lock: mutex_lock failed with code %d", ret );
#endif
   }
   else if (mode & CRYPTO_UNLOCK)
   {
      ret = pthread_mutex_unlock( &(lp->lock) );
#ifdef UFDB_DEBUG
      if (ret != 0)
	 ufdbLogError( "ufdb_ssl_dyn_lock: mutex_unlock failed with code %d", ret );
#endif
   }
   else
      ufdbLogError( "ufdb_ssl_dyn_lock: no LOCK|UNLOCK in mode" );
}


static void ufdb_ssl_dyn_destroy(
   CRYPTO_dynlock_value * lp,
   const char *           file,
   int                    line )
{
   pthread_mutex_destroy( &(lp->lock) );
}


/* tmp_rsa_cb - call-back to generate ephemeral RSA key */
static RSA * tmp_rsa_cb( 
   SSL * unused_ssl,
   int   unused_export,
   int   keylength )
{
   static RSA * rsa_tmp;

   /* Code adapted from OpenSSL apps/s_cb.c */

   if (rsa_tmp == 0)
     rsa_tmp = RSA_generate_key( keylength, RSA_F4, NULL, NULL );
   return rsa_tmp;
}


/* openssl_bug_bits - SSL bug compatibility bits for this OpenSSL version */
static long openssl_bug_bits( void )
{
    long    bits = SSL_OP_ALL;          /* Work around all known bugs */

#if OPENSSL_VERSION_NUMBER >= 0x00908000L
    long    lib_version = SSLeay();

    /* 
     * In OpenSSL 0.9.8[ab], enabling zlib compression breaks the padding bug
     * work-around, leading to false positives and failed connections. We may
     * not interoperate with systems with the bug, but this is better than
     * breaking on all 0.9.8[ab] systems that have zlib support enabled.
     */
    if (lib_version >= 0x00908000L && lib_version <= 0x0090802fL) 
    {
        STACK_OF(SSL_COMP) * comp_methods;

        comp_methods = SSL_COMP_get_compression_methods();
        if (comp_methods != 0  &&  sk_SSL_COMP_num(comp_methods) > 0)
            bits &= ~SSL_OP_TLS_BLOCK_PADDING_BUG;
    }
#endif
    return bits;
}


/* tls_version_split - Split OpenSSL version number into major, minor, ... */

static void tls_version_split( 
   long        version, 
   TLS_VINFO * info )
{
    /*
     * OPENSSL_VERSION_NUMBER(3):
     * 
     * OPENSSL_VERSION_NUMBER is a numeric release version identifier:
     * 
     * MMNNFFPPS: major minor fix patch status
     * 
     * The status nibble has one of the values 0 for development, 1 to e for
     * betas 1 to 14, and f for release. Parsed OpenSSL version number. for
     * example
     * 
     * 0x000906000 == 0.9.6 dev 
     * 0x000906023 == 0.9.6b beta 3 
     * 0x00090605f == 0.9.6e release
     * 
     * Versions prior to 0.9.3 have identifiers < 0x0930.  Versions between
     * 0.9.3 and 0.9.5 had a version identifier with this interpretation:
     * 
     * MMNNFFRBB major minor fix final beta/patch
     * 
     * for example
     * 
     * 0x000904100 == 0.9.4 release 
     * 0x000905000 == 0.9.5 dev
     * 
     * Version 0.9.5a had an interim interpretation that is like the current
     * one, except the patch level got the highest bit set, to keep continu-
     * ity.  The number was therefore 0x0090581f.
     */

    if (version < 0x0930) {
	info->status = 0;
	info->patch = version & 0x0f;
	version >>= 4;
	info->micro = version & 0x0f;
	version >>= 4;
	info->minor = version & 0x0f;
	version >>= 4;
	info->major = version & 0x0f;
    } else if (version < 0x00905800L) {
	info->patch = version & 0xff;
	version >>= 8;
	info->status = version & 0xf;
	version >>= 4;
	info->micro = version & 0xff;
	version >>= 8;
	info->minor = version & 0xff;
	version >>= 8;
	info->major = version & 0xff;
    } else {
	info->status = version & 0xf;
	version >>= 4;
	info->patch = version & 0xff;
	version >>= 8;
	info->micro = version & 0xff;
	version >>= 8;
	info->minor = version & 0xff;
	version >>= 8;
	info->major = version & 0xff;
	if (version < 0x00906000L)
	    info->patch &= ~0x80;
    }
}


static void OpenSSLversionCheck( void )
{
   TLS_VINFO hdr_info;
   TLS_VINFO lib_info;

   tls_version_split( OPENSSL_VERSION_NUMBER, &hdr_info );
   tls_version_split( SSLeay(), &lib_info );

   if ( lib_info.major != hdr_info.major ||
        lib_info.minor != hdr_info.minor ||
        lib_info.micro != hdr_info.micro)
   {
      ufdbLogMessage( "run-time library vs. compile-time header version mismatch: "
		      "OpenSSL %d.%d.%d may not be compatible with OpenSSL %d.%d.%d",
		      lib_info.major, lib_info.minor, lib_info.micro, 
		      hdr_info.major, hdr_info.minor, hdr_info.micro );
   }
   else
   {
      char status[16];
      char rev[2];

      if (lib_info.status == 0x0f)
         strcpy( status, "R" );
      else
         sprintf( status, "beta %d", lib_info.status );
      if (lib_info.patch == 0)
         rev[0] = '\0';
      else
      {
         rev[0] = lib_info.patch - 1 + 'a';
	 rev[1] = '\0';
      }

      ufdbLogMessage( "using OpenSSL library %d.%d.%d%s %s",
		      lib_info.major, lib_info.minor, lib_info.micro, rev, status );
   }
}


int UFDBinitHTTPSchecker( void )
{
   static volatile int    inited = 0;
   struct stat            statres;
   SSL_MUTEX_FN_INIT 	

   if (!inited)
   {
      int    i;
      char * dbdirectory;

      inited = 1;

      SSL_MUTEX_LOCK( "UFDBinitHTTPSchecker" )

      myht = UFDBcreateHashtable( 167, hostname2hash, mystrcmp );

      srandom( ((getpid() << 8) ^ time(NULL)) + ((unsigned long)myht << 26) );

      OpenSSLversionCheck();

      ufdbGetMallocMutex( "UFDBinitHTTPSchecker 1" );
      SSL_load_error_strings();
      SSL_library_init();
      /* 
       * The SSL_library_init man page does not mention which "old" version... 
       * But some "old" versions need to call OpenSSL_add_all_algorithms()
       */
      OpenSSL_add_all_algorithms();	
      OPENSSL_config( NULL );
      ENGINE_load_builtin_engines();
      ufdbReleaseMallocMutex( "UFDBinitHTTPSchecker 1" );

      CRYPTO_set_id_callback( ufdb_pthread_id_callback );	/* TODO: #ifdef for CRYPTO_THREADID_set_callback() */
      num_static_ssl_locks = CRYPTO_num_locks();
      crypto_mutexes = ufdbMalloc( (num_static_ssl_locks+1) * sizeof(pthread_mutex_t) );
      for (i = 0; i <= num_static_ssl_locks; i++)
         pthread_mutex_init( &crypto_mutexes[i], NULL );
      CRYPTO_set_locking_callback( ufdb_pthread_locking_callback );

      CRYPTO_set_dynlock_create_callback( ufdb_ssl_dyn_create );
      CRYPTO_set_dynlock_lock_callback( ufdb_ssl_dyn_lock );
      CRYPTO_set_dynlock_destroy_callback( ufdb_ssl_dyn_destroy );

      ufdbGetMallocMutex( "UFDBinitHTTPSchecker 2" );
      ssl_ctx = SSL_CTX_new( SSLv23_client_method() );
      ufdbReleaseMallocMutex( "UFDBinitHTTPSchecker 2" );
      if (ssl_ctx == NULL)
      {
	 UFDBglobalHttpsOfficialCertificate = 0;
         SSL_MUTEX_UNLOCK( "UFDBinitHTTPSchecker" )
	 errno = EINVAL;
         return UFDB_API_ERR_ERRNO;
      }

      SSL_CTX_set_options( ssl_ctx, openssl_bug_bits() );
      SSL_CTX_set_session_cache_mode( ssl_ctx, SSL_SESS_CACHE_OFF );
      SSL_CTX_set_timeout( ssl_ctx, 120 );
      SSL_CTX_set_default_verify_paths( ssl_ctx );
      SSL_CTX_set_verify_depth( ssl_ctx, 12 );
      
      if (UFDBglobalCAcertsFile[0] == '\0')
      {
         ufdbLogError( "CA certificates are not defined, using default file" );
	 dbdirectory = ufdbSettingGetValue( "dbhome" );
	 if (dbdirectory == NULL)
	    dbdirectory = DEFAULT_DBHOME;
	 strcpy( UFDBglobalCAcertsFile, dbdirectory );
	 strcat( UFDBglobalCAcertsFile, "/security/cacerts" );
      }
      if (UFDBglobalDebug)
	 ufdbLogMessage( "UFDBinitHTTPSchecker: CA certficates are in \"%s\"", UFDBglobalCAcertsFile );
      if (stat( UFDBglobalCAcertsFile, &statres ) < 0)
      {
         ufdbLogError( "SSL library initialisation failed because of a problem with the CA certificate file:  *****\n"
	               "file: \"%s\", error: %s", UFDBglobalCAcertsFile, strerror(errno) );
	 if (UFDBglobalHttpsOfficialCertificate)
	 {
	    ufdbLogFatalError( "Cannot perform mandatory check of SSL certificates." );
	    ufdbLogMessage( "ABORTING.  Fix the problem and start ufdbguardd again." );
	    ufdbLogMessage( "In case that there is no cacerts file, option enforce-https-official-certificate must be \"off\"" );
	    abort();
	 }
         SSL_MUTEX_UNLOCK( "UFDBinitHTTPSchecker" )
	 errno = ENOENT;
         return UFDB_API_ERR_ERRNO;
      }
      
      ufdbGetMallocMutex( "UFDBinitHTTPSchecker 4" );
      ufdbCacertsLoaded = SSL_CTX_load_verify_locations( ssl_ctx, UFDBglobalCAcertsFile, NULL ); 
      ufdbReleaseMallocMutex( "UFDBinitHTTPSchecker 4" );
      if (ufdbCacertsLoaded != 1)
      {
         ufdbLogError( "Failure to load the CA database; HTTPS/SSL certificates cannot be verified *****\n"
                       "CA database should be %s", UFDBglobalCAcertsFile );
      }
      else
	 ufdbLogMessage( "HTTPS/SSL verification with trusted certificates from \"%s\"", UFDBglobalCAcertsFile );

      /*
       * According to the OpenSSL documentation, temporary RSA key is needed
       * when export ciphers are in use. 
       */
      SSL_CTX_set_tmp_rsa_callback( ssl_ctx, tmp_rsa_cb );

      /* use SSL_VERIFY_NONE and verify the certificate in ssl_check_certificate */
      SSL_CTX_set_verify( ssl_ctx, SSL_VERIFY_NONE, NULL );

#if 0
      SSL_CTX_set_mode( ssl_ctx, SSL_MODE_ENABLE_PARTIAL_WRITE );
#endif
      SSL_CTX_set_mode( ssl_ctx, SSL_MODE_AUTO_RETRY );

      SSL_MUTEX_UNLOCK( "UFDBinitHTTPSchecker" )

   }

   return UFDB_API_OK;
}


static char * sslver2string( 
   int version )
{
   switch (version)
   {
      case SSL2_VERSION:   return "SSLv2";
      case SSL3_VERSION:   return "SSLv3";
      case TLS1_VERSION:   return "TLS1";
#ifdef DTLS1_VERSION
      case DTLS1_VERSION:  return "DTLS1";
#endif
   }

   return "unknown";
}


static char * httpsGETroot( 
   int         s,
   char *      hostname, 
   int         portnumber, 
   int *       status )
{
   int         n;
   SSL_MUTEX_FN_INIT
   SSL *       ssl;
   char        request[2048];
   char        reply[4096+4];

   /*******************************
   GET / HTTP/1.0
   User-Agent: Mozilla/4.0 (xxx) Gecko/20100722 Firefox/3.6.8
   Host: www.urlfilterdb.com:9443
   Accept: * / *
   Connection: Keep-Alive
   ********************************/

   if (ssl_ctx == NULL)
   {
      *status = UFDB_API_ERR_NULL;
      ufdbLogError( "httpsGETroot: ssl_ctx is NULL" );
      return NULL;
   }

   SSL_MUTEX_LOCK( "httpsGETroot" )

   /* ssl_connect returns OK (have SSL), SKYPE (detected Skype), or UNKNOWN_PROTOCOL or TUNNEL */
   ssl = NULL;
   n = ssl_connect( hostname, portnumber, s, &ssl );
   if (n != UFDB_API_OK)
   {
      SSL_MUTEX_UNLOCK( "httpsGETroot" )
      ufdbLogMessage( "SSL connect failure to %s:%d.  The server %s.", 
		      hostname, portnumber,
		      (n==UFDB_API_ERR_IS_AIM) ? "has AIM" : 
			 (n==UFDB_API_ERR_IS_GTALK) ? "has Google Talk" : 
			    (n==UFDB_API_ERR_IS_SKYPE) ? "has Skype" : 
			       (n==UFDB_API_ERR_IS_YAHOOMSG) ? "has Yahoo IM" : 
				  (n==UFDB_API_ERR_UNKNOWN_PROTOCOL) ? "uses an unknown protocol" :
				     (n==UFDB_API_ERR_NULL) ? "caused an internal error" :
					(n==UFDB_API_ERR_TUNNEL) ? "has a tunnel" :
					   (n==UFDB_API_ERR_SOCKET) ? "has a connection error" : 
					      (n==UFDB_API_ERR_IP_ADDRESS) ? "has no FQDN but an IP address" : 
						 "does not speak SSL+HTTP" );
      *status = n;
      return NULL;
   }

   ufdbLogMessage( "SSL connection to %s:%d established with protocol %s", 
		   hostname, portnumber, sslver2string(ssl->version) );

   n = ssl_check_certificate( ssl, hostname, portnumber );
   /* n = OK|ERR_INVALID_CERT|ERR_IS_YAHOOMSG|ERR_IS_AIM */
   if (!UFDBglobalHttpsOfficialCertificate)
   {
      if (n == UFDB_API_ERR_INVALID_CERT)
      {
	 ufdbLogMessage( "   SSL certificate of %s:%d has an issue but enforce-https-official-certificate is off", hostname, portnumber );
	 n = UFDB_API_OK;
      }
   }

   SSL_MUTEX_UNLOCK( "httpsGETroot" )

   if (n == UFDB_API_ERR_IS_AIM)
   {
      *status = n;
      openssl_close( ssl );
      ufdbLogMessage( "SSL connection to %s:%d has AIM", hostname, portnumber );
      return NULL;
   }

   if (n == UFDB_API_ERR_IS_YAHOOMSG)
   {
      *status = n;
      openssl_close( ssl );
      ufdbLogMessage( "SSL connection to %s:%d has Yahoo IM", hostname, portnumber );
      return NULL;
   }

   if (n != UFDB_API_OK)
   {
      *status = UFDB_API_ERR_INVALID_CERT;
      openssl_close( ssl );
      ufdbLogMessage( "SSL connection to %s:%d has a SSL certificate issue *****", hostname, portnumber );
      return NULL;
   }

   *status = UFDB_API_OK;

   /* TO-DO: if there is a proxy, "CONNECT" must be used */
   sprintf( request, "GET / HTTP/1.1\r\n"
                     "User-Agent: " UFDB_USER_AGENT "\r\n"
		     "Host: %s:%d\r\n"
		     "Accept: */*\r\n"
		     "Connection: Close\r\n"
		     "\r\n",
		     hostname, portnumber );
   n = openssl_write( s, request, strlen(request), ssl );
   if (n < 0)
   {
      *status = UFDB_API_ERR_SOCKET;
      strcpy( reply, "failed to send SSL message" );
   }
   else
   {
      reply[0] = '\0';
      n = openssl_read( s, reply, sizeof(reply)-4, ssl );
      if (n > 0)
      {
	 reply[n] = '\0';
#if 0
	 reply[n+1] = '\0';	/* to make valgrind happy */
	 reply[n+2] = '\0';
	 reply[n+3] = '\0';
#endif
      }
      else
         *status = UFDB_API_ERR_SOCKET;
   }
   openssl_close( ssl );

   return ufdbStrdup( reply );
}


/* Check for a tunnel.
 *
 * Valid flags are:
 * UFDB_API_ALLOW_QUEUING
 * UFDB_API_VERBOSE_OUTPUT
 *
 * return values are:
 * UFDB_API_OK:                   regular https traffic
 * UFDB_API_REQ_QUEUED:           request is queued for an other thread
 * UFDB_API_ERR_TUNNEL:           https channel is tunneled
 * UFDB_API_ERR_UNKNOWN_PROTOCOL: https channel is tunneled
 * UFDB_API_ERR_IS_SKYPE:         https channel is used by Skype
 * UFDB_API_ERR_IS_GTALK:         https channel is used by Gtalk
 * UFDB_API_ERR_CERTIFICATE:      SSL certificate is self-signed or has wrong common name
 * UFDB_API_ERR_IP_ADDRESS:       hostname is an IP address (only when flag UFDB_API_NO_IP is used)
 * UFDB_API_ERR_NULL:             tunnel verification is OFF.
 */
int UFDBcheckForHTTPStunnel( 
   char *             hostname, 
   int                portnumber,
   int                flags )
{
   int                ret;
   struct httpsInfo * info;

   if (UFDBglobalTunnelCheckMethod == UFDB_API_HTTPS_CHECK_OFF)
      return UFDB_API_ERR_NULL;

   if (UFDBglobalDebug)
      ufdbLogMessage( "UFDBcheckForHTTPStunnel  %s:%d  %04x", hostname, portnumber, flags );

   if (flags & UFDB_API_ALLOW_QUEUING)
   {
      int   status;
      char  key[1024+16];

      status = lookupHTTPScache( hostname, portnumber, 1 );
      if (status == UFDB_API_ERR_NULL)
      {
	 /* The hostname:port is entered in the hash *NOW* so that future calls
	  * know that it is already queued.
	  */
	 sprintf( key, "%s:%d", hostname, portnumber );
	 info = ufdbMalloc( sizeof(struct httpsInfo) );
	 info->t = time( NULL );
	 info->status = UFDB_API_REQ_QUEUED;
	 UFDBinsertHashtable( myht, ufdbStrdup(key), (void *) info, 1 );

	 ret = ufdbHttpsQueueRequest( hostname, portnumber );
	 if (ret != UFDB_API_OK)
	 {  
	    if (ret == UFDB_API_ERR_FULL)
	    {
	       /* The queue is full. The status in the hashtable is updated to "socket error" 
	        * because this forces a removal from the cache in 5 minutes and a retry.
		*/
	       info->status = UFDB_API_ERR_SOCKET;
	       return UFDB_API_ERR_SOCKET;
	    }
	    else if (ret == UFDB_API_REQ_QUEUED)
	    {
	       /* The HTTP verification request was already queued... */
	       return UFDB_API_BEING_VERIFIED;
	    }
	    else
	       ufdbLogError( "UFDBcheckForHTTPStunnel: ufdbHttpsQueueRequest( %s:%d ) returned with code %d", 
	                     hostname, portnumber, ret );
	 }

	 /* We just queued a request for a verification thread. Be nice and yield */
#ifdef _POSIX_PRIORITY_SCHEDULING
	 sched_yield();
#endif
	 return UFDB_API_BEING_VERIFIED;
      }
      return status;
   }

   /* request are not queued, so we perform the verification ourselves */
   return UFDBverifyPortHasHTTPS( hostname, portnumber, flags );
}


/*
 * in case that the caller of UFDBcheckForHTTPStunnel uses the UFDB_API_ALLOW_QUEUING flag,
 * the caller MUST have created 1-14 threads with UFDBhttpsTunnelVerifier() as main.
 */
void * UFDBhttpsTunnelVerifier( 
   void *    ptr )
{
   sigset_t  signals;
   int       portnumber;
   char      hostname[1028];

   /* 
    * All signals must be blocked.  
    */
   sigemptyset( &signals );
   sigaddset( &signals, SIGHUP );
   sigaddset( &signals, SIGUSR1 );
   sigaddset( &signals, SIGCHLD );
   sigaddset( &signals, SIGTERM );
   sigaddset( &signals, SIGINT );
   pthread_sigmask( SIG_BLOCK, &signals, NULL );

   ufdbSetThreadCPUaffinity( (int) ((long) ptr) );

   UFDBinitHTTPSchecker();

   while (1)
   {
      ufdbGetHttpsRequest( hostname, &portnumber );

      pthread_testcancel();

      if (UFDBglobalDebug)
	 ufdbLogMessage( "T%02ld: UFDBhttpsTunnelVerifier: start SSL verification %s:%d ...", 
	                 (long) ptr, hostname, portnumber );
      (void) UFDBverifyPortHasHTTPS( hostname, portnumber, 0 );

      pthread_testcancel();

      while (UFDBglobalReconfig)
	 sleep( 1 );			/* be nice to the reconfiguration thread */
   }

   /*NOTREACHED*/
   return NULL;
}


static int hostnameIsIP( const char * hostname )
{
   while (*hostname != '\0')
   {
      if (*hostname != '.'  &&  !isdigit(*hostname))
         return 0;
      hostname++;
   }

   return 1;
}


/* Probe port 443 to see what protocol is used.
 *
 * NOTE: This function takes 0-22 seconds to complete !
 */
static int UFDBverifyPortHasHTTPS( 
   char *         hostname, 
   int            portnumber, 
   int            flags )
{
   /* When a HTTPS port is used, we need to check the following:
    * if SSL+HTTP is spoken
    * if not SSL, is XMPP/Jabber/Google Talk
    * if not SSL, is Skype or an unknown protocol
    * if SSL has a valid certificate
    * if SSL+HTTP, detect known tunnels
    * TODO: if SSL+HHTP, detect proxy with content analysis (NEW)
    */
   int            s;
   int            status;
   char *         content;
   struct timeval tv;
   int            n;
#if defined(UFDB_INITIAL_SOCKET_CHECKS)
   char           line[132];
#endif

   if (UFDBglobalDebug  ||  flags & UFDB_API_VERBOSE_OUTPUT)
      ufdbLogMessage( "UFDBverifyPortHasHTTPS( %s, %d )", hostname, portnumber );

   if (UFDBglobalHttpsWithHostname  &&  !UFDBglobalUnknownProtocolOverHttps  &&  
       !UFDBglobalSkypeOverHttps  &&  !UFDBglobalGtalkOverHttps  &&
       hostnameIsIP(hostname) )
   {
      ufdbLogError( "IP address in URL is not allowed for the HTTPS protocol. IP=%s", hostname );
      status = UFDB_API_ERR_IP_ADDRESS;
      updateHTTPScache( hostname, portnumber, status, 0 );
      return status;
   }

   status = lookupHTTPScache( hostname, portnumber, 1 );
   if (status == UFDB_API_ERR_NULL)
   {
      /* this thread is going to do the verification now */
      insertHTTPScache( hostname, portnumber, UFDB_API_BEING_VERIFIED, 1 );
   }
   else if (status == UFDB_API_REQ_QUEUED)
   {
      /* this thread is going to do the verification now */
      updateHTTPScache( hostname, portnumber, UFDB_API_BEING_VERIFIED, 1 );
   }
   else
   {
      if (status != UFDB_API_BEING_VERIFIED)
	 return status;
      
#ifdef UFDB_HTTPS_CACHE_DEBUG
      ufdbLogMessage( "   UFDBverifyPortHasHTTPS %s:%d status is BEING_VERIFIED", hostname, portnumber );
#endif
      /* The status is UFDB_API_BEING_VERIFIED...
       * When the mode is "aggressive" we use an idle-wait loop waiting
       * for the other thread to finish its job, otherwise
       * we return UFDB_API_BEING_VERIFIED.
       */
      if (UFDBglobalTunnelCheckMethod != UFDB_API_HTTPS_CHECK_AGGRESSIVE)
      {
	 if (UFDBglobalTunnelCheckMethod == UFDB_API_HTTPS_CHECK_QUEUE_CHECKS)
	    usleep( 100000 );		/* give 0.1 sec CPU to other threads */
         return UFDB_API_BEING_VERIFIED;
      }

      /* The method is aggressive so this thread is going to wait for another thread to terminate its probe */

      usleep( 25000 + (random() % 29999) );
      s = 0;

      while (status == UFDB_API_BEING_VERIFIED)			
      {
	 status = lookupHTTPScache( hostname, portnumber, 0 );
	 if (status == UFDB_API_BEING_VERIFIED)
	 {
	    usleep( 25000 + (random() % 7777) );
	    s++;
	    if (s == 7)
	    {
	       if (UFDBglobalDebug  ||  flags & UFDB_API_VERBOSE_OUTPUT)
		  ufdbLogMessage( "   UFDBverifyPortHasHTTPS: waiting for other thread on status for %s:%d", 
				  hostname, portnumber );
	    }
	 }
	 else
	 {
	    if (s >= 7)
	    {
	       if (UFDBglobalDebug  ||  flags & UFDB_API_VERBOSE_OUTPUT)
		  ufdbLogMessage( "   UFDBverifyPortHasHTTPS: finished waiting %d intervals for other thread on %s:%d", 
				  s, hostname, portnumber );
	    }
	    return status;
	 }

	 if (s == 120)		/* Aaaargh. I am tired of waiting! */
	 {
	       if (UFDBglobalDebug  ||  flags & UFDB_API_VERBOSE_OUTPUT)
		  ufdbLogMessage( "   UFDBverifyPortHasHTTPS: waited too long for other thread on status for %s:%d",
				  hostname, portnumber );
	       return UFDB_API_BEING_VERIFIED;
	 }
      }
   }

#ifdef UFDB_HTTPS_CACHE_DEBUG
   if (UFDBglobalDebug  ||  (flags & UFDB_API_VERBOSE_OUTPUT))
      ufdbLogMessage( "   UFDBverifyPortHasHTTPS: %s:%d  opening socket...", hostname, portnumber );
#endif

   s = UFDBopenSocket( hostname, portnumber );
   if (s < 0)
   {
      ufdbLogError( "HTTPS protocol verification for %s:%d FAILED: cannot open communication socket",
                     hostname, portnumber );
      updateHTTPScache( hostname, portnumber, UFDB_API_ERR_SOCKET, 0 );
      return UFDB_API_ERR_SOCKET;
   }

   if (UFDBglobalDebug  ||  (flags & UFDB_API_VERBOSE_OUTPUT))
      ufdbLogMessage( "UFDBverifyPortHasHTTPS: socket to %s is opened successfully. fd=%d", hostname, s );

   content = NULL;
   status = UFDB_API_OK;

   if (status == UFDB_API_OK)
   {
      /* TODO: check for vtunnel */

      tv.tv_sec = 3;		/* NOTE: ufdbgclient times out in 12 seconds! but we also have Skype,Gtalk,SSH probes */
      tv.tv_usec = 500000;
      n = setsockopt( s, SOL_SOCKET, SO_RCVTIMEO, (void *) &tv, sizeof(tv) );
      if (n < 0)
         ufdbLogError( "UFDBverifyPortHasHTTPS: cannot set socket timeout to %d seconds: %s", tv.tv_sec, strerror(errno) );
      (void) setsockopt( s, SOL_SOCKET, SO_SNDTIMEO, (void *) &tv, sizeof(tv) );

      /* Setup a TLS connection to connect to the HTTPS port, do a "GET /", 
       * and see what the server has to say.  An error is returned if
       * the other end does not speak SSL.
       */
      status = UFDB_API_OK;
      content = httpsGETroot( s, hostname, portnumber, &status );
      if (UFDBglobalDebug  ||  (flags & UFDB_API_VERBOSE_OUTPUT)  || 
          UFDBglobalDebugAim || UFDBglobalDebugGtalk || UFDBglobalDebugSkype || UFDBglobalDebugYahooMsg)
      {
         ufdbLogMessage( "   httpsGETroot for %s:%d returned status %s, content is %sNULL", 
	 		 hostname, portnumber, ufdbAPIstatusString(status),  content==NULL ? "" : "not " );
      }

      if (content == NULL  ||  *content == '\0')
      {
	 if (status == UFDB_API_OK)
	 {
	    /* We did not read anything from the server...  so we cannot draw a conclusion.
	     * Therefore we return "OK" and hope that the next check gives an answer.
	     */
	    ufdbLogError( "HTTPS server %s:%d is lame.  close fd %d", hostname, portnumber, s );
	    close( s );
	    if (content != NULL)
	       ufdbFree( content );
	    return UFDB_API_OK;
	 }
      }
      else
      if (status != UFDB_API_OK)
      {
         /* we have already an error condition or ERR_IS_AIM|GTALK|SKYPE|YAHOOMSG ; do no more checks */
	 ;
      }
      else
      if (strncmp( content, "HTTP/", 5 ) == 0  ||
          strncmp( content, "<?xml", 5 ) == 0)		/* detect known tunnels */
      {
         if (UFDBglobalDebug)
	 {
	    int   i, j;
	    char  debuginfo[600];
	    /* TODO: put this debug code in a seperate function */

	    for (j = 0, i = 0; content[i] != '\0' && i < 599; i++)
	    {
	       if (content[i] == '\r'  ||  content[i] == '\n')
		  debuginfo[j++] = '_';
	       else if (isprint(content[i]))
		  debuginfo[j++] = content[i];
	       else
		  debuginfo[j++] = '.';
	    }
	    debuginfo[j] = '\0';
	    ufdbLogMessage( "   HTTPS protocol reply for %s:%d:\n   %s", hostname, portnumber, debuginfo );
	 }

#if 0
         if (strstr( content, "X-Kazaa-" ) != NULL)			/* TODO */
	 {
	    ufdbLogError( "HTTPS protocol on %s:%d uses Kazaa P2P",
	    		  hostname, portnumber );
	    status = UFDB_API_ERR_P2P;
	 }
#endif
	 /* TODO: investigate nomachine.com */
	 if (strstr( content, "logmein.com/" ) != NULL  ||
	     strstr( content, "hamachi.cc/" ) != NULL)
	 {
	    ufdbLogError( "HTTPS protocol on %s:%d uses a hamachi/logmein TUNNEL", 
	                  hostname, portnumber );
	    status = UFDB_API_ERR_TUNNEL;
	 }
	 else
	 if (strstr( content, "Set-Cookie: SSLX_SSESHID=" ) != NULL)
	 {
	    ufdbLogError( "HTTPS protocol on %s:%d uses an SSL-Explorer TUNNEL", 
	                  hostname, portnumber );
	    status = UFDB_API_ERR_TUNNEL;
	 }
	 else
	 if (strstr( content, "BarracudaServer.com" ) != NULL   ||
	     strstr( content, "barracudaserver.com" ) != NULL   ||
	     strstr( content, "BarracudaDrive" ) != NULL)
	 {
	    ufdbLogError( "HTTPS protocol on %s:%d uses a BARRACUDA proxy TUNNEL",
	     		  hostname, portnumber );
	    status = UFDB_API_ERR_TUNNEL;
	 }
	 else
	 if (strstr( content, "  index.vnc -" ) != NULL   ||
	     strstr( content, "  used with Xvnc" ) != NULL   ||
	     strstr( content, "TightVNC Java viewer applet" ) != NULL)
	 {
	    ufdbLogError( "HTTPS protocol on %s:%d uses a VNC proxy TUNNEL",
	                  hostname, portnumber );
	    status = UFDB_API_ERR_TUNNEL;
	 }
	 else
	 if (strstr( content, "X-Webtunnel-Status" ) != NULL   ||
	     strstr( content, "X-webtunnel-status" ) != NULL  ||
	     strstr( content, "X-webtunnel-version" ) != NULL  ||
	     strstr( content, "X-Webtunnel-Version" ) != NULL)
	 {
	    ufdbLogError( "HTTPS protocol on %s:%d uses Webtunnel TUNNEL",
	                  hostname, portnumber );
	    status = UFDB_API_ERR_TUNNEL;
	 }
      }
      else					/* server does not speak HTTP */
      {
	 int   i,j;
	 char  debuginfo[80];

	 ufdbLogError( "HTTPS protocol on %s:%d encapsulates a non-HTTP protocol %s",
		       hostname, portnumber,
		       UFDBglobalUnknownProtocolOverHttps ? "and an unknown protocol is allowed" : 
		          "and is considered a PROXY TUNNEL because allow-unknown-protocol-over-https is OFF"
		       );
	 /* TODO: put this code in a separate function */
	 for (j = 0, i = 0; content[i] != '\0' && i < 64; i++)
	 {
	    if (content[i] == '\r'  ||  content[i] == '\n')
	       debuginfo[j++] = '_';
	    else if (isprint(content[i]))
	       debuginfo[j++] = content[i];
	    else
	       debuginfo[j++] = '.';
	 }
	 debuginfo[j] = '\0';
	 ufdbLogMessage( "   HTTPS protocol reply for %s:%d:\n   %s", hostname, portnumber, debuginfo );

	 status = UFDBglobalUnknownProtocolOverHttps ? UFDB_API_ERR_UNKNOWN_PROTOCOL : UFDB_API_ERR_TUNNEL;
      }
   }

   close( s );

   if (UFDBglobalDebug  ||  UFDBglobalLogAllRequests  ||
       UFDBglobalDebugAim || UFDBglobalDebugGtalk || UFDBglobalDebugSkype || UFDBglobalDebugYahooMsg)
   {
      ufdbLogMessage( "HTTPS protocol on %s:%d has been verified and status is %s", 
      		      hostname, portnumber, ufdbAPIstatusString(status) );
   }

   if (UFDBglobalTunnelCheckMethod == UFDB_API_HTTPS_LOG_ONLY)
      status = UFDB_API_OK;

   updateHTTPScache( hostname, portnumber, status, 0 );

   ufdbFree( content );
   return status;
}


static int lookupHTTPScache( 
   char *             hostname, 
   int                portnumber,
   int                keepLockForInsert )
{
   struct httpsInfo * info;
   int                status;
   time_t             now;
   int                removeItem;
   char               key[1024+16];

   sprintf( key, "%s:%d", hostname, portnumber );

   info = (struct httpsInfo *) UFDBsearchHashtable( myht, (void *) key, keepLockForInsert );
   if (info == NULL)
   {
#ifdef UFDB_HTTPS_CACHE_DEBUG
      ufdbLogMessage( "   lookupHTTPScache %s%s is NULL", key, keepLockForInsert ? " keeplock" : "" );
#endif
      return UFDB_API_ERR_NULL;
   }

   now = time( NULL );
   status = info->status;

   removeItem = 0;
   if (info->t < UFDBglobalDatabaseLoadTime)
   {
#ifdef UFDB_HTTPS_CACHE_DEBUG
      ufdbLogMessage( "   lookupHTTPScache %s%s is removed from hash table because the configuration changed", 
		      key, keepLockForInsert ? " keeplock" : "" );
#endif
      removeItem = 1;
   }
   else if (status == UFDB_API_ERR_SOCKET)
   {
      if (info->t < now - 300  ||  
          (info->t < now - 30  &&  UFDBhttpsVerificationQueued() < UFDB_NUM_HTTPS_VERIFIERS))
      {
#ifdef UFDB_HTTPS_CACHE_DEBUG
	 ufdbLogMessage( "   lookupHTTPScache %s%s is removed from hash table because it has aged", 
			 key, keepLockForInsert ? " keeplock" : "" );
#endif
         removeItem = 1;
      }
   }

   if (removeItem)	 /* force the other routines to perform a new evalution of the HTTPS site */
   {
      info = (struct httpsInfo *) UFDBremoveHashtable( myht, key );
      ufdbFree( info );
#ifdef UFDB_HTTPS_CACHE_DEBUG
      ufdbLogMessage( "   lookupHTTPScache %s%s is NULL (removed from hash table)", 
		      key, keepLockForInsert ? " keeplock" : "" );
#endif
      return UFDB_API_ERR_NULL;
   }
   else
      info->t = now;

#ifdef UFDB_HTTPS_CACHE_DEBUG
   ufdbLogMessage( "   lookupHTTPScache %s%s is %d", 
                   key, keepLockForInsert ? " keeplock" : "", status );
#endif

   return status;
}


static void updateHTTPScache( 
   char *             hostname, 
   int                portnumber, 
   int                status,
   int                lockSetBySearch )
{
   time_t             now;
   struct httpsInfo * info;
   char               key[1024+16];

   sprintf( key, "%s:%d", hostname, portnumber );

#ifdef UFDB_HTTPS_CACHE_DEBUG
   ufdbLogMessage( "   updateHTTPScache  %s  %d", key, status );
#endif

   /* The hostname:portnumber is most likely already in the hashlist with status QUEUED.
    * So we only have to update the status...
    * Unless the cache was rebuilt, we need to add the entry to the hash.
    */
   if (lockSetBySearch)
      UFDBunlockHashtable( myht );

   now = time( NULL );
   info = UFDBsearchHashtable( myht, (void *) key, 1 );
   if (info == NULL)
   {
      info = ufdbMalloc( sizeof(struct httpsInfo) );
      info->t = now;
      info->status = status;
      UFDBinsertHashtable( myht, (void *) ufdbStrdup(key), (void *) info, 1 );
   }
   else
   {
      info->status = status;
      info->t = now;
   }
}


static void insertHTTPScache( 
   char *             hostname, 
   int                portnumber, 
   int                status,
   int                lockSetbySearch )
{
   time_t             now;
   struct httpsInfo * info;
   char               key[1024+16];

   sprintf( key, "%s:%d", hostname, portnumber );

#ifdef UFDB_HTTPS_CACHE_DEBUG
   ufdbLogMessage( "   insertHTTPScache  %s  %d  %s", key, status, lockSetbySearch ? "lockSetBySearch" : "" );
#endif

   now = time( NULL );
   info = ufdbMalloc( sizeof(struct httpsInfo) );
   info->t = now;
   info->status = status;
   UFDBinsertHashtable( myht, (void *) ufdbStrdup(key), (void *) info, lockSetbySearch );
}


int select_fd( 
   int      fd, 
   double   maxtime, 
   int      wait_for )
{
   int      result;
   fd_set   fdset;
   fd_set * rd;
   fd_set * wr;
   struct timeval tmout;

   FD_ZERO( &fdset );
   FD_SET( fd, &fdset );
   rd = &fdset;
   wr = &fdset;

   tmout.tv_sec = (long) maxtime;
   tmout.tv_usec = 1000000 * (maxtime - (long) maxtime);

   errno = 0;
   do
      result = select( fd+1, rd, wr, NULL, &tmout );
   while (result < 0  &&  errno == EINTR);

   return result;
}


static int openssl_read( int fd, char * buf, int bufsize, SSL * ssl )
{
   SSL_MUTEX_FN_INIT
   int  ret;

   SSL_MUTEX_LOCK( "openssl_read" )

   errno = 0;
   ret = -2;
   do
   {
      ret = SSL_read( ssl, buf, bufsize );
   }
   while (ret == -1  &&
	  SSL_get_error(ssl, ret) == SSL_ERROR_SYSCALL  &&
	  errno == EINTR);

   SSL_MUTEX_UNLOCK( "openssl_read" )

   return ret;
}


static int openssl_write( int fd, char * buf, int bufsize, SSL * ssl )
{
   SSL_MUTEX_FN_INIT
   int  ret;

   SSL_MUTEX_LOCK( "openssl_write" )

   errno = 0;
   do
   {
      ret = SSL_write( ssl, buf, bufsize );
   }
   while (ret == -1  &&
	  SSL_get_error(ssl,ret) == SSL_ERROR_SYSCALL  &&
	  errno == EINTR);

   SSL_MUTEX_UNLOCK( "openssl_write" )

   return ret;
}


static void openssl_close( SSL * ssl )
{
   SSL_MUTEX_FN_INIT

   if (ssl == NULL)
      return;

   SSL_MUTEX_LOCK( "openssl_close" )

   ERR_clear_error();
   if (SSL_shutdown( ssl ) < 0)
      ERR_clear_error();
   SSL_free( ssl );

   SSL_MUTEX_UNLOCK( "openssl_close" )
}



static int detectSkypeSN( 
   char *          hostname, 
   int             portnumber )
{
   int             n;
   int  	   s;
   struct timeval  tv;
   unsigned char   snbuffer[100] = { 
      0x80, 0x46, 0x01, 0x03, 0x01, 0x00, 0x2d, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x05, 0x00, 0x00,
      0x04, 0x00, 0x00, 0x0a, 0x00, 0x00, 0x09, 0x00, 0x00, 0x64, 0x00, 0x00, 0x62, 0x00, 0x00, 0x08,
      0x00, 0x00, 0x03, 0x00, 0x00, 0x06, 0x01, 0x00, 0x80, 0x07, 0x00, 0xc0, 0x03, 0x00, 0x80, 0x06,
      0x00, 0x40, 0x02, 0x00, 0x80, 0x04, 0x00, 0x80,
      0x16, 0x03, 0x01, 0x00, 0x00, 0x06, 0xa3, 0x66, 0x09, 0x33, 0x5d, 0x3f, 0xf2, 0xb4, 0xca, 0x44
   };
   unsigned char   expected[11] = {
      0x16, 0x03, 0x01, 0x00, 0x4a, 0x02, 0x00, 0x00, 0x46, 0x03, 0x01  /* , 0x4d, 0xbe, 0xde */
   };

   if (UFDBglobalDebug  ||  UFDBglobalDebugSkype)
      ufdbLogMessage( "   probing for Skype node on %s:%d ...", hostname, portnumber );

   s = UFDBopenSocket( hostname, portnumber );
   if (s < 0)
   {
      if (UFDBglobalDebugSkype)
	 ufdbLogMessage( "     socket open failed for %s:%d", hostname, portnumber );
      return UFDB_API_ERR_SOCKET;
   }

   tv.tv_sec = 3;
   tv.tv_usec = 0;
   setsockopt( s, SOL_SOCKET, SO_RCVTIMEO, (void *) &tv, sizeof(tv) );
   tv.tv_sec = 3;
   tv.tv_usec = 0;
   setsockopt( s, SOL_SOCKET, SO_SNDTIMEO, (void *) &tv, sizeof(tv) );

   /* Skype node detection */

   if (72 != write( s, snbuffer, 72 ))
   {
      if (UFDBglobalDebug || UFDBglobalDebugSkype)
	 ufdbLogMessage( "      write failed for %s:%d: %s", hostname, portnumber, strerror(errno) );
      close( s );
      return UFDB_API_ERR_READ;
   }

   n = read( s, snbuffer, 50 );
   close( s );
   
#if UFDB_DEBUG_PROBES
   if (UFDBglobalDebugSkype)
   {
      int i;
      if (n > 20)
         n = 20;
      for (i = 0; i < n; i++)
	 ufdbLogMessage( "      skype node probe reply byte %2d, 0x%02x  %c", i, snbuffer[i], (snbuffer[i] & 0x7f)|0x20 );
   }
#endif

   if (n >= sizeof(expected)  &&  memcmp( snbuffer, expected, sizeof(expected) ) == 0)
   {
      if (UFDBglobalDebug || UFDBglobalDebugSkype)
	 ufdbLogMessage( "      received Skype Node Reply Message from %s:%d", hostname, portnumber );
      return UFDB_API_ERR_IS_SKYPE;
   }

   if (n < 0)
   {
      if (UFDBglobalDebugSkype)
	 ufdbLogMessage( "      read failed from %s:%d: n=%d, %s", hostname, portnumber, n, strerror(errno) );
      return UFDB_API_ERR_READ;
   }
   else
   {
      if (UFDBglobalDebugSkype)
	 ufdbLogMessage( "      got reply of %d bytes to Skype Node probe from %s:%d", n, hostname, portnumber );
#if UFDB_DEBUG_PROBES
      if (UFDBglobalDebugSkype)
      {
	 int i;
	 if (n > 20)
	    n = 20;
	 for (i = 0; i < n; i++)
	    ufdbLogMessage( "      reply byte %2d, 0x%02x  %c", i, snbuffer[i], (snbuffer[i] & 0x7f)|0x20 );
      }
#endif
   }

   return UFDB_API_OK;
}


/* 
 * Detect Gtalk protocol for hostname:port.
 * example of gtalk server on port 443: 209.85.157.126
 * returns  UFDB_API_OK, UFBD_API_ERR_IS_GTALK, UFDB_API_ERR_TUNNEL or UFDB_API_ERR_SOCKET.
 */
int UFDBdetectGtalk( 
   char *          hostname, 
   int             portnumber )
{
   int             s;
   int             n;
   struct timeval  tv;
   unsigned char   loginbuffer[100] = { 
      0x80, 0x46, 0x01, 0x03, 0x01, 0x00, 0x2d, 0x00, 0x00, 0x00, 0x10, 0x01, 0x00, 0x80, 0x03, 0x00, 
      0x80, 0x07, 0x00, 0xc0, 0x06, 0x00, 0x40, 0x02, 0x00, 0x80, 0x04, 0x00, 0x80, 0x00, 0x00, 0x04, 
      0x00, 0xfe, 0xff, 0x00, 0x00, 0x0a, 0x00, 0xfe, 0xfe, 0x00, 0x00, 0x09, 0x00, 0x00, 0x64, 0x00, 
      0x00, 0x62, 0x00, 0x00, 0x03, 0x00, 0x00, 0x06, 
      0x1f, 0x17, 0x0c, 0xa6, 0x2f, 0x00, 0x78, 0xfc, 0x46, 0x55, 0x2e, 0xb1, 0x83, 0x39, 0xf1, 0xea
      };
   unsigned char   expected[30] = {
      0x16, 0x03, 0x01, 0x00, 0x4a, 0x02, 0x00, 0x00, 0x46, 0x03, 0x01, 0x42, 0x85, 0x45, 0xa7, 0x27,
      0xa9, 0x5d, 0xa0, 0xb3, 0xc5, 0xe7, 0x53, 0xda, 0x48, 0x2b, 0x3f, 0xc6, 0x5a, 0xca
      };

   if (UFDBglobalDebug  ||  UFDBglobalDebugGtalk)
      ufdbLogMessage( "   probing for Gtalk on %s:%d ...", hostname, portnumber );

   s = UFDBopenSocket( hostname, portnumber );
   if (s < 0)
   {
      if (UFDBglobalDebug || UFDBglobalDebugGtalk)
	 ufdbLogMessage( "     socket open failed for %s:%d", hostname, portnumber );
      return UFDB_API_ERR_SOCKET;
   }

   tv.tv_sec = 3;
   tv.tv_usec = 0;
   setsockopt( s, SOL_SOCKET, SO_RCVTIMEO, (void *) &tv, sizeof(tv) );
   tv.tv_sec = 3;
   tv.tv_usec = 0;
   setsockopt( s, SOL_SOCKET, SO_SNDTIMEO, (void *) &tv, sizeof(tv) );

   if (72 != write( s, loginbuffer, 72 ))
   {
      if (UFDBglobalDebug || UFDBglobalDebugGtalk)
	 ufdbLogMessage( "      write to %s:%d failed: %s", hostname, portnumber, strerror(errno) );
      close( s );
      return UFDB_API_ERR_SOCKET;
   }

   n = read( s, loginbuffer, 50 );
   close( s );

#if 0
   if (UFDBglobalDebugGtalk)
   {
      int i;
      if (n > 34)
         n = 34;
      for (i = 0; i < n; i++)
	 ufdbLogMessage( "      gtalk reply byte %2d, 0x%02x  %c", i, loginbuffer[i], (loginbuffer[i] & 0x7f)|0x20 );
   }
#endif

   if (n >= sizeof(expected)  &&  memcmp( loginbuffer, expected, sizeof(expected) ) == 0)
   {
      if (UFDBglobalDebug  ||  UFDBglobalDebugGtalk)
	 ufdbLogMessage( "      received Gtalk Reply Message from %s:%d", hostname, portnumber );
      return UFDB_API_ERR_IS_GTALK;
   }

   if (n < 0)
   {
      if (UFDBglobalDebugGtalk)
	 ufdbLogMessage( "      read from %s:%d failed: n=%d, %s", hostname, portnumber, n, strerror(errno) );
      return UFDB_API_ERR_READ;
   }
   else
   {
      if (UFDBglobalDebugGtalk)
	 ufdbLogMessage( "      Got reply of %d bytes to Gtalk probe message from %s:%d.", n, hostname, portnumber );

      if (strncmp( (char *) loginbuffer, "SSH", 3 ) == 0)
      {
	 ufdbLogMessage( "%s:%d responded with SSH so it has a TUNNEL.", hostname, portnumber );
	 return UFDB_API_ERR_TUNNEL;
      }
#if UFDB_DEBUG_PROBES
      else 
      {
	 /* it is not a Gtalk server, print the reply */
         if (UFDBglobalDebugGtalk)
	 {
	    int i;
	    if (n > 34)
	       n = 34;
	    for (i = 0; i < n; i++)
	       ufdbLogMessage( "      gtalk probe reply byte %2d, 0x%02x  %c", i, loginbuffer[i], (loginbuffer[i] & 0x7f)|0x20 );
	 }
      }
#endif
   }

   return UFDB_API_OK;
}


/* 
 * Detect Skype protocol for hostname:port.
 * returns  UFDB_API_OK, UFBD_API_ERR_IS_SKYPE, UFDB_API_ERR_TUNNEL or UFDB_API_ERR_SOCKET.
 */
int UFDBdetectSkype( 
   char *          hostname, 
   int             portnumber )
{
   int             n;
   int  	   s;
   struct timeval  tv;
   unsigned char   loginbuffer[100] = { 
      0x16, 0x03, 0x01, 0x00, 0x00,
      0x80, 0x46, 0x01, 0x03, 0x01, 0x00, 0x2d, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x05, 0x00, 0x00,
      0x04, 0x00, 0x00, 0x0a, 0x00, 0x00, 0x09, 0x00, 0x00, 0x64, 0x00, 0x00, 0x62, 0x00, 0x00, 0x08,
      0x00, 0x00, 0x03, 0x00, 0x00, 0x06, 0x01, 0x00, 0x80, 0x07, 0x00, 0xc0, 0x03, 0x00, 0x80, 0x06,
      0x00, 0x40, 0x02, 0x00, 0x80, 0x04, 0x00, 0x80,
      0x16, 0x03, 0x01, 0x00, 0x00, 0x06, 0xa3, 0x66, 0x09, 0x33, 0x5d, 0x3f, 0xf2, 0xb4, 0xca, 0x44
   };
   unsigned char   expected[5] = {
      0x17, 0x03, 0x01, 0x00, 0x00
   };

   if (UFDBglobalDebug  ||  UFDBglobalDebugSkype)
      ufdbLogMessage( "   probing for Skype login on %s:%d ...", hostname, portnumber );

   s = UFDBopenSocket( hostname, portnumber );
   if (s < 0)
   {
      if (UFDBglobalDebug || UFDBglobalDebugSkype)
	 ufdbLogMessage( "     socket open failed for %s:%d", hostname, portnumber );
      return UFDB_API_ERR_SOCKET;
   }

   tv.tv_sec = 3;
   tv.tv_usec = 0;
   setsockopt( s, SOL_SOCKET, SO_RCVTIMEO, (void *) &tv, sizeof(tv) );
   tv.tv_sec = 3;
   tv.tv_usec = 0;
   setsockopt( s, SOL_SOCKET, SO_SNDTIMEO, (void *) &tv, sizeof(tv) );

   /* Skype login server detection */

   if (72 != write( s, loginbuffer, 72 ))
   {
      if (UFDBglobalDebug || UFDBglobalDebugSkype)
	 ufdbLogMessage( "      write to %s:%d failed: %s", hostname, portnumber, strerror(errno) );
      close( s );
      return UFDB_API_ERR_READ;
   }

   n = read( s, loginbuffer, 30 );
   close( s );
   
#if UFDB_DEBUG_PROBES
   if (UFDBglobalDebugSkype)
   {
      int i;
      if (n > 20)
	 n = 20;
      for (i = 0; i < n; i++)
	 ufdbLogMessage( "      skype server probe reply byte %2d, 0x%02x  %c", i, loginbuffer[i], (loginbuffer[i] & 0x7f)|0x20 );
   }
#endif

   if (n >= sizeof(expected)  &&  memcmp( loginbuffer, expected, sizeof(expected) ) == 0)
   {
      if (UFDBglobalDebug || UFDBglobalDebugSkype)
	 ufdbLogMessage( "      received Skype Login Reply Message from %s:%d", hostname, portnumber );
      return UFDB_API_ERR_IS_SKYPE;
   }

   if (n < 0)
   {
      if (UFDBglobalDebugSkype)
	 ufdbLogMessage( "      read from %s:%d failed: n=%d, %s", hostname, portnumber, n, strerror(errno) );
      return UFDB_API_ERR_READ;
   }
   else
   {
      if (UFDBglobalDebugSkype)
	 ufdbLogMessage( "      Got reply of %d bytes to Skype login probe from %s:%d.", n, hostname, portnumber );

      if (strncmp( (char *) loginbuffer, "SSH", 3 ) == 0)
      {
	 ufdbLogMessage( "%s:%d responded with SSH so it has a TUNNEL.", hostname, portnumber );
	 return UFDB_API_ERR_TUNNEL;
      }
      else 
      {
	 /* it is not a Skype login server, try to detect a Skype node */
	 return detectSkypeSN( hostname, portnumber );
      }
   }

   return UFDB_API_OK;
}


int UFDBdetectSSH( 
   char *          hostname, 
   int             portnumber )
{
   int             n;
   int  	   s;
   struct timeval  tv;
   char            loginbuffer[8];

   if (UFDBglobalDebug)
      ufdbLogMessage( "   probing for SSH server on %s:%d ...", hostname, portnumber );

   s = UFDBopenSocket( hostname, portnumber );
   if (s < 0)
   {
      if (UFDBglobalDebug)
	 ufdbLogMessage( "     opening of socket failed for %s:%d", hostname, portnumber );
      return UFDB_API_ERR_SOCKET;
   }

   tv.tv_sec = 3;
   tv.tv_usec = 0;
   setsockopt( s, SOL_SOCKET, SO_RCVTIMEO, (void *) &tv, sizeof(tv) );
   tv.tv_sec = 3;
   tv.tv_usec = 0;
   setsockopt( s, SOL_SOCKET, SO_SNDTIMEO, (void *) &tv, sizeof(tv) );

   /* SSH server detection */

   n = read( s, loginbuffer, 4 );
   close( s );
   
   if (n >= 3  &&  strncmp( loginbuffer, "SSH", 3 ) == 0)
   {
      if (UFDBglobalDebug)
	 ufdbLogMessage( "      received SSH Server Message from %s:%d", hostname, portnumber );
      return UFDB_API_ERR_TUNNEL;
   }

   return UFDB_API_OK;
}


/* Perform the SSL handshake on file descriptor FD, which is assumed
   to be connected to an SSL server.  The SSL handle provided by
   OpenSSL is registered with the file descriptor FD using
   fd_register_transport, so that subsequent calls to fd_read,
   fd_write, etc., will use the corresponding SSL functions.

   Returns UFDB_API_OK on success  */

static int ssl_connect( 
   char * hostname, 
   int    portnumber, 
   int    fd, 
   SSL ** ssl )
{
   int    ret;
   int    connect_status;
   int    otherStat;
   long   state;

   if (ssl_ctx == NULL) 
   {
      ufdbLogError( "ssl_connect: ssl_ctx is NULL" );
      *ssl = NULL;
      return UFDB_API_ERR_NULL;
   }

   connect_status = UFDB_API_OK;
   ERR_clear_error();
   errno = 0;

   *ssl = SSL_new( ssl_ctx );
   if (*ssl == NULL)
   {
      ufdbLogError( "ssl_connect: SSL_new failed fd=%d", fd );
      connect_status = UFDB_API_ERR_NULL;
      goto error;
   }

   ret = SSL_set_fd( *ssl, fd );
   if (ret == 0)
   {
      ufdbLogError( "ssl_connect: SSL_set_fd failed fd=%d", fd );
      connect_status = UFDB_API_ERR_NULL;
      goto error;
   }

   otherStat = UFDB_API_OK;
   SSL_set_connect_state( *ssl );
   ret = SSL_connect( *ssl );
   state = (*ssl)->state;
   if (ret <= 0  ||  state != SSL_ST_OK)
   {
      int  SSL_error;
      int  old_errno;
      char errstring[256];

      /* tls_uncache_session( ssl_ctx, TLScontext ); ?? */

      connect_status = UFDB_API_ERR_NULL;
      SSL_error = SSL_get_error( *ssl, ret );
      ERR_error_string_n( SSL_error, errstring, sizeof(errstring) );
      old_errno = errno;
      ufdbLogError( "HTTPS/SSL connection: SSL_connect failed for %s:%d.\n   ret=%d  err=%d  state=0x%08x  %s", 
                    hostname, portnumber, ret, SSL_error, state, errstring );

      if (SSL_error != SSL_ERROR_NONE)
      {
         if (hostnameIsIP(hostname))
	 {
	    otherStat = UFDBdetectGtalk( hostname, portnumber );
	    if (otherStat == UFDB_API_ERR_IS_GTALK)
	    {
	       connect_status = otherStat;
	       ufdbLogMessage( "Gtalk detected on %s:%d", hostname, portnumber );
	       goto done;
	    }
	    else if (otherStat == UFDB_API_ERR_TUNNEL)
	    {
	       connect_status = otherStat;
	       ufdbLogMessage( "Tunnel detected on %s:%d", hostname, portnumber );
	       goto done;
	    }

	    if (otherStat == UFDB_API_ERR_SOCKET)
	    {
	       connect_status = otherStat;
	       goto done;
	    }
	    else
	    {
	       otherStat = UFDBdetectSkype( hostname, portnumber );
	       if (otherStat == UFDB_API_ERR_IS_SKYPE)
	       {
		  connect_status = otherStat;
		  ufdbLogMessage( "Skype detected on %s:%d", hostname, portnumber );
		  goto done;
	       }
	       else if (otherStat == UFDB_API_ERR_TUNNEL)
	       {
		  connect_status = otherStat;
		  ufdbLogMessage( "Tunnel detected on %s:%d", hostname, portnumber );
		  goto done;
	       }
	    }
	 }

	 /* NOTE: UFDBdetectSkype() and UFDBdetectGtalk() also detect SSH tunnels,
	  * so we only call UFDBdetectSSH() when hostname is not an IP address.
	  */
         if (otherStat != UFDB_API_ERR_SOCKET  &&
             !hostnameIsIP(hostname)  &&
	     UFDBdetectSSH( hostname, portnumber ) == UFDB_API_ERR_TUNNEL)
	 {
	    ufdbLogMessage( "SSH detected on %s:%d", hostname, portnumber );
	    connect_status = UFDB_API_ERR_TUNNEL;
	    goto done;
	 }

	 /* we have an unknown protocol.  It is dealt with below */
      }

      if (SSL_error == SSL_ERROR_WANT_READ)
      {
	 /* 
	  * We assume that there is a timeout (the socket timeout is aggressive)
	  * and therefore set the verification state to UFDB_API_ERR_SOCKET
	  * so it can be re-verified at a later time.
	  */
         connect_status = UFDB_API_ERR_SOCKET;
	 SSL_error = SSL_ERROR_NONE;
	 goto error;
      }

      if (SSL_error == SSL_ERROR_SYSCALL)
      {
         connect_status = UFDB_API_ERR_SOCKET;
         ufdbLogError( "system call by SSL library for %s:%d produced error %d: %s",
	               hostname, portnumber, old_errno, strerror(old_errno) );
	 goto error;
      }

      if (UFDBglobalUnknownProtocolOverHttps)
      {
	 connect_status = UFDB_API_ERR_UNKNOWN_PROTOCOL;
	 ufdbLogMessage( "Unknown Protocol is used on %s:%d", hostname, portnumber );
	 goto done;
      }
      else
      {
	 /* There is an unknown protocol and the option allow-unknown-protocol-over-https=off */
	 connect_status = UFDB_API_ERR_TUNNEL;
	 ufdbLogMessage( "forbidden Unknown Protocol used on %s:%d so it is flagged as a TUNNEL *****", 
			 hostname, portnumber  );
         goto error;
      }
   }

   if (connect_status == UFDB_API_OK  &&  *ssl != NULL)
   {
      if ((*ssl)->version == SSL2_VERSION  &&  UFDBglobalHttpsNoSSLv2)
      {
	 ufdbLogError( "HTTPS/SSL connection for %s:%d has an insecure SSLv2 protocol. It is blocked. *****",
		       hostname, portnumber );
	 openssl_close( *ssl );
	 return UFDB_API_ERR_INVALID_CERT;
      }
   }

   return UFDB_API_OK;

error:
   print_errors();
done:
   openssl_close( *ssl );
   return connect_status;
}


/* Return 1 if hostname (case-insensitively) matches common, 0
   otherwise.  The recognized wildcard character is "*", which matches
   any character in common except ".".  

   This is used to match of hosts as indicated in rfc2818: "Names may
   contain the wildcard character * which is considered to match any
   single domain name component or component fragment. E.g., *.a.com
   matches foo.a.com but not bar.foo.a.com. f*.com matches foo.com but
   not bar.com [or foo.bar.com]."

   If the pattern contain no wildcards, matchHostname(a,b) is
   equivalent to !strcasecmp(a,b)
*/

static int matchHostname( 
   const char * common, 
   const char * hostname )
{
   const char * c = common;
   const char * h = hostname;
   char         ch;

#if 0
   if (UFDBglobalDebug)
      ufdbLogMessage( "      matchHostname( %s, %s )", common, hostname );
#endif

   for (; (ch = tolower(*c++)) != '\0'; h++)
   {
      if (ch == '*')
      {
	 for (ch = tolower(*c); ch == '*'; ch = tolower(*++c))
	    ;
	 for (; *h != '\0'; h++)
	 {
	    if (tolower(*h) == ch  &&  matchHostname(c,h))
	       return 1;
	    else if (*h == '.')
	       return 0;
	 }
	 return ch == '\0';
      }
      else
      {
	 if (ch != tolower(*h))
	    return 0;
      }
   }

   return (*h == '\0');
}


#ifdef UFDB_LOAD_CERT_CHAIN

static X509 * convert_bytes_to_cert(
   unsigned char * bytes,
   int             nbytes )
{
   X509 *          cert;

   if (UFDBglobalDebug)
      ufdbLogMessage( "      convert_bytes_to_cert: certificate has %d bytes", nbytes );

#if 0
   BIO *  in;
   if (!(in = BIO_new_mem_buf( bytes, nbytes ))
   {
      ufdbLogError( "cannot convert bytes to certificate. BIO_new_mem_buf failed" );
      return NULL;
   }
#endif

   /* certificate format is FORMAT_ASN1 or FORMAT_PEM or FORMAT_NETSCAPE */
   if (bytes[0] == '-'  &&  bytes[1] == '-')				/* FORMAT_PEM */
   {
      /* x = PEM_read_bio_X509_AUX(cert,NULL, (pem_password_cb *)password_callback, NULL); */
      ufdbLogError( "cannot convert bytes to certificate. PEM_read_bio_X509_AUX not yet implemented" );
   }
   else									/* FORMAT_ASN1 (DER) */
   {
      /* d2i = DER/BER to internal */
      cert = d2i_X509( NULL, (unsigned char **) &bytes, nbytes );
      if (cert == NULL)
      {
	 ufdbLogError( "cannot convert bytes to certificate. d2i_X509 failed" );
	 print_errors();
	 return NULL;
      }
      return cert;
   }

   {
      int i;
      if (nbytes > 100) nbytes = 100;
      for (i = 0; i < nbytes; i++)
	 ufdbLogMessage( "      byte %2d, 0x%02x  %c", i, bytes[i], (bytes[i] & 0x7f)|0x20 );
   }

   ufdbLogError( "cannot convert bytes to certificate" );
   return NULL;
}
#endif


static const char * knownEVcertIssuerOIDTable[] =
{
	"1.1.1.1.1",					/* Test */
	"1.3.6.1.4.1.6449.1.2.1.5.1",			/* Comodo */
	"1.3.6.1.4.1.6334.1.100.1",			/* Cybertrust */
	"2.16.840.1.114412.2.1",			/* DigiCert */
	"2.16.840.1.114028.10.1.2",			/* Entrust */
	"1.3.6.1.4.1.14370.1.6",			/* GeoTrust */
	"2.16.840.1.114413.1.7.23.3",			/* Go Daddy */
	"1.3.6.1.4.1.8024.0.2.100.1.2",		        /* QuoVadis */
	"2.16.840.1.114414.1.7.23.3",			/* Starfield Technologies */
	"2.16.840.1.113733.1.7.48.1",			/* Thawte */
	"2.16.840.1.113733.1.7.23.6",			/* VeriSign */
	NULL
};


int certificateIsEV(
   X509 *                  cert )
{
   POLICYINFO *            pinfo;
   STACK_OF(POLICYINFO) *  policies;
   char *                  oid;
   int                     idlen;
   int                     i, j;
   
   /* EV certificates are certificates with a well-known OID in the "Certificate Policies" 
    * extension field.
    */
   
   if ((policies = (STACK_OF(POLICYINFO) *) X509_get_ext_d2i( cert, NID_certificate_policies, NULL, NULL)) == NULL)
   {
      return 0;
   }
   
   for (i = 0; i < sk_POLICYINFO_num(policies); i++)
   {
      if ((pinfo = sk_POLICYINFO_value(policies,i)) == NULL)
      {
         continue;
      }
      
      if ((idlen = i2t_ASN1_OBJECT( NULL, 0, pinfo->policyid )) <= 0)
      {
         continue;
      }
      
      oid = ufdbMalloc( idlen + 1 );
      if (i2t_ASN1_OBJECT( oid, idlen + 1, pinfo->policyid ) != idlen)
      {
         ufdbLogError( "cannot convert certificate OID" );
      }
      
      if (UFDBglobalDebug)
	 ufdbLogMessage( "X.509 policy extension OID = %s", oid );
         
      for (j = 0; knownEVcertIssuerOIDTable[j] != NULL; j++)
      {
         if( strcmp( oid, knownEVcertIssuerOIDTable[j] ) == 0)
         {
	    if (UFDBglobalDebug)
	       ufdbLogMessage( "Certificate is EV. Issuer is %s", oid );
	    ufdbFree( oid );
	    sk_POLICYINFO_pop_free( policies, POLICYINFO_free );
	    return 1;
         }
      }
      
      ufdbFree( oid );
   }

   sk_POLICYINFO_pop_free( policies, POLICYINFO_free );
   return 0;
}


#ifdef UFDB_LOAD_CERT_CHAIN

static X509 * LoadCertificateByURL(
   char * URL )
{
   int    s;
   int    n;
   int    port;
   char * path;
   char * p;
   X509 * cert;
   char   protocol[16];
   char   domain[1024];
   char   strippedUrl[UFDB_MAX_URL_LENGTH];
   char   request[1024+UFDB_MAX_URL_LENGTH];
   char   certbuffer[1024*16];

   port = 80;
   UFDBstripURL( URL, strippedUrl, domain, protocol, &port );
   if (strcmp(protocol,"http") != 0  &&  port != 80)
   {
      ufdbLogError( "cannot load certificate of %s since it has no HTTP protocol", URL );
      return NULL;
   }

   s = UFDBopenSocket( domain, port );
   if (s < 0)
   {
      ufdbLogError( "cannot load certificate of %s", URL );
      return NULL;
   }

   path = strchr( URL, ':' );		/* strip http:// */
   if (path == NULL)
      path = URL;
   else
      path += 3;
   path = strchr( path, '/' );		/* strip domainname */
   if (path == NULL)
      path = "/";

   /*******************************
   GET / HTTP/1.0
   User-Agent: Mozilla/5.0 (xxx) Gecko/20100722 Firefox/3.6.8
   Host: www.urlfilterdb.com:9443
   Accept: * / *
   Connection: Keep-Alive
   ********************************/

   if (UFDBglobalDebug)
      ufdbLogMessage( "   LoadCertificateByURL %s %s", domain, path );

   /* TO-DO: if there is a proxy, "CONNECT" must be used */
   sprintf( request, "GET %s HTTP/1.1\r\n"
                     "User-Agent: " UFDB_USER_AGENT "\r\n"
		     "Host: %s:%d\r\n"
		     "Accept: */*\r\n"
		     "Connection: Close\r\n"
		     "\r\n",
		     path,
		     domain, port );
   n = strlen( request );
   if (write( s, request, n ) != n)
   {
      ufdbLogError( "cannot retrieve certificate for %s: write failed: %s", URL, strerror(errno) );
      close( s );
      return NULL;
   }

   n = UFDBread( s, certbuffer, sizeof(certbuffer) );
   if (n < 0)
   {
      ufdbLogError( "cannot retrieve certificate for %s: read failed: %s", URL, strerror(errno) );
      close( s );
      return NULL;
   }
   close( s );
   p = strstr( certbuffer, "\r\n\r\n" );
   if (p != NULL)
   {
      if (UFDBglobalDebug)
	 ufdbLogMessage( "      downloaded file from %s has no <CR><LF<CR><LF> separator", URL );
      n -= (p - certbuffer) - 4;
      p += 4;
   }
   else
      p = certbuffer;
   if (UFDBglobalDebug)
      ufdbLogMessage( "      certificate has %d bytes", n );

   /* And now begins the real magic: add the certificate to the certificate list.
    * It is a recursive process since a EV chain usually adds more than one certificate.
    */
   cert = convert_bytes_to_cert( (unsigned char *) p, n );
   if (cert == NULL)
      return cert;

   if (UFDBglobalDebug)
   {
      char issuer[1024];
      char subject[1024];

      issuer[0] = '\0';
      (void) X509_NAME_oneline( X509_get_issuer_name(cert), issuer, 1023 );
      issuer[1023] = '\0';

      subject[0] = '\0';
      (void) X509_NAME_oneline( X509_get_subject_name(cert), subject, 1023 );
      subject[1023] = '\0';

      ufdbLogMessage( "         issuer is %s\nsubject is %s", issuer, subject );
   }
   UFDBloadIntermediateCertificates( cert );

   return cert;
   /* TODO: after validation: add URL to a cache to prevent multiple downloads */
}
#endif


#ifdef UFDB_LOAD_CERT_CHAIN

static void CAaddURL( 
   STACK_OF(STRING) ** list, 
   ASN1_IA5STRING * url )
{
   X509 * cert;
   char * tmp;

   if (url->type != V_ASN1_IA5STRING) 
      return;
   if (!url->data || !url->length) 
      return;

#ifdef UFDB_STORE_X509_URIS
   if (!*list) 
      *list = sk_ASN1_STRING_TABLE_new( *list );
   if (!*list) 
      return;

   /* prevent duplicates */
   if (sk_ASN1_STRING_TABLE_find( *list, (char *) url->data ) != -1) 
      return;
   tmp = ufdbStrdup( (char *) url->data );
#else
   tmp = (char *) url->data;
#endif

   if (UFDBglobalDebug)
      ufdbLogMessage( "      CAaddURL %s", tmp );

   if (!ufdbCacertsLoaded)
      return;

#if 1
   return;   /* just for now: do not load the certificate */
#endif

   cert = LoadCertificateByURL( tmp );
   if (cert == NULL)
   {
      ufdbLogMessage( "error parsing certificate from %s", tmp );
   }
   else
   {
      X509_free( cert );
   }

#ifdef UFDB_STORE_X509_URIS
   if (!tmp || !sk_ASN1_STRING_TABLE_push(*list, tmp)) 
   {
      X509_email_free( *list );
      *list = NULL;
   }
#endif
}

#endif


#ifdef UFDB_LOAD_CERT_CHAIN

static int UFDBloadIntermediateCertificates(
   X509 * cert )
{
   AUTHORITY_INFO_ACCESS * aia_data;
   ACCESS_DESCRIPTION *    aia_descr;
   STACK_OF(STRING) *      ocsp_list;
   int                     i;
   int                     is_ev;

   aia_data = X509_get_ext_d2i( cert, NID_info_access, NULL, NULL );

   if (UFDBglobalDebug)
      ufdbLogMessage( "UFDBloadIntermediateCertificates: aia_data %s", (aia_data==NULL?"is NULL":"has data") );

   if (aia_data == NULL)
      return 0;

   is_ev = certificateIsEV( cert );
   ocsp_list = NULL;
   if (UFDBglobalDebug)
      ufdbLogMessage( "   #aia_data = %d", sk_ACCESS_DESCRIPTION_num(aia_data) );
   for (i = 0; i < sk_ACCESS_DESCRIPTION_num(aia_data); i++)
   {
      int nid;
      aia_descr = sk_ACCESS_DESCRIPTION_value( aia_data, i );
      nid = OBJ_obj2nid(aia_descr->method);
#if 0
      if (UFDBglobalDebug && nid != NID_ad_OCSP && nid != NID_ad_ca_issuers)
	 ufdbLogMessage( "   OBJ_obj2nid = %d", nid );
#endif
      /* We do not do OCSP.  This is left for the browser. */
      if (nid == NID_ad_ca_issuers)
      {
         if (aia_descr->location->type == GEN_URI)
	 {
	    if (UFDBglobalDebug)
	       ufdbLogMessage( "   certificate AIA CA-ISSUERS URI:" );
	    CAaddURL( &ocsp_list, aia_descr->location->d.uniformResourceIdentifier );
	    is_ev = 1;
         }
      }
   }

   /* TODO: download and verify intermediate certificates: UFDB_STORE_X509_URIS */
#ifdef UFDB_STORE_X509_URIS
   X509_email_free( ocsp_list );
#endif

   return is_ev;
}

#endif


static int CertificatesHasAIA(
   X509 * cert )
{
   AUTHORITY_INFO_ACCESS * aia_data;
   ACCESS_DESCRIPTION *    aia_descr;
   int                     i;
   int                     is_ev;

   aia_data = X509_get_ext_d2i( cert, NID_info_access, NULL, NULL );

   if (UFDBglobalDebug)
      ufdbLogMessage( "CertificatesHasAIA: aia_data %s", (aia_data==NULL?"is NULL":"has data") );

   if (aia_data == NULL)
      return 0;

   is_ev = 0;
   for (i = 0; i < sk_ACCESS_DESCRIPTION_num(aia_data); i++)
   {
      int nid;
      aia_descr = sk_ACCESS_DESCRIPTION_value( aia_data, i );
      nid = OBJ_obj2nid(aia_descr->method);
      if (nid == NID_ad_ca_issuers)
      {
         if (aia_descr->location->type == GEN_URI)
         {
	    if (UFDBglobalDebug)
	       ufdbLogMessage( "   URI: %s", aia_descr->location->d.uniformResourceIdentifier->data );
	    if (strncasecmp( "http", (char*) aia_descr->location->d.uniformResourceIdentifier->data, 4 ) == 0)
	       is_ev = 1;
	 }
      }
   }

   sk_ACCESS_DESCRIPTION_pop_free( aia_data, ACCESS_DESCRIPTION_free );

   return is_ev;
}


#if 0
static int my_verify_cert( 
   X509_STORE *     trustedCertStore, 
   X509 *           cert, 
   STACK_OF(X509) * certStack )
{
   X509_STORE_CTX * csc;
   int              retval;

   if (UFDBglobalDebug)
      ufdbLogMessage( "my_verify_cert" );

   csc = X509_STORE_CTX_new();
   if (csc == NULL)
   {
      ufdbLogError( "cannot verify certificate.  X509_STORE_CTX_new failed" );
      /* we simply don't know if the certificate is OK or not... OK */
      return X509_V_OK;
   }
   if (!X509_STORE_CTX_init( csc, trustedCertStore, cert, certStack ))
   {
      ufdbLogError( "cannot verify certificate.  X509_STORE_CTX_init failed" );
      /* we simply don't know if the certificate is OK or not... OK */
      return X509_V_OK;
   }
   /* X509_VERIFY_PARAM_set_flags( X509_V_FLAG_CRL_CHECK | X509_V_FLAG_CRL_CHECK_ALL ); */
   X509_STORE_CTX_set_purpose( csc,  X509_PURPOSE_SSL_SERVER );		/* or ??? X509_PURPOSE_SSL_SERVER */

#if 0
   X509_STORE_CTX_set_verify_cb( csc, certVerifycallback );
#endif

   retval = X509_verify_cert( csc );
   if (retval > 0)
   {
      if (UFDBglobalDebug)
	 ufdbLogMessage( "my_verify_cert: certificate is OK" );
      retval = X509_V_OK;
   }
   else
   {
      retval = X509_STORE_CTX_get_error( csc );
      if (UFDBglobalDebug)
      {
	 ufdbLogError( "my_verify_cert: certificate verification failed\n"
	               "certificate error code is %d", 
		       retval );
	 print_errors();
      }
   }
   X509_STORE_CTX_free( csc );

   return retval;
}
#endif


/* ssl_dns_name - Extract valid DNS name from subjectAltName value */
static const char * ssl_dns_name( 
   const GENERAL_NAME * gn )
{
   const char * dnsname;
   int          len;

   /*
    * We expect the OpenSSL library to construct GEN_DNS extension objects as
    * ASN1_IA5STRING values. Check we got the right union member.
    */
   if (ASN1_STRING_type(gn->d.ia5) != V_ASN1_IA5STRING) 
   {
      ufdbLogError( "ssl_dns_name: invalid ASN1 value type in subjectAltName of SSL certificate" );
      return NULL;
   }

#define TRIM0(s, l) do { while ((l) > 0 && (s)[(l)-1] == 0) --(l); } while (0)
   /*
    * Safe to treat as an ASCII string possibly holding a DNS name
    */
   dnsname = (char *) ASN1_STRING_data( gn->d.ia5 );
   len = ASN1_STRING_length( gn->d.ia5 );
   TRIM0( dnsname, len );

   /*
    * Per Dr. Steven Henson of the OpenSSL development team, ASN1_IA5STRING
    * values can have internal ASCII NUL values in this context because
    * their length is taken from the decoded ASN1 buffer, a trailing NUL is
    * always appended to make sure that the string is terminated, but the
    * ASN.1 length may differ from strlen().
    */
   if (len != strlen(dnsname)) 
   {
      ufdbLogError( "ssl_dns_name: internal NUL in subjectAltName of SSL certificate" );
      return NULL;
   }

   return dnsname;
}


/* return values:    UFDB_API_OK
 *                   UFDB_API_ERR_INVALID_CERT
 *		     UFDB_API_ERR_IS_YAHOOMSG
 *		     UFDB_API_ERR_IS_AIM
 */
static int ssl_check_certificate( 
   SSL *         ssl, 
   const char *  hostname,
   int           portnumber )
{
   const char *  altPtr;
   X509 *        cert;
   long          vresult;
   int           success;
   int		 is_chained;
   int           is_ev;
   int           match;
   int           altNameSeen;
   char          issuer[1024];
   STACK_OF(X509) * certStack;
   STACK_OF(GENERAL_NAME) * altNames;

   success = 2;

   cert = SSL_get_peer_certificate( ssl );
   certStack = SSL_get_peer_cert_chain( ssl );

   if (cert == NULL)
   {
      success = 0;
      ufdbLogError( "site %s:%d has NO SSL certificate !", hostname, portnumber );
      goto no_cert;		/* must bail out since CERT is NULL */
   }

   issuer[0] = '\0';
   (void) X509_NAME_oneline( X509_get_issuer_name(cert), issuer, 1023 );
   issuer[1023] = '\0';
   if (issuer[0] == '\0')
   {
      strcpy( issuer, "--no-issuer-found-in-certificate--" );
      success = 0;
   }

#if 0
   UFDBloadIntermediateCertificates( cert );
#endif

   is_ev = CertificatesHasAIA( cert )  ||  certificateIsEV( cert );
   is_chained = (sk_X509_num(certStack) > 1);

#if 0
   if (UFDBglobalDebug || UFDBglobalDebugAim)
   {
      ufdbLogMessage( "   issuer: %s\nhostnameIsIP(%s): %d", issuer, hostname, hostnameIsIP(hostname) );
   }
#endif

   /* Yuck! AIM connects with a (chained) certificate but does not allow HTTP/1.1 */
   if (strstr( issuer, "/O=America Online Inc./CN=AOL Member CA" ) != NULL)
   {
      ufdbLogMessage( "SSL certificate on %s:%d is used for AIM", hostname, portnumber );
      X509_free( cert );
      return UFDB_API_ERR_IS_AIM;
   }

#if 0
   if (certStack != NULL)
   {
      AUTHORITY_INFO_ACCESS * aia_data;

      /* the server uses a chain of certificates ? */
      if (UFDBglobalDebug)
      {
	 int i;
         ufdbLogMessage( "size of stack of certificates is %d", sk_X509_num(certStack) );
	 for (i = 0; i < sk_X509_num(certStack); i++)
	 {
	    X509 * x;
	    char   buffer[512];

	    x = sk_X509_value( certStack, i );
	    (void) X509_NAME_oneline( X509_get_subject_name(x), buffer, sizeof(buffer) );
	    ufdbLogMessage( "   %s", buffer );
	 }
      }
      aia_data = X509_get_ext_d2i( cert, NID_info_access, NULL, NULL );
      is_ev = (aia_data != NULL);
   }
#endif

   if (UFDBglobalDebug)
   {
      char subject[1024];

      ufdbLogMessage( "   %s%sSSL certificate for %s has issuer '%s'", 
		      is_chained ? "chained " : "",
                      is_ev ? "EV " : "",
		      hostname, issuer );
      subject[0] = '\0';
      (void) X509_NAME_oneline( X509_get_subject_name(cert), subject, 1023 );
      subject[1023] = '\0';
      if (subject[0] == '\0')
      {
	 strcpy( subject, "--no-subject-found-in-certificate--" );
      }
      ufdbLogMessage( "   %s%sSSL certificate for %s has subject '%s'", 
		      is_chained ? "chained " : "",
                      is_ev ? "EV " : "",
                      hostname, subject );
   }

   if (!ufdbCacertsLoaded)
   {
      ufdbLogMessage( "No CA certificates are loaded.  Cannot verify signature of "
                      "%s%scertificate for %s:%d.  Marking it as invalid.", 
		      is_chained ? "chained " : "",
		      is_ev ? "EV " : "",
		      hostname, portnumber );
      /* TODO: investigate free cert chain */
      X509_free( cert );
      return UFDB_API_ERR_INVALID_CERT;
   }

#if 1
   vresult = SSL_get_verify_result( ssl );
#else
   /* optimize STORE creation: can we re-use it ? */
   cert_ctx = X509_STORE_new();
   /* X509_STORE_set_verify_cb_func( cert_ctx, cb ); */
   /* X509_STORE_set1_param( cert_ctx, (X509_VERIFY_PARAM *) NULL ); */
   if (is_ev)
      X509_STORE_set_flags( cert_ctx, X509_V_FLAG_CRL_CHECK | X509_V_FLAG_CRL_CHECK_ALL );
   lookup = X509_STORE_add_lookup( cert_ctx, X509_LOOKUP_file() );
   if (!X509_LOOKUP_load_file( lookup, UFDBglobalCAcertsFile, X509_FILETYPE_PEM ))
      ufdbLogError( "X509_LOOKUP_load_file failed for %s", UFDBglobalCAcertsFile );
   ERR_clear_error();
   vresult = my_verify_cert( cert_ctx, cert, certStack );
#endif

   if (vresult == X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT_LOCALLY)
   {
      if (ufdbCacertsLoaded)
      {
	 if (is_ev || is_chained)
	 {
	    success = 1;
	    ufdbLogMessage( "%s%sSSL certificate signature of %s:%d cannot be verified, assuming it is OK", 
			    is_chained ? "chained " : "",
			    is_ev ? "EV " : "",
	       		    hostname, portnumber );
	 }
	 else
	 {
	    ufdbLogError( "%s%sSSL certificate for %s:%d: UNRECOGNISED ISSUER *****\nissuer: %s",
			  is_chained ? "chained " : "",
			  is_ev ? "EV " : "",
	                  hostname, portnumber,
			  issuer );
	    success = 0;
	 }
      }
      else
	 if (UFDBglobalDebug)
	 {
	    ufdbLogError( "SSL_get_verify_result() is X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT_LOCALLY (ignored)\n"
	                  "Check the existence and file permissions of %s", 
			  UFDBglobalCAcertsFile );
	 }
   }
   else if (vresult != X509_V_OK)
   {
      success = 0;
      switch (vresult)
      {
      case X509_V_ERR_SELF_SIGNED_CERT_IN_CHAIN:
	 ufdbLogError( "SSL certificate for %s:%d has a SELF-SIGNED CERTIFICATE in chain *****\nissuer: %s",
		       hostname, portnumber, issuer );
	 break;
      case X509_V_ERR_DEPTH_ZERO_SELF_SIGNED_CERT:
	 /* Yuck! Yahoo IM connect to addresses like <IP>:443 with a self-signed certificate. */
	 if (strstr( issuer, "/O=Yahoo/OU=Messenger/CN=undermine.corp/" ) != NULL)
	 {
	    ufdbLogMessage( "SSL certificate on %s:%d is used for Yahoo IM", hostname, portnumber );
	    X509_free( cert );
	    return UFDB_API_ERR_IS_YAHOOMSG;
	 }
	 else
	 {
	    ufdbLogError( "SSL certificate for %s:%d has a DEPTH-ZERO SELF-SIGNED CERTIFICATE *****\nissuer: %s",
			  hostname, portnumber, issuer );
	 }
	 break;
      case X509_V_ERR_CERT_NOT_YET_VALID:
	 ufdbLogError( "SSL certificate for %s:%d has a NOT YET VALID DATE *****", hostname, portnumber );
	 break;
      case X509_V_ERR_CERT_HAS_EXPIRED:
	 ufdbLogError( "SSL certificate for %s:%d has EXPIRED *****", hostname, portnumber );
	 break;
      case X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT_LOCALLY:
	 ufdbLogError( "SSL certificate for %s:%d has an UNRECOGNISED ISSUER *****\nissuer: %s", 
		       hostname, portnumber, issuer );
	 break;
      case X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT:
         ufdbLogMessage( "%s%sSSL certificate for %s:%d cannot be verified.", 
			 is_chained ? "chained " : "",
			 is_ev ? "EV " : "",
	                 hostname, portnumber );
	 break;
      case X509_V_ERR_UNABLE_TO_DECRYPT_CERT_SIGNATURE:
         ufdbLogMessage( "%s%sSSL certificate for %s:%d CANNOT DECRYPT SIGNATURE.", 
			 is_chained ? "chained " : "",
			 is_ev ? "EV " : "",
	                 hostname, portnumber );
	 break;
      case X509_V_ERR_CERT_SIGNATURE_FAILURE:
	 if (is_chained || is_ev)
	 {
	    success = 1;
	    ufdbLogMessage( "%s%sSSL certificate for %s:%d has an unresolvable certificate signature failure.\n"
	    		    "assuming that the signature is OK.", 
			    is_chained ? "chained " : "",
			    is_ev ? "EV " : "",
			    hostname, portnumber );
	 }
	 else
	    ufdbLogMessage( "%s%sSSL certificate for %s:%d CERTIFICATE SIGNATURE FAILURE.", 
			    is_chained ? "chained " : "",
			    is_ev ? "EV " : "",
			    hostname, portnumber );
	 break;
      default:
	 ufdbLogError( "SSL certificate VERIFICATION ERROR for %s:%d %d %s *****",
		       hostname, portnumber, vresult, X509_verify_cert_error_string(vresult) );
      }
      /* Fall through */
   }

   if (success>1 && UFDBglobalDebug)
      ufdbLogMessage( "   %s%sSSL certificate for %s:%d is signed by a CA (OK)", 
		      is_chained ? "chained " : "",
		      is_ev ? "EV " : "",
                      hostname, portnumber );

   /* 
    * Check that hostname matches the common name in the certificate.
    * rfc2818:
    * "If a subjectAltName extension of type dNSName is present, 
    * that MUST be used as the identity."
    * If not, we will use the commonName as the identity.
    */
   match = 0;
   
   altNameSeen = 0;

   altNames = X509_get_ext_d2i( cert, NID_subject_alt_name, NULL, NULL );
   if (altNames)
   {
      const GENERAL_NAME * check;
      int numAltNames;
      int i;

      altPtr = "-";
      numAltNames = sk_GENERAL_NAME_num( altNames );
      for (i = 0; i < numAltNames; i++)
      {
	 check = sk_GENERAL_NAME_value( altNames, i );
	 if (check->type == GEN_DNS)
	 {
	    altPtr = ssl_dns_name( check );
	    altNameSeen = 1;
	    if (matchHostname( altPtr, hostname ))
	    {
	       match = 1;
	       break;
	    }
	 }
      }

      if (altNameSeen)
      {
	 if (!match)
	 {
	    ufdbLogError( "SSL certificate has subjectAltName \"%s\" which does NOT MATCH hostname \"%s\"", 
	                  altPtr, hostname );
	    success = 0;
	 }
	 else
	    if (UFDBglobalDebug)
	       ufdbLogMessage( "   SSL certificate with subjectAltName \"%s\" matches hostname \"%s\"", 
	                       altPtr, hostname );
      }
      sk_GENERAL_NAME_pop_free( altNames, GENERAL_NAME_free );
   }
   
   if (!altNameSeen)
   {
      int             i, j;
      X509_NAME *     name;
      unsigned char * nulstr = (unsigned char *) "";
      unsigned char * commonName = nulstr;

      if (UFDBglobalDebug)
	 ufdbLogMessage( "SSL certificate for %s:%d has no subjectAltName", hostname, portnumber );

      i = -1;
      name = X509_get_subject_name( cert );
      if (name != NULL)
         while ((j=X509_NAME_get_index_by_NID(name,NID_commonName,i)) >= 0)
	    i = j;

      /* now we have the name entry and convert it to a string */
      if (i >= 0)
      {
         ASN1_STRING * tmp;

	 tmp = X509_NAME_ENTRY_get_data( X509_NAME_get_entry(name,i) );

	 /* In OpenSSL 0.9.7d and earlier ASN1_STRING_to_UTF8 fails if string is already UTF8 :-) */
#if defined(OPENSSL_VERSION_NUMBER)
#if OPENSSL_VERSION_NUMBER <= 0x0090704fL
	 if (UFDBglobalDebug)
	    ufdbLogMessage( "   OpenSSL library version is %08X", OPENSSL_VERSION_NUMBER );
	 if (tmp != NULL  &&  ASN1_STRING_type(tmp) == V_ASN1_UTF8STRING)
	 {
	    j = ASN1_STRING_length( tmp );
	    if (j >= 0)
	    {
	       commonName = ufdbMalloc( j+1 );
	       memcpy( commonName, ASN1_STRING_data(tmp), j );
	       commonName[j] = '\0';
	    }
	 }
	 else
	 {
	    j = ASN1_STRING_to_UTF8( &commonName, tmp );
	 }
#else
         j = ASN1_STRING_to_UTF8( &commonName, tmp );
#endif
#else
         j = ASN1_STRING_to_UTF8( &commonName, tmp );
#endif
      }

      if (commonName == nulstr)
	 commonName = NULL;
      if (commonName == NULL)
      {
	 ufdbLogError( "SSL certificate for %s:%d has NO \"COMMON NAME\" *****", hostname, portnumber );
	 success = 0;
      }
      else
      {
	 if ( !matchHostname( (char *) commonName, hostname ))
	 {
	    ufdbLogError( "SSL certificate common name `%s' does NOT MATCH hostname `%s' *****",
			  commonName, hostname );
	    success = 0;
	 }
         OPENSSL_free( commonName );
      }
   }

   print_errors();		/* verify why this is here ... */

   if (success && UFDBglobalDebug)
      ufdbLogMessage( "SSL certificate matches hostname %s", hostname );

   /* TODO: investigate free cert chain */
   X509_free( cert );

no_cert:
   return success ? UFDB_API_OK : UFDB_API_ERR_INVALID_CERT;
}

