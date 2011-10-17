%{

/*
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
 * (GPL2) along with this program.
 */

#define YYMALLOC  ufdbMalloc
#define YYFREE    ufdbFree

#define UFDB_LOG_USER_QUOTA   0   /* TODO remove */

#undef UFDB_DEBUG_ACL_ACCESS

#include "ufdb.h"
#include "sg.h"
#include "ufdblib.h"
#include "ufdbchkport.h"

#ifdef UFDB_DEBUG
#undef YYDEBUG
#define YYDEBUG 1
#endif

#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <netdb.h>
#include <grp.h>
#include <syslog.h>
#include <signal.h>
#include <sched.h>
#include <fcntl.h>
#include <sys/stat.h>


FILE * yyin;
FILE * yyout;
int    lineno;

extern struct ufdbSetting * lastSetting ;
extern struct ufdbSetting * Setting;

extern struct Source * lastSource ;
extern struct Source * Source ;

extern struct Category * lastDest ;
extern struct Category * Dest ;

extern struct sgRewrite * lastRewrite;
extern struct sgRewrite * Rewrite;
extern struct ufdbRegExp *  lastRewriteRegExec;

extern struct Time * lastTime;
extern struct Time * Time;

extern struct LogFile * globalLogFile;

extern struct LogFileStat * lastLogFileStat;
extern struct LogFileStat * LogFileStat;

extern struct TimeElement * lastTimeElement;
extern struct TimeElement * TimeElement;

int   numTimeElements;
int * TimeElementsEvents = NULL;

extern struct Acl * lastAcl;
extern struct Acl * defaultAcl;
extern struct Acl * Acl;
extern struct AclDest * lastAclDest;
 
extern struct ufdbRegExp * lastRegExpDest;

extern int    httpsConnectionCacheSize;

extern int globalDebugTimeDelta;

static int time_switch = 0;
static int date_switch = 0;

static void ufdbSourceGroup( int groupType, char * groupName );
static void ufdbSourceUserQuota( char * seconds, char * sporadic, char * renew );
void ufdbFreeDomainDb( struct sgDb * dbp );

%}

%union {
  char * string;
  char * tval;
  char * dval;
  char * dvalcron;
  int    integer;
}

%token ADMINISTRATOR QSTRING
%token CHECK_PROXY_TUNNELS QUEUE_CHECKS AGGRESSIVE LOG_ONLY 
%token ON OFF 
%token CHAR_MINUS CHAR_I CHAR_EXCLAMATION
%token IGNORECASE
%token COMMA EQUAL PORT
%token HTTP_SERVER INTERFACE IMAGES 
%token HTTPS_PROHIBIT_INSECURE_SSLV2
%token HTTPS_CONNECTION_CACHE_SIZE
%token IDENTIFIER WORD END START_BRACKET STOP_BRACKET WEEKDAY
%token CATEGORY REWRITE ACL CPUS TIME TVAL DVAL DVALCRON
%token SOURCE CIDR IPCLASS CONTINUE
%token IPADDR DBHOME DOMAINLIST URLLIST EXPRESSIONLIST CACERTS IPLIST
%token DOMAIN UNIX LDAP USER USERLIST USERQUOTA GROUP 
%token IP PORT_NUMBER PROXY_PORT NL NUMBER NUMBERS
%token PASS REDIRECT SUBST CHAR MINUTELY HOURLY DAILY WEEKLY DATE
%token REDIRECT_FATAL_ERROR REDIRECT_LOADING_DATABASE
%token WITHIN OUTSIDE ELSE ANONYMOUS SPORADIC
%token LOGFILE LOGDIR LOGBLOCK LOGALL LOGALL_HTTPD
%token MAIL_SERVER ADMIN_EMAIL EXTERNAL_STATUS_COMMAND
%token TOKEN_ALLOW TOKEN_DENY
%token URL_LOOKUP_RESULT_DB_RELOAD URL_LOOKUP_RESULT_FATAL_ERROR
%token OPTION UFDB_SHOW_URL_DETAILS 
%token UFDB_DEBUG_FILTER
%token UFDB_DEBUG_SKYPE_PROBES UFDB_DEBUG_GTALK_PROBES 
%token UFDB_DEBUG_YAHOOMSG_PROBES UFDB_DEBUG_AIM_PROBES
%token UFDB_EXPRESSION_OPTIMISATION UFDB_EXPRESSION_DEBUG
%token ENFORCE_HTTPS_WITH_HOSTNAME ENFORCE_HTTPS_OFFICAL_CERTIFICATE
%token ALLOW_SKYPE_OVER_HTTPS ALLOW_UNKNOWN_PROTOCOL_OVER_HTTPS
%token ALLOW_GTALK_OVER_HTTPS ALLOW_YAHOOMSG_OVER_HTTPS
%token ALLOW_AIM_OVER_HTTPS
%token ANALYSE_UNCATEGORISED UPLOAD_STATS
%token SAFE_SEARCH MAX_LOGFILE_SIZE


%type <string> IDENTIFIER
%type <string> WORD 
%type <string> QSTRING
%type <string> WEEKDAY
%type <string> NUMBER
%type <string> NUMBERS
%type <tval>   TVAL
%type <string> DVAL
%type <string> DVALCRON
%type <string> CHAR
%type <string> SUBST 
%type <string> IPADDR
%type <string> DBHOME LOGDIR
%type <string> CIDR
%type <string> IPCLASS
%type <string> dval
%type <string> dvalcron
%type <string> tval
%type <string> date
%type <string> ttime
%type <string> qidentifier

%type <integer> check_proxy_tunnel_option
%type <integer> allow_or_deny
%type <integer> on_or_off

%%

start: statements
       ; 

qidentifier:
	   	  IDENTIFIER    { $$ = $1; }
	 	| QSTRING       { $$ = $1; }
	 	;

https_cache_size:
           	  HTTPS_CONNECTION_CACHE_SIZE NUMBER   { httpsConnectionCacheSize = atoi( $2 ); ufdbFree( $2 ); }
	 	;

allow_or_deny:
		  TOKEN_ALLOW   { $$ = UFDB_ALLOW; }
	        | TOKEN_DENY    { $$ = UFDB_DENY; }
		;

on_or_off:	
	   	  ON    { $$ = 1; }
	 	| OFF   { $$ = 0; }
	 	;

log_block:
	   	  LOGBLOCK on_or_off   	{ UFDBglobalLogBlock = $2; }
	 	;

log_all:
	   	  LOGALL on_or_off     	{ UFDBglobalLogAllRequests = $2; }
	 	;

logall_httpd:
	   	  LOGALL_HTTPD on_or_off     	{ UFDBglobalDebugHttpd = $2; }
	 	;

debug_filter:
		  UFDB_DEBUG_FILTER on_or_off   { UFDBglobalDebug = $2; }
	        ;

enforce_https_with_hostname:
	   	  ENFORCE_HTTPS_WITH_HOSTNAME on_or_off         
	   	    { ufdbLogError( "line %d: option enforce-https-with-hostname must be part of the security category", lineno );  }
	 	;

enforce_https_offical_certificate:
           	ENFORCE_HTTPS_OFFICAL_CERTIFICATE on_or_off   
	   	  { ufdbLogError( "line %d: option enforce-https-official-certificate must be part of the security category", lineno );  }
	 	;

https_prohibit_insecure_sslv2:
	   	  HTTPS_PROHIBIT_INSECURE_SSLV2 on_or_off       
	   	    { ufdbLogError( "line %d: option https-prohibit-insecure-sslv2 must be part of the security category", lineno );  }
	 	;


check_proxy_tunnel_option:
	   	  OFF           { $$ = UFDB_API_HTTPS_CHECK_OFF; }
	 	| QUEUE_CHECKS  { $$ = UFDB_API_HTTPS_CHECK_QUEUE_CHECKS; }
	 	| AGGRESSIVE    { $$ = UFDB_API_HTTPS_CHECK_AGGRESSIVE; }
	 	| LOG_ONLY      { $$ = UFDB_API_HTTPS_LOG_ONLY; }
	 	;

check_proxy_tunnels:
           	  CHECK_PROXY_TUNNELS check_proxy_tunnel_option { UFDBsetTunnelCheckMethod( $2 ); }
	 	;

admin_spec:    
		  ADMINISTRATOR QSTRING  { ufdbSetting("administrator",$2); }
	   	;

dbhome:    
		  DBHOME qidentifier { ufdbSetting("dbhome",$2); }
		| DBHOME WORD        { ufdbSetting("dbhome",$2); }
           	;

url_lookup_result_db_reload:
		  URL_LOOKUP_RESULT_DB_RELOAD allow_or_deny
		  			{ UFDBglobalURLlookupResultDBreload = $2; }

url_lookup_result_fatal_error:
		  URL_LOOKUP_RESULT_FATAL_ERROR allow_or_deny
		  			{ UFDBglobalURLlookupResultFatalError = $2; }

ufdb_show_url_details:
		  UFDB_SHOW_URL_DETAILS on_or_off  { UFDBglobalShowURLdetails = $2; }
	        ;

ufdb_debug_skype_probes:
		  UFDB_DEBUG_SKYPE_PROBES on_or_off  { UFDBglobalDebugSkype = $2; }
	        ;

ufdb_debug_gtalk_probes:
		  UFDB_DEBUG_GTALK_PROBES on_or_off  { UFDBglobalDebugGtalk = $2; }
	        ;

ufdb_debug_yahoomsg_probes:
		  UFDB_DEBUG_YAHOOMSG_PROBES on_or_off  { UFDBglobalDebugYahooMsg = $2; }
	        ;

ufdb_debug_aim_probes:
		  UFDB_DEBUG_AIM_PROBES on_or_off  { UFDBglobalDebugAim = $2; }
	        ;

ufdb_expression_optimisation:
		  UFDB_EXPRESSION_OPTIMISATION on_or_off  { UFDBglobalExpressionOptimisation = $2; }
	        ;

ufdb_expression_debug:
		  UFDB_EXPRESSION_DEBUG on_or_off  { UFDBglobalDebugRegexp = $2; }
	        ;

mail_server:
		  MAIL_SERVER QSTRING  { ufdbFree( UFDBglobalEmailServer ); UFDBglobalEmailServer = $2; }
	        ;

admin_email:
		  ADMIN_EMAIL QSTRING  { ufdbFree( UFDBglobalAdminEmail ); UFDBglobalAdminEmail = $2; }
	        ;

external_status_command:
		  EXTERNAL_STATUS_COMMAND  QSTRING  {  ufdbFree( UFDBglobalExternalStatusCommand );  UFDBglobalExternalStatusCommand = $2; }
	        ;

logdir:    
		  LOGDIR qidentifier { ufdbSetting("logdir",$2); }
		| LOGDIR WORD        { ufdbSetting("logdir",$2); }
            	;

port:	   
		  PORT NUMBER        { ufdbSetting( "port", $2 ); }
                ;

interface:
	          INTERFACE qidentifier    { 
#if HAVE_UNIX_SOCKETS
                                             ufdbLogError( "ufdbguardd is configured to use UNIX sockets.  \"interface\" is ignored." );
#else
		                             if (strcmp($2,"all")== 0)
						strcpy( UFDBglobalInterface, "all" );    
					     else
					        ufdbLogFatalError( "interface must be \"all\" or IP address" );
#endif
					     ufdbFree( $2 );
					   }
	        | INTERFACE IPADDR   { strcpy( UFDBglobalInterface, $2 );  ufdbFree( $2 ); }
		;

cpus:	   
		  CPUS NUMBER        { ufdbSetCPU( $2 ); ufdbFree( $2 ); }
         	| CPUS NUMBERS       { ufdbSetCPU( $2 ); ufdbFree( $2 ); }
           	;

upload_stats: 
               	  UPLOAD_STATS on_or_off
		     { UFDBglobalUploadStats = $2;   }
	        ;

redirect_loading_database:
		  REDIRECT_LOADING_DATABASE  QSTRING 
		     { strcpy( UFDBglobalLoadingDatabaseRedirect, $2 );  ufdbFree( $2 );  }
	        ;

redirect_fatal_error:
		  REDIRECT_FATAL_ERROR  QSTRING 
		     { strcpy( UFDBglobalFatalErrorRedirect, $2 );  ufdbFree( $2 );  }
	        ;

analyse_uncategorised: 
               	  ANALYSE_UNCATEGORISED on_or_off
		     { 
		        if (UFDBglobalAnalyseUncategorisedURLs < 0)
			   ufdbLogMessage( "option analyse-uncategorised-urls is overruled by -N option *****" );
		        else
		           UFDBglobalAnalyseUncategorisedURLs = $2;   
		     }
	        |
               	  ANALYSE_UNCATEGORISED NUMBER
		     {
		        if (UFDBglobalAnalyseUncategorisedURLs < 0)
			   ufdbLogMessage( "option analyse-uncategorised-urls is overruled by -N option *****" );
		        else
			{
		           UFDBglobalAnalyseUncategorisedURLs = atoi( $2 ); 
			   ufdbFree( $2 ); 
			}
		     }
	       	;

safe_search:
		  SAFE_SEARCH on_or_off
		     { UFDBglobalSafeSearch = $2; }
	        ;

max_logfile_size:
		  MAX_LOGFILE_SIZE NUMBER
		     { 
		       UFDBglobalMaxLogfileSize = strtoul( $2, NULL, 10 );
		       ufdbFree( $2 );
		       if (UFDBglobalMaxLogfileSize < 2 * 1024 * 1024)		/* minimum is 2 MB */
		          UFDBglobalMaxLogfileSize = 2 * 1024 * 1024;
		       if (UFDBglobalMaxLogfileSize > 2000000000)		/* maximum is 2 GB */
		          UFDBglobalMaxLogfileSize = 2000000000;
		     }
	        ;

httpd_option:
		  PORT EQUAL NUMBER        { UFDBglobalHttpdPort = atoi( $3 ); ufdbFree( $3 ); }
	        | INTERFACE EQUAL qidentifier { if (strcmp($3,"all")== 0)
						strcpy( UFDBglobalHttpdInterface, "all" );    
					     else
					        ufdbLogFatalError( "http-server interface must be \"all\" or IP address" );
					     ufdbFree( $3 );
					   }
	        | INTERFACE EQUAL WORD     { if (strcmp($3,"all")== 0)
						strcpy( UFDBglobalHttpdInterface, "all" );    
					     else
					        ufdbLogFatalError( "http-server interface must be \"all\" or IP address" );
					     ufdbFree( $3 );
					   }
	        | INTERFACE EQUAL IPADDR   { strcpy( UFDBglobalHttpdInterface, $3 );       ufdbFree( $3 ); }
		| IMAGES EQUAL qidentifier { strcpy( UFDBglobalHttpdImagesDirectory, $3 ); ufdbFree( $3 ); }
		| IMAGES EQUAL WORD        { strcpy( UFDBglobalHttpdImagesDirectory, $3 ); ufdbFree( $3 ); }
		;

