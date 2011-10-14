/*
 * ufdb.h - URLfilterDB
 *
 * ufdbGuard is copyrighted (C) 2005-2011 by URLfilterDB with all rights reserved.
 *
 * Parts of the ufdbGuard daemon are based on squidGuard.
 * This module is NOT based on squidGuard.
 *
 * RCS $Id: ufdb.h.in,v 1.19 2011/06/03 01:59:54 root Exp root $
 */

/* 
 * An ordered table looks like this:
 *
 * UNIVERSE
 *         com 
 *             foobar
 *                    girls
 *                    boys
 *             pussy
 *             sex
 *             sluts
 *         net 
 *             sluts
 *             whores
 *     
 * The ordered table is implemented with a tree and each node in the tree
 * has nNextLevels children.
 *
 * "UNIVERSE" 0xSUBLEVEL "com" 0xSUBLEVEL "foobar"  0xSUBLEVEL "girls" 0xSAMELEVEL 
 *                                                             "boys"  0xPREVLEVEL
 *                                        "pussy"   0xSAMELEVEL
 *                                        "sex"     0xSAMELEVEL
 *                                        "slut"    0xPREVLEVEL
 *                       "net" 0xSUBLEVEL "mysluts" 0xSAMELEVEL
 *                                        "whores"  0xPREVLEVEL
 *                       0xPREVLEVEL
 * 0xENDTABLE
 *
 * The idea behind this is that the whole table can be loaded into 1 malloc-ed area
 * and that the 0xXXXX tokens can be substitued by a '\0' in memory.
 * The levelTag pointer will point into the table without malloc-ing memory which
 * will save a lot on malloc overhead, memory and CPU time to load the table.
 *
 * The major drawback is that the number of children of a node is unknown when
 * a table is loaded and the arrays of child nodes (nextLevel[]) must be reallocated
 * many times.  This implies a relatively large malloc()/realloc() overhead.
 *
 * To get rid of the malloc() overhead, the number of child nodes (nNextLevels) is 
 * coded into the file.  This is a new feature for database file format 1.2.
 *
 * 254034 lines
 * 173984 nodes with 0 children
 *  67779 nodes with 1 child
 *   7762 nodes with 2 children
 *   2075            3
 *    772            4
 */

#ifndef UFDB_UFDB_H_INCLUDED
#define UFDB_UFDB_H_INCLUDED

#ifndef _REENTRANT
#define _REENTRANT
#endif

#ifndef _XOPEN_SOURCE
#ifdef __sun
#if __STDC_VERSION__ - 0 >= 199901L
#define _XOPEN_SOURCE 600
#else
#define _XOPEN_SOURCE 500
#endif
#else
#define _XOPEN_SOURCE  600
#endif
#endif

#if 0
#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE  200112L
#endif
#endif

#if 0
#ifndef _XOPEN_SOURCE_EXTENDED
#define _XOPEN_SOURCE_EXTENDED  1
#endif
#endif

#ifndef _BSD_SOURCE
#define _BSD_SOURCE 1
#endif

#ifndef UFDB_MALLOC_IS_THREADSAFE
#define UFDB_MALLOC_IS_THREADSAFE 1
#endif

#ifdef UFDB_DEBUG
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#ifdef PTHREAD_ERRORCHECK_MUTEX_INITIALIZER_NP
#define UFDB_STATIC_MUTEX_INIT  PTHREAD_ERRORCHECK_MUTEX_INITIALIZER_NP
#else
#define UFDB_STATIC_MUTEX_INIT  PTHREAD_MUTEX_INITIALIZER
#endif
#else
#define UFDB_STATIC_MUTEX_INIT  PTHREAD_MUTEX_INITIALIZER
#endif

#include "config.h"
#include "version.h"
#include "ufdb-api-private.h"

#if HAVE_VSPRINTF != 1
#error The C library does not have the vsprintf function.
#endif

#if HAVE_STRERROR != 1
#error The C library does not have the strerror function.
#endif

#if HAVE_REGCOMP != 1
#error The C library does not have the regcomp function.
#endif

#if HAVE_SIGACTION != 1
#error The C library does not have the sigaction function.
#endif

#define UFDB_USE_SSL 1


#include <sys/types.h>
#include <regex.h>

#define UFDB_GDB_PATH	   "/usr/bin/gdb"

#define DEFAULT_DBHOME     "/local/squid/blacklists"
#define DEFAULT_IMAGESDIR  "/local/squid/images"
#define DEFAULT_BINDIR     "/local/squid/bin"
#define DEFAULT_LOGDIR     "/local/squid/logs"
#define DEFAULT_LOGFILE    "ufdbGuard.log"
#define DEFAULT_CONFIGFILE "/local/squid/etc/ufdbGuard.conf"

#define UFDB_LICENSE_STATUS_FILENAME  "license-status"

