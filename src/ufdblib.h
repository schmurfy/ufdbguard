/*
 * ufdblib.h - URLfilterDB
 *
 * ufdbGuard is copyrighted (C) 2005-2010 by URLfilterDB with all rights reserved.
 *
 * RCS $Id: ufdblib.h,v 1.56 2011/05/20 14:27:01 root Exp root $
 */

#ifndef UFDB_UFDBLIB_H_INCLUDED
#define UFDB_UFDBLIB_H_INCLUDED

#include "ufdb.h"
#include <time.h>
#include <stdio.h>
#include <sys/times.h>
#include <sys/utsname.h>


extern volatile int    UFDBglobalStatus;
extern volatile int    UFDBglobalDatabaseStatus;
extern volatile int    UFDBglobalReconfig;
extern time_t          UFDBglobalDatabaseLoadTime;
extern int    	       UFDBglobalFatalError;
extern char            UFDBglobalLicenseStatus[512];
extern char * UFDBglobalConfigFile;
extern int    UFDBglobalDebug;
extern int    UFDBglobalDebugSkype;
extern int    UFDBglobalDebugGtalk;
extern int    UFDBglobalDebugYahooMsg;
extern int    UFDBglobalDebugAim;
extern int    UFDBglobalDebugHttpd;
extern int    UFDBglobalDebugRegexp;
extern int    UFDBglobalExpressionOptimisation;
extern char   UFDBglobalUserName[31+1];
extern char * UFDBglobalLogDir;
extern char * UFDBglobalEmailServer;
extern char * UFDBglobalAdminEmail;
extern char * UFDBglobalExternalStatusCommand;
extern int    UFDBglobalLogBlock;
extern int    UFDBglobalLogAllRequests;
extern int    UFDBglobalURLlookupResultDBreload;
extern int    UFDBglobalURLlookupResultFatalError;
extern int    UFDBglobalUploadStats;
extern int    UFDBglobalAnalyseUncategorisedURLs;
extern int    UFDBglobalPortNum;
extern char   UFDBglobalInterface[256];
extern char   UFDBglobalCAcertsFile[1024];
extern char   UFDBglobalFatalErrorRedirect[1024];
extern char   UFDBglobalLoadingDatabaseRedirect[1024];
extern int    UFDBglobalSafeSearch;
extern int    UFDBglobalShowURLdetails;
extern int    UFDBglobalTunnelCheckMethod;
extern int    UFDBglobalHttpsWithHostname;
extern int    UFDBglobalHttpsOfficialCertificate;
extern int    UFDBglobalSkypeOverHttps;
extern int    UFDBglobalGtalkOverHttps;
extern int    UFDBglobalYahooMsgOverHttps;
extern int    UFDBglobalAimOverHttps;
extern int    UFDBglobalUnknownProtocolOverHttps;
extern int    UFDBglobalHttpsNoSSLv2;
extern int    UFDBglobalHttpdPort;
extern char   UFDBglobalHttpdInterface[256];
extern char   UFDBglobalHttpdImagesDirectory[256];
extern struct UFDBmemTable UFDBglobalCheckedDB;
extern struct ufdbRegExp * UFDBglobalCheckedExpressions;
extern unsigned long UFDBglobalMaxLogfileSize;
volatile extern unsigned long UFDBglobalTunnelCounter;

extern struct ufdbSetting * lastSetting;
extern struct ufdbSetting * Setting;

extern struct Source * lastSource;
extern struct Source * Source;
extern struct Source * saveSource;

extern struct Category * lastDest;
extern struct Category * Dest;

extern struct sgRewrite * lastRewrite;
extern struct sgRewrite * Rewrite;
extern struct ufdbRegExp *  lastRewriteRegExec;

extern struct Time * lastTime;
extern struct Time * Time;

extern FILE * globalErrorLog;
extern struct LogFile * globalLogFile;

extern struct LogFileStat * lastLogFileStat;
extern struct LogFileStat * LogFileStat;

extern struct TimeElement *lastTimeElement;
extern struct TimeElement *TimeElement;

extern struct Acl * lastAcl;
extern struct Acl * defaultAcl;
extern struct Acl * Acl;
extern struct AclDest * lastAclDest;

extern struct ufdbRegExp * lastRegExpDest;

extern char ** globalArgv;
extern char ** globalEnvp;
extern int globalDebugTimeDelta;
extern int globalDebugRedirect;
extern int globalPid;
extern int globalUpdate;
extern char * globalCreateDb;
extern int failsafe_mode;
extern int sig_hup;
extern int sig_other;
extern int httpsConnectionCacheSize;
extern char progname[80];