httpd_options:
		  httpd_option COMMA httpd_options
		| httpd_option
	        ;

http_server_def:
		  HTTP_SERVER START_BRACKET httpd_options STOP_BRACKET
	        ;

category: 
		  CATEGORY qidentifier { ufdbCategory($2); }
             	;

category_block: 
		  category START_BRACKET category_contents STOP_BRACKET 
                       { ufdbCategoryEnd(); }
                ;

category_contents:
                | category_contents category_content
		;

category_content:  
 	      	  DOMAINLIST qidentifier  { ufdbCategoryDomainList( $2 ); }
 	      	| DOMAINLIST WORD         { ufdbCategoryDomainList( $2 ); }
            	| DOMAINLIST CHAR_MINUS   { ufdbCategoryDomainList( NULL ); }
            	| URLLIST qidentifier     { ufdbCategoryUrlList( $2 ); }
            	| URLLIST WORD            { ufdbCategoryUrlList( $2 ); }
            	| URLLIST CHAR_MINUS      { ufdbCategoryUrlList( NULL ); }
            	| EXPRESSIONLIST qidentifier { ufdbCategoryExpressionList( $2, "n" ); }
            	| EXPRESSIONLIST WORD     { ufdbCategoryExpressionList( $2, "n" ); }
            	| EXPRESSIONLIST CHAR_MINUS { ufdbCategoryExpressionList( NULL, NULL ); }
            	| EXPRESSIONLIST CHAR_I qidentifier { ufdbCategoryExpressionList( $3, "i" ); }
            	| EXPRESSIONLIST CHAR_I WORD { ufdbCategoryExpressionList( $3, "i" ); }
            	| EXPRESSIONLIST IGNORECASE qidentifier { ufdbCategoryExpressionList( $3, "i" ); }
            	| EXPRESSIONLIST IGNORECASE WORD { ufdbCategoryExpressionList( $3, "i" ); }
		| CACERTS qidentifier     { ufdbCategoryCACerts( $2 ); }
		| CACERTS WORD            { ufdbCategoryCACerts( $2 ); }
            	| REDIRECT qidentifier    { ufdbCategoryRedirect( $2 ); }
            	| REDIRECT WORD           { ufdbCategoryRedirect( $2 ); }
            	| REWRITE qidentifier     { ufdbCategoryRewrite( $2 ); }
            	| REWRITE WORD            { ufdbCategoryRewrite( $2 ); }
            	| WITHIN qidentifier      { ufdbCategoryTime( $2, UFDB_ACL_WITHIN ); }
            	| OUTSIDE qidentifier     { ufdbCategoryTime( $2, UFDB_ACL_OUTSIDE ); }
		| OPTION SAFE_SEARCH on_or_off                       { ufdbCategoryOption( $3, UFDB_OPT_SAFE_SEARCH );  }
		| OPTION ENFORCE_HTTPS_WITH_HOSTNAME                 { ufdbCategoryOption(  1, UFDB_OPT_HTTPS_WITH_HOSTNAME );  }
		| OPTION ENFORCE_HTTPS_WITH_HOSTNAME on_or_off       { ufdbCategoryOption( $3, UFDB_OPT_HTTPS_WITH_HOSTNAME );  }
		| OPTION ENFORCE_HTTPS_OFFICAL_CERTIFICATE           { ufdbCategoryOption(  1, UFDB_OPT_HTTPS_OFFICAL_CERTIFICATE );  }
		| OPTION ENFORCE_HTTPS_OFFICAL_CERTIFICATE on_or_off { ufdbCategoryOption( $3, UFDB_OPT_HTTPS_OFFICAL_CERTIFICATE );  }
		| OPTION HTTPS_PROHIBIT_INSECURE_SSLV2 on_or_off     { ufdbCategoryOption( $3, UFDB_OPT_PROHIBIT_INSECURE_SSLV2 );  }
		| OPTION ALLOW_SKYPE_OVER_HTTPS on_or_off	     { ufdbCategoryOption( $3, UFDB_OPT_SKYPE_OVER_HTTPS );  }
		| OPTION ALLOW_GTALK_OVER_HTTPS on_or_off	     { ufdbCategoryOption( $3, UFDB_OPT_GTALK_OVER_HTTPS );  }
		| OPTION ALLOW_YAHOOMSG_OVER_HTTPS on_or_off	     { ufdbCategoryOption( $3, UFDB_OPT_YAHOOMSG_OVER_HTTPS );  }
		| OPTION ALLOW_AIM_OVER_HTTPS on_or_off	             { ufdbCategoryOption( $3, UFDB_OPT_AIM_OVER_HTTPS );  }
		| OPTION ALLOW_UNKNOWN_PROTOCOL_OVER_HTTPS on_or_off { ufdbCategoryOption( $3, UFDB_OPT_UNKNOWN_PROTOCOL_OVER_HTTPS );  }
            	| LOGFILE ANONYMOUS qidentifier { ufdbLogError( "line %d: unsupported logfile context for %s", lineno, $3 ); 
		                                  ufdbFree( $3 ); }
            	| LOGFILE ANONYMOUS WORD        { ufdbLogError( "line %d: unsupported logfile context for %s", lineno, $3 ); 
		                                  ufdbFree( $3 ); }
            	| LOGFILE qidentifier           { ufdbLogError( "line %d: unsupported logfile context for %s", lineno, $2 ); 
		                                  ufdbFree( $2 ); }
            	| LOGFILE WORD                  { ufdbLogError( "line %d: unsupported logfile context for %s", lineno, $2 ); 
		                                  ufdbFree( $2 ); }
            	;

source:      
		  SOURCE qidentifier { sgSource( $2 ); }
             	;

source_block: 
		  source START_BRACKET source_contents STOP_BRACKET { sgSourceEnd(); }
             	;

source_contents:
		  /* empty */
		| source_contents source_content
		;

source_content:       
		  DOMAIN domain
                | USER user 	
                | UNIX USER user 		     
                | USERLIST qidentifier 		 { ufdbSourceUserList( $2 ); } 
                | USERLIST WORD 		 { ufdbSourceUserList( $2 ); } 
                | UNIX USERLIST qidentifier      { ufdbSourceUserList( $3 ); } 
                | UNIX USERLIST WORD 	         { ufdbSourceUserList( $3 ); } 
		| UNIX GROUP qidentifier         { ufdbSourceGroup( UFDB_GROUPTYPE_UNIX, $3 ); }
		| UNIX GROUP WORD                { ufdbSourceGroup( UFDB_GROUPTYPE_UNIX, $3 ); }
                | USERQUOTA NUMBER NUMBER HOURLY { ufdbSourceUserQuota( $2, $3, "3600" );  
		                                   ufdbFree( $2 ); ufdbFree( $3 ); }
                | USERQUOTA NUMBER NUMBER DAILY  { ufdbSourceUserQuota( $2, $3, "86400" );  
		                                   ufdbFree( $2 ); ufdbFree( $3 ); }  
                | USERQUOTA NUMBER NUMBER WEEKLY { ufdbSourceUserQuota( $2, $3, "604800" );  
		                                   ufdbFree( $2 ); ufdbFree( $3 ); } 
                | USERQUOTA NUMBER NUMBER NUMBER { ufdbSourceUserQuota( $2, $3, $4 );  
		                                   ufdbFree( $2 ); ufdbFree( $3 ); ufdbFree( $4 ); } 
                | IP ips
                | PROXY_PORT NUMBER              { sgProxyPort($2); }
                | IPLIST qidentifier           	 { sgSourceIpList( $2 ); }
                | IPLIST WORD             	 { sgSourceIpList( $2 ); }
                | WITHIN qidentifier           	 { sgSourceTime( $2, UFDB_ACL_WITHIN ); }
                | OUTSIDE qidentifier          	 { sgSourceTime( $2, UFDB_ACL_OUTSIDE ); }
                | LOGFILE ANONYMOUS qidentifier	 { ufdbLogError( "line %d: unsupported logfile context for %s", lineno, $3 ); 
						   ufdbFree( $3 ); }
                | LOGFILE ANONYMOUS WORD  	 { ufdbLogError( "line %d: unsupported logfile context for %s", lineno, $3 ); 
						   ufdbFree( $3 ); }
                | LOGFILE qidentifier          	 { ufdbLogError( "line %d: unsupported logfile context for %s", lineno, $2 ); 
						   ufdbFree( $2 ); }
                | LOGFILE WORD            	 { ufdbLogError( "line %d: unsupported logfile context for %s", lineno, $2 ); 
						   ufdbFree( $2 ); }
                | CONTINUE                	 { lastSource->cont_search = 1; }
                ;


domain:		    
		  /* empty */
		| domain qidentifier  	{ sgSourceDomain( $2 ); }
		| domain WORD  		{ sgSourceDomain( $2 ); }
                | domain COMMA
		;

user:		    
		  /* empty */
		| user qidentifier	{ ufdbSourceUser( $2 ); }
		| user WORD    		{ ufdbSourceUser( $2 ); }
                | user COMMA
		;

acl_block: 
		  ACL START_BRACKET acl_contents STOP_BRACKET 
                ;

acl_contents:     /* empty */
                | acl_contents acl_content
	        ;

acl_header:       
		  qidentifier                     START_BRACKET  { sgAcl( $1, NULL, UFDB_ACL_NONE );  }
		| qidentifier WITHIN qidentifier  START_BRACKET  { sgAcl( $1, $3, UFDB_ACL_WITHIN );  }
                | qidentifier OUTSIDE qidentifier START_BRACKET  { sgAcl( $1, $3, UFDB_ACL_OUTSIDE ); } 
                ;

acl_content:      
		  acl_header access_contents STOP_BRACKET
                | acl_header access_contents STOP_BRACKET ELSE
                     { sgAcl( NULL, NULL, UFDB_ACL_ELSE );    } 
                     START_BRACKET access_contents STOP_BRACKET
                ;

access_contents:  
		  /* empty */
                | access_contents access_content
                ;

access_content:     
		  PASS access_pass              { }
                | REWRITE qidentifier           { sgAclSetValue( "rewrite", $2, 0 ); }
                | REWRITE WORD                  { sgAclSetValue( "rewrite", $2, 0 ); }
                | REDIRECT qidentifier          { sgAclSetValue( "redirect", $2, 0 ); }
                | REDIRECT WORD                 { sgAclSetValue( "redirect", $2, 0 ); }
                | LOGFILE ANONYMOUS qidentifier { ufdbLogError( "line %d: unsupported logfile context for %s", lineno, $3 ); 
						  ufdbFree( $3 ); }
                | LOGFILE ANONYMOUS WORD        { ufdbLogError( "line %d: unsupported logfile context for %s", lineno, $3 ); 
						  ufdbFree( $3 ); }
                | LOGFILE qidentifier           { ufdbLogError( "line %d: unsupported logfile context for %s", lineno, $2 ); 
						  ufdbFree( $2 ); }
                | LOGFILE WORD                  { ufdbLogError( "line %d: unsupported logfile context for %s", lineno, $2 ); 
						  ufdbFree( $2 ); }
                ;

access_pass:      
		  /* empty */
                | access_pass qidentifier     		    { sgAclSetValue( "pass", $2, 1 ); }
                | access_pass CHAR_EXCLAMATION qidentifier  { sgAclSetValue( "pass", $3, 0 ); }
		| access_pass COMMA
                ;

cidr:             
		  CIDR 		{ sgIp( $1 ); ufdbFree( $1 ); }
                ;

ipclass:          
		  IPCLASS 	{ sgIp( $1 ); ufdbFree( $1 ); }
                ;

ips: 		  
		  /* empty */
                | ips ip         	{ sgIp( "255.255.255.255" );  sgSetIpType( SG_IPTYPE_HOST, NULL, 0 ); }
                | ips ip cidr    	{ sgSetIpType( SG_IPTYPE_CIDR, NULL, 0 ); }
                | ips ip ipclass 	{ sgSetIpType( SG_IPTYPE_CLASS, NULL, 0 ); }
                | ips ip CHAR_MINUS ip  { sgSetIpType( SG_IPTYPE_RANGE, NULL, 0 ); }
                | ips COMMA
		;

ip:  		  
		  IPADDR 		{ sgIp( $1 );  ufdbFree( $1 );  }
     		;

rew:       	  
		  REWRITE qidentifier  	{ sgRewrite( $2 ); }
		| REWRITE WORD 		{ sgRewrite( $2 ); } 
		;

rew_block:  	  
		  rew START_BRACKET rew_contents STOP_BRACKET 
             	;

rew_contents:     
		  /* empty */
		| rew_contents rew_content
		;


rew_content:      
		  SUBST                         { sgRewriteSubstitute( $1 ); ufdbFree( $1 ); }
                | WITHIN qidentifier            { sgRewriteTime( $2, UFDB_ACL_WITHIN ); }
                | OUTSIDE qidentifier           { sgRewriteTime( $2, UFDB_ACL_OUTSIDE ); }
                | LOGFILE ANONYMOUS qidentifier { ufdbLogError( "line %d: unsupported logfile context for %s", lineno, $3 ); 							      ufdbFree( $3 ); }
                | LOGFILE ANONYMOUS WORD        { ufdbLogError( "line %d: unsupported logfile context for %s", lineno, $3 ); 							      ufdbFree( $3 ); }
                | LOGFILE qidentifier           { ufdbLogError( "line %d: unsupported logfile context for %s", lineno, $2 ); 							      ufdbFree( $2 ); }
                | LOGFILE WORD                  { ufdbLogError( "line %d: unsupported logfile context for %s", lineno, $2 ); 							      ufdbFree( $2 ); }
                ;


time:       	  
		  TIME qidentifier 		{ sgTime( $2 ); } 
		;

time_block:  	  
		  time START_BRACKET time_contents STOP_BRACKET 
                ;

time_contents:    
		  /* empty */
		| time_contents time_content
		;