#define UFDB_USER_AGENT	   "Mozilla/5.0 (Windows; U; Windows NT 5.1; en-US; rv:1.9.2.8) Gecko/20100722 Firefox/3.6.8"

#define UFDB_ROTATE_LOG_FILES
#define UFDB_MAX_INITIAL_LOGFILE_SIZE	(200*1024*1024)
#define UFDB_MAX_LOGFILE_SIZE           (200*1024*1024)

#ifndef UFDB_UPLOAD_UNCATEGORISED_URLS_WEBSITE
#define UFDB_UPLOAD_UNCATEGORISED_URLS_WEBSITE  "updates.urlfilterdb.com"
#endif

#ifndef UFDB_UPLOAD_UNCATEGORISED_URLS_CGI
#define UFDB_UPLOAD_UNCATEGORISED_URLS_CGI      "/cgi-bin/uncat.pl"
#endif

#ifndef UFDB_UPLOAD_ANALYSE_SQUID_LOG_CGI
#define UFDB_UPLOAD_ANALYSE_SQUID_LOG_CGI       "/cgi-bin/analyseSquidLog.pl"
#endif

#ifndef UFDB_EXPLAIN_DENY_REASON_URL
#define UFDB_EXPLAIN_DENY_REASON_URL            "http://cgibin.urlfilterdb.com/cgi-bin/explain-denial.cgi"
#endif

#define UFDB_USERQUOTA_SUPPORT  0

#define UFDB_MAX_THREADS	(128+1)
#define UFDB_DAEMON_PORT        3977

/* The HTTP 1.1 spec says that proxies must be able to handle requests 
 * with a very long URI.  So what we will do is that ufdbgclient
 * will support this and truncate the URL to have a maximum length
 * of UFDB_MAX_URL_LENGTH.
 */
#define UFDB_HTTP_1_1_MAX_URI_LENGTH	65536
#define UFDB_MAX_URL_LENGTH     8192

void   ufdbFree( void * ptr );
void * ufdbMalloc( size_t n );
void * ufdbRealloc( void * ptr, size_t n );
void * ufdbCalloc( size_t n, size_t num );
void   ufdbGetMallocMutex( char * fn );
void   ufdbReleaseMallocMutex( char * fn );

char * ufdbStrdup( const char * s );

void   ufdbFreeAllMemory( void );

typedef enum { UFDBdomain, UFDBurl } UFDBurlType;

struct UFDBtable;

struct UFDBtable
{
   unsigned char *     tag;
   int                 nNextLevels;
   struct UFDBtable *  nextLevel;
};

struct UFDBfileHeader
{
   char   string[99];  /* UFDB version category numEntries key */
};

struct UFDBfile
{
   struct UFDBfileHeader header;
   struct UFDBtable      table;
};

struct UFDBmemTable
{
   void *              mem;
   char		       version[8];
   char                key[16];
   char                flags[8];
   int                 numEntries;
   long		       tableSize;
   struct UFDBtable    table;
   int		       indexSize;
   struct UFDBtable *  index;
};

typedef struct
{
   unsigned int x;
   unsigned int y;
   unsigned char state[256];
} ufdbCrypt;

void ufdbCryptInit(   ufdbCrypt * uc, const unsigned char * key, unsigned int keySize );
void ufdbEncryptText( ufdbCrypt * uc, unsigned char * dest, const unsigned char * src, unsigned int len );

#define UFDBsubLevel  '\001'
#define UFDBsameLevel '\002'
#define UFDBprevLevel '\003'
#define UFDBendTable  '\020'

#define UFDBmaxDomainNameLength 255
#define UFDBmaxURLsize          512

#define UFDBdbVersion  "2.0"
#define UFDBtableTag   "Xcyh-TghU-7Vbj-ttC7-KaLu-tbWx"

#define UFDBfileSuffix ".ufdb"

/* when URL is www.mydomain.com/asubdir/index.html,
 * urlPart is asubdir/index.html
 */
typedef unsigned char UFDBurlPart[120];

struct UFDBrevURL_s;

typedef struct UFDBrevURL_s
{
   UFDBurlPart           part;
   struct UFDBrevURL_s * next;
} UFDBrevURL;

#define MAX_REVURLS	 32

typedef struct UFDBthreadAdmin_s
{
   int                   nAlloced;
   int                   myArrayUsage[MAX_REVURLS];
   UFDBrevURL            myArray[MAX_REVURLS];
} UFDBthreadAdmin;


UFDBthreadAdmin * UFDBallocThreadAdmin( void );

int ufdbguard_main( int argc, char ** argv, char ** envp );

#define UFDB_ALLOW			0
#define UFDB_DENY			1

#define UFDB_GROUPTYPE_UNIX    		1
#define UFDB_GROUPTYPE_LDAP    		2

#define UFDB_ACL_ACCESS_DUNNO		0
#define UFDB_ACL_ACCESS_ALLOW		1
#define UFDB_ACL_ACCESS_BLOCK		2