UFDBrevURL * UFDBgenRevURL( UFDBthreadAdmin * admin, unsigned char * URL );
void UFDBprintRevURL( UFDBrevURL * revURL );
void UFDBfreeRevURL( UFDBthreadAdmin * admin, UFDBrevURL * revURL );

int UFDBparseTableHeader( struct UFDBmemTable * memTable );
void UFDBparseTable( struct UFDBmemTable * memTable );
void UFDBfreeTableIndex_1_2( struct UFDBtable * t );

int UFDBlookup( UFDBthreadAdmin * admin, struct UFDBmemTable * mt, char * request );
int UFDBlookupRevUrl( struct UFDBtable * t, UFDBrevURL * revUrl );

void UFDBdropPrivileges( const char * username );

struct UFDBmemDB;	/* forward declaration */

struct UFDBmemDB
{
   char * key;
   char * value;
   int    length;
   struct UFDBmemDB * next;
};

struct UFDBmemDB * UFDBmemDBinit( void );
void UFDBmemDBinsert( struct UFDBmemDB * db, char * key, char * value, int length );
int UFDBmemDBfind( struct UFDBmemDB * db, char *  key, char ** value );
void UFDBmemDBdeleteDB( struct UFDBmemDB * db );
void UFDBmemDBprintUserDB( struct UFDBmemDB * db );

void UFDBappInit( void );
unsigned long UFDBappMemUsage( void );

void UFDBtimerInit( struct tms * t );
void UFDBtimerStop( struct tms * t );
void UFDBtimerPrint( struct tms * t, char * tag );
void UFDBtimerPrintString( char * line, struct tms * t, char * tag );
void UFDBlogTimer( struct tms * t, char * tag );

int UFDBloadDatabase( 
   struct UFDBmemTable * mtable, 
   char *                file    );

int UFDBloadExpressions( 
   struct ufdbRegExp ** exprList,
   char *               file     );

struct ufdbRegExp * UFDBoptimizeExprList( 
   char *               reSource,
   struct ufdbRegExp *  reList );

void ufdbFreeRegExprList( struct ufdbRegExp * re );

int  UFDBregcomp( regex_t * preg, const char * regex, int cflags );
void UFDBregfree( regex_t * preg );
int  UFDBregexec( const regex_t * preg, const char * string, size_t nmatch, regmatch_t pmatch[], int eflags );

int  ufdbSetCPU( char * CPUspec );
int  ufdbSetThreadCPUaffinity( int thread_num );
void ufdbResetCPUs( void );

char * ufdbAPIstatusString( int api_code );
char * ufdbDBstat2string( int status );
char * ufdbStatus2string( int status );

#define URL_HIST_SIZE 120000

int    ufdbVerifyURLallCats( UFDBrevURL * revURL, char * URL );
int    ufdbRegisterUnknownURL( char * webserver, int portnumber );
char * ufdbGetUnknownURLs( void );
void   ufdbResetUnknownURLs( void );

int UFDBopenSocket( char * serverName, int port );

void UFDBregisterCountedIP( const char * address );
void UFDBinitializeIPcounters( void );
unsigned long UFDBgetNumberOfRegisteredIPs( void );

struct ufdbSetting {
  char *           name;
  char *           value;
  struct ufdbSetting * next;
};

void   ufdbSetting( char *, char * );
struct ufdbSetting * ufdbSettingFindName( char * );
char * ufdbSettingGetValue( char * );

void   ufdbLogFile( int, int, char * );
void   ufdbGlobalSetLogging( int logging );
void   ufdbRotateLogfile( char * );
void   UFDBrotateLogfile( void );
void   ufdbLogMessage( char * format, ... );
void   ufdbLogError( char *, ... );
void   ufdbLogFatalError( char *, ... );
void   ufdbSetGlobalErrorLogFile( void );
void   UFDBcloseFilesNoLog( void );

void ufdbGetSysInfo( struct utsname * si );
long ufdbGetNumCPUs( void );

void UFDBlogConfig( void );
int  UFDBchangeStatus( int status );

char * UFDBparseIPv6address( char * url, char * domain );
void   UFDBupdateURLwithNormalisedDomain( char * url, char * newDomain );

/*
 * strip a URL;
 * remove http:// prefix, remove www[0-9*]. prefix,
 * remove port number, remove username and password
 */
void UFDBstripURL(
   char * URL,                  /* input URL string */
   char * strippedUrl,          /* output char array (must be 1024 bytes) */
   char * domain,               /* output char array (must be 1024 bytes) */
   char * protocol,             /* output char array (must be 16 bytes) */
   int  * portnumber );         /* output integer */

char * UFDBprintable( char * string );

char * UFDBfgets( 
   char * requestBuffer, 
   int    bufferSize, 
   FILE * fp );

#endif