time_content:     
                  WEEKLY { sgTimeElementInit(); } WEEKDAY     { sgTimeElementAdd($3,T_WEEKDAY); } ttime
		| WEEKLY { sgTimeElementInit(); } qidentifier { sgTimeElementAdd($3,T_WEEKLY); }  ttime
		| WEEKLY { sgTimeElementInit(); } WORD        { sgTimeElementAdd($3,T_WEEKLY); }  ttime
                | DATE   { sgTimeElementInit(); } date        { sgTimeElementEnd(); }
		| error  { ufdbLogFatalError( "invalid time specification at line %d", lineno );   }
                ;

ttime:            
		  ttime  { sgTimeElementClone(); }  tval CHAR_MINUS tval
		| tval CHAR_MINUS tval 
                ;

date:             
		  dval ttime
                | dval 
                | dval  CHAR_MINUS dval ttime
                | dval  CHAR_MINUS dval 
                | dvalcron  ttime
                | dvalcron
                ;

dval:		  DVAL 		{ sgTimeElementAdd( $1, T_DVAL ); }
                ;

tval:		  TVAL 		{ sgTimeElementAdd( $1, T_TVAL ); }
                ;

dvalcron:	  DVALCRON 	{ sgTimeElementAdd( $1, T_DVALCRON ); }
                ;

an_error:
		  error  	{  yyerror( "syntax error" );  }
	        ;

statements:       /* empty */
       		| statements statement
       		;

statement:   
                  category
	     	| source_block
	     	| category_block
		| http_server_def
	     	| admin_spec
	     	| https_cache_size
	     	| check_proxy_tunnels
	     	| log_block
	     	| log_all
		| logall_httpd
		| debug_filter
		| enforce_https_with_hostname
		| enforce_https_offical_certificate
		| https_prohibit_insecure_sslv2
             	| dbhome
	     	| logdir
		| mail_server
		| admin_email
		| external_status_command
		| url_lookup_result_db_reload
		| url_lookup_result_fatal_error
		| ufdb_show_url_details
		| ufdb_debug_skype_probes
		| ufdb_debug_gtalk_probes
		| ufdb_debug_yahoomsg_probes
		| ufdb_debug_aim_probes
		| ufdb_expression_optimisation
		| ufdb_expression_debug
		| interface
	     	| port
	     	| cpus
	     	| upload_stats
		| redirect_loading_database
		| redirect_fatal_error
	     	| analyse_uncategorised
		| safe_search
		| max_logfile_size
	     	| acl_block
	     	| rew_block
	     	| time_block
	     	| NL
	     	| an_error
             	;

%%

void sgReadConfig( 
   char *      file )
{
   char *      defaultFile = DEFAULT_CONFIGFILE;
   struct Source * source;

   lineno = 1;
   ufdbResetCPUs();

   UFDBglobalConfigFile = file;
   if (UFDBglobalConfigFile == NULL)
      UFDBglobalConfigFile = defaultFile;

   yyin = fopen( UFDBglobalConfigFile, "r" );
   if (yyin == NULL) 
   {
      syslog( LOG_ALERT, "%s: cannot open configuration file %s", progname, UFDBglobalConfigFile );
      ufdbLogFatalError( "cannot open configuration file %s", UFDBglobalConfigFile );
      return;
   }

   ufdbLogMessage( "configuration file: %s", UFDBglobalConfigFile );

   /* UFDBglobalDebug = 0; */
   strcpy( UFDBglobalInterface, "all" );
   /* UFDBglobalPortNum = UFDB_DAEMON_PORT;  the port can be set with a command line switch so do not reset it */
   UFDBglobalDebugHttpd = 0;
   UFDBglobalLogBlock = 0;
   UFDBglobalLogAllRequests = 0;
   UFDBglobalURLlookupResultDBreload = UFDB_ALLOW;
   UFDBglobalURLlookupResultFatalError = UFDB_ALLOW;
   UFDBglobalShowURLdetails = 0;
   UFDBglobalDebugSkype = 0;
   UFDBglobalDebugGtalk = 0;
   UFDBglobalDebugYahooMsg = 0;
   UFDBglobalDebugAim = 0;
   UFDBglobalDebugRegexp = 0;
   UFDBglobalExpressionOptimisation = 1;
   if (UFDBglobalAnalyseUncategorisedURLs >= 0)		/* obey -N option which uses value -1 */
      UFDBglobalAnalyseUncategorisedURLs = 1;
   UFDBglobalUploadStats = 1;
   UFDBglobalSafeSearch = 1;
   UFDBglobalTunnelCheckMethod = UFDB_API_HTTPS_CHECK_OFF;
   UFDBglobalHttpsWithHostname = 0;
   UFDBglobalHttpsOfficialCertificate = 0;
   UFDBglobalUnknownProtocolOverHttps = 1;
   UFDBglobalHttpsNoSSLv2 = 0;
   UFDBglobalHttpdPort = 0;
   strcpy( UFDBglobalHttpdInterface, "all" );
   strcpy( UFDBglobalHttpdImagesDirectory, "." );
 
   (void) yyparse();
   fclose( yyin );

   UFDBglobalDatabaseLoadTime = time( NULL );

   /*
    * For analysis of uncategorised URLs we also load a "checked" category
    * that contains URLs that are reviewed and checked not to be part of
    * any other category.
    */
   if (UFDBglobalAnalyseUncategorisedURLs > 0)
   {
      char * dbhome;
      char   dbfname[1024];

      dbhome = ufdbSettingGetValue( "dbhome" );
      if (dbhome == NULL)
         dbhome = DEFAULT_DBHOME;
      sprintf( dbfname, "%s/checked/domains.ufdb", dbhome );
      if (UFDBloadDatabase( &UFDBglobalCheckedDB, dbfname ) != UFDB_API_OK)
      {
         UFDBglobalCheckedDB.mem = NULL;
         UFDBglobalCheckedDB.index = NULL;
         UFDBglobalCheckedDB.table.nNextLevels = 0;
	 if (UFDBglobalDebug)
	    ufdbLogError( "the URL database does not have a category called \"checked\"" );
      }
      else
      {
	 sprintf( dbfname, "%s/checked/expressions", dbhome );
	 (void) UFDBloadExpressions( &UFDBglobalCheckedExpressions, dbfname );
         ufdbLogMessage( "a sample of uncategorised URLs will be analysed." );
      }
   }

   if (defaultAcl == NULL)
      ufdbLogFatalError( "\"default\" ACL not defined in configuration file  %s", UFDBglobalConfigFile );

   /* A configuration file may have a source definition which is not used in the ACL list.
    * This raises the question "what to do with this ?".
    * If we do nothing, the source is matched but has no ACL definition and is silently skipped.
    * Most likely the administrator overlooked something so a warning is useful.
    */
   for (source = Source;  source != NULL;  source = source->next)
   {
      int found;
      struct Acl * acl;

      found = 0;
      for (acl = Acl;  acl != NULL;  acl = acl->next)
      {
         if (strcmp( acl->name, source->name ) == 0)
	 {
	    found = 1;
	    break;
	 }
      }
      if (!found)
      {
         ufdbLogError( "source \"%s\" has no ACL; the default ACL will be used.  *****", source->name );
      }
   }

   if (UFDBglobalAdminEmail != NULL  &&  UFDBglobalEmailServer == NULL)
   {
      ufdbLogError( "No email server is defined; cannot send email to %s  *****", UFDBglobalAdminEmail );
   }
}



/*
 * Source functions
 */

void sgSource( 
  char *          source )
{
  struct Source * sp;

#if defined(UFDB_DEBUG)
   ufdbLogMessage( "sgSource %s", source );
#endif

  if (Source != NULL)  
  {
    if ((struct Source *) sgSourceFindName(source) != NULL)
    {
      ufdbLogFatalError( "line %d: source %s is already defined in configuration file %s",
		         lineno, source, UFDBglobalConfigFile );
      ufdbFree( source );
      return;
    }
  }

  sp = (struct Source *) ufdbMalloc( sizeof(struct Source) );
  sp->name = source;
  sp->active = 1;
  sp->ip = NULL;
  sp->proxy_port = 0;
  sp->lastip = NULL;
  sp->domainDb = NULL;
  sp->userDb = NULL;
  sp->time = NULL;
  sp->within = UFDB_ACL_NONE;
  sp->cont_search = 0;
  sp->next = NULL;
  sp->userquota.seconds = 0;
  sp->userquota.renew = 0;
  sp->userquota.sporadic = 0;

  if (Source == NULL) {
    Source = sp;
    lastSource = sp;
  } else {
    lastSource->next = sp;
    lastSource = sp;
  }
}


void sgSourceEnd( void )
{
  struct Source * s;

  s = lastSource;
  if (s->ip == NULL  &&  s->domainDb == NULL  &&  s->userDb == NULL && s->proxy_port == NULL)
  {
    ufdbLogError( "source \"%s\" missing active content, set inactive", s->name );
    s->time = NULL;
    s->active = 0;
  }
}


void ufdbSourceUser( 
  char *               user )
{
  char *               lc;
  struct Source *      sp;
  struct UserQuotaInfo q;

  sp = lastSource;
  if (sp->userDb == NULL)
    sp->userDb = (void *) UFDBmemDBinit();

  for (lc = user; *lc != '\0'; lc++)    /* convert username to lowercase chars */
  {
    if (*lc >= 'A'  &&  *lc <= 'Z')
       *lc += 'a' - 'A';
  }

  UFDBmemDBinsert( (struct UFDBmemDB *) sp->userDb, user, (char *) setuserquota(&q),
	           sizeof(struct UserQuotaInfo) );
  ufdbFree( user );
}


static void ufdbSourceUnixGroup(
   char *          groupName  )
{
   struct Source * sp;
   struct group *  grp;
   int             n;
   char *          user;

   sp = lastSource;
   if (sp->userDb == NULL) 
      sp->userDb = (void *) UFDBmemDBinit();
 
   /* walk through the /etc/group file with getgrent() to be sure that groups
    * that are on multiple lines are completely parsed.
    */
   n = 0;
   setgrent();
   while ((grp = getgrent()) != NULL)
   {
      if (strcmp( grp->gr_name, groupName ) == 0)
      {
         int i;

         n++;
	 for (i = 0; (user = grp->gr_mem[i]) != NULL; i++)
	 {
	    char * lc;
	    struct UserQuotaInfo q;

	    for (lc = user; *lc != '\0'; lc++)  	/* convert username to lowercase chars */
	    {
	      if (*lc >= 'A'  &&  *lc <= 'Z')
		 *lc += 'a' - 'A';
	    }
	    UFDBmemDBinsert( (struct UFDBmemDB *) sp->userDb, user, (char *) setuserquota(&q),
			     sizeof(struct UserQuotaInfo));
	 }
      }
   }
   endgrent();

   if (n == 0)
   {
      ufdbLogFatalError( "%s: %s is not a unix group", UFDBglobalConfigFile, groupName );
      return;
   }
}


static void ufdbSourceGroup( 
   int    groupType,
   char * groupName  )
{
   switch (groupType)
   {
   case UFDB_GROUPTYPE_UNIX:
      ufdbSourceUnixGroup( groupName );
      break;
   default:     
      ufdbLogFatalError( "ufdbSourceGroup: unknown group type %d", groupType );
   }

   ufdbFree( groupName );
}


void ufdbSourceUserList( 
  char * file  )
{
  char * dbhome;
  char * f;
  FILE * fp;
  char * p;
  char * c;
  char * s;
  char * lc;
  char * lineptr;
  int    ullineno;
  struct Source * sp;
  struct UserQuotaInfo q;
  char   line[10000];

  sp = lastSource;
  if (sp->userDb == NULL) 
    sp->userDb = (void *) UFDBmemDBinit();
 
  dbhome = ufdbSettingGetValue( "dbhome" );
  if (dbhome == NULL)
    dbhome = DEFAULT_DBHOME;

  if (file[0] == '/')
    f = file;
  else
  {
    f = (char *) ufdbMalloc( strlen(dbhome) + strlen(file) + 2 );
    strcpy( f, dbhome );
    strcat( f, "/" );
    strcat( f, file );
    ufdbFree( file );
  }

  if ((fp = fopen(f,"r")) == NULL)
  {
    ufdbLogError( "line %d: can't open userlist %s: %s  *****", lineno, f, strerror(errno) );
    ufdbFree( f );
    return;
  }
  ullineno = 0;

  while (fgets(line,sizeof(line),fp) != NULL)
  {
    ullineno++;
    if (line[0] == '#')			/* skip comments */
      continue;

    p = strchr( line, '\n' );
    if (p != NULL)
    {
      *p = '\0';
      if (p != line) 
      {
	 if (*(p - 1) == '\r') 		/* remove ^M  */
	   *(p-1) = '\0';
      }
    }

    if (line[0] == '\0')
    {
       ufdbLogError( "userlist %s: line %d: line is empty", f, ullineno );
       continue;
    }

    c = strchr( line, '#' );		/* remove comment */
    if (c != NULL)
       *c = '\0';

    p = strtok_r( line, " \t,", &lineptr );
    if (p == NULL  ||  *p == '\0')
       continue;

    if ((s = strchr(p,':')) != NULL) 
      *s = '\0';

    do {
       for (lc = p;  *lc != '\0';  lc++)   /* convert username to lowercase chars */
       {
	  if (*lc >= 'A'  &&  *lc <= 'Z')
	     *lc += 'a' - 'A';
       }
       if (*p != '\0')
	  UFDBmemDBinsert( (struct UFDBmemDB *) sp->userDb, p, (char *) setuserquota(&q),
			   sizeof(struct UserQuotaInfo) );
    } while ((p = strtok_r(NULL," \t,",&lineptr)) != NULL  &&  *p != '\0');
  }
  fclose( fp );

#if defined(UFDB_DEBUG)
  ufdbLogMessage( "userlist %s", f );
  UFDBmemDBprintUserDB( (struct UFDBmemDB *) sp->userDb );
#endif

  ufdbFree( f );
}


static void ufdbSourceUserQuota( 
   char * seconds, 
   char * sporadic,  
   char * renew )
{
   int    s;
   struct UserQuota * uq;
   struct Source * sp;
 
   sp = lastSource;
   uq = &sp->userquota;
   s = atoi(seconds);
   if (s <= 0)
      ufdbLogError( "line %d: userquota seconds sporadic hourly|daily|weekly", lineno );
   uq->seconds = s; 
   s = atoi(sporadic);
   if (s <= 0)
      ufdbLogError( "line %d: userquota seconds sporadic hourly|daily|weekly", lineno );
   uq->sporadic = s; 
   s = atoi(renew);
   if (s <= 0)
      ufdbLogError( "line %d: userquota seconds sporadic hourly|daily|weekly", lineno );
   uq->renew = s;
}


