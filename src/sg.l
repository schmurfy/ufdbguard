/*
 * ufdbGuard is copyrighted (C) 2005-2010 by URLfilterDB with all rights reserved.
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
 * (GPL) along with this program.
 */


/* %pointer */

%{

#include "ufdb.h"
#include "sg.h"
#include "y.tab.h"

#include <stdio.h>
#include <string.h>

#if defined(UFDB_DEBUG) && 0
#define YY_USER_ACTION  ufdbLogMessage( "yytext line %d: \"%s\"", lineno, yytext );
#endif

#ifdef FLEX_SCANNER
#define ECHO  (void) fprintf( yyout, "%*.*s", yyleng, yyleng, yytext );
#endif

%}

d        [0-9]

ip       {d}{1,3}\.{d}{1,3}\.{d}{1,3}\.{d}{1,3}

tval     {d}{d}[:]{d}{d}
dval     {d}{d}{d}{d}[-/\.]{d}{d}[-/\.]{d}{d}
dvalcron ({d}{d}{d}{d}|[\*])[-/\.]({d}{d}|[\*])[-/\.]({d}{d}|[\*])

s        [\t ]
weekday  (mon|mondays?|tue|tuesdays?|wed|wednesdays?|thu|thursdays?|fri|fridays?|sat|saturdays?|sun|sundays?)
weekdays (({weekday}{s}*[, \t]+{s}*)*{weekday})|[\*]

%e 14000
%p 30000
%n 4000
%a 8000

%x REDIRECT_STATE

%%

#[^\n]*         { /* comment */ }
[ \t\f\r]+  	{ /* whitespace */ }
"\n"      	{ lineno++; }

category    	return CATEGORY;
cat         	return CATEGORY;
destination 	return CATEGORY;
dest        	return CATEGORY;
source      	return SOURCE;
src         	return SOURCE;

rewrite     	return REWRITE;
rew         	return REWRITE;
time        	return TIME;
acl         	return ACL;

http-server 	{ return HTTP_SERVER; }
images      	{ return IMAGES; }
interface   	{ return INTERFACE; }
port        	{ return PORT; }

dbhome     	             { return DBHOME; }
logdir     	             { return LOGDIR; }
mail-server                  { return MAIL_SERVER; }
admin-email     	     { return ADMIN_EMAIL; }
external-status-command      { return EXTERNAL_STATUS_COMMAND; }
cpus       		     { return CPUS; }
analyse-uncategorised-urls   { return ANALYSE_UNCATEGORISED; }
analyze-uncategorised-urls   { return ANALYSE_UNCATEGORISED; }
upload-stats                 { return UPLOAD_STATS; }

administrator               return ADMINISTRATOR;

safe-search          	    return SAFE_SEARCH;
max-logfile-size       	    return MAX_LOGFILE_SIZE;
logblock		    return LOGBLOCK;
logdeny			    return LOGBLOCK;
logall-httpd  		    return LOGALL_HTTPD;
logall  		    return LOGALL;
off                         return OFF;
on                          return ON;

option			    return OPTION;
ufdb-expression-optimisation return UFDB_EXPRESSION_OPTIMISATION;
ufdb-expression-debug       return UFDB_EXPRESSION_DEBUG;
ufdb-show-url-details       return UFDB_SHOW_URL_DETAILS;
ufdb-debug-filter      	    return UFDB_DEBUG_FILTER;
ufdb-debug-skype-probes	    return UFDB_DEBUG_SKYPE_PROBES;
ufdb-debug-gtalk-probes	    return UFDB_DEBUG_GTALK_PROBES;
ufdb-debug-yahoomsg-probes  return UFDB_DEBUG_YAHOOMSG_PROBES;
ufdb-debug-aim-probes       return UFDB_DEBUG_AIM_PROBES;
url-lookup-result-during-database-reload return URL_LOOKUP_RESULT_DB_RELOAD;
url-lookup-result-when-fatal-error       return URL_LOOKUP_RESULT_FATAL_ERROR;
deny			    return TOKEN_DENY;
allow			    return TOKEN_ALLOW;
check-proxy-tunnels         return CHECK_PROXY_TUNNELS;
queue-checks	            return QUEUE_CHECKS;
agressive                   return AGGRESSIVE;
aggressive                  return AGGRESSIVE;
log-only                    return LOG_ONLY;
allow-skype-over-https      		return ALLOW_SKYPE_OVER_HTTPS;
allow-gtalk-over-https      		return ALLOW_GTALK_OVER_HTTPS;
allow-yahoomsg-over-https      		return ALLOW_YAHOOMSG_OVER_HTTPS;
allow-aim-over-https      		return ALLOW_AIM_OVER_HTTPS;
allow-unknown-protocol-over-https	return ALLOW_UNKNOWN_PROTOCOL_OVER_HTTPS;
enforce-https-with-hostname 		return ENFORCE_HTTPS_WITH_HOSTNAME;
enforce-https-offical-certificate 	return ENFORCE_HTTPS_OFFICAL_CERTIFICATE;
enforce-https-official-certificate 	return ENFORCE_HTTPS_OFFICAL_CERTIFICATE;
https-prohibit-insecure-sslv2 		return HTTPS_PROHIBIT_INSECURE_SSLV2;
https-connection-cache-size 		return HTTPS_CONNECTION_CACHE_SIZE;

