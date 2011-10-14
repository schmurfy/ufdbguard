/* A Bison parser, made by GNU Bison 2.3.  */

/* Skeleton interface for Bison's Yacc-like parsers in C

   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     ADMINISTRATOR = 258,
     QSTRING = 259,
     CHECK_PROXY_TUNNELS = 260,
     QUEUE_CHECKS = 261,
     AGGRESSIVE = 262,
     LOG_ONLY = 263,
     ON = 264,
     OFF = 265,
     CHAR_MINUS = 266,
     CHAR_I = 267,
     CHAR_EXCLAMATION = 268,
     IGNORECASE = 269,
     COMMA = 270,
     EQUAL = 271,
     PORT = 272,
     HTTP_SERVER = 273,
     INTERFACE = 274,
     IMAGES = 275,
     HTTPS_PROHIBIT_INSECURE_SSLV2 = 276,
     HTTPS_CONNECTION_CACHE_SIZE = 277,
     IDENTIFIER = 278,
     WORD = 279,
     END = 280,
     START_BRACKET = 281,
     STOP_BRACKET = 282,
     WEEKDAY = 283,
     CATEGORY = 284,
     REWRITE = 285,
     ACL = 286,
     CPUS = 287,
     TIME = 288,
     TVAL = 289,
     DVAL = 290,
     DVALCRON = 291,
     SOURCE = 292,
     CIDR = 293,
     IPCLASS = 294,
     CONTINUE = 295,
     IPADDR = 296,
     DBHOME = 297,
     DOMAINLIST = 298,
     URLLIST = 299,
     EXPRESSIONLIST = 300,
     CACERTS = 301,
     IPLIST = 302,
     DOMAIN = 303,
     UNIX = 304,
     LDAP = 305,
     USER = 306,
     USERLIST = 307,
     USERQUOTA = 308,
     GROUP = 309,
     IP = 310,
     NL = 311,
     NUMBER = 312,
     NUMBERS = 313,
     PASS = 314,
     REDIRECT = 315,
     SUBST = 316,
     CHAR = 317,
     MINUTELY = 318,
     HOURLY = 319,
     DAILY = 320,
     WEEKLY = 321,
     DATE = 322,
     REDIRECT_FATAL_ERROR = 323,
     REDIRECT_LOADING_DATABASE = 324,
     WITHIN = 325,
     OUTSIDE = 326,
     ELSE = 327,
     ANONYMOUS = 328,
     SPORADIC = 329,
     LOGFILE = 330,
     LOGDIR = 331,
     LOGBLOCK = 332,
     LOGALL = 333,
     LOGALL_HTTPD = 334,
     MAIL_SERVER = 335,
     ADMIN_EMAIL = 336,
     EXTERNAL_STATUS_COMMAND = 337,
     TOKEN_ALLOW = 338,
     TOKEN_DENY = 339,
     URL_LOOKUP_RESULT_DB_RELOAD = 340,
     URL_LOOKUP_RESULT_FATAL_ERROR = 341,
     OPTION = 342,
     UFDB_SHOW_URL_DETAILS = 343,
     UFDB_DEBUG_FILTER = 344,
     UFDB_DEBUG_SKYPE_PROBES = 345,
     UFDB_DEBUG_GTALK_PROBES = 346,
     UFDB_DEBUG_YAHOOMSG_PROBES = 347,
     UFDB_DEBUG_AIM_PROBES = 348,
     UFDB_EXPRESSION_OPTIMISATION = 349,
     UFDB_EXPRESSION_DEBUG = 350,
     ENFORCE_HTTPS_WITH_HOSTNAME = 351,
     ENFORCE_HTTPS_OFFICAL_CERTIFICATE = 352,
     ALLOW_SKYPE_OVER_HTTPS = 353,
     ALLOW_UNKNOWN_PROTOCOL_OVER_HTTPS = 354,
     ALLOW_GTALK_OVER_HTTPS = 355,
     ALLOW_YAHOOMSG_OVER_HTTPS = 356,
     ALLOW_AIM_OVER_HTTPS = 357,
     ANALYSE_UNCATEGORISED = 358,
     UPLOAD_STATS = 359,
     SAFE_SEARCH = 360,
     MAX_LOGFILE_SIZE = 361
   };
#endif
/* Tokens.  */
#define ADMINISTRATOR 258
#define QSTRING 259
#define CHECK_PROXY_TUNNELS 260
#define QUEUE_CHECKS 261
#define AGGRESSIVE 262
#define LOG_ONLY 263
#define ON 264
#define OFF 265
#define CHAR_MINUS 266
#define CHAR_I 267
#define CHAR_EXCLAMATION 268
#define IGNORECASE 269
#define COMMA 270
#define EQUAL 271
#define PORT 272
#define HTTP_SERVER 273
#define INTERFACE 274
#define IMAGES 275
#define HTTPS_PROHIBIT_INSECURE_SSLV2 276
#define HTTPS_CONNECTION_CACHE_SIZE 277
#define IDENTIFIER 278
#define WORD 279
#define END 280
#define START_BRACKET 281
#define STOP_BRACKET 282
#define WEEKDAY 283
#define CATEGORY 284
#define REWRITE 285
#define ACL 286
#define CPUS 287
#define TIME 288
#define TVAL 289
#define DVAL 290
#define DVALCRON 291
#define SOURCE 292
#define CIDR 293
#define IPCLASS 294
#define CONTINUE 295
#define IPADDR 296
#define DBHOME 297
#define DOMAINLIST 298
#define URLLIST 299
#define EXPRESSIONLIST 300
#define CACERTS 301
#define IPLIST 302
#define DOMAIN 303
#define UNIX 304
#define LDAP 305
#define USER 306
#define USERLIST 307
#define USERQUOTA 308
#define GROUP 309
#define IP 310
#define NL 311
#define NUMBER 312
#define NUMBERS 313
#define PASS 314
#define REDIRECT 315
#define SUBST 316
#define CHAR 317
#define MINUTELY 318
#define HOURLY 319
#define DAILY 320
#define WEEKLY 321
#define DATE 322
#define REDIRECT_FATAL_ERROR 323
#define REDIRECT_LOADING_DATABASE 324
#define WITHIN 325
#define OUTSIDE 326
#define ELSE 327
#define ANONYMOUS 328
#define SPORADIC 329
#define LOGFILE 330
#define LOGDIR 331
#define LOGBLOCK 332
#define LOGALL 333
#define LOGALL_HTTPD 334
#define MAIL_SERVER 335
#define ADMIN_EMAIL 336
#define EXTERNAL_STATUS_COMMAND 337
#define TOKEN_ALLOW 338
#define TOKEN_DENY 339
#define URL_LOOKUP_RESULT_DB_RELOAD 340
#define URL_LOOKUP_RESULT_FATAL_ERROR 341
#define OPTION 342
#define UFDB_SHOW_URL_DETAILS 343
#define UFDB_DEBUG_FILTER 344
#define UFDB_DEBUG_SKYPE_PROBES 345
#define UFDB_DEBUG_GTALK_PROBES 346
#define UFDB_DEBUG_YAHOOMSG_PROBES 347
#define UFDB_DEBUG_AIM_PROBES 348
#define UFDB_EXPRESSION_OPTIMISATION 349
#define UFDB_EXPRESSION_DEBUG 350
#define ENFORCE_HTTPS_WITH_HOSTNAME 351
#define ENFORCE_HTTPS_OFFICAL_CERTIFICATE 352
#define ALLOW_SKYPE_OVER_HTTPS 353
#define ALLOW_UNKNOWN_PROTOCOL_OVER_HTTPS 354
#define ALLOW_GTALK_OVER_HTTPS 355
#define ALLOW_YAHOOMSG_OVER_HTTPS 356
#define ALLOW_AIM_OVER_HTTPS 357
#define ANALYSE_UNCATEGORISED 358
#define UPLOAD_STATS 359
#define SAFE_SEARCH 360
#define MAX_LOGFILE_SIZE 361




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
#line 102 "sg.y"
{
  char * string;
  char * tval;
  char * dval;
  char * dvalcron;
  int    integer;
}
/* Line 1529 of yacc.c.  */
#line 269 "y.tab.h"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE yylval;