void sgSourceDomain( 
   char * domain )
{
   struct Source * sp;

   sp = lastSource;
   if (sp->domainDb == NULL) 
      sp->domainDb = (struct sgDb *) UFDBmemDBinit();
   UFDBmemDBinsert( (struct UFDBmemDB *) sp->domainDb, domain, NULL, 0 );
   ufdbFree( domain );
}


void sgSourceTime( 
   char *         name, 
   int            within )
{
   struct Time *  t;

   if ((t = sgTimeFindName(name)) == NULL)
   {
      ufdbLogFatalError( "line %d: time \"%s\" is not defined in configuration file %s",
		         lineno, name, UFDBglobalConfigFile );
      ufdbFree( name );
      return;
   }

   lastSource->within = within;
   lastSource->time = t;
   ufdbFree( name );
}


struct Source * sgSourceFindName( 
   char *          name )
{
   struct Source * p;

   for (p = Source; p != NULL; p = p->next)
   {
     if (strcmp(name,p->name) == 0)
        return p;
   }

   return NULL;
}


/*
 * TODO: make this more efficient for a large list.
 */
void sgSourceIpList( 
  char * file )
{
  char * dbhome = NULL;
  char * f;
  FILE * fp;
  char * p;
  char * c;
  char * cidr;
  int    i;
  int    l = 0;
  char * lineptr;
  char   line[UFDB_MAX_URL_LENGTH];

  dbhome = ufdbSettingGetValue( "dbhome" );
  if (dbhome == NULL)
    dbhome = DEFAULT_DBHOME;

  if (file[0] == '/') 
  {
    f = file;
  }
  else
  {
    f = (char *) ufdbMalloc( strlen(dbhome) + strlen(file) + 2 );
    strcpy( f, dbhome );
    strcat( f, "/" );
    strcat( f, file );
    ufdbFree( file );
  }

  if ((fp = fopen(f,"r")) == NULL) 
  {
    ufdbLogError( "line %d: can't open iplist %s: %s", lineno, f, strerror(errno) );
    ufdbFree( f );
    return;
  }

  ufdbLogMessage( "init iplist \"%s\"", f );

  while (fgets(line,sizeof(line),fp) != NULL)
  {
    l++;
    if (*line == '#')
      continue;
    p = strchr( line, '\n' );
    if (p != NULL && p != line) 
    {
      if (*(p - 1) == '\r') 		/* removing ^M  */
	p--;
      *p = '\0';
    }
    c = strchr( line, '#' );
    p = strtok_r( line, " \t,", &lineptr );
    do {
      if (c != NULL && p >= c) 		/* find the comment */
	break;
      i = strspn( p, ".0123456789/-" );
      if (i == 0)
	break;
      *(p + i) = '\0';
      if ((cidr = strchr(p,'/')) != NULL) 
      {
	*cidr = '\0';
	cidr++;
	sgIp( p );
	sgIp( cidr );
	if (strchr(cidr,'.') == NULL)
	  sgSetIpType( SG_IPTYPE_CIDR, f, l );
	else 
	  sgSetIpType( SG_IPTYPE_CLASS, f, l );
      }
      else if ((cidr = strchr(p,'-')) != NULL) 
      {
	*cidr = '\0';
	cidr++;
	sgIp( p );
	sgIp( cidr );
	sgSetIpType( SG_IPTYPE_RANGE, f, l );
      }
      else
      {
	sgIp( p );
	sgIp( ufdbStrdup("255.255.255.255") );
	sgSetIpType( SG_IPTYPE_HOST, f, l );
      }
    } while ((p = strtok_r(NULL," \t,",&lineptr)) != NULL);
  }

  fclose( fp );
  ufdbFree( f );
}


/*
 *  called by main loop: find a source definition based on the user ident/domain/ip
 */
struct Source * sgFindSource( 
  struct Source *    allSrcs, 
  char *             net, 
  struct SquidInfo * si )
{
  struct Source *    s;
  struct Ip *        ip;
  int                foundip, founduser, founddomain, foundport, unblockeduser;
  unsigned long      i, octet, op;

  if (UFDBglobalFatalError  ||  UFDBglobalReconfig)
  {
#ifdef UFDB_DEBUG_ACL_ACCESS
     ufdbLogMessage( "   sgFindSource: fatal-error or reconfig: returning NULL" );
#endif
     return NULL;
  }

#if defined(UFDB_USERQUOTA_SUPPORT) || defined(UFDB_DEBUG_ACL_ACCESS)
  if (UFDBglobalDebug)
    ufdbLogMessage( "   sgFindSource( [%s], %s )", allSrcs->name, si->ident );
#endif

  octet = 0;
  if (net != NULL)
  {
    if (sgConvDot(net,&op) != NULL)
      octet = op;
  }

  for (s = allSrcs;  s != NULL;  s = s->next)
  {
    if (s->active == 0)
      continue;

    foundip = founduser = founddomain = foundport = 0;
    unblockeduser = 1;
    
    if(s->proxy_port != 0){
      if(s->proxy_port == si->proxy_port){
        ufdbLogMessage("sgFindSource: proxy port matched");
        foundport = 1;
      } 
    }
    else {
      ufdbLogMessage("sgFindSource: no proxy port");
      foundport = 1;
    }
    
    if (s->ip != NULL)
    {
      if (net == NULL)
	foundip = 0;
      else 
      {
	for (ip = s->ip;  ip != NULL;  ip = ip->next)
	{
	  if (!ip->net_is_set)
	    continue;

	  if (ip->type == SG_IPTYPE_RANGE) 
	  {
	    if (octet >= ip->net  &&  octet <= ip->mask) {
	      foundip = 1;
	      break;
	    }
	  }
	  else				/* CIDR or HOST */
	  { 
	    i = octet & ip->mask;
	    if (i == ip->net) {
	      foundip = 1;
	      break;
	    }
	  }
	}
      }
    }
    else
      foundip = 1;

#if defined(UFDB_USERQUOTA_SUPPORT) || defined(UFDB_DEBUG_ACL_ACCESS)
   if (UFDBglobalDebug)
      ufdbLogMessage( "   sgFindSource  s->name = %s  s->userDb = %08x", s->name==NULL ? "NULL" : s->name, s->userDb );
#endif

    if (s->userDb != NULL)
    {
      if (si->ident[0] == '\0'  ||
          (si->ident[0] == '-'  &&  si->ident[1] == '\0'))
      {
	founduser = 0;
      }
      else 
      {
        struct UserQuotaInfo * q = NULL;

	/* see if the user has any restriction */
	if (UFDBmemDBfind( (struct UFDBmemDB *) s->userDb, si->ident, (char **) (char *) &q ) == 1)
	{
#if UFDB_LOG_USER_QUOTA
          ufdbLogMessage( "found user \"%s\" in userdb   q=%08x", si->ident, q );
#endif
	  founduser = 1;
	  unblockeduser = 1;
#if UFDB_USERQUOTA_SUPPORT
	  /* see if user has any time-related restriction */
	  if (s->userquota.seconds != 0) 
	  {
	    time_t t = time(NULL) + globalDebugTimeDelta;
	    /* TODO: find out why q might be NULL */
#if UFDB_LOG_USER_QUOTA
	    ufdbLogMessage( "   status %d time %d lasttime %d consumed %d", 
	                    q->status, q->time, q->last, q->consumed );
	    ufdbLogMessage( "   renew %d seconds %d", s->userquota.renew, s->userquota.seconds );
#endif
	    if (q->status == UDFB_Q_STAT_FIRSTTIME)
	    {
	      q->status = UDFB_Q_STAT_ACTIVE;
	      q->time = t;
	      q->last = t;
#if UFDB_LOG_USER_QUOTA
	      ufdbLogMessage( "   user %s first time %d", si->ident, q->time );
#endif
	    }
	    else if (q->status == UDFB_Q_STAT_ACTIVE)
	    {
#if UFDB_LOG_USER_QUOTA
	      ufdbLogMessage( "   user %s other time %d %d", si->ident, q->time, t );
#endif
	      if (s->userquota.sporadic > 0) 
	      {
		if (t - q->last  < s->userquota.sporadic) 
		{
		  q->consumed += t - q->last;
		  q->time = t;
		}
		if (q->consumed > s->userquota.seconds) 
		{
		  q->status = UDFB_Q_STAT_TIMEISUP;
		  unblockeduser = 0;
		}
		q->last = t;
#if UFDB_LOG_USER_QUOTA
		ufdbLogMessage( "user %s consumed %d %d", si->ident, q->consumed, q->last );
#endif
	      }
	      else if (q->time + s->userquota.seconds < t) 
	      {
		ufdbLogError( "time is up: user %s is blocked", si->ident);
		q->status = UDFB_Q_STAT_TIMEISUP;
		unblockeduser = 0;
	      } 
	    }
	    else
	    {
	      ufdbLogError( "user %s blocked %d %d %d %d", 
	                    si->ident, q->status, 
		            q->time, t, (q->time + s->userquota.renew) - t);
	      if (q->time + s->userquota.renew < t)  /* new chance */
	      {
#if UFDB_LOG_USER_QUOTA
		ufdbLogMessage( "user %s new chance", si->ident );
#endif
		unblockeduser = 1;
		q->status = UDFB_Q_STAT_ACTIVE;
		q->time = t;
		q->consumed = 0;
	      }
	      else 
		unblockeduser = 0;
	    }
	    UFDBmemDBinsert( (struct UFDBmemDB *) s->userDb, si->ident, (char *) q,
		             sizeof(struct UserQuotaInfo) );
	  }
#endif  /* UFDB_USERQUOTA_SUPPORT */
	}
      }
    }
    else
      founduser = 1;

    if (s->domainDb != NULL)
    {
      if (si->srcDomain[0] == '\0')
	founddomain = 0;
      else 
      {
	if (UFDBmemDBfind( (struct UFDBmemDB *) s->domainDb, si->srcDomain, (char **) NULL) == 1)
	  founddomain = 1;
      }
    }
    else
      founddomain = 1;

    if (founduser && foundip && founddomain && foundport)
    {
      if (unblockeduser)
	return s;
      else 
      {
	si->lastActiveSource = s;
	return NULL;
      }
    }
  }

  return NULL;
}


/* category block functions */

void ufdbCategory( 
  char *            cat )
{
  struct Category * sp;

#if defined(UFDB_DEBUG)
   ufdbLogMessage( "ufdbCategory %s", cat );
#endif

  if (Dest != NULL) 
  {
    if ((struct Category *) ufdbCategoryFindName(cat) != NULL)
    {
      ufdbLogFatalError( "line %d: category %s is already defined in configuration file %s",
		         cat, UFDBglobalConfigFile );
      ufdbFree( cat );
      return;
    }
  }

  sp = (struct Category *) ufdbMalloc( sizeof(struct Category) );
  sp->name = cat;
  sp->active = 1;
  sp->domainlist = NULL;
  sp->domainlistDb = NULL;
  sp->cse = NULL;
  sp->expressionlist = NULL;
  sp->regExp = NULL;
  sp->rewrite = NULL;
  sp->redirect = NULL;
  sp->options = 0;
  sp->time = NULL;
  sp->within = UFDB_ACL_NONE;
  sp->next = NULL;

  if (Dest == NULL) {
    Dest = sp;
    lastDest = sp;
  } else {
    lastDest->next = sp;
    lastDest = sp;
  }
}


void ufdbCategoryEnd( void )
{
  struct Category * d;
 
  d = lastDest;
  if (d->domainlist == NULL  &&  d->expressionlist == NULL  && 
      d->redirect == NULL  &&  d->rewrite == NULL  &&
      d->options == 0)
  {
    ufdbLogError( "category \"%s\" is missing content, set inactive", d->name );
    d->time = NULL;
    d->active = 0;
  }
}


void ufdbCategoryOption( int value, int option )
{
   struct Category * sp;

   sp = lastDest;
   if (value)
      sp->options = sp->options | option;
   else
      sp->options = sp->options & (~option);

   if (option == UFDB_OPT_HTTPS_WITH_HOSTNAME)
      UFDBglobalHttpsWithHostname = value;
   else if (option == UFDB_OPT_HTTPS_OFFICAL_CERTIFICATE)
      UFDBglobalHttpsOfficialCertificate = value;
   else if (option == UFDB_OPT_SKYPE_OVER_HTTPS)
      UFDBglobalSkypeOverHttps = value;
   else if (option == UFDB_OPT_GTALK_OVER_HTTPS)
      UFDBglobalGtalkOverHttps = value;
   else if (option == UFDB_OPT_YAHOOMSG_OVER_HTTPS)
      UFDBglobalYahooMsgOverHttps = value;
   else if (option == UFDB_OPT_AIM_OVER_HTTPS)
      UFDBglobalAimOverHttps = value;
   else if (option == UFDB_OPT_PROHIBIT_INSECURE_SSLV2)
      UFDBglobalHttpsNoSSLv2 = value;
   else if (option == UFDB_OPT_UNKNOWN_PROTOCOL_OVER_HTTPS)
      UFDBglobalUnknownProtocolOverHttps = value;
   else if (option == UFDB_OPT_SAFE_SEARCH)
      ;
   else
      ufdbLogError( "ufdbCategoryOption: unrecognised option %d *****", option );

   if (UFDBglobalDebug)
      ufdbLogMessage( "ufdbCategoryOption: %s: option %d set to %d", sp->name, option, value );
}