#define UFDB_API_STATUS_VIRGIN  	0
#define UFDB_API_STATUS_STARTED_OK 	1
#define UFDB_API_STATUS_TERMINATED	2
#define UFDB_API_STATUS_RELOADING	3
#define UFDB_API_STATUS_RELOAD_OK	4
#define UFDB_API_STATUS_FATAL_ERROR	5
#define UFDB_API_STATUS_ROLLING_LOGFILE	6
#define UFDB_API_STATUS_UPDATE          7

#define UFDB_API_STATUS_DATABASE_OK	   0
#define UFDB_API_STATUS_DATABASE_OLD	   1
#define UFDB_API_STATUS_DATABASE_EXPIRED   2

#define UFDB_API_OK                        0
#define UFDB_API_MATCH			   1
#define UFDB_API_ERR_NULL                  2
#define UFDB_API_ERR_NOFILE                3
#define UFDB_API_ERR_READ                  4
#define UFDB_API_ERR_EXPR                  5
#define UFDB_API_ERR_RANGE                 6
#define UFDB_API_ERR_ERRNO                 7
#define UFDB_API_ERR_SOCKET                8
#define UFDB_API_ERR_NOMEM                 9
#define UFDB_API_REQ_QUEUED               10
#define UFDB_API_ERR_TUNNEL               11
#define UFDB_API_ERR_INVALID_CERT         12
#define UFDB_API_ERR_IP_ADDRESS           13
#define UFDB_API_ERR_OLD_TABLE            14
#define UFDB_API_ERR_INVALID_TABLE        15
#define UFDB_API_ERR_INVALID_KEY          16
#define UFDB_API_ERR_IS_SKYPE		  17
#define UFDB_API_ERR_FULL                 18
#define UFDB_API_ERR_UNKNOWN_PROTOCOL     19
#define UFDB_API_ERR_IS_GTALK		  20
#define UFDB_API_ERR_IS_YAHOOMSG	  21
#define UFDB_API_ERR_IS_AIM     	  22
#define UFDB_API_BEING_VERIFIED           50
#define UFDB_API_MODIFIED_FOR_SAFESEARCH  51

/* reasons why a URL is blocked */
#define UFDB_API_BLOCK_NONE			0
#define UFDB_API_BLOCK_ACL			1
#define UFDB_API_BLOCK_HTTPS_OPTION		2
#define UFDB_API_BLOCK_SKYPE			3
#define UFDB_API_BLOCK_SAFESARCH		4
#define UFDB_API_BLOCK_LOADING_DB		5
#define UFDB_API_BLOCK_FATAL_ERROR		6

#define UFDB_API_ALLOW_QUEUING     	        1
#define UFDB_API_VERBOSE_OUTPUT    	        2
#define UFDB_OPT_HTTPS_WITH_HOSTNAME            4
#define UFDB_OPT_HTTPS_OFFICAL_CERTIFICATE      8
#define UFDB_OPT_SKYPE_OVER_HTTPS	       16
#define UFDB_OPT_PROHIBIT_INSECURE_SSLV2       32
#define UFDB_OPT_SAFE_SEARCH                   64
#define UFDB_OPT_UNKNOWN_PROTOCOL_OVER_HTTPS  128
#define UFDB_OPT_GTALK_OVER_HTTPS	      256
#define UFDB_OPT_YAHOOMSG_OVER_HTTPS	      512
#define UFDB_OPT_AIM_OVER_HTTPS	             1024

#define UFDB_API_HTTPS_CHECK_OFF           0
#define UFDB_API_HTTPS_CHECK_QUEUE_CHECKS  1
#define UFDB_API_HTTPS_CHECK_AGGRESSIVE    2
#define UFDB_API_HTTPS_CHECK_AGRESSIVE     UFDB_API_HTTPS_CHECK_AGGRESSIVE
#define UFDB_API_HTTPS_LOG_ONLY            3

#define UFDB_ACL_NONE		0
#define UFDB_ACL_WITHIN   	1
#define UFDB_ACL_OUTSIDE   	2
#define UFDB_ACL_ELSE   	3

#define UFDB_PARAM_INIT		1
#define UFDB_PARAM_ALARM	2

#define UFDB_DEF_HTTPS_CONN_CACHE_SIZE     1000

struct ufdbRegExp {
  char *    pattern;
  char *    substitute;
  regex_t * compiled;
  int       error;
  int       flags;
  int       global;
  char *    httpcode;
  struct ufdbRegExp * next;
};

struct ufdbRegExp * ufdbNewPatternBuffer( char * pattern, int flags );

int ufdbRegExpMatch(   
   struct ufdbRegExp * regexp, 
   char *              str );

void ufdbSetSignalHandler( 
   int signal, 
   void (*handler)(int)  );

int UFDBaddSafeSearch(
   char * domain,
   char * strippedURL,
   char * originalURL  );

void ufdbHandleAlarmForTimeEvents( int why );

#endif