domainlist      return DOMAINLIST;
urllist         return URLLIST;
expressionlist  return EXPRESSIONLIST;
cacerts		return CACERTS;

ignorecase	return IGNORECASE;

unix        	return UNIX;
ldap        	return LDAP;
userlist    	return USERLIST;
user        	return USER;
group       	return GROUP;

userquota   	return USERQUOTA;
iplist      	return IPLIST;
domain      	return DOMAIN;
pass        	return PASS;
redirect    	{ BEGIN REDIRECT_STATE; return REDIRECT; }

redirect-fatal-error        return REDIRECT_FATAL_ERROR;
redirect-loading-database   return REDIRECT_LOADING_DATABASE;

minutely    	return MINUTELY;
hourly      	return HOURLY;
daily       	return DAILY;
weekly      	return WEEKLY;
continue    	return CONTINUE;
sporadic    	return SPORADIC;
date        	return DATE;

anonymous   	{ return ANONYMOUS; }
logfile     	{ return LOGFILE; }
log         	{ return LOGFILE; }
{weekdays}  	{ yylval.string = ufdbStrdup(yytext); return WEEKDAY; }
within      	{ return WITHIN; }
outside     	{ return OUTSIDE; }
else        	{ return ELSE; }
ip          	{ return IP; }
proxy_port      { return PROXY_PORT; }
{ip}        	{ yylval.string = ufdbStrdup(yytext); return IPADDR; }
[/]{d}+     	{ yylval.string = ufdbStrdup(yytext+1); return CIDR; }
[/]{ip}     	{ yylval.string = ufdbStrdup(yytext+1); return IPCLASS; }
{tval}      	{ yylval.string = ufdbStrdup(yytext); return TVAL; }
{dval}      	{ yylval.string = ufdbStrdup(yytext); return DVAL; }
{dvalcron}  	{ yylval.string = ufdbStrdup(yytext); return DVALCRON; }
{d}+        	{ yylval.string = ufdbStrdup(yytext); return NUMBER; }
{d}+","[0-9,]+  { yylval.string = ufdbStrdup(yytext); return NUMBERS; }

\"[^"]*\"       { yylval.string = ufdbStrdup(yytext+1); *(strchr(yylval.string,'"')) = '\0'; return QSTRING; }

"{"          	{ return START_BRACKET; }
"}"          	{ return STOP_BRACKET;  }
","	     	{ return COMMA; }
"="	     	{ return EQUAL; }
"!"	  	{ return CHAR_EXCLAMATION; }
"i"	  	{ return CHAR_I; }
"-"	  	{ return CHAR_MINUS; }

[a-zA-Z][-a-zA-Z0-9_]*         			  { yylval.string = ufdbStrdup(yytext); return IDENTIFIER; }
[a-zA-Z\/\.][a-zA-Z0-9/_\-\.\/\:\%\+\?&]*         { yylval.string = ufdbStrdup(yytext); return WORD; }

s@(([^@}]|\\@|\\\})+)@(([^@}]|\\@|\\\})+)@[girR]* { yylval.string = ufdbStrdup(yytext); return SUBST; }

.         	{ ufdbLogError( "Unrecognized character on line %d: 0x%02x", lineno, ((int) yytext[0]) & 0xff ); }


<REDIRECT_STATE>\"[^"]*\"     	{ yylval.string = ufdbStrdup(yytext+1); *(strchr(yylval.string,'"')) = '\0'; BEGIN 0; return WORD; }
<REDIRECT_STATE>[0-9a-zA-Z\/\.][a-zA-Z0-9/_\-\.\/\:\%\+\?=&]*  { yylval.string = ufdbStrdup(yytext); BEGIN 0; return WORD; }
<REDIRECT_STATE>[ \t\f\r]+    ;
<REDIRECT_STATE>\n            	{ lineno++; }
<REDIRECT_STATE>.             	{ return yytext[0]; }

<<EOF>>	  			{ yyterminate(); }

%%


void UFDBdummyToSuppressCompilerWarnings( void )
{
   (void) unput( '\n' );

#ifdef __cplusplus
   (void) yyinput();
#else
   (void) input();
#endif
}