void ufdbCategoryDomainList( 
  char * domainlist )
{
  struct Category * sp;
  char * dbhome = NULL;
  char * dl = domainlist;
  char * name;

#if defined(UFDB_DEBUG)
   ufdbLogMessage( "ufdbCategoryDomainList %s", domainlist );
#endif

  dbhome = ufdbSettingGetValue("dbhome");
  sp = lastDest;
  if (dbhome == NULL)
    dbhome = DEFAULT_DBHOME;

  if (domainlist == NULL)
  {
    name = sp->name;
    dl = (char *) ufdbMalloc( sizeof("/dest/") + strlen(name) + sizeof("/domainlist") + 2 );
    strcpy(dl,"/dest/");
    strcat(dl,name);
    strcat(dl,"/domainlist");

    sp->domainlist = (char *) ufdbMalloc( strlen(dbhome) + strlen(dl) + 2 );
    strcpy(sp->domainlist,dbhome);
    strcat(sp->domainlist,"/");
    strcat(sp->domainlist,dl);
  }
  else
  {
    if (domainlist[0] == '/') 
      sp->domainlist = domainlist;
    else
    {
       sp->domainlist = (char *) ufdbMalloc( strlen(dbhome) + strlen(domainlist) + 2 );
       strcpy( sp->domainlist, dbhome );
       strcat( sp->domainlist, "/" );
       strcat( sp->domainlist, domainlist );
       ufdbFree( domainlist );
    }
  }

  sp->domainlistDb = (struct sgDb *) ufdbCalloc( 1, sizeof(struct sgDb) );
  sp->domainlistDb->type = SGDBTYPE_DOMAINLIST;
  sgDbInit( sp->domainlistDb, sp->domainlist );
  ufdbLogMessage( "init domainlist \"%s\"", sp->domainlist );
  if (sp->domainlistDb->dbcp == NULL  ||
      ((struct UFDBmemTable *) sp->domainlistDb->dbcp)->table.nNextLevels == 0)
  {
    ufdbLogMessage( "database table %s is empty !", sp->domainlist );
    ufdbFreeDomainDb( sp->domainlistDb );
    sp->domainlistDb = NULL;
  }
}


void ufdbFreeDomainDb( 
   struct sgDb * dbp )
{
   struct UFDBmemTable * mt;

   if (dbp == NULL)
      return;

   mt = (struct UFDBmemTable *) dbp->dbcp;
   if (mt != NULL)
   {
      if (mt->index != NULL)
	 ufdbFree( mt->index );				/* version 2.0 */
      else
	 UFDBfreeTableIndex_1_2( &(mt->table) ); 	/* version 1.2 */
      ufdbFree( (void *) mt->mem );
   }
   ufdbFree( mt );
   ufdbFree( dbp );
}


void ufdbFreeIpList( struct Ip * p )
{
   if (p == NULL)
      return;

   ufdbFreeIpList( p->next );
   if (p->str != NULL)
      ufdbFree( p->str );
   ufdbFree( p );
}


void ufdbFreeSourceList( struct Source * p )
{
   if (p == NULL)
      return;

   ufdbFreeSourceList( p->next );

   ufdbFreeIpList( p->ip );
   ufdbFree( p->name );
   UFDBmemDBdeleteDB( (struct UFDBmemDB *) p->domainDb );
   UFDBmemDBdeleteDB( (struct UFDBmemDB *) p->userDb );

   ufdbFree( p );
}


void ufdbFreeAclDestList( struct AclDest * p )
{
   if (p == NULL)
      return;

   ufdbFreeAclDestList( p->next );
   ufdbFree( p->name );
   ufdbFree( p );
}


void ufdbFreeAclList( struct Acl * p )
{
   if (p == NULL)
      return;

   ufdbFreeAclList( p->next );

   ufdbFree( p->name );
   ufdbFreeAclDestList( p->pass );
   ufdbFree( p->redirect );

   ufdbFree( p );
}


void ufdbFreeSettingList( struct ufdbSetting * p )
{
   if (p == NULL)
      return;

   ufdbFreeSettingList( p->next );
   ufdbFree( p->name );
   ufdbFree( p->value );
   ufdbFree( p );
}


static void ufdbFreeCategoryDomainList( struct Category * p )
{
   if (p == NULL)
      return;

   ufdbFreeCategoryDomainList( p->next );

   ufdbFree( p->name );
   ufdbFree( p->domainlist );
   ufdbFreeDomainDb( p->domainlistDb );
   ufdbFree( p->cse );
   ufdbFree( p->expressionlist );
   ufdbFreeRegExprList( p->regExp );
   ufdbFree( p->redirect );
   ufdbFree( p->rewrite );
   ufdbFree( p );
}


void ufdbFreeRewriteList( 
   struct sgRewrite * p )
{
   if (p == NULL)
      return;

   ufdbFreeRegExprList( p->rewrite );
   ufdbFreeRewriteList( p->next );
   ufdbFree( p->name );
   ufdbFree( p );
}


void ufdbFreeTimeElement( 
   struct TimeElement * t )
{
   if (t == NULL)
      return;

   ufdbFreeTimeElement( t->next );
   ufdbFree( t );
}


void ufdbFreeTime( 
   struct Time * t )
{
   if (t == NULL)
      return;

   ufdbFreeTime( t->next );
   ufdbFree( t->name );
   ufdbFreeTimeElement( t->element );
   ufdbFree( t );
}


void ufdbFreeAllMemory( void )
{
   if (UFDBglobalLogDir != NULL)
      ufdbFree( UFDBglobalLogDir );
   UFDBglobalLogDir = NULL;
   globalLogFile = NULL;

   if (UFDBglobalEmailServer != NULL)
   {
      ufdbFree( UFDBglobalEmailServer );
      UFDBglobalEmailServer = NULL;
   }
   if (UFDBglobalAdminEmail != NULL)
   {
      ufdbFree( UFDBglobalAdminEmail );
      UFDBglobalAdminEmail = NULL;
   }
   if (UFDBglobalExternalStatusCommand != NULL)
   {
      ufdbFree( UFDBglobalExternalStatusCommand );
      UFDBglobalExternalStatusCommand = NULL;
   }

   ufdbFreeSettingList( Setting );
   ufdbFreeRewriteList( Rewrite );
   ufdbFreeCategoryDomainList( Dest );
   ufdbFreeAclList( Acl );
   ufdbFreeSourceList( Source );

   /* free UFDBglobalCheckedDB */
   if (UFDBglobalCheckedDB.table.nNextLevels > 0  &&  UFDBglobalCheckedDB.mem != NULL)
   {
      if (UFDBglobalCheckedDB.index != NULL)
         ufdbFree( (void *) UFDBglobalCheckedDB.index );
      else
	 UFDBfreeTableIndex_1_2( &(UFDBglobalCheckedDB.table) ); 		/* .nextLevel[0]) ); */
      ufdbFree( (void *) UFDBglobalCheckedDB.mem );
      UFDBglobalCheckedDB.table.nNextLevels = 0;
      UFDBglobalCheckedDB.mem = NULL;
      UFDBglobalCheckedDB.index = NULL;
   }
   if (UFDBglobalCheckedExpressions != NULL)
   {
      ufdbFreeRegExprList( UFDBglobalCheckedExpressions );
      UFDBglobalCheckedExpressions = NULL;
   }

   /* set all pointers back to NULL */

   lastSetting = NULL;
   Setting = NULL;

   lastSource = NULL;
   Source = NULL;
   /*** saveSource = NULL; ***/

   lastDest = NULL;
   Dest = NULL;

   lastRewrite = NULL;
   Rewrite = NULL;
   lastRewriteRegExec = NULL;

   ufdbFreeTime( Time );
   Time = NULL;
   lastTime = NULL;

   ufdbFree( TimeElementsEvents );
   TimeElementsEvents = NULL;

   lastTimeElement = NULL;
   TimeElement = NULL;

   lastAcl = NULL;
   defaultAcl = NULL;
   Acl = NULL;
   lastAclDest = NULL;

   lastRegExpDest = NULL;
}


void ufdbCategoryUrlList( 
   char * urllist )
{
   if (urllist == NULL)
      urllist = "-";
   ufdbLogError( "line %d: \"urllist %s\" is deprecated and ignored *****\n"
                 "ufdbGenTable should be called with the -u option to include URLs",
		 lineno, urllist );
   if (urllist[0] != '-')
      ufdbFree( urllist );
}


void ufdbCategoryExpressionList( 
  char * exprlist, 
  char * chcase  )
{
  FILE * fp;
  char * dbhome;
  char * dl;
  char * name;
  char * p;
  int    flags;
  struct Category *   sp;
  struct ufdbRegExp * regexp;
  char   buf[UFDB_MAX_URL_LENGTH];
  char   errbuf[256];

#if defined(UFDB_DEBUG)
   ufdbLogMessage( "ufdbCategoryExpressionList %s", exprlist );
#endif

  flags = REG_EXTENDED | REG_NOSUB;
  dbhome = ufdbSettingGetValue( "dbhome" );
  sp = lastDest;
  if (dbhome == NULL)
    dbhome = DEFAULT_DBHOME;

  if (exprlist == NULL)
  {
    name = sp->name;
    dl = (char *) ufdbMalloc( sizeof("/dest/") + strlen(name) + sizeof("/expressionlist") + 2 );
    strcpy(dl,"/dest/");
    strcat(dl,name);
    strcat(dl,"/expressionlist");

    flags |= REG_ICASE; 	/* default case insensitive */

    sp->expressionlist = (char *) ufdbMalloc( strlen(dbhome) + strlen(dl) + 2 );
    strcpy(sp->expressionlist,dbhome);
    strcat(sp->expressionlist,"/");
    strcat(sp->expressionlist,dl);
  }
  else
  {
    if (exprlist[0] == '/') 
    {
      sp->expressionlist = exprlist;
    }
    else
    {
       sp->expressionlist = (char *) ufdbMalloc( strlen(dbhome) + strlen(exprlist) + 2 );
       strcpy( sp->expressionlist, dbhome );
       strcat( sp->expressionlist, "/" );
       strcat( sp->expressionlist, exprlist );
       ufdbFree( exprlist );
    }
    if (*chcase == 'i')
       flags |= REG_ICASE;     /* set case insensitive */
  }

  ufdbLogMessage( "init expressionlist \"%s\"", sp->expressionlist );

  if ((fp = fopen(sp->expressionlist, "r")) == NULL) 
  {
    ufdbLogFatalError( "expression list %s: %s", sp->expressionlist, strerror(errno) );
    return;
  }

  while (!feof(fp)  &&  fgets(buf, sizeof(buf), fp) != NULL) 
  {
    if (buf[0] == '#')
       continue;

    p = (char *) strchr( buf, '\n' );
    if (p != NULL  &&  p != buf) 
    {
      if (*(p-1) == '\r') 	/* removing ^M  */
	p--;
      *p = '\0';
    }
    regexp = ufdbNewPatternBuffer( buf, flags );
    if (regexp->error) 
    {
      regerror( regexp->error, regexp->compiled, errbuf, sizeof(errbuf) );
      ufdbLogError( "error in %s: %s: %s", sp->expressionlist, strerror(errno), buf );
    }
    if (lastDest->regExp == NULL) 
    {
      lastDest->regExp = regexp;
      lastRegExpDest = regexp;
    }
    else
    {
      lastRegExpDest->next = regexp;
      lastRegExpDest = regexp;
    }
  }

  fclose( fp );

  if (UFDBglobalExpressionOptimisation)
     lastDest->regExp = UFDBoptimizeExprList( sp->expressionlist, lastDest->regExp );
}


void ufdbCategoryCACerts(
   char *              cacerts )
{
   struct Category *   cat;

   cat = lastDest;
   if (cat == NULL  ||  strcmp( cat->name, "security" ) != 0)
   {
      ufdbLogFatalError( "cacerts can only be defined inside the \"security\" category" );
      return;
   }

   cat->cse = cacerts;
   if (*cacerts == '/')
      strcpy( UFDBglobalCAcertsFile, cacerts );
   else
   {
      char * dbh;
      dbh = ufdbSettingGetValue( "dbhome" );
      if (dbh == NULL)
         dbh = DEFAULT_DBHOME;
      strcpy( UFDBglobalCAcertsFile, dbh );
      strcat( UFDBglobalCAcertsFile, "/" );
      strcat( UFDBglobalCAcertsFile, cacerts );
   }
}


void ufdbCategoryRedirect( 
   char * value )
{
   struct Category * sp;

#if defined(UFDB_DEBUG)
   ufdbLogMessage( "ufdbCategoryRedirect %s", value );
#endif

   sp = lastDest;
   sp->redirect = value;
   /* TODO: check that "localhost" is not here if no 302 is used */
}


void ufdbCategoryRewrite( 
  char *              value )
{
  struct sgRewrite *  rewrite;

  if ((rewrite = sgRewriteFindName(value)) == NULL)
  {
    ufdbLogFatalError( "line %d: rewrite %s is not defined in configuration file %s",
		       lineno, value, UFDBglobalConfigFile );
    return;
  }

  lastDest->rewrite = rewrite;
}


void ufdbCategoryTime(  
  char *        name, 
  int  		within )
{
  struct Time * t;

  if ((t = sgTimeFindName(name)) == NULL) 
  {
    ufdbLogFatalError( "line %d: time \"%s\" is not defined in configuration file %s",
		       lineno, name, UFDBglobalConfigFile );
    ufdbFree( name );
    return;
  }

  lastDest->within = within;
  lastDest->time = t;
  ufdbFree( name );
}


struct Category * ufdbCategoryFindName( 
  char *               name )
{
  struct Category * p;

  for (p = Dest; p != NULL; p = p->next)
  {
    if (strcmp(name,p->name) == 0)
      return p;
  }

  return NULL;
}


void sgRewrite( 
  char * rewrite )
{
  struct sgRewrite * rew;

#if defined(UFDB_DEBUG)
   ufdbLogMessage( "sgRewrite %s", rewrite );
#endif

  if (Rewrite != NULL)
  {
    if ((struct sgRewrite *) sgRewriteFindName(rewrite) != NULL)
    {
      ufdbLogFatalError( "line %d: rewrite \"%s\" is not defined in configuration file %s",
		       lineno, rewrite, UFDBglobalConfigFile );
      ufdbFree( rewrite );
      return;
    }
  }

  rew = (struct sgRewrite *) ufdbMalloc( sizeof(struct sgRewrite) );
  rew->name = rewrite;
  rew->active = 1;
  rew->rewrite = NULL;
  rew->time = NULL;
  rew->within = UFDB_ACL_NONE;
  rew->next = NULL;

  if (Rewrite == NULL) 
  {
    Rewrite = rew;
    lastRewrite = rew;
  }
  else
  {
    lastRewrite->next = rew;
    lastRewrite = rew;
  }
}


void sgRewriteTime( 
  char * name, 
  int    within )
{
  struct Time * t;

#if defined(UFDB_DEBUG)
   ufdbLogMessage( "sgRewriteTime %s %d", name, within );
#endif

  if ((t = sgTimeFindName(name)) == NULL)
  {
    ufdbLogFatalError( "line %d: time \"%s\" is not defined in configuration file %s",
		       lineno, name, UFDBglobalConfigFile );
    ufdbFree( name );
    return;
  }

  lastRewrite->within = within;
  lastRewrite->time = t;
  ufdbFree( name );
}


void sgRewriteSubstitute( 
  char * string )
{
  char * pattern; 
  char * subst = NULL;
  char * p;
  int    flags = REG_EXTENDED;
  int    global = 0;
  char * httpcode = NULL;
  struct ufdbRegExp * regexp;
  char   errbuf[256];

  pattern = string + 2; 	/* skipping s@ */
  p = pattern;
  while ((p = strchr(p,'@')) != NULL)
  {
    if (*(p - 1) != '\\') 
    {
      *p = '\0';
      subst = p + 1;
      break;
    }
    p++;
  }

  p = strrchr( subst, '@' );
  while (p != NULL  &&  *p != '\0')
  {
    if (*p == 'r' )
      httpcode =  REDIRECT_TEMPORARILY;
    if (*p == 'R' )
      httpcode =  REDIRECT_PERMANENT;
    if (*p == 'i' || *p == 'I')
      flags |= REG_ICASE;
    if (*p == 'g')
      global = 1;
    *p = '\0'; 		/* removes @i from string */
    p++;
  } 

  regexp = ufdbNewPatternBuffer( pattern, flags );
  if (regexp->error) 
  {
      regerror( regexp->error, regexp->compiled, errbuf, sizeof(errbuf) );
      ufdbLogError( "line %d: error in regexp %s: %s *****", lineno, pattern, errbuf );
  }
  else {
    regexp->substitute = ufdbStrdup( subst );
  }

  if (lastRewrite->rewrite == NULL)
    lastRewrite->rewrite = regexp;
  else 
    lastRewriteRegExec->next = regexp;
  regexp->httpcode = httpcode;
  regexp->global = global;
  lastRewriteRegExec = regexp;
}


struct sgRewrite * sgRewriteFindName( 
  char * name )
{
  struct sgRewrite * p;

  for (p = Rewrite; p != NULL; p = p->next)
  {
    if (strcmp(name,p->name) == 0)
      return p;
  }

  return NULL;
}


/*
 * Time functions
 */

/*
 * sgTime - parse configuration time statement.
 */
void sgTime( 
  char *        name )
{
  struct Time * t;

#if defined(UFDB_DEBUG)
   ufdbLogMessage( "sgTime %s", name );
#endif

  if (Time != NULL)
  {
    if ((struct Time *) sgTimeFindName(name) != NULL)
    {
      ufdbLogFatalError( "line %d: time \"%s\" is not defined in configuration file %s",
		         lineno, name, UFDBglobalConfigFile );
      ufdbFree( name );
      return;
    }
  }
  else 
    numTimeElements = 0;

  t = (struct Time *) ufdbMalloc( sizeof(struct Time) );
  t->name = name;
  t->active = 1;
  t->element = NULL;
  t->next = NULL;

  TimeElement = NULL;
  lastTimeElement = NULL;
  if (Time == NULL) 
  {
    Time = t;
    lastTime = t;
  }
  else
  {
    lastTime->next = t;
    lastTime = t;
  }
}


/*
 * sgTimeElementInit - initialise parsing of a configuration time element.
 */
void sgTimeElementInit( void )
{
   struct TimeElement * te;

   te = (struct TimeElement *) ufdbCalloc( 1, sizeof(struct TimeElement) );
   numTimeElements++;
   if (lastTime->element == NULL)
     lastTime->element = te;
   if (lastTimeElement != NULL)
     lastTimeElement->next = te;
   lastTimeElement = te;
}


/*
 * sgTimeElementEnd - finalise parsing of configuration time element.
 */
void sgTimeElementEnd( void )
{
  time_switch = 0;
  date_switch = 0;

  if (lastTimeElement->fromdate != 0)
  {
    if (lastTimeElement->todate == 0)
      lastTimeElement->todate = lastTimeElement->fromdate + 86399;
    else 
      lastTimeElement->todate = lastTimeElement->todate + 86399;
  }

  if (lastTimeElement->from == 0  &&  lastTimeElement->to == 0)
    lastTimeElement->to = 1439;  /* set time to 23:59 */
}


/*
 * sgTimeElementAdd - add configuration time element.
 */
void sgTimeElementAdd( 
  char * element, 
  char   type ) 
{
  struct TimeElement * te;
  char * p;
  char   wday;
  int    h, m, Y, M, D;
  time_t sec;
  char * lineptr;

  wday = 0;
  M = 0;
  D = -1;
  te = lastTimeElement;

  switch (type)
  {
  case T_WEEKDAY:
    p = strtok_r( element, " \t,", &lineptr );
    do {
      if (*p == '*') {
	wday = 0x7F;
      } else if (!strncmp(p,"sun",3)) {
	wday = wday | 0x01;
      } else if (!strncmp(p,"mon",3)) {
	wday = wday | 0x02;
      } else if (!strncmp(p,"tue",3)) {
	wday = wday | 0x04;
      } else if (!strncmp(p,"wed",3)) {
	wday = wday | 0x08;
      } else if (!strncmp(p,"thu",3)) {
	wday = wday | 0x10;
      } else if (!strncmp(p,"fri",3)) {
	wday = wday | 0x20;
      } else if (!strncmp(p,"sat",3)) {
	wday = wday | 0x40;
      }
      p = strtok_r( NULL, " \t,", &lineptr );
    } while (p != NULL);
    te->wday = wday;
    break;

  case T_TVAL:
    h = -1;
    m = -1;
    sscanf( element, "%d:%d", &h, &m );
    if ((h < 0 || h > 24) || (m < 0 || m > 59))
    {
      ufdbLogFatalError( "line %d: time format error in %s", lineno, UFDBglobalConfigFile );
      h = 0;
      m = 0;
    }
    if (time_switch == 0) 
    {
      time_switch++;
      te->from = (h * 60) + m ;
    }
    else
    {
      time_switch = 0;
      te->to = (h * 60) + m ;
    }
    break;

  case T_DVAL:
    sec = date2sec( element );
    if (sec == -1) 
    {
      ufdbLogFatalError( "line %d: date format error in %s", lineno, UFDBglobalConfigFile );
      sec = 1;
    }
    if (date_switch == 0) {
      date_switch++;
      te->fromdate = sec;
    } else {
      date_switch = 0;
      te->todate = sec;
    }
    break;

  case T_DVALCRON:
    p = strtok_r( element, "-./", &lineptr );
    Y = atoi(p);
    if (*p == '*')
      Y = -1;
    else
      Y = atoi(p);
    while ((p=strtok_r(NULL,"-./",&lineptr)) != NULL) 
    {
      if (*p == '*')
	if (M == 0)
	  M = -1;
	else 
	  D = -1;
      else
	if (M == 0)
	  M = atoi(p);
	else
	  D = atoi(p);
    }
    te->y = Y;
    te->m = M;
    te->d = D;
    break;

  case T_WEEKLY:
    p = element;
    while (*p != '\0') 
    {
      switch (*p) {
      case 'S':
      case 's':
	wday = wday | 0x01;
	break;
      case 'M':
      case 'm':
	wday = wday | 0x02;
	break;
      case 'T':
      case 't':
	wday = wday | 0x04;
	break;
      case 'W':
      case 'w':
	wday = wday | 0x08;
	break;
      case 'H':
      case 'h':
	wday = wday | 0x10;
	break;
      case 'F':
      case 'f':
	wday = wday | 0x20;
	break;
      case 'A':
      case 'a':
	wday = wday | 0x40;
	break;
      default:
	ufdbLogFatalError( "line %d: weekday format error in %s", lineno, UFDBglobalConfigFile );
	break;
      }
      p++;
    }
    te->wday = wday;
    break;
  }

  ufdbFree( element );
}


/* 
 * lookup a Time element by name.
 */
struct Time * sgTimeFindName( 
  char *        name )
{
  struct Time * p;

  for (p = Time; p != NULL; p = p->next)
  {
    if (strcmp(name,p->name) == 0)
      return p;
  }

  return NULL;
}


/*
 * sgTimeCmp - Time array sort function.
 */
static int sgTimeCmp( const void * a, const void * b )
{
   const int * aa = (const int *) a;
   const int * bb = (const int *) b;

   return *aa - *bb;
}


/*
 * sgTimeElementSortEvents - order all events and leave events in TimeElementsEvents[numTimeElements]
 */
void sgTimeElementSortEvents( void )
{
   struct Time * p;
   struct TimeElement * te;
   int    i, j;
   int *  t;
 
   if (Time == NULL)
      return;

   ufdbFree( TimeElementsEvents );
   TimeElementsEvents = (int *) ufdbCalloc( numTimeElements * 2 , sizeof(int) ); 
   i = 0;
   for (p = Time;  p != NULL;  p = p->next) 
   {
      for (te = p->element;  te != NULL;  te = te->next) 
      {
         TimeElementsEvents[i++] = te->from == 0 ? 1440 : te->from;
         TimeElementsEvents[i++] = te->to == 0   ? 1440 : te->to;
      }
   }
 
   qsort( TimeElementsEvents, numTimeElements * 2, sizeof(int), sgTimeCmp );

   t = (int *) ufdbCalloc( numTimeElements * 2, sizeof(int) ); 
   for (i=0,j=0;  i < numTimeElements * 2;  i++) 
   {
      if (j == 0)
      {
         t[j++] = TimeElementsEvents[i];
      }
      else
      {
         if (t[j-1] != TimeElementsEvents[i]) 
	 {
 	    t[j++] = TimeElementsEvents[i];
         }
      }
   }

   ufdbFree( TimeElementsEvents );
   numTimeElements = j;
   TimeElementsEvents = t;
}


/*
 * _TimeEvaluateElements - evaluate all elements if they match the current time/date and mark them active.
 */
static void _TimeEvaluateElements( 
  struct tm * tm_now, 
  time_t      now )
{
  struct Time *        tlist;
  struct TimeElement * te;
  int                  min;

  for (tlist = Time;  tlist != NULL;  tlist = tlist->next)
  {
    tlist->active = 0;
    for (te = tlist->element;  te != NULL;  te = te->next)
    {
      if (te->wday != 0) 			/* check wday */
      {
	if (((1 << tm_now->tm_wday) & te->wday) != 0)  
	{
	  min = (tm_now->tm_hour * 60 ) + tm_now->tm_min;
	  if (min >= te->from  &&  min < te->to) 
	  {
	    tlist->active = 1;
	    break;
	  }
	}
      }
      else if (te->fromdate != 0)		/* check date */
      {
	if (now >= te->fromdate  &&  now <= te->todate) 
	{
	  min = (tm_now->tm_hour * 60 ) + tm_now->tm_min;
	  if (min >= te->from  &&  min < te->to) 
	  {
	    tlist->active = 1;
	    break;
	  }
	}
      }
      else					/* check crondate */
      {
	if (te->y == -1  ||  te->y == (tm_now->tm_year + 1900)) 
	{
	  if (te->m == -1  ||  te->m == (tm_now->tm_mon + 1)) 
	  {
	    if (te->d == -1  ||  te->d == (tm_now->tm_mday)) 
	    {
	      min = (tm_now->tm_hour * 60 ) + tm_now->tm_min;
	      if (min >= te->from  &&  min < te->to) 
	      {
		tlist->active = 1;
		break;
	      }
	    }
	  }
	}
      }
    }
#ifdef UFDB_DEBUG
    ufdbLogMessage( "      _TimeEvaluateElements: time %s is %sactive", tlist->name, tlist->active?"":"not " );
#endif
  }
}


/*
 * _TimeSetAclSrcDestRew - mark all acl/source/dest/rew (in)active.
 */
static void _TimeSetAclSrcDestRew( void )
{
  struct Acl *         acl;
  struct Category *    cat;
  struct Source *      s;
  struct sgRewrite *   rew;
  int                  a;

  for (acl = Acl;  acl != NULL;  acl = acl->next)
  {
    if (acl->time != NULL  &&  acl->within != UFDB_ACL_ELSE) 
    {
      /* Be careful here: we are multithreaded and other threads use the value 
       * of acl->active at the same time.
       */
      a = acl->time->active;
      if (acl->within == UFDB_ACL_OUTSIDE)
	a = !a;
      if (acl->next != NULL  &&  acl->next->within == UFDB_ACL_ELSE) 
	acl->next->active = !a;
      acl->active = a;
    }
#ifdef UFDB_DEBUG
    ufdbLogMessage( "      _TimeSetAclSrcDestRew: acl %s %s is %sactive", acl->name, 
		    acl->within==UFDB_ACL_ELSE ? "ELSE" :
		       acl->within==UFDB_ACL_WITHIN ? "WITHIN" :
		          acl->within==UFDB_ACL_OUTSIDE ? "OUTSIDE" : "",
                    acl->active?"":"not " );
#endif
  }

  for (cat = Dest;  cat != NULL;  cat = cat->next)
  {
    if (cat->time != NULL) 
    {
      cat->active = cat->time->active;
      if (cat->within == UFDB_ACL_OUTSIDE)
	cat->active = !cat->active;
    }
#ifdef UFDB_DEBUG
    ufdbLogMessage( "      _TimeSetAclSrcDestRew: category %s is %sactive", cat->name, cat->active?"":"not " );
#endif
  }

  for (s = Source;  s != NULL;  s = s->next)
  {
    if (s->time != NULL) 
    {
      s->active = s->time->active;
      if (s->within == UFDB_ACL_OUTSIDE)
	s->active = !s->active;
    }
#ifdef UFDB_DEBUG
    ufdbLogMessage( "      _TimeSetAclSrcDestRew: source %s is %sactive", s->name, s->active?"":"not " );
#endif
  }

  for (rew = Rewrite; rew != NULL; rew = rew->next)
  {
    if (rew->time != NULL) 
    {
      rew->active = rew->time->active;
      if (rew->within == UFDB_ACL_OUTSIDE)
	 rew->active = !rew->active;
    }
#ifdef UFDB_DEBUG
    ufdbLogMessage( "      _TimeSetAclSrcDestRew: rewite %s is %sactive", rew->name, rew->active?"":"not " );
#endif
  }
}


/*
 *  ufdbHandleAlarmForTimeEvents - the alarm for the next time event went off so 
 *                                 recalculate the time-dependent active state.
 */
void ufdbHandleAlarmForTimeEvents( 
  int        why )
{
  time_t     now;
  struct tm  tm_now;
  int        m;
  int        tindex;
  int        lastval;

  if (Time == NULL)
    return;

  if (why == UFDB_PARAM_INIT)
     ufdbLogMessage( "time definitions are used; evaluating current ACLs" );
  else
     ufdbLogMessage( "alarm went of to recalculate time ACLs" );

  sgTimeElementSortEvents();

  now = time(NULL) + 30 + globalDebugTimeDelta;
  localtime_r( &now, &tm_now ); 
  m = (tm_now.tm_hour * 60) + tm_now.tm_min;
  
  for (tindex = 0;  tindex < numTimeElements;  tindex++)
  {
#ifdef UFDB_DEBUG
    ufdbLogMessage( "   TimeElementsEvents[%d] = %d", tindex, TimeElementsEvents[tindex] );
#endif
    if (TimeElementsEvents[tindex] >= m)
      break;
  }
  lastval = TimeElementsEvents[tindex];

#ifdef UFDB_DEBUG
  ufdbLogMessage( "   ufdbHandleAlarmForTimeEvents: m = %d  tindex = %d, lastval = %d", m, tindex, lastval );
#endif

  if (lastval < m)
    m = (((1440 - m) + lastval) * 60) - tm_now.tm_sec;
  else
    m = ((lastval - m) * 60) - tm_now.tm_sec;
  if (m <= 0)
    m = 30;

  _TimeEvaluateElements( &tm_now, now );
  _TimeSetAclSrcDestRew();

  ufdbLogMessage( "next alarm is in %d seconds", (unsigned int) m );
  ufdbSetSignalHandler( SIGALRM, sgAlarm );
  (void) alarm( (unsigned int) m );
}


/*
 * sgTimeElementClone - copy a time specification.
 */
void sgTimeElementClone( void )
{
  struct TimeElement * te;
  struct TimeElement * tmp;

  te = lastTimeElement;
  if (te == NULL) 
  {
    ufdbLogFatalError( "No previous TimeElement in sgTimeElementClone !" );
    return;
  }
  else
  {
    sgTimeElementInit();
    lastTimeElement->wday = te->wday;
    lastTimeElement->from = te->from;
    lastTimeElement->to = te->to;
    lastTimeElement->y = te->y;
    lastTimeElement->m = te->m;
    lastTimeElement->d = te->d;
    lastTimeElement->fromdate = te->fromdate;
    lastTimeElement->todate = te->todate;
    tmp = lastTimeElement;
    lastTimeElement = te;
    sgTimeElementEnd();
    lastTimeElement = tmp;
  }
}


/*
 * IP functions
 */

void sgSetIpType( 
  int         type, 
  char *      file, 
  int         line )
{
  struct Ip * ip;
  char *      p;
  char *      f;
  int         l;
  unsigned long octet;
  unsigned long op;

  ip = sgIpLast( lastSource );
  f = (file == NULL) ? UFDBglobalConfigFile : file;
  l = (line == 0) ? lineno : line;
  if (type == SG_IPTYPE_HOST)
    ip->mask = 0xffffffff;

  if (type == SG_IPTYPE_RANGE)
  {
    if (sgConvDot(ip->str,&op) == NULL)
    {
      ufdbLogFatalError( "IP address error in %s line %d", f, l );
      ip->mask = 0;
    }
    else 
      ip->mask = op;
    if ((unsigned long) ip->net > (unsigned long) ip->mask)
    {
      ufdbLogFatalError( "IP range error in %s line %d   %08x %08x", f, l, ip->net, ip->mask );
    }
  }

  if (type == SG_IPTYPE_CLASS)
  {
    p = ip->str;
    if (*p == '/')
      p++;
    if (sgConvDot(p,&op) == NULL)
    {
      ufdbLogFatalError( "IP address error in %s line %d", f, l );
      ip->mask = 0;
    }
    else 
      ip->mask = op;
  }

  if (type == SG_IPTYPE_CIDR)
  {
    p = ip->str;
    if (*p == '/')
      p++;
    octet = (unsigned long) atoi( p );
    if (octet > 32) 
    {
      ufdbLogFatalError( "IP address CIDR out of range \"%s\" in %s line %d", p, f, l );
      octet = 32;
    }
    if (octet == 32)
      ip->mask = 0xffffffff;
    else
      ip->mask = 0xffffffff ^ (0xffffffff >> octet);
    ip->net = ip->net & ip->mask;
  }

  ip->type = type;
  ip->next = (struct Ip *) ufdbCalloc( 1, sizeof(struct Ip) );

#if 0
  ufdbLogMessage( "   sgSetIpType %2d  %08lx  %08lx", ip->type, ip->net, ip->mask );
#endif

  /* TO-DO: fix messy code where the last struct must contain zeros */
}


void sgIp( 
  char *      name )
{
  struct Ip * ip;
  ulong       op;

#if 0
  ufdbLogMessage( "   sgIp( %s )", name );
#endif

  if (lastSource->ip == NULL) 
  {
    ip = (struct Ip *) ufdbCalloc( 1, sizeof(struct Ip) );
#if 0
    ip->str = NULL;
    ip->next = NULL;
#endif
    lastSource->ip = ip;
    lastSource->lastip = ip;
  }
  else
  {
    ip = sgIpLast( lastSource );
  }

  if (ip->net_is_set == 0) 
  {
    ip->net_is_set = 1;
    if (sgConvDot(name,&op) == NULL) 
    {
      ufdbLogFatalError( "line %d: IP address error in %s", lineno, UFDBglobalConfigFile );
      ip->net = 0;
    }
    else 
      ip->net = op;
  }
  else
  {
    ip->str = ufdbStrdup( name );
  }

#if 0
  ufdbLogMessage( "   sgIp %2d  %08lx  %08lx  %s", ip->type, ip->net, ip->mask, ip->str==NULL?"NULL":ip->str );
#endif

}

void sgProxyPort(char *port)
{
  int tmp;
  tmp = atoi(port);
  lastSource->proxy_port = tmp;
  ufdbLogMessage("source port: %d", tmp);
}


struct Ip * sgIpLast( 
  struct Source * s )
{
  struct Ip * ip;
  struct Ip * last;

  last = NULL;
  for (ip = s->ip;  ip != NULL;  ip = ip->next)
    last = ip;

  return last;
}


/*
 * ACL functions
 */

void sgAcl( 
   char *          name,
   char *          value, 
   int             within )
{
   struct Acl *    acl;
   struct Source * source;

#if defined(UFDB_DEBUG)
   ufdbLogMessage( "sgAcl name=\"%s\" value=\"%s\" within=%d line=%d", 
                   name==NULL?"NULL":name,  value==NULL?"NULL":value, within, lineno );
#endif

   if (Acl != NULL) 
   {
      if (sgAclFindName(name) != NULL)
      {
	 ufdbLogFatalError( "line %d: ACL \"%s\" is already defined in configuration file %s",
			    lineno, name, UFDBglobalConfigFile );
      }
   }

  if (within == UFDB_ACL_ELSE)
  {
     if (lastAcl == NULL)
     {
        ufdbLogFatalError( "line %d: ACL \"else\" has no parent ACL", lineno );
        return;
     }
     if (name == NULL)
        name = ufdbStrdup( lastAcl->name );
  }

  acl = (struct Acl *) ufdbMalloc( sizeof(struct Acl) );

  source = NULL;
  if (strcmp(name,"default") == 0)
  {
    defaultAcl = acl;
  }
  else
  {
    if ((source = sgSourceFindName(name)) == NULL)
    {
      ufdbLogFatalError( "line %d: ACL source \"%s\" is not defined in configuration file %s",
		         lineno, name, UFDBglobalConfigFile );
      ufdbFree( name );
      return;
    }
  }

  acl->name = name;
  acl->active = within == UFDB_ACL_ELSE ? 0 : 1;
  acl->source = source;
  acl->pass = NULL;
  acl->rewriteDefault = 1;
  acl->rewrite = NULL;
  acl->redirect = NULL;
  acl->time = NULL;
  acl->within = within;
  acl->next = NULL;

  if (value != NULL) 
  {
    struct Time * t;
    if ((t = sgTimeFindName(value)) == NULL) 
    {
      ufdbLogFatalError( "line %d: ACL time %s is not defined in configuration file %s",
		         lineno, value, UFDBglobalConfigFile );
      return;
    }
    acl->time = t;
  }

  if (Acl == NULL) 
  {
    Acl = acl;
    lastAcl = acl;
  }
  else
  {
    lastAcl->next = acl;
    lastAcl = acl;
  }
}


void sgAclSetValue( 
  char *               what,
  char *               value, 
  int                  allowed ) 
{
  struct Category *    dest = NULL;
  struct AclDest *     acldest;
  int                  type;
  
#if defined(UFDB_DEBUG)
   ufdbLogMessage( "sgAclSetValue %s %s%s", what, allowed?"":"!", value );
#endif

  if (lastAcl == NULL)
  {
     ufdbLogError( "error in configuration file on line %d: cannot set value for \"%s\" because there is no defined ACL", 
                   lineno, what );
     ufdbFree( value );
     return;
  }

  type = ACL_TYPE_TERMINATOR;

  if (strcmp(what,"pass") == 0)
  {
    if (strcmp(value,"any")==0 || strcmp(value,"all")==0)
      allowed = 1;
    else if (strcmp(value,"none") == 0)
      allowed = 0;
    else if (strcmp(value,"in-addr") == 0) {
      type = ACL_TYPE_INADDR;
    }
    else
    {
      if ((dest = ufdbCategoryFindName(value)) == NULL) 
      {
	ufdbLogFatalError( "ACL category %s is not defined in configuration file %s",
			   value, UFDBglobalConfigFile );
	ufdbFree( value );
        return;
      } 
      type = ACL_TYPE_DEFAULT;
    }

    acldest = ufdbMalloc( sizeof(struct AclDest) );
    acldest->name = value;
    acldest->dest = dest;
    acldest->access = allowed;
    acldest->type = type;
    acldest->next = NULL;

    if (lastAcl->pass == NULL) {
      lastAcl->pass = acldest;
    } else {
      lastAclDest->next = acldest;
    }
    lastAclDest = acldest;
  }

  if (strcmp(what,"rewrite") == 0)
  {
    if (strcmp(value,"none") == 0)
    {
      lastAcl->rewriteDefault = 0;
      lastAcl->rewrite = NULL;
    }
    else
    {
      struct sgRewrite * rewrite;

      if ((rewrite = sgRewriteFindName(value)) == NULL) 
      {
	ufdbLogFatalError( "rewrite %s is not defined in configuration file %s",
			   value, UFDBglobalConfigFile );
      }
      lastAcl->rewriteDefault = 0;
      lastAcl->rewrite = rewrite;
    }
    ufdbFree( value );
  }

  if (strcmp(what,"redirect") == 0)
  {
    if (strcmp(value,"default") != 0)
    {
      lastAcl->redirect = value;
    }
    else 
    {
      lastAcl->redirect = NULL;
      ufdbFree( value );
    }
  }
}


struct Acl * sgAclFindName( 
  char *       name )
{
  struct Acl * p;

  if (name == NULL)
     return NULL;

  for (p = Acl;  p != NULL;  p = p->next)
  {
    if (strcmp(name,p->name) == 0)
      return p;
  }

  return NULL;
}



/*
 * called by main loop: find the first active ACL that matches a source
 */
struct Acl * sgAclCheckSource( 
   struct Source * source )
{
   struct Acl *    acl;

   if (source == NULL)
   {
#ifdef UFDB_DEBUG_ACL_ACCESS
      if (UFDBglobalDebug)
	 ufdbLogMessage( "   sgAclCheckSource: source=NULL returning defaultAcl" );
#endif
      return defaultAcl;
   }

   for (acl = Acl;  acl != NULL;  acl = acl->next)
   {
      if (acl->source == source  &&  acl->active)
      {
#ifdef UFDB_DEBUG_ACL_ACCESS
	 if (UFDBglobalDebug)
	    ufdbLogMessage( "   sgAclCheckSource: active source for %s found", source->name );
#endif

	 return acl;
      }
   }

#ifdef UFDB_DEBUG_ACL_ACCESS
   if (UFDBglobalDebug)
      ufdbLogMessage( "   sgAclCheckSource: no active source for %s found, returning defaultAcl", source->name );
#endif

   return defaultAcl;
}


/*
 * called by main loop: scan the full access matrix for the URL in parameter 'req'
 * return value:
 * UFDB_ACL_ACCESS_DUNNO:  don't know.
 * UFDB_ACL_ACCESS_ALLOW:  explicit allow.
 * UFDB_ACL_ACCESS_BLOCK:  explicit block; a redirection string is returned also.
 */
int UFDBCheckACLaccess( 
  struct Source *    src, 
  struct Acl *       acl, 
  struct SquidInfo * si, 
  char *             redirectURL )
{
  int    access;
  char * redirect;
  struct sgRewrite *    rewrite = NULL;
  struct AclDest *      aclpass = NULL;
  struct Category *     category;
  char   newstring[UFDB_MAX_URL_LENGTH];
  char   newredir[UFDB_MAX_URL_LENGTH + UFDB_MAX_URL_LENGTH];

  si->blockReason = UFDB_API_BLOCK_NONE;
  si->aclpass = NULL;

#ifdef UFDB_DEBUG_ACL_ACCESS
   if (UFDBglobalDebug)
      ufdbLogMessage( "   UFDBCheckACLaccess :" );
#endif

  if (UFDBglobalReconfig)
  {
#ifdef _POSIX_PRIORITY_SCHEDULING
    static volatile int yieldcounter = 0;
    if (++yieldcounter > 32)
    {
       yieldcounter = 0;
       sched_yield();		
    }
#endif
    si->blockReason = UFDB_API_BLOCK_LOADING_DB;
    if (UFDBglobalURLlookupResultDBreload == UFDB_DENY)
    {
       strcpy( redirectURL, UFDBglobalLoadingDatabaseRedirect );
       return UFDB_ACL_ACCESS_BLOCK;
    }
    *redirectURL = '\0';
    return UFDB_ACL_ACCESS_ALLOW;
  }

  if (UFDBglobalFatalError)
  {
    si->blockReason = UFDB_API_BLOCK_FATAL_ERROR;
    if (UFDBglobalURLlookupResultFatalError == UFDB_DENY)
    {
       strcpy( redirectURL, UFDBglobalFatalErrorRedirect );
       return UFDB_ACL_ACCESS_BLOCK;
    }
    *redirectURL = '\0';
    return UFDB_ACL_ACCESS_ALLOW;
  }

  if (acl == NULL)
  {
#ifdef UFDB_DEBUG_ACL_ACCESS
    if (UFDBglobalDebug)
       ufdbLogMessage( "   UFDBCheckACLaccess  acl = NULL,  return DUNNO" );
#endif
    return UFDB_ACL_ACCESS_DUNNO;
  }

  si->aclpass = NULL;
  if (acl->pass == NULL)
    acl->pass = defaultAcl->pass;

  if (acl->pass == NULL)
  {  
     access = 0;
     redirect = defaultAcl->redirect;
  }
  else
  {
    access = 1;
    redirect = NULL;
    for (aclpass = acl->pass;  aclpass != NULL;  aclpass = aclpass->next)
    {
      category = aclpass->dest;
      if (category != NULL  &&  !category->active)
	continue;

#if defined(UFDB_DEBUG) || defined(UFDB_DEBUG_ACL_ACCESS)
      if (UFDBglobalDebug  &&  category != NULL)
	 ufdbLogMessage( "   sgAclAccess: testing acl \"%s\" with category \"%s\"", acl->name, category->name );
#endif

      if (aclpass->type == ACL_TYPE_TERMINATOR) 
      {
        si->aclpass = aclpass;
	access = aclpass->access;
	if (!access)
	   si->blockReason = UFDB_API_BLOCK_ACL;
	break;
      }

      if (aclpass->type == ACL_TYPE_INADDR) 
      {
	si->aclpass = aclpass;
	if (si->dot) 
	{
	   access = aclpass->access;
	   if (!access)
	      si->blockReason = UFDB_API_BLOCK_ACL;
	   break;
	}
	continue;
      }

      /* domain name lookup for this source */
      if (category->domainlistDb != NULL) 
      {
        struct UFDBmemTable * mt;

        mt = (struct UFDBmemTable *) category->domainlistDb->dbcp;
        if (UFDBlookupRevUrl( &(mt->table.nextLevel[0]), si->revUrl ))
	{
#ifdef UFDB_DEBUG_ACL_ACCESS
	  if (UFDBglobalDebug)
	    ufdbLogMessage( "   sgAclAccess: acl \"%s\" with category \"%s\": UFDBlookupRevUrl  returned a match", 
	                    acl->name, category->name  );
#endif
	  si->blockReason = UFDB_API_BLOCK_ACL;
	  si->aclpass = aclpass;
	  access = aclpass->access;
	  break;
	}
      }

      if (si->dot  &&
          (si->port == 443 || si->port == 563 || strcmp( si->protocol, "https" ) == 0) )
      {
	 if (category->name != NULL  &&  strcmp( category->name, "security" ) == 0)
	 {
	    int httpsStatus;

	    /* We MUST verify for Skype without queueing since if the connection is a valid Skype 
	     * it might otherwise be blocked due to the UFDB_OPT_HTTPS_WITH_HOSTNAME option.
	     * This is known to slow down connections to IP-based URLs.
	     */
	    httpsStatus = UFDBcheckForHTTPStunnel( si->orig_domain, si->port, 0 );
	    if (UFDBglobalDebug || UFDBglobalDebugSkype || UFDBglobalDebugGtalk || UFDBglobalDebugYahooMsg || UFDBglobalDebugAim)
	       ufdbLogMessage( "      HTTPS protocol check for %s:%d returned status = %d/%s", 
			       si->orig_domain, si->port, httpsStatus, ufdbAPIstatusString(httpsStatus) );

	    if (httpsStatus == UFDB_API_ERR_SOCKET)
	    {
	       si->blockReason = UFDB_API_BLOCK_HTTPS_OPTION;
	       si->aclpass = aclpass;
	       return UFDB_ACL_ACCESS_ALLOW;
	    }

	    if (httpsStatus == UFDB_API_ERR_IS_SKYPE)
	    {
	       if (UFDBglobalDebug || UFDBglobalDebugSkype)
	       {
	          if (UFDBglobalSkypeOverHttps)
		     ufdbLogMessage( "   UFDBCheckACLaccess  Skype detected, return ALLOW" );
		  else
		     ufdbLogMessage( "   UFDBCheckACLaccess  Skype detected, return BLOCK" );
	       }
	       si->blockReason = UFDB_API_BLOCK_HTTPS_OPTION;
	       si->aclpass = aclpass;
	       access = UFDBglobalSkypeOverHttps;
	       if (access)
	          return UFDB_ACL_ACCESS_ALLOW;
	       break;
	    }

	    if (httpsStatus == UFDB_API_ERR_IS_GTALK)
	    {
	       if (UFDBglobalDebug || UFDBglobalDebugGtalk)
	       {
	          if (UFDBglobalGtalkOverHttps)
		     ufdbLogMessage( "   UFDBCheckACLaccess  Google Talk detected, return ALLOW" );
		  else
		     ufdbLogMessage( "   UFDBCheckACLaccess  Google Talk detected, return BLOCK" );
	       }
	       si->blockReason = UFDB_API_BLOCK_HTTPS_OPTION;
	       si->aclpass = aclpass;
	       access = UFDBglobalGtalkOverHttps;
	       if (access)
	          return UFDB_ACL_ACCESS_ALLOW;
	       break;
	    }

	    if (httpsStatus == UFDB_API_ERR_IS_YAHOOMSG)
	    {
	       if (UFDBglobalDebug || UFDBglobalDebugYahooMsg)
	       {
	          if (UFDBglobalYahooMsgOverHttps)
		     ufdbLogMessage( "   UFDBCheckACLaccess  Yahoo IM detected, return ALLOW" );
		  else
		     ufdbLogMessage( "   UFDBCheckACLaccess  Yahoo IM detected, return BLOCK" );
	       }
	       si->blockReason = UFDB_API_BLOCK_HTTPS_OPTION;
	       si->aclpass = aclpass;
	       access = UFDBglobalYahooMsgOverHttps;
	       if (access)
	          return UFDB_ACL_ACCESS_ALLOW;
	       break;
	    }

	    if (httpsStatus == UFDB_API_ERR_IS_AIM)
	    {
	       if (UFDBglobalDebug || UFDBglobalDebugAim)
	       {
	          if (UFDBglobalAimOverHttps)
		     ufdbLogMessage( "   UFDBCheckACLaccess  AIM detected, return ALLOW" );
		  else
		     ufdbLogMessage( "   UFDBCheckACLaccess  AIM detected, return BLOCK" );
	       }
	       si->blockReason = UFDB_API_BLOCK_HTTPS_OPTION;
	       si->aclpass = aclpass;
	       access = UFDBglobalAimOverHttps;
	       if (access)
	          return UFDB_ACL_ACCESS_ALLOW;
	       break;
	    }

	    if (httpsStatus == UFDB_API_ERR_TUNNEL)
	    {
	       if (UFDBglobalDebug || UFDBglobalDebugSkype || UFDBglobalDebugGtalk || UFDBglobalDebugYahooMsg || UFDBglobalDebugAim)
	       {
	          ufdbLogMessage( "   UFDBCheckACLaccess  Tunnel detected, return BLOCK" );
	       }
	       si->blockReason = UFDB_API_BLOCK_HTTPS_OPTION;
	       si->aclpass = aclpass;
	       access = 0;
	       break;
	    }

	    if (httpsStatus == UFDB_OPT_UNKNOWN_PROTOCOL_OVER_HTTPS)
	    {
	       if (UFDBglobalDebug || UFDBglobalDebugSkype || UFDBglobalDebugGtalk || UFDBglobalDebugYahooMsg || UFDBglobalDebugAim)
	       {
	          if (UFDBglobalUnknownProtocolOverHttps)
		     ufdbLogMessage( "   UFDBCheckACLaccess  unknown protocol detected, return ALLOW" );
		  else
		     ufdbLogMessage( "   UFDBCheckACLaccess  unknown protocol detected, return BLOCK" );
	       }
	       si->blockReason = UFDB_API_BLOCK_HTTPS_OPTION;
	       si->aclpass = aclpass;
	       access = UFDBglobalUnknownProtocolOverHttps;
	       if (access)
	          return UFDB_ACL_ACCESS_ALLOW;
	       break;
	    }

	    if (category->options & UFDB_OPT_HTTPS_WITH_HOSTNAME)
	    {
	       ufdbLogError( "HTTPS protocol used to access IP %s (FQDN is required)", si->domain );
	       si->blockReason = UFDB_API_BLOCK_HTTPS_OPTION;
	       si->aclpass = aclpass;
	       access = 0;
	       break;
	    }
	 }
      }

      if ((category->options & UFDB_OPT_HTTPS_OFFICAL_CERTIFICATE)  &&
	  (UFDBgetTunnelCheckMethod() != UFDB_API_HTTPS_CHECK_OFF)  &&
          (si->port == 443 || si->port == 563 || strcmp( si->protocol, "https" ) == 0) )
      {
         int httpsStatus;

	 httpsStatus = UFDBcheckForHTTPStunnel( si->orig_domain, si->port,
	    UFDBgetTunnelCheckMethod() == UFDB_API_HTTPS_CHECK_AGGRESSIVE ? 0 : UFDB_API_ALLOW_QUEUING );
	 if (UFDBglobalDebug)
	    ufdbLogMessage( "      HTTPS certificate check: %s:%d has status = %d/%s", 
	                    si->orig_domain, si->port, httpsStatus, ufdbAPIstatusString(httpsStatus) );
	 if (httpsStatus == UFDB_API_ERR_INVALID_CERT)
	 {
	    si->blockReason = UFDB_API_BLOCK_HTTPS_OPTION;
	    access = 0;
	    break;
	 }
      }

      if ((UFDBgetTunnelCheckMethod() != UFDB_API_HTTPS_CHECK_OFF)  &&  
	  (strcmp( aclpass->name, "proxies" ) == 0)  &&
          (si->port == 443 || si->port == 563 || strcmp( si->protocol, "https" ) == 0) )
      {
         int httpsStatus;

	 httpsStatus = UFDBcheckForHTTPStunnel( si->orig_domain, si->port,
	    UFDBgetTunnelCheckMethod() == UFDB_API_HTTPS_CHECK_AGGRESSIVE ? 0 : UFDB_API_ALLOW_QUEUING );
	 if (UFDBglobalDebug)
	    ufdbLogMessage( "      HTTPS proxy check: %s:%d has status = %d/%s",
	                    si->orig_domain, si->port, httpsStatus, ufdbAPIstatusString(httpsStatus) );
	 if (httpsStatus == UFDB_API_ERR_TUNNEL)
	 {
	    UFDBglobalTunnelCounter++;
	    if (UFDBgetTunnelCheckMethod() != UFDB_API_HTTPS_LOG_ONLY)
	    {
	       si->blockReason = UFDB_API_BLOCK_HTTPS_OPTION;
	       si->aclpass = aclpass;
	       access = 0;
	       break;
	    }
	 }
      }

      /* expr lookup for this source */
      if (access  &&  category->regExp != NULL)
      {
        int result;

	result = ufdbRegExpMatch( category->regExp, si->surl );
	if (UFDBglobalDebugRegexp)
	   ufdbLogMessage( "      expressionlist %s: ufdbRegExpMatch( %s ) : %s", 
	                   category->name,  si->surl, 
			   result==0 ? "no match" : "matched" );
	if (result) 
	{
	  si->blockReason = UFDB_API_BLOCK_ACL;
	  si->aclpass = aclpass;
	  access = aclpass->access;
	  break;
	}
      }

       /* new: per-ACL SafeSearch */
       if (access  &&  category != NULL  &&  category->options & UFDB_OPT_SAFE_SEARCH)
       {
	   if (UFDBaddSafeSearch( si->domain, si->surl, si->orig )
		   == UFDB_API_MODIFIED_FOR_SAFESEARCH)
	   {
	      si->blockReason = UFDB_API_BLOCK_SAFESARCH;
	      si->aclpass = aclpass;
	      strcpy( redirectURL, si->orig );
#ifdef UFDB_DEBUG_ACL_ACCESS
	      if (UFDBglobalDebug)
		 ufdbLogMessage( "   UFDBCheckACLaccess  per-acl SafeSearch, return BLOCK" );
#endif
	      return UFDB_ACL_ACCESS_BLOCK;
	   }
       }
    }

    if (!access) 
    {
      if (category != NULL  &&  category->redirect != NULL)
	 redirect = category->redirect;
      else if (category != NULL  &&  category->rewrite != NULL  &&
	       (redirect = sgRewriteExpression(category->rewrite,si->orig,newstring)) != NULL)
      {
	 ;
      }
      else if (acl->redirect == NULL)
	 redirect = defaultAcl->redirect;
      else
	 redirect = acl->redirect;
      if (redirect == NULL)
      {
#ifdef UFDB_DEBUG_ACL_ACCESS
         /* suppress many errors. we only want it once! */
         ufdbLogError( "!!!!! there is no \"redirect\" rule for ACL %s/%s  *****", 
		       acl->name, aclpass->name );
#endif
         /* TODO: copy the redirect rule for the security or proxy category */
         /* TODO: instead of referring to cgibin.urlfilterdb.com */
         redirect = "http://cgibin.urlfilterdb.com/cgi-bin/URLblocked.cgi?"
	            "clientgroup=%s&category=%t&url=%u";
       }
     }
   }

   if (acl->rewrite == NULL)
      rewrite = defaultAcl->rewrite;
   else
      rewrite = acl->rewrite;
   if (rewrite != NULL  &&  access)
   {
      redirect = sgRewriteExpression( rewrite, si->orig, newstring );
   }
   else if (redirect != NULL) 
   {
      redirect = sgParseRedirect( redirect, si, acl, aclpass, newredir, strcmp(si->protocol,"https")==0 );
   }

   if (redirect != NULL)
   {
      strcpy( redirectURL, redirect );
      si->aclpass = aclpass;
#ifdef UFDB_DEBUG_ACL_ACCESS
      if (UFDBglobalDebug)
        ufdbLogMessage( "   UFDBCheckACLaccess  redirected, return BLOCK  redirect: %s", redirectURL );
#endif
      return UFDB_ACL_ACCESS_BLOCK;
   }

   *redirectURL = '\0';
#ifdef UFDB_DEBUG_ACL_ACCESS
   if (UFDBglobalDebug)
     ufdbLogMessage( "   UFDBCheckACLaccess  end, return DUNNO" );
#endif

   return UFDB_ACL_ACCESS_DUNNO;
}


void yyerror( char * s )
{
   ufdbLogFatalError( "line %d: %s in configuration file %s", lineno, s, UFDBglobalConfigFile );
}


int yywrap()
{
  return 1;
}

