/* A Bison parser, made by GNU Bison 2.3.  */

/* Skeleton implementation for Bison's Yacc-like parsers in C

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

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "2.3"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Using locations.  */
#define YYLSP_NEEDED 0



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




/* Copy the first part of user declarations.  */
#line 1 "sg.y"


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



/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* Enabling the token table.  */
#ifndef YYTOKEN_TABLE
# define YYTOKEN_TABLE 0
#endif

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
/* Line 193 of yacc.c.  */
#line 417 "y.tab.c"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif



/* Copy the second part of user declarations.  */


/* Line 216 of yacc.c.  */
#line 430 "y.tab.c"

#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#elif (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
typedef signed char yytype_int8;
#else
typedef short int yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(e) ((void) (e))
#else
# define YYUSE(e) /* empty */
#endif

/* Identity function, used to suppress warnings about constant conditions.  */
#ifndef lint
# define YYID(n) (n)
#else
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static int
YYID (int i)
#else
static int
YYID (i)
    int i;
#endif
{
  return i;
}
#endif

#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#     ifndef _STDLIB_H
#      define _STDLIB_H 1
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (YYID (0))
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined _STDLIB_H \
       && ! ((defined YYMALLOC || defined malloc) \
	     && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef _STDLIB_H
#    define _STDLIB_H 1
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
	 || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss;
  YYSTYPE yyvs;
  };

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  YYSIZE_T yyi;				\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (YYID (0))
#  endif
# endif

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack)					\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack, Stack, yysize);				\
	Stack = &yyptr->Stack;						\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (YYID (0))

#endif

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  3
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   361

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  107
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  88
/* YYNRULES -- Number of rules.  */
#define YYNRULES  252
/* YYNRULES -- Number of states.  */
#define YYNSTATES  371

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   361

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     5,     7,     9,    12,    14,    16,    18,
      20,    23,    26,    29,    32,    35,    38,    41,    43,    45,
      47,    49,    52,    55,    58,    61,    64,    67,    70,    73,
      76,    79,    82,    85,    88,    91,    94,    97,   100,   103,
     106,   109,   112,   115,   118,   121,   124,   127,   130,   133,
     136,   139,   143,   147,   151,   155,   159,   163,   167,   169,
     174,   177,   182,   183,   186,   189,   192,   195,   198,   201,
     204,   207,   210,   213,   217,   221,   225,   229,   232,   235,
     238,   241,   244,   247,   250,   253,   257,   260,   264,   267,
     271,   275,   279,   283,   287,   291,   295,   299,   303,   306,
     309,   312,   317,   318,   321,   324,   327,   331,   334,   337,
     341,   345,   349,   353,   358,   363,   368,   373,   376,   379,
     382,   385,   388,   392,   396,   399,   402,   404,   405,   408,
     411,   414,   415,   418,   421,   424,   429,   430,   433,   436,
     441,   446,   450,   451,   460,   461,   464,   467,   470,   473,
     476,   479,   483,   487,   490,   493,   494,   497,   501,   504,
     506,   508,   509,   512,   516,   520,   525,   528,   530,   533,
     536,   541,   542,   545,   547,   550,   553,   557,   561,   564,
     567,   570,   575,   576,   579,   580,   581,   587,   588,   589,
     595,   596,   597,   603,   604,   608,   610,   611,   617,   621,
     624,   626,   631,   635,   638,   640,   642,   644,   646,   648,
     649,   652,   654,   656,   658,   660,   662,   664,   666,   668,
     670,   672,   674,   676,   678,   680,   682,   684,   686,   688,
     690,   692,   694,   696,   698,   700,   702,   704,   706,   708,
     710,   712,   714,   716,   718,   720,   722,   724,   726,   728,
     730,   732,   734
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
     108,     0,    -1,   193,    -1,    23,    -1,     4,    -1,    22,
      57,    -1,    83,    -1,    84,    -1,     9,    -1,    10,    -1,
      77,   112,    -1,    78,   112,    -1,    79,   112,    -1,    89,
     112,    -1,    96,   112,    -1,    97,   112,    -1,    21,   112,
      -1,    10,    -1,     6,    -1,     7,    -1,     8,    -1,     5,
     120,    -1,     3,     4,    -1,    42,   109,    -1,    42,    24,
      -1,    85,   111,    -1,    86,   111,    -1,    88,   112,    -1,
      90,   112,    -1,    91,   112,    -1,    92,   112,    -1,    93,
     112,    -1,    94,   112,    -1,    95,   112,    -1,    80,     4,
      -1,    81,     4,    -1,    82,     4,    -1,    76,   109,    -1,
      76,    24,    -1,    17,    57,    -1,    19,   109,    -1,    19,
      41,    -1,    32,    57,    -1,    32,    58,    -1,   104,   112,
      -1,    69,     4,    -1,    68,     4,    -1,   103,   112,    -1,
     103,    57,    -1,   105,   112,    -1,   106,    57,    -1,    17,
      16,    57,    -1,    19,    16,   109,    -1,    19,    16,    24,
      -1,    19,    16,    41,    -1,    20,    16,   109,    -1,    20,
      16,    24,    -1,   146,    15,   147,    -1,   146,    -1,    18,
      26,   147,    27,    -1,    29,   109,    -1,   149,    26,   151,
      27,    -1,    -1,   151,   152,    -1,    43,   109,    -1,    43,
      24,    -1,    43,    11,    -1,    44,   109,    -1,    44,    24,
      -1,    44,    11,    -1,    45,   109,    -1,    45,    24,    -1,
      45,    11,    -1,    45,    12,   109,    -1,    45,    12,    24,
      -1,    45,    14,   109,    -1,    45,    14,    24,    -1,    46,
     109,    -1,    46,    24,    -1,    60,   109,    -1,    60,    24,
      -1,    30,   109,    -1,    30,    24,    -1,    70,   109,    -1,
      71,   109,    -1,    87,   105,   112,    -1,    87,    96,    -1,
      87,    96,   112,    -1,    87,    97,    -1,    87,    97,   112,
      -1,    87,    21,   112,    -1,    87,    98,   112,    -1,    87,
     100,   112,    -1,    87,   101,   112,    -1,    87,   102,   112,
      -1,    87,    99,   112,    -1,    75,    73,   109,    -1,    75,
      73,    24,    -1,    75,   109,    -1,    75,    24,    -1,    37,
     109,    -1,   153,    26,   155,    27,    -1,    -1,   155,   156,
      -1,    48,   157,    -1,    51,   158,    -1,    49,    51,   158,
      -1,    52,   109,    -1,    52,    24,    -1,    49,    52,   109,
      -1,    49,    52,    24,    -1,    49,    54,   109,    -1,    49,
      54,    24,    -1,    53,    57,    57,    64,    -1,    53,    57,
      57,    65,    -1,    53,    57,    57,    66,    -1,    53,    57,
      57,    57,    -1,    55,   169,    -1,    47,   109,    -1,    47,
      24,    -1,    70,   109,    -1,    71,   109,    -1,    75,    73,
     109,    -1,    75,    73,    24,    -1,    75,   109,    -1,    75,
      24,    -1,    40,    -1,    -1,   157,   109,    -1,   157,    24,
      -1,   157,    15,    -1,    -1,   158,   109,    -1,   158,    24,
      -1,   158,    15,    -1,    31,    26,   160,    27,    -1,    -1,
     160,   162,    -1,   109,    26,    -1,   109,    70,   109,    26,
      -1,   109,    71,   109,    26,    -1,   161,   164,    27,    -1,
      -1,   161,   164,    27,    72,   163,    26,   164,    27,    -1,
      -1,   164,   165,    -1,    59,   166,    -1,    30,   109,    -1,
      30,    24,    -1,    60,   109,    -1,    60,    24,    -1,    75,
      73,   109,    -1,    75,    73,    24,    -1,    75,   109,    -1,
      75,    24,    -1,    -1,   166,   109,    -1,   166,    13,   109,
      -1,   166,    15,    -1,    38,    -1,    39,    -1,    -1,   169,
     170,    -1,   169,   170,   167,    -1,   169,   170,   168,    -1,
     169,   170,    11,   170,    -1,   169,    15,    -1,    41,    -1,
      30,   109,    -1,    30,    24,    -1,   171,    26,   173,    27,
      -1,    -1,   173,   174,    -1,    61,    -1,    70,   109,    -1,
      71,   109,    -1,    75,    73,   109,    -1,    75,    73,    24,
      -1,    75,   109,    -1,    75,    24,    -1,    33,   109,    -1,
     175,    26,   177,    27,    -1,    -1,   177,   178,    -1,    -1,
      -1,    66,   179,    28,   180,   186,    -1,    -1,    -1,    66,
     181,   109,   182,   186,    -1,    -1,    -1,    66,   183,    24,
     184,   186,    -1,    -1,    67,   185,   188,    -1,     1,    -1,
      -1,   186,   187,   190,    11,   190,    -1,   190,    11,   190,
      -1,   189,   186,    -1,   189,    -1,   189,    11,   189,   186,
      -1,   189,    11,   189,    -1,   191,   186,    -1,   191,    -1,
      35,    -1,    34,    -1,    36,    -1,     1,    -1,    -1,   193,
     194,    -1,   149,    -1,   154,    -1,   150,    -1,   148,    -1,
     122,    -1,   110,    -1,   121,    -1,   113,    -1,   114,    -1,
     115,    -1,   116,    -1,   117,    -1,   118,    -1,   119,    -1,
     123,    -1,   136,    -1,   133,    -1,   134,    -1,   135,    -1,
     124,    -1,   125,    -1,   126,    -1,   127,    -1,   128,    -1,
     129,    -1,   130,    -1,   131,    -1,   132,    -1,   138,    -1,
     137,    -1,   139,    -1,   140,    -1,   141,    -1,   142,    -1,
     143,    -1,   144,    -1,   145,    -1,   159,    -1,   172,    -1,
     176,    -1,    56,    -1,   192,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   173,   173,   177,   178,   182,   186,   187,   191,   192,
     196,   200,   204,   208,   212,   217,   222,   228,   229,   230,
     231,   235,   239,   243,   244,   248,   252,   256,   260,   264,
     268,   272,   276,   280,   284,   288,   292,   296,   297,   301,
     305,   316,   320,   321,   325,   330,   335,   340,   348,   361,
     366,   378,   379,   385,   391,   392,   393,   397,   398,   402,
     406,   410,   414,   415,   419,   420,   421,   422,   423,   424,
     425,   426,   427,   428,   429,   430,   431,   432,   433,   434,
     435,   436,   437,   438,   439,   440,   441,   442,   443,   444,
     445,   446,   447,   448,   449,   450,   451,   453,   455,   457,
     462,   466,   469,   471,   475,   476,   477,   478,   479,   480,
     481,   482,   483,   484,   486,   488,   490,   492,   493,   494,
     495,   496,   497,   499,   501,   503,   505,   509,   511,   512,
     513,   516,   518,   519,   520,   524,   527,   528,   532,   533,
     534,   538,   540,   539,   544,   546,   550,   551,   552,   553,
     554,   555,   557,   559,   561,   565,   567,   568,   569,   573,
     577,   580,   582,   583,   584,   585,   586,   590,   594,   595,
     599,   602,   604,   609,   610,   611,   612,   613,   614,   615,
     620,   624,   627,   629,   634,   634,   634,   635,   635,   635,
     636,   636,   636,   637,   637,   638,   642,   642,   643,   647,
     648,   649,   650,   651,   652,   655,   658,   661,   665,   668,
     669,   673,   674,   675,   676,   677,   678,   679,   680,   681,
     682,   683,   684,   685,   686,   687,   688,   689,   690,   691,
     692,   693,   694,   695,   696,   697,   698,   699,   700,   701,
     702,   703,   704,   705,   706,   707,   708,   709,   710,   711,
     712,   713,   714
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "ADMINISTRATOR", "QSTRING",
  "CHECK_PROXY_TUNNELS", "QUEUE_CHECKS", "AGGRESSIVE", "LOG_ONLY", "ON",
  "OFF", "CHAR_MINUS", "CHAR_I", "CHAR_EXCLAMATION", "IGNORECASE", "COMMA",
  "EQUAL", "PORT", "HTTP_SERVER", "INTERFACE", "IMAGES",
  "HTTPS_PROHIBIT_INSECURE_SSLV2", "HTTPS_CONNECTION_CACHE_SIZE",
  "IDENTIFIER", "WORD", "END", "START_BRACKET", "STOP_BRACKET", "WEEKDAY",
  "CATEGORY", "REWRITE", "ACL", "CPUS", "TIME", "TVAL", "DVAL", "DVALCRON",
  "SOURCE", "CIDR", "IPCLASS", "CONTINUE", "IPADDR", "DBHOME",
  "DOMAINLIST", "URLLIST", "EXPRESSIONLIST", "CACERTS", "IPLIST", "DOMAIN",
  "UNIX", "LDAP", "USER", "USERLIST", "USERQUOTA", "GROUP", "IP", "NL",
  "NUMBER", "NUMBERS", "PASS", "REDIRECT", "SUBST", "CHAR", "MINUTELY",
  "HOURLY", "DAILY", "WEEKLY", "DATE", "REDIRECT_FATAL_ERROR",
  "REDIRECT_LOADING_DATABASE", "WITHIN", "OUTSIDE", "ELSE", "ANONYMOUS",
  "SPORADIC", "LOGFILE", "LOGDIR", "LOGBLOCK", "LOGALL", "LOGALL_HTTPD",
  "MAIL_SERVER", "ADMIN_EMAIL", "EXTERNAL_STATUS_COMMAND", "TOKEN_ALLOW",
  "TOKEN_DENY", "URL_LOOKUP_RESULT_DB_RELOAD",
  "URL_LOOKUP_RESULT_FATAL_ERROR", "OPTION", "UFDB_SHOW_URL_DETAILS",
  "UFDB_DEBUG_FILTER", "UFDB_DEBUG_SKYPE_PROBES",
  "UFDB_DEBUG_GTALK_PROBES", "UFDB_DEBUG_YAHOOMSG_PROBES",
  "UFDB_DEBUG_AIM_PROBES", "UFDB_EXPRESSION_OPTIMISATION",
  "UFDB_EXPRESSION_DEBUG", "ENFORCE_HTTPS_WITH_HOSTNAME",
  "ENFORCE_HTTPS_OFFICAL_CERTIFICATE", "ALLOW_SKYPE_OVER_HTTPS",
  "ALLOW_UNKNOWN_PROTOCOL_OVER_HTTPS", "ALLOW_GTALK_OVER_HTTPS",
  "ALLOW_YAHOOMSG_OVER_HTTPS", "ALLOW_AIM_OVER_HTTPS",
  "ANALYSE_UNCATEGORISED", "UPLOAD_STATS", "SAFE_SEARCH",
  "MAX_LOGFILE_SIZE", "$accept", "start", "qidentifier",
  "https_cache_size", "allow_or_deny", "on_or_off", "log_block", "log_all",
  "logall_httpd", "debug_filter", "enforce_https_with_hostname",
  "enforce_https_offical_certificate", "https_prohibit_insecure_sslv2",
  "check_proxy_tunnel_option", "check_proxy_tunnels", "admin_spec",
  "dbhome", "url_lookup_result_db_reload", "url_lookup_result_fatal_error",
  "ufdb_show_url_details", "ufdb_debug_skype_probes",
  "ufdb_debug_gtalk_probes", "ufdb_debug_yahoomsg_probes",
  "ufdb_debug_aim_probes", "ufdb_expression_optimisation",
  "ufdb_expression_debug", "mail_server", "admin_email",
  "external_status_command", "logdir", "port", "interface", "cpus",
  "upload_stats", "redirect_loading_database", "redirect_fatal_error",
  "analyse_uncategorised", "safe_search", "max_logfile_size",
  "httpd_option", "httpd_options", "http_server_def", "category",
  "category_block", "category_contents", "category_content", "source",
  "source_block", "source_contents", "source_content", "domain", "user",
  "acl_block", "acl_contents", "acl_header", "acl_content", "@1",
  "access_contents", "access_content", "access_pass", "cidr", "ipclass",
  "ips", "ip", "rew", "rew_block", "rew_contents", "rew_content", "time",
  "time_block", "time_contents", "time_content", "@2", "@3", "@4", "@5",
  "@6", "@7", "@8", "ttime", "@9", "date", "dval", "tval", "dvalcron",
  "an_error", "statements", "statement", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,   317,   318,   319,   320,   321,   322,   323,   324,
     325,   326,   327,   328,   329,   330,   331,   332,   333,   334,
     335,   336,   337,   338,   339,   340,   341,   342,   343,   344,
     345,   346,   347,   348,   349,   350,   351,   352,   353,   354,
     355,   356,   357,   358,   359,   360,   361
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,   107,   108,   109,   109,   110,   111,   111,   112,   112,
     113,   114,   115,   116,   117,   118,   119,   120,   120,   120,
     120,   121,   122,   123,   123,   124,   125,   126,   127,   128,
     129,   130,   131,   132,   133,   134,   135,   136,   136,   137,
     138,   138,   139,   139,   140,   141,   142,   143,   143,   144,
     145,   146,   146,   146,   146,   146,   146,   147,   147,   148,
     149,   150,   151,   151,   152,   152,   152,   152,   152,   152,
     152,   152,   152,   152,   152,   152,   152,   152,   152,   152,
     152,   152,   152,   152,   152,   152,   152,   152,   152,   152,
     152,   152,   152,   152,   152,   152,   152,   152,   152,   152,
     153,   154,   155,   155,   156,   156,   156,   156,   156,   156,
     156,   156,   156,   156,   156,   156,   156,   156,   156,   156,
     156,   156,   156,   156,   156,   156,   156,   157,   157,   157,
     157,   158,   158,   158,   158,   159,   160,   160,   161,   161,
     161,   162,   163,   162,   164,   164,   165,   165,   165,   165,
     165,   165,   165,   165,   165,   166,   166,   166,   166,   167,
     168,   169,   169,   169,   169,   169,   169,   170,   171,   171,
     172,   173,   173,   174,   174,   174,   174,   174,   174,   174,
     175,   176,   177,   177,   179,   180,   178,   181,   182,   178,
     183,   184,   178,   185,   178,   178,   187,   186,   186,   188,
     188,   188,   188,   188,   188,   189,   190,   191,   192,   193,
     193,   194,   194,   194,   194,   194,   194,   194,   194,   194,
     194,   194,   194,   194,   194,   194,   194,   194,   194,   194,
     194,   194,   194,   194,   194,   194,   194,   194,   194,   194,
     194,   194,   194,   194,   194,   194,   194,   194,   194,   194,
     194,   194,   194
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     1,     1,     2,     1,     1,     1,     1,
       2,     2,     2,     2,     2,     2,     2,     1,     1,     1,
       1,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     3,     3,     3,     3,     3,     3,     3,     1,     4,
       2,     4,     0,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     3,     3,     3,     3,     2,     2,     2,
       2,     2,     2,     2,     2,     3,     2,     3,     2,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     2,     2,
       2,     4,     0,     2,     2,     2,     3,     2,     2,     3,
       3,     3,     3,     4,     4,     4,     4,     2,     2,     2,
       2,     2,     3,     3,     2,     2,     1,     0,     2,     2,
       2,     0,     2,     2,     2,     4,     0,     2,     2,     4,
       4,     3,     0,     8,     0,     2,     2,     2,     2,     2,
       2,     3,     3,     2,     2,     0,     2,     3,     2,     1,
       1,     0,     2,     3,     3,     4,     2,     1,     2,     2,
       4,     0,     2,     1,     2,     2,     3,     3,     2,     2,
       2,     4,     0,     2,     0,     0,     5,     0,     0,     5,
       0,     0,     5,     0,     3,     1,     0,     5,     3,     2,
       1,     4,     3,     2,     1,     1,     1,     1,     1,     0,
       2,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
     209,     0,     0,     1,   208,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   251,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   216,   218,   219,   220,   221,
     222,   223,   224,   217,   215,   225,   230,   231,   232,   233,
     234,   235,   236,   237,   238,   227,   228,   229,   226,   240,
     239,   241,   242,   243,   244,   245,   246,   247,   214,   211,
     213,     0,   212,   248,     0,   249,     0,   250,   252,   210,
      22,    18,    19,    20,    17,    21,    39,     0,     4,     3,
      41,    40,     8,     9,    16,     5,    60,   169,   168,   136,
      42,    43,   180,   100,    24,    23,    46,    45,    38,    37,
      10,    11,    12,    34,    35,    36,     6,     7,    25,    26,
      27,    13,    28,    29,    30,    31,    32,    33,    14,    15,
      48,    47,    44,    49,    50,    62,   102,   171,   182,     0,
       0,     0,    58,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    59,   135,     0,   144,   137,    61,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    63,
     101,   126,     0,   127,     0,   131,     0,     0,   161,     0,
       0,     0,   103,   170,   173,     0,     0,     0,   172,   195,
     181,   187,   193,   183,    51,    53,    54,    52,    56,    55,
      57,   138,     0,     0,     0,    82,    81,    66,    65,    64,
      69,    68,    67,    72,     0,     0,    71,    70,    78,    77,
      80,    79,    83,    84,    99,     0,    98,     0,    86,    88,
       0,     0,     0,     0,     0,     0,   119,   118,   104,   131,
       0,     0,   105,   108,   107,     0,   117,   120,   121,   125,
       0,   124,   174,   175,   179,     0,   178,     0,     0,     0,
       0,     0,     0,   141,     0,   155,     0,     0,   145,    74,
      73,    76,    75,    97,    96,    90,    87,    89,    91,    95,
      92,    93,    94,    85,   130,   129,   128,   106,   110,   109,
     112,   111,   134,   133,   132,     0,   166,   167,   162,   123,
     122,   177,   176,   185,   188,   191,   205,   207,   194,   200,
     204,   139,   140,   142,   148,   147,   146,   150,   149,   154,
       0,   153,   116,   113,   114,   115,     0,   159,   160,   163,
     164,     0,     0,     0,     0,   206,   199,     0,   203,     0,
       0,   158,   156,   152,   151,   165,   186,   189,   192,   202,
       0,     0,   144,   157,   201,     0,   198,     0,     0,   143,
     197
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,   304,    45,   128,   104,    46,    47,    48,    49,
      50,    51,    52,    95,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,    74,    75,    76,    77,   152,
     153,    78,    79,    80,   155,   179,    81,    82,   156,   192,
     248,   252,    83,   154,   166,   167,   349,   214,   278,   326,
     339,   340,   256,   308,    84,    85,   157,   198,    86,    87,
     158,   203,   267,   341,   268,   342,   269,   343,   270,   346,
     360,   318,   319,   347,   320,    88,     2,    89
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -150
static const yytype_int16 yypact[] =
{
    -150,    10,    36,  -150,  -150,    -2,   214,   -39,    23,    15,
     139,   -10,    41,   155,    26,   117,    41,    41,   246,  -150,
      56,    59,   251,   139,   139,   139,    91,   102,   104,   169,
     169,   139,   139,   139,   139,   139,   139,   139,   139,   139,
     139,     2,   139,   139,    66,  -150,  -150,  -150,  -150,  -150,
    -150,  -150,  -150,  -150,  -150,  -150,  -150,  -150,  -150,  -150,
    -150,  -150,  -150,  -150,  -150,  -150,  -150,  -150,  -150,  -150,
    -150,  -150,  -150,  -150,  -150,  -150,  -150,  -150,  -150,   109,
    -150,   112,  -150,  -150,   124,  -150,   145,  -150,  -150,  -150,
    -150,  -150,  -150,  -150,  -150,  -150,  -150,   208,  -150,  -150,
    -150,  -150,  -150,  -150,  -150,  -150,  -150,  -150,  -150,  -150,
    -150,  -150,  -150,  -150,  -150,  -150,  -150,  -150,  -150,  -150,
    -150,  -150,  -150,  -150,  -150,  -150,  -150,  -150,  -150,  -150,
    -150,  -150,  -150,  -150,  -150,  -150,  -150,  -150,  -150,  -150,
    -150,  -150,  -150,  -150,  -150,  -150,  -150,  -150,  -150,   160,
     177,   181,   170,   172,    75,   217,   143,    83,    21,   153,
      96,   272,   208,  -150,  -150,   -20,  -150,  -150,  -150,   274,
     260,   262,   234,   276,   278,    41,    41,    -3,   -16,  -150,
    -150,  -150,   285,  -150,   292,  -150,   287,   183,  -150,    41,
      41,    38,  -150,  -150,  -150,    41,    41,    70,  -150,  -150,
    -150,    79,  -150,  -150,  -150,  -150,  -150,  -150,  -150,  -150,
    -150,  -150,    41,    41,    16,  -150,  -150,  -150,  -150,  -150,
    -150,  -150,  -150,  -150,   289,   299,  -150,  -150,  -150,  -150,
    -150,  -150,  -150,  -150,  -150,   301,  -150,   139,   139,   139,
     139,   139,   139,   139,   139,   139,  -150,  -150,   132,  -150,
     303,   310,   185,  -150,  -150,   186,    -1,  -150,  -150,  -150,
     312,  -150,  -150,  -150,  -150,   314,  -150,   221,    41,   230,
     296,   253,   255,   200,   316,  -150,   324,    73,  -150,  -150,
    -150,  -150,  -150,  -150,  -150,  -150,  -150,  -150,  -150,  -150,
    -150,  -150,  -150,  -150,  -150,  -150,  -150,   185,  -150,  -150,
    -150,  -150,  -150,  -150,  -150,   288,  -150,  -150,    33,  -150,
    -150,  -150,  -150,  -150,  -150,  -150,  -150,  -150,  -150,   123,
     256,  -150,  -150,  -150,  -150,  -150,    86,  -150,  -150,  -150,
     326,  -150,  -150,  -150,  -150,  -150,   265,  -150,  -150,  -150,
    -150,   256,   256,   256,   259,  -150,   281,   308,   281,   325,
      41,  -150,  -150,  -150,  -150,  -150,   281,   281,   281,   256,
     256,   256,  -150,  -150,   281,   318,  -150,   142,   256,  -150,
    -150
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -150,  -150,    -9,  -150,   327,    -8,  -150,  -150,  -150,  -150,
    -150,  -150,  -150,  -150,  -150,  -150,  -150,  -150,  -150,  -150,
    -150,  -150,  -150,  -150,  -150,  -150,  -150,  -150,  -150,  -150,
    -150,  -150,  -150,  -150,  -150,  -150,  -150,  -150,  -150,  -150,
     180,  -150,  -150,  -150,  -150,  -150,  -150,  -150,  -150,  -150,
    -150,   106,  -150,  -150,  -150,  -150,  -150,    -6,  -150,  -150,
    -150,  -150,  -150,    22,  -150,  -150,  -150,  -150,  -150,  -150,
    -150,  -150,  -150,  -150,  -150,  -150,  -150,  -150,  -150,  -136,
    -150,  -150,    17,  -149,  -150,  -150,  -150,  -150
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -197
static const yytype_int16 yytable[] =
{
     101,    98,    90,   106,   108,   237,   211,   112,   113,   115,
       3,   102,   103,   119,   306,   120,   121,   122,    96,    98,
      99,   234,   199,   130,   131,   132,   133,   134,   135,   136,
     137,   138,   139,   141,   142,   143,    -2,     4,    99,     5,
     307,     6,    98,   273,   336,    98,   274,   105,   200,    97,
     212,   213,   109,     7,     8,     9,   100,    10,    11,   140,
     116,    99,   259,   117,    99,    12,    13,    14,    15,    16,
     235,   337,   338,    17,    98,   275,   276,    98,    18,    98,
     238,   239,   240,   241,   242,   243,   244,   201,   202,   245,
      98,   277,    19,    99,   264,   123,    99,   329,    99,   350,
      98,   351,   164,  -190,    20,    21,   124,  -184,   125,    99,
     193,   260,    22,    23,    24,    25,    26,    27,    28,    99,
     205,    29,    30,   144,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,   344,   145,    98,   206,   146,    41,
      42,    43,    44,   265,   194,   165,   330,   294,   102,   103,
     147,   207,   209,   195,   196,    99,   295,   345,   197,    98,
     216,   219,   222,   227,   229,   231,   232,   233,   236,   369,
     180,   148,   274,   247,   110,   111,   159,   254,    99,   107,
     257,   258,   261,   181,   348,   162,   262,   263,   266,    98,
     182,   183,   184,   160,   185,   186,   187,   161,   188,   163,
     302,   275,   276,   271,   272,   356,   357,   358,    99,   303,
     204,   365,   366,   189,   190,   280,   282,   277,   191,   370,
      91,    92,    93,   364,    94,   149,   284,   150,   151,   285,
     286,   287,   288,   289,   290,   291,   292,   293,    98,   296,
     255,   299,   301,   305,   168,   223,   224,   169,   225,   313,
      98,   310,   126,   127,   315,    98,   312,    99,   226,   314,
     170,   171,   172,   173,    98,   325,    98,   328,   331,    99,
     114,   217,   323,   220,    99,   118,    98,   174,    98,   321,
      98,   322,    98,    99,   218,    99,   221,   175,   176,    98,
     345,    98,   177,    98,   316,    99,   208,    99,   215,    99,
     228,    99,   230,    98,   178,    98,   307,    98,    99,   246,
      99,   253,    99,   279,    98,  -196,    98,   352,    98,   361,
      98,   354,    99,   281,    99,   283,    99,   298,    98,   368,
      98,   316,   317,    99,   300,    99,   309,    99,   311,    99,
     324,   363,   210,   249,   250,   332,   251,    99,   327,    99,
     353,   362,   333,   334,   335,   297,   367,   129,   355,     0,
       0,   359
};

static const yytype_int16 yycheck[] =
{
       9,     4,     4,    12,    13,    21,    26,    16,    17,    18,
       0,     9,    10,    22,    15,    23,    24,    25,    57,     4,
      23,    24,     1,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,     0,     1,    23,     3,
      41,     5,     4,    27,    11,     4,    30,    57,    27,    26,
      70,    71,    26,    17,    18,    19,    41,    21,    22,    57,
       4,    23,    24,     4,    23,    29,    30,    31,    32,    33,
      73,    38,    39,    37,     4,    59,    60,     4,    42,     4,
      96,    97,    98,    99,   100,   101,   102,    66,    67,   105,
       4,    75,    56,    23,    24,     4,    23,    24,    23,    13,
       4,    15,    27,    24,    68,    69,     4,    28,     4,    23,
      27,    73,    76,    77,    78,    79,    80,    81,    82,    23,
      24,    85,    86,    57,    88,    89,    90,    91,    92,    93,
      94,    95,    96,    97,    11,    26,     4,    41,    26,   103,
     104,   105,   106,    73,    61,   154,    73,    15,     9,    10,
      26,   160,   161,    70,    71,    23,    24,    34,    75,     4,
     169,   170,   171,   172,   173,   174,   175,   176,   177,    27,
      27,    26,    30,   182,    57,    58,    16,   186,    23,    24,
     189,   190,   191,    40,   320,    15,   195,   196,   197,     4,
      47,    48,    49,    16,    51,    52,    53,    16,    55,    27,
      15,    59,    60,   212,   213,   341,   342,   343,    23,    24,
      57,   360,   361,    70,    71,   224,   225,    75,    75,   368,
       6,     7,     8,   359,    10,    17,   235,    19,    20,   237,
     238,   239,   240,   241,   242,   243,   244,   245,     4,   248,
      57,   250,   251,    57,    27,    11,    12,    30,    14,    28,
       4,   260,    83,    84,    24,     4,   265,    23,    24,   268,
      43,    44,    45,    46,     4,   274,     4,   276,   277,    23,
      24,    11,    72,    11,    23,    24,     4,    60,     4,    26,
       4,    26,     4,    23,    24,    23,    24,    70,    71,     4,
      34,     4,    75,     4,    35,    23,    24,    23,    24,    23,
      24,    23,    24,     4,    87,     4,    41,     4,    23,    24,
      23,    24,    23,    24,     4,    34,     4,   326,     4,    11,
       4,   330,    23,    24,    23,    24,    23,    24,     4,    11,
       4,    35,    36,    23,    24,    23,    24,    23,    24,    23,
      24,   350,   162,    51,    52,    57,    54,    23,    24,    23,
      24,    26,    64,    65,    66,   249,   362,    30,   336,    -1,
      -1,   344
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,   108,   193,     0,     1,     3,     5,    17,    18,    19,
      21,    22,    29,    30,    31,    32,    33,    37,    42,    56,
      68,    69,    76,    77,    78,    79,    80,    81,    82,    85,
      86,    88,    89,    90,    91,    92,    93,    94,    95,    96,
      97,   103,   104,   105,   106,   110,   113,   114,   115,   116,
     117,   118,   119,   121,   122,   123,   124,   125,   126,   127,
     128,   129,   130,   131,   132,   133,   134,   135,   136,   137,
     138,   139,   140,   141,   142,   143,   144,   145,   148,   149,
     150,   153,   154,   159,   171,   172,   175,   176,   192,   194,
       4,     6,     7,     8,    10,   120,    57,    26,     4,    23,
      41,   109,     9,    10,   112,    57,   109,    24,   109,    26,
      57,    58,   109,   109,    24,   109,     4,     4,    24,   109,
     112,   112,   112,     4,     4,     4,    83,    84,   111,   111,
     112,   112,   112,   112,   112,   112,   112,   112,   112,   112,
      57,   112,   112,   112,    57,    26,    26,    26,    26,    17,
      19,    20,   146,   147,   160,   151,   155,   173,   177,    16,
      16,    16,    15,    27,    27,   109,   161,   162,    27,    30,
      43,    44,    45,    46,    60,    70,    71,    75,    87,   152,
      27,    40,    47,    48,    49,    51,    52,    53,    55,    70,
      71,    75,   156,    27,    61,    70,    71,    75,   174,     1,
      27,    66,    67,   178,    57,    24,    41,   109,    24,   109,
     147,    26,    70,    71,   164,    24,   109,    11,    24,   109,
      11,    24,   109,    11,    12,    14,    24,   109,    24,   109,
      24,   109,   109,   109,    24,    73,   109,    21,    96,    97,
      98,    99,   100,   101,   102,   105,    24,   109,   157,    51,
      52,    54,   158,    24,   109,    57,   169,   109,   109,    24,
      73,   109,   109,   109,    24,    73,   109,   179,   181,   183,
     185,   109,   109,    27,    30,    59,    60,    75,   165,    24,
     109,    24,   109,    24,   109,   112,   112,   112,   112,   112,
     112,   112,   112,   112,    15,    24,   109,   158,    24,   109,
      24,   109,    15,    24,   109,    57,    15,    41,   170,    24,
     109,    24,   109,    28,   109,    24,    35,    36,   188,   189,
     191,    26,    26,    72,    24,   109,   166,    24,   109,    24,
      73,   109,    57,    64,    65,    66,    11,    38,    39,   167,
     168,   180,   182,   184,    11,    34,   186,   190,   186,   163,
      13,    15,   109,    24,   109,   170,   186,   186,   186,   189,
     187,    11,    26,   109,   186,   190,   190,   164,    11,    27,
     190
};

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrorlab


/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */

#define YYFAIL		goto yyerrlab

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)					\
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    {								\
      yychar = (Token);						\
      yylval = (Value);						\
      yytoken = YYTRANSLATE (yychar);				\
      YYPOPSTACK (1);						\
      goto yybackup;						\
    }								\
  else								\
    {								\
      yyerror (YY_("syntax error: cannot back up")); \
      YYERROR;							\
    }								\
while (YYID (0))


#define YYTERROR	1
#define YYERRCODE	256


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#define YYRHSLOC(Rhs, K) ((Rhs)[K])
#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)				\
    do									\
      if (YYID (N))                                                    \
	{								\
	  (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;	\
	  (Current).first_column = YYRHSLOC (Rhs, 1).first_column;	\
	  (Current).last_line    = YYRHSLOC (Rhs, N).last_line;		\
	  (Current).last_column  = YYRHSLOC (Rhs, N).last_column;	\
	}								\
      else								\
	{								\
	  (Current).first_line   = (Current).last_line   =		\
	    YYRHSLOC (Rhs, 0).last_line;				\
	  (Current).first_column = (Current).last_column =		\
	    YYRHSLOC (Rhs, 0).last_column;				\
	}								\
    while (YYID (0))
#endif


/* YY_LOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

#ifndef YY_LOCATION_PRINT
# if YYLTYPE_IS_TRIVIAL
#  define YY_LOCATION_PRINT(File, Loc)			\
     fprintf (File, "%d.%d-%d.%d",			\
	      (Loc).first_line, (Loc).first_column,	\
	      (Loc).last_line,  (Loc).last_column)
# else
#  define YY_LOCATION_PRINT(File, Loc) ((void) 0)
# endif
#endif


/* YYLEX -- calling `yylex' with the right arguments.  */

#ifdef YYLEX_PARAM
# define YYLEX yylex (YYLEX_PARAM)
#else
# define YYLEX yylex ()
#endif

/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (YYID (0))

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)			  \
do {									  \
  if (yydebug)								  \
    {									  \
      YYFPRINTF (stderr, "%s ", Title);					  \
      yy_symbol_print (stderr,						  \
		  Type, Value); \
      YYFPRINTF (stderr, "\n");						  \
    }									  \
} while (YYID (0))


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_value_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# else
  YYUSE (yyoutput);
# endif
  switch (yytype)
    {
      default:
	break;
    }
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (yytype < YYNTOKENS)
    YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_stack_print (yytype_int16 *bottom, yytype_int16 *top)
#else
static void
yy_stack_print (bottom, top)
    yytype_int16 *bottom;
    yytype_int16 *top;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (; bottom <= top; ++bottom)
    YYFPRINTF (stderr, " %d", *bottom);
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (YYID (0))


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_reduce_print (YYSTYPE *yyvsp, int yyrule)
#else
static void
yy_reduce_print (yyvsp, yyrule)
    YYSTYPE *yyvsp;
    int yyrule;
#endif
{
  int yynrhs = yyr2[yyrule];
  int yyi;
  unsigned long int yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
	     yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      fprintf (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr, yyrhs[yyprhs[yyrule] + yyi],
		       &(yyvsp[(yyi + 1) - (yynrhs)])
		       		       );
      fprintf (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (yyvsp, Rule); \
} while (YYID (0))

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef	YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif



#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static YYSIZE_T
yystrlen (const char *yystr)
#else
static YYSIZE_T
yystrlen (yystr)
    const char *yystr;
#endif
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static char *
yystpcpy (char *yydest, const char *yysrc)
#else
static char *
yystpcpy (yydest, yysrc)
    char *yydest;
    const char *yysrc;
#endif
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
	switch (*++yyp)
	  {
	  case '\'':
	  case ',':
	    goto do_not_strip_quotes;

	  case '\\':
	    if (*++yyp != '\\')
	      goto do_not_strip_quotes;
	    /* Fall through.  */
	  default:
	    if (yyres)
	      yyres[yyn] = *yyp;
	    yyn++;
	    break;

	  case '"':
	    if (yyres)
	      yyres[yyn] = '\0';
	    return yyn;
	  }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into YYRESULT an error message about the unexpected token
   YYCHAR while in state YYSTATE.  Return the number of bytes copied,
   including the terminating null byte.  If YYRESULT is null, do not
   copy anything; just return the number of bytes that would be
   copied.  As a special case, return 0 if an ordinary "syntax error"
   message will do.  Return YYSIZE_MAXIMUM if overflow occurs during
   size calculation.  */
static YYSIZE_T
yysyntax_error (char *yyresult, int yystate, int yychar)
{
  int yyn = yypact[yystate];

  if (! (YYPACT_NINF < yyn && yyn <= YYLAST))
    return 0;
  else
    {
      int yytype = YYTRANSLATE (yychar);
      YYSIZE_T yysize0 = yytnamerr (0, yytname[yytype]);
      YYSIZE_T yysize = yysize0;
      YYSIZE_T yysize1;
      int yysize_overflow = 0;
      enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
      char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
      int yyx;

# if 0
      /* This is so xgettext sees the translatable formats that are
	 constructed on the fly.  */
      YY_("syntax error, unexpected %s");
      YY_("syntax error, unexpected %s, expecting %s");
      YY_("syntax error, unexpected %s, expecting %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s");
# endif
      char *yyfmt;
      char const *yyf;
      static char const yyunexpected[] = "syntax error, unexpected %s";
      static char const yyexpecting[] = ", expecting %s";
      static char const yyor[] = " or %s";
      char yyformat[sizeof yyunexpected
		    + sizeof yyexpecting - 1
		    + ((YYERROR_VERBOSE_ARGS_MAXIMUM - 2)
		       * (sizeof yyor - 1))];
      char const *yyprefix = yyexpecting;

      /* Start YYX at -YYN if negative to avoid negative indexes in
	 YYCHECK.  */
      int yyxbegin = yyn < 0 ? -yyn : 0;

      /* Stay within bounds of both yycheck and yytname.  */
      int yychecklim = YYLAST - yyn + 1;
      int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
      int yycount = 1;

      yyarg[0] = yytname[yytype];
      yyfmt = yystpcpy (yyformat, yyunexpected);

      for (yyx = yyxbegin; yyx < yyxend; ++yyx)
	if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
	  {
	    if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
	      {
		yycount = 1;
		yysize = yysize0;
		yyformat[sizeof yyunexpected - 1] = '\0';
		break;
	      }
	    yyarg[yycount++] = yytname[yyx];
	    yysize1 = yysize + yytnamerr (0, yytname[yyx]);
	    yysize_overflow |= (yysize1 < yysize);
	    yysize = yysize1;
	    yyfmt = yystpcpy (yyfmt, yyprefix);
	    yyprefix = yyor;
	  }

      yyf = YY_(yyformat);
      yysize1 = yysize + yystrlen (yyf);
      yysize_overflow |= (yysize1 < yysize);
      yysize = yysize1;

      if (yysize_overflow)
	return YYSIZE_MAXIMUM;

      if (yyresult)
	{
	  /* Avoid sprintf, as that infringes on the user's name space.
	     Don't have undefined behavior even if the translation
	     produced a string with the wrong number of "%s"s.  */
	  char *yyp = yyresult;
	  int yyi = 0;
	  while ((*yyp = *yyf) != '\0')
	    {
	      if (*yyp == '%' && yyf[1] == 's' && yyi < yycount)
		{
		  yyp += yytnamerr (yyp, yyarg[yyi++]);
		  yyf += 2;
		}
	      else
		{
		  yyp++;
		  yyf++;
		}
	    }
	}
      return yysize;
    }
}
#endif /* YYERROR_VERBOSE */


/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep)
#else
static void
yydestruct (yymsg, yytype, yyvaluep)
    const char *yymsg;
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  YYUSE (yyvaluep);

  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  switch (yytype)
    {

      default:
	break;
    }
}


/* Prevent warnings from -Wmissing-prototypes.  */

#ifdef YYPARSE_PARAM
#if defined __STDC__ || defined __cplusplus
int yyparse (void *YYPARSE_PARAM);
#else
int yyparse ();
#endif
#else /* ! YYPARSE_PARAM */
#if defined __STDC__ || defined __cplusplus
int yyparse (void);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */



/* The look-ahead symbol.  */
int yychar;

/* The semantic value of the look-ahead symbol.  */
YYSTYPE yylval;

/* Number of syntax errors so far.  */
int yynerrs;



/*----------.
| yyparse.  |
`----------*/

#ifdef YYPARSE_PARAM
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void *YYPARSE_PARAM)
#else
int
yyparse (YYPARSE_PARAM)
    void *YYPARSE_PARAM;
#endif
#else /* ! YYPARSE_PARAM */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void)
#else
int
yyparse ()

#endif
#endif
{
  
  int yystate;
  int yyn;
  int yyresult;
  /* Number of tokens to shift before error messages enabled.  */
  int yyerrstatus;
  /* Look-ahead token as an internal (translated) token number.  */
  int yytoken = 0;
#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

  /* Three stacks and their tools:
     `yyss': related to states,
     `yyvs': related to semantic values,
     `yyls': related to locations.

     Refer to the stacks thru separate pointers, to allow yyoverflow
     to reallocate them elsewhere.  */

  /* The state stack.  */
  yytype_int16 yyssa[YYINITDEPTH];
  yytype_int16 *yyss = yyssa;
  yytype_int16 *yyssp;

  /* The semantic value stack.  */
  YYSTYPE yyvsa[YYINITDEPTH];
  YYSTYPE *yyvs = yyvsa;
  YYSTYPE *yyvsp;



#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  YYSIZE_T yystacksize = YYINITDEPTH;

  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;


  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;		/* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss;
  yyvsp = yyvs;

  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack.  Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	yytype_int16 *yyss1 = yyss;


	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow (YY_("memory exhausted"),
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),

		    &yystacksize);

	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
	goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
	yystacksize = YYMAXDEPTH;

      {
	yytype_int16 *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyexhaustedlab;
	YYSTACK_RELOCATE (yyss);
	YYSTACK_RELOCATE (yyvs);

#  undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;


      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
	YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     look-ahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to look-ahead token.  */
  yyn = yypact[yystate];
  if (yyn == YYPACT_NINF)
    goto yydefault;

  /* Not known => get a look-ahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid look-ahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yyn == 0 || yyn == YYTABLE_NINF)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the look-ahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  yystate = yyn;
  *++yyvsp = yylval;

  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     `$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 3:
#line 177 "sg.y"
    { (yyval.string) = (yyvsp[(1) - (1)].string); }
    break;

  case 4:
#line 178 "sg.y"
    { (yyval.string) = (yyvsp[(1) - (1)].string); }
    break;

  case 5:
#line 182 "sg.y"
    { httpsConnectionCacheSize = atoi( (yyvsp[(2) - (2)].string) ); ufdbFree( (yyvsp[(2) - (2)].string) ); }
    break;

  case 6:
#line 186 "sg.y"
    { (yyval.integer) = UFDB_ALLOW; }
    break;

  case 7:
#line 187 "sg.y"
    { (yyval.integer) = UFDB_DENY; }
    break;

  case 8:
#line 191 "sg.y"
    { (yyval.integer) = 1; }
    break;

  case 9:
#line 192 "sg.y"
    { (yyval.integer) = 0; }
    break;

  case 10:
#line 196 "sg.y"
    { UFDBglobalLogBlock = (yyvsp[(2) - (2)].integer); }
    break;

  case 11:
#line 200 "sg.y"
    { UFDBglobalLogAllRequests = (yyvsp[(2) - (2)].integer); }
    break;

  case 12:
#line 204 "sg.y"
    { UFDBglobalDebugHttpd = (yyvsp[(2) - (2)].integer); }
    break;

  case 13:
#line 208 "sg.y"
    { UFDBglobalDebug = (yyvsp[(2) - (2)].integer); }
    break;

  case 14:
#line 213 "sg.y"
    { ufdbLogError( "line %d: option enforce-https-with-hostname must be part of the security category", lineno );  }
    break;

  case 15:
#line 218 "sg.y"
    { ufdbLogError( "line %d: option enforce-https-official-certificate must be part of the security category", lineno );  }
    break;

  case 16:
#line 223 "sg.y"
    { ufdbLogError( "line %d: option https-prohibit-insecure-sslv2 must be part of the security category", lineno );  }
    break;

  case 17:
#line 228 "sg.y"
    { (yyval.integer) = UFDB_API_HTTPS_CHECK_OFF; }
    break;

  case 18:
#line 229 "sg.y"
    { (yyval.integer) = UFDB_API_HTTPS_CHECK_QUEUE_CHECKS; }
    break;

  case 19:
#line 230 "sg.y"
    { (yyval.integer) = UFDB_API_HTTPS_CHECK_AGGRESSIVE; }
    break;

  case 20:
#line 231 "sg.y"
    { (yyval.integer) = UFDB_API_HTTPS_LOG_ONLY; }
    break;

  case 21:
#line 235 "sg.y"
    { UFDBsetTunnelCheckMethod( (yyvsp[(2) - (2)].integer) ); }
    break;

  case 22:
#line 239 "sg.y"
    { ufdbSetting("administrator",(yyvsp[(2) - (2)].string)); }
    break;

  case 23:
#line 243 "sg.y"
    { ufdbSetting("dbhome",(yyvsp[(2) - (2)].string)); }
    break;

  case 24:
#line 244 "sg.y"
    { ufdbSetting("dbhome",(yyvsp[(2) - (2)].string)); }
    break;

  case 25:
#line 249 "sg.y"
    { UFDBglobalURLlookupResultDBreload = (yyvsp[(2) - (2)].integer); }
    break;

  case 26:
#line 253 "sg.y"
    { UFDBglobalURLlookupResultFatalError = (yyvsp[(2) - (2)].integer); }
    break;

  case 27:
#line 256 "sg.y"
    { UFDBglobalShowURLdetails = (yyvsp[(2) - (2)].integer); }
    break;

  case 28:
#line 260 "sg.y"
    { UFDBglobalDebugSkype = (yyvsp[(2) - (2)].integer); }
    break;

  case 29:
#line 264 "sg.y"
    { UFDBglobalDebugGtalk = (yyvsp[(2) - (2)].integer); }
    break;

  case 30:
#line 268 "sg.y"
    { UFDBglobalDebugYahooMsg = (yyvsp[(2) - (2)].integer); }
    break;

  case 31:
#line 272 "sg.y"
    { UFDBglobalDebugAim = (yyvsp[(2) - (2)].integer); }
    break;

  case 32:
#line 276 "sg.y"
    { UFDBglobalExpressionOptimisation = (yyvsp[(2) - (2)].integer); }
    break;

  case 33:
#line 280 "sg.y"
    { UFDBglobalDebugRegexp = (yyvsp[(2) - (2)].integer); }
    break;

  case 34:
#line 284 "sg.y"
    { ufdbFree( UFDBglobalEmailServer ); UFDBglobalEmailServer = (yyvsp[(2) - (2)].string); }
    break;

  case 35:
#line 288 "sg.y"
    { ufdbFree( UFDBglobalAdminEmail ); UFDBglobalAdminEmail = (yyvsp[(2) - (2)].string); }
    break;

  case 36:
#line 292 "sg.y"
    {  ufdbFree( UFDBglobalExternalStatusCommand );  UFDBglobalExternalStatusCommand = (yyvsp[(2) - (2)].string); }
    break;

  case 37:
#line 296 "sg.y"
    { ufdbSetting("logdir",(yyvsp[(2) - (2)].string)); }
    break;

  case 38:
#line 297 "sg.y"
    { ufdbSetting("logdir",(yyvsp[(2) - (2)].string)); }
    break;

  case 39:
#line 301 "sg.y"
    { ufdbSetting( "port", (yyvsp[(2) - (2)].string) ); }
    break;

  case 40:
#line 305 "sg.y"
    { 
#if HAVE_UNIX_SOCKETS
                                             ufdbLogError( "ufdbguardd is configured to use UNIX sockets.  \"interface\" is ignored." );
#else
		                             if (strcmp((yyvsp[(2) - (2)].string),"all")== 0)
						strcpy( UFDBglobalInterface, "all" );    
					     else
					        ufdbLogFatalError( "interface must be \"all\" or IP address" );
#endif
					     ufdbFree( (yyvsp[(2) - (2)].string) );
					   }
    break;

  case 41:
#line 316 "sg.y"
    { strcpy( UFDBglobalInterface, (yyvsp[(2) - (2)].string) );  ufdbFree( (yyvsp[(2) - (2)].string) ); }
    break;

  case 42:
#line 320 "sg.y"
    { ufdbSetCPU( (yyvsp[(2) - (2)].string) ); ufdbFree( (yyvsp[(2) - (2)].string) ); }
    break;

  case 43:
#line 321 "sg.y"
    { ufdbSetCPU( (yyvsp[(2) - (2)].string) ); ufdbFree( (yyvsp[(2) - (2)].string) ); }
    break;

  case 44:
#line 326 "sg.y"
    { UFDBglobalUploadStats = (yyvsp[(2) - (2)].integer);   }
    break;

  case 45:
#line 331 "sg.y"
    { strcpy( UFDBglobalLoadingDatabaseRedirect, (yyvsp[(2) - (2)].string) );  ufdbFree( (yyvsp[(2) - (2)].string) );  }
    break;

  case 46:
#line 336 "sg.y"
    { strcpy( UFDBglobalFatalErrorRedirect, (yyvsp[(2) - (2)].string) );  ufdbFree( (yyvsp[(2) - (2)].string) );  }
    break;

  case 47:
#line 341 "sg.y"
    { 
		        if (UFDBglobalAnalyseUncategorisedURLs < 0)
			   ufdbLogMessage( "option analyse-uncategorised-urls is overruled by -N option *****" );
		        else
		           UFDBglobalAnalyseUncategorisedURLs = (yyvsp[(2) - (2)].integer);   
		     }
    break;

  case 48:
#line 349 "sg.y"
    {
		        if (UFDBglobalAnalyseUncategorisedURLs < 0)
			   ufdbLogMessage( "option analyse-uncategorised-urls is overruled by -N option *****" );
		        else
			{
		           UFDBglobalAnalyseUncategorisedURLs = atoi( (yyvsp[(2) - (2)].string) ); 
			   ufdbFree( (yyvsp[(2) - (2)].string) ); 
			}
		     }
    break;

  case 49:
#line 362 "sg.y"
    { UFDBglobalSafeSearch = (yyvsp[(2) - (2)].integer); }
    break;

  case 50:
#line 367 "sg.y"
    { 
		       UFDBglobalMaxLogfileSize = strtoul( (yyvsp[(2) - (2)].string), NULL, 10 );
		       ufdbFree( (yyvsp[(2) - (2)].string) );
		       if (UFDBglobalMaxLogfileSize < 2 * 1024 * 1024)		/* minimum is 2 MB */
		          UFDBglobalMaxLogfileSize = 2 * 1024 * 1024;
		       if (UFDBglobalMaxLogfileSize > 2000000000)		/* maximum is 2 GB */
		          UFDBglobalMaxLogfileSize = 2000000000;
		     }
    break;

  case 51:
#line 378 "sg.y"
    { UFDBglobalHttpdPort = atoi( (yyvsp[(3) - (3)].string) ); ufdbFree( (yyvsp[(3) - (3)].string) ); }
    break;

  case 52:
#line 379 "sg.y"
    { if (strcmp((yyvsp[(3) - (3)].string),"all")== 0)
						strcpy( UFDBglobalHttpdInterface, "all" );    
					     else
					        ufdbLogFatalError( "http-server interface must be \"all\" or IP address" );
					     ufdbFree( (yyvsp[(3) - (3)].string) );
					   }
    break;

  case 53:
#line 385 "sg.y"
    { if (strcmp((yyvsp[(3) - (3)].string),"all")== 0)
						strcpy( UFDBglobalHttpdInterface, "all" );    
					     else
					        ufdbLogFatalError( "http-server interface must be \"all\" or IP address" );
					     ufdbFree( (yyvsp[(3) - (3)].string) );
					   }
    break;

  case 54:
#line 391 "sg.y"
    { strcpy( UFDBglobalHttpdInterface, (yyvsp[(3) - (3)].string) );       ufdbFree( (yyvsp[(3) - (3)].string) ); }
    break;

  case 55:
#line 392 "sg.y"
    { strcpy( UFDBglobalHttpdImagesDirectory, (yyvsp[(3) - (3)].string) ); ufdbFree( (yyvsp[(3) - (3)].string) ); }
    break;

  case 56:
#line 393 "sg.y"
    { strcpy( UFDBglobalHttpdImagesDirectory, (yyvsp[(3) - (3)].string) ); ufdbFree( (yyvsp[(3) - (3)].string) ); }
    break;

  case 60:
#line 406 "sg.y"
    { ufdbCategory((yyvsp[(2) - (2)].string)); }
    break;

  case 61:
#line 411 "sg.y"
    { ufdbCategoryEnd(); }
    break;

  case 64:
#line 419 "sg.y"
    { ufdbCategoryDomainList( (yyvsp[(2) - (2)].string) ); }
    break;

  case 65:
#line 420 "sg.y"
    { ufdbCategoryDomainList( (yyvsp[(2) - (2)].string) ); }
    break;

  case 66:
#line 421 "sg.y"
    { ufdbCategoryDomainList( NULL ); }
    break;

  case 67:
#line 422 "sg.y"
    { ufdbCategoryUrlList( (yyvsp[(2) - (2)].string) ); }
    break;

  case 68:
#line 423 "sg.y"
    { ufdbCategoryUrlList( (yyvsp[(2) - (2)].string) ); }
    break;

  case 69:
#line 424 "sg.y"
    { ufdbCategoryUrlList( NULL ); }
    break;

  case 70:
#line 425 "sg.y"
    { ufdbCategoryExpressionList( (yyvsp[(2) - (2)].string), "n" ); }
    break;

  case 71:
#line 426 "sg.y"
    { ufdbCategoryExpressionList( (yyvsp[(2) - (2)].string), "n" ); }
    break;

  case 72:
#line 427 "sg.y"
    { ufdbCategoryExpressionList( NULL, NULL ); }
    break;

  case 73:
#line 428 "sg.y"
    { ufdbCategoryExpressionList( (yyvsp[(3) - (3)].string), "i" ); }
    break;

  case 74:
#line 429 "sg.y"
    { ufdbCategoryExpressionList( (yyvsp[(3) - (3)].string), "i" ); }
    break;

  case 75:
#line 430 "sg.y"
    { ufdbCategoryExpressionList( (yyvsp[(3) - (3)].string), "i" ); }
    break;

  case 76:
#line 431 "sg.y"
    { ufdbCategoryExpressionList( (yyvsp[(3) - (3)].string), "i" ); }
    break;

  case 77:
#line 432 "sg.y"
    { ufdbCategoryCACerts( (yyvsp[(2) - (2)].string) ); }
    break;

  case 78:
#line 433 "sg.y"
    { ufdbCategoryCACerts( (yyvsp[(2) - (2)].string) ); }
    break;

  case 79:
#line 434 "sg.y"
    { ufdbCategoryRedirect( (yyvsp[(2) - (2)].string) ); }
    break;

  case 80:
#line 435 "sg.y"
    { ufdbCategoryRedirect( (yyvsp[(2) - (2)].string) ); }
    break;

  case 81:
#line 436 "sg.y"
    { ufdbCategoryRewrite( (yyvsp[(2) - (2)].string) ); }
    break;

  case 82:
#line 437 "sg.y"
    { ufdbCategoryRewrite( (yyvsp[(2) - (2)].string) ); }
    break;

  case 83:
#line 438 "sg.y"
    { ufdbCategoryTime( (yyvsp[(2) - (2)].string), UFDB_ACL_WITHIN ); }
    break;

  case 84:
#line 439 "sg.y"
    { ufdbCategoryTime( (yyvsp[(2) - (2)].string), UFDB_ACL_OUTSIDE ); }
    break;

  case 85:
#line 440 "sg.y"
    { ufdbCategoryOption( (yyvsp[(3) - (3)].integer), UFDB_OPT_SAFE_SEARCH );  }
    break;

  case 86:
#line 441 "sg.y"
    { ufdbCategoryOption(  1, UFDB_OPT_HTTPS_WITH_HOSTNAME );  }
    break;

  case 87:
#line 442 "sg.y"
    { ufdbCategoryOption( (yyvsp[(3) - (3)].integer), UFDB_OPT_HTTPS_WITH_HOSTNAME );  }
    break;

  case 88:
#line 443 "sg.y"
    { ufdbCategoryOption(  1, UFDB_OPT_HTTPS_OFFICAL_CERTIFICATE );  }
    break;

  case 89:
#line 444 "sg.y"
    { ufdbCategoryOption( (yyvsp[(3) - (3)].integer), UFDB_OPT_HTTPS_OFFICAL_CERTIFICATE );  }
    break;

  case 90:
#line 445 "sg.y"
    { ufdbCategoryOption( (yyvsp[(3) - (3)].integer), UFDB_OPT_PROHIBIT_INSECURE_SSLV2 );  }
    break;

  case 91:
#line 446 "sg.y"
    { ufdbCategoryOption( (yyvsp[(3) - (3)].integer), UFDB_OPT_SKYPE_OVER_HTTPS );  }
    break;

  case 92:
#line 447 "sg.y"
    { ufdbCategoryOption( (yyvsp[(3) - (3)].integer), UFDB_OPT_GTALK_OVER_HTTPS );  }
    break;

  case 93:
#line 448 "sg.y"
    { ufdbCategoryOption( (yyvsp[(3) - (3)].integer), UFDB_OPT_YAHOOMSG_OVER_HTTPS );  }
    break;

  case 94:
#line 449 "sg.y"
    { ufdbCategoryOption( (yyvsp[(3) - (3)].integer), UFDB_OPT_AIM_OVER_HTTPS );  }
    break;

  case 95:
#line 450 "sg.y"
    { ufdbCategoryOption( (yyvsp[(3) - (3)].integer), UFDB_OPT_UNKNOWN_PROTOCOL_OVER_HTTPS );  }
    break;

  case 96:
#line 451 "sg.y"
    { ufdbLogError( "line %d: unsupported logfile context for %s", lineno, (yyvsp[(3) - (3)].string) ); 
		                                  ufdbFree( (yyvsp[(3) - (3)].string) ); }
    break;

  case 97:
#line 453 "sg.y"
    { ufdbLogError( "line %d: unsupported logfile context for %s", lineno, (yyvsp[(3) - (3)].string) ); 
		                                  ufdbFree( (yyvsp[(3) - (3)].string) ); }
    break;

  case 98:
#line 455 "sg.y"
    { ufdbLogError( "line %d: unsupported logfile context for %s", lineno, (yyvsp[(2) - (2)].string) ); 
		                                  ufdbFree( (yyvsp[(2) - (2)].string) ); }
    break;

  case 99:
#line 457 "sg.y"
    { ufdbLogError( "line %d: unsupported logfile context for %s", lineno, (yyvsp[(2) - (2)].string) ); 
		                                  ufdbFree( (yyvsp[(2) - (2)].string) ); }
    break;

  case 100:
#line 462 "sg.y"
    { sgSource( (yyvsp[(2) - (2)].string) ); }
    break;

  case 101:
#line 466 "sg.y"
    { sgSourceEnd(); }
    break;

  case 107:
#line 478 "sg.y"
    { ufdbSourceUserList( (yyvsp[(2) - (2)].string) ); }
    break;

  case 108:
#line 479 "sg.y"
    { ufdbSourceUserList( (yyvsp[(2) - (2)].string) ); }
    break;

  case 109:
#line 480 "sg.y"
    { ufdbSourceUserList( (yyvsp[(3) - (3)].string) ); }
    break;

  case 110:
#line 481 "sg.y"
    { ufdbSourceUserList( (yyvsp[(3) - (3)].string) ); }
    break;

  case 111:
#line 482 "sg.y"
    { ufdbSourceGroup( UFDB_GROUPTYPE_UNIX, (yyvsp[(3) - (3)].string) ); }
    break;

  case 112:
#line 483 "sg.y"
    { ufdbSourceGroup( UFDB_GROUPTYPE_UNIX, (yyvsp[(3) - (3)].string) ); }
    break;

  case 113:
#line 484 "sg.y"
    { ufdbSourceUserQuota( (yyvsp[(2) - (4)].string), (yyvsp[(3) - (4)].string), "3600" );  
		                                   ufdbFree( (yyvsp[(2) - (4)].string) ); ufdbFree( (yyvsp[(3) - (4)].string) ); }
    break;

  case 114:
#line 486 "sg.y"
    { ufdbSourceUserQuota( (yyvsp[(2) - (4)].string), (yyvsp[(3) - (4)].string), "86400" );  
		                                   ufdbFree( (yyvsp[(2) - (4)].string) ); ufdbFree( (yyvsp[(3) - (4)].string) ); }
    break;

  case 115:
#line 488 "sg.y"
    { ufdbSourceUserQuota( (yyvsp[(2) - (4)].string), (yyvsp[(3) - (4)].string), "604800" );  
		                                   ufdbFree( (yyvsp[(2) - (4)].string) ); ufdbFree( (yyvsp[(3) - (4)].string) ); }
    break;

  case 116:
#line 490 "sg.y"
    { ufdbSourceUserQuota( (yyvsp[(2) - (4)].string), (yyvsp[(3) - (4)].string), (yyvsp[(4) - (4)].string) );  
		                                   ufdbFree( (yyvsp[(2) - (4)].string) ); ufdbFree( (yyvsp[(3) - (4)].string) ); ufdbFree( (yyvsp[(4) - (4)].string) ); }
    break;

  case 118:
#line 493 "sg.y"
    { sgSourceIpList( (yyvsp[(2) - (2)].string) ); }
    break;

  case 119:
#line 494 "sg.y"
    { sgSourceIpList( (yyvsp[(2) - (2)].string) ); }
    break;

  case 120:
#line 495 "sg.y"
    { sgSourceTime( (yyvsp[(2) - (2)].string), UFDB_ACL_WITHIN ); }
    break;

  case 121:
#line 496 "sg.y"
    { sgSourceTime( (yyvsp[(2) - (2)].string), UFDB_ACL_OUTSIDE ); }
    break;

  case 122:
#line 497 "sg.y"
    { ufdbLogError( "line %d: unsupported logfile context for %s", lineno, (yyvsp[(3) - (3)].string) ); 
						   ufdbFree( (yyvsp[(3) - (3)].string) ); }
    break;

  case 123:
#line 499 "sg.y"
    { ufdbLogError( "line %d: unsupported logfile context for %s", lineno, (yyvsp[(3) - (3)].string) ); 
						   ufdbFree( (yyvsp[(3) - (3)].string) ); }
    break;

  case 124:
#line 501 "sg.y"
    { ufdbLogError( "line %d: unsupported logfile context for %s", lineno, (yyvsp[(2) - (2)].string) ); 
						   ufdbFree( (yyvsp[(2) - (2)].string) ); }
    break;

  case 125:
#line 503 "sg.y"
    { ufdbLogError( "line %d: unsupported logfile context for %s", lineno, (yyvsp[(2) - (2)].string) ); 
						   ufdbFree( (yyvsp[(2) - (2)].string) ); }
    break;

  case 126:
#line 505 "sg.y"
    { lastSource->cont_search = 1; }
    break;

  case 128:
#line 511 "sg.y"
    { sgSourceDomain( (yyvsp[(2) - (2)].string) ); }
    break;

  case 129:
#line 512 "sg.y"
    { sgSourceDomain( (yyvsp[(2) - (2)].string) ); }
    break;

  case 132:
#line 518 "sg.y"
    { ufdbSourceUser( (yyvsp[(2) - (2)].string) ); }
    break;

  case 133:
#line 519 "sg.y"
    { ufdbSourceUser( (yyvsp[(2) - (2)].string) ); }
    break;

  case 138:
#line 532 "sg.y"
    { sgAcl( (yyvsp[(1) - (2)].string), NULL, UFDB_ACL_NONE );  }
    break;

  case 139:
#line 533 "sg.y"
    { sgAcl( (yyvsp[(1) - (4)].string), (yyvsp[(3) - (4)].string), UFDB_ACL_WITHIN );  }
    break;

  case 140:
#line 534 "sg.y"
    { sgAcl( (yyvsp[(1) - (4)].string), (yyvsp[(3) - (4)].string), UFDB_ACL_OUTSIDE ); }
    break;

  case 142:
#line 540 "sg.y"
    { sgAcl( NULL, NULL, UFDB_ACL_ELSE );    }
    break;

  case 146:
#line 550 "sg.y"
    { }
    break;

  case 147:
#line 551 "sg.y"
    { sgAclSetValue( "rewrite", (yyvsp[(2) - (2)].string), 0 ); }
    break;

  case 148:
#line 552 "sg.y"
    { sgAclSetValue( "rewrite", (yyvsp[(2) - (2)].string), 0 ); }
    break;

  case 149:
#line 553 "sg.y"
    { sgAclSetValue( "redirect", (yyvsp[(2) - (2)].string), 0 ); }
    break;

  case 150:
#line 554 "sg.y"
    { sgAclSetValue( "redirect", (yyvsp[(2) - (2)].string), 0 ); }
    break;

  case 151:
#line 555 "sg.y"
    { ufdbLogError( "line %d: unsupported logfile context for %s", lineno, (yyvsp[(3) - (3)].string) ); 
						  ufdbFree( (yyvsp[(3) - (3)].string) ); }
    break;

  case 152:
#line 557 "sg.y"
    { ufdbLogError( "line %d: unsupported logfile context for %s", lineno, (yyvsp[(3) - (3)].string) ); 
						  ufdbFree( (yyvsp[(3) - (3)].string) ); }
    break;

  case 153:
#line 559 "sg.y"
    { ufdbLogError( "line %d: unsupported logfile context for %s", lineno, (yyvsp[(2) - (2)].string) ); 
						  ufdbFree( (yyvsp[(2) - (2)].string) ); }
    break;

  case 154:
#line 561 "sg.y"
    { ufdbLogError( "line %d: unsupported logfile context for %s", lineno, (yyvsp[(2) - (2)].string) ); 
						  ufdbFree( (yyvsp[(2) - (2)].string) ); }
    break;

  case 156:
#line 567 "sg.y"
    { sgAclSetValue( "pass", (yyvsp[(2) - (2)].string), 1 ); }
    break;

  case 157:
#line 568 "sg.y"
    { sgAclSetValue( "pass", (yyvsp[(3) - (3)].string), 0 ); }
    break;

  case 159:
#line 573 "sg.y"
    { sgIp( (yyvsp[(1) - (1)].string) ); ufdbFree( (yyvsp[(1) - (1)].string) ); }
    break;

  case 160:
#line 577 "sg.y"
    { sgIp( (yyvsp[(1) - (1)].string) ); ufdbFree( (yyvsp[(1) - (1)].string) ); }
    break;

  case 162:
#line 582 "sg.y"
    { sgIp( "255.255.255.255" );  sgSetIpType( SG_IPTYPE_HOST, NULL, 0 ); }
    break;

  case 163:
#line 583 "sg.y"
    { sgSetIpType( SG_IPTYPE_CIDR, NULL, 0 ); }
    break;

  case 164:
#line 584 "sg.y"
    { sgSetIpType( SG_IPTYPE_CLASS, NULL, 0 ); }
    break;

  case 165:
#line 585 "sg.y"
    { sgSetIpType( SG_IPTYPE_RANGE, NULL, 0 ); }
    break;

  case 167:
#line 590 "sg.y"
    { sgIp( (yyvsp[(1) - (1)].string) );  ufdbFree( (yyvsp[(1) - (1)].string) );  }
    break;

  case 168:
#line 594 "sg.y"
    { sgRewrite( (yyvsp[(2) - (2)].string) ); }
    break;

  case 169:
#line 595 "sg.y"
    { sgRewrite( (yyvsp[(2) - (2)].string) ); }
    break;

  case 173:
#line 609 "sg.y"
    { sgRewriteSubstitute( (yyvsp[(1) - (1)].string) ); ufdbFree( (yyvsp[(1) - (1)].string) ); }
    break;

  case 174:
#line 610 "sg.y"
    { sgRewriteTime( (yyvsp[(2) - (2)].string), UFDB_ACL_WITHIN ); }
    break;

  case 175:
#line 611 "sg.y"
    { sgRewriteTime( (yyvsp[(2) - (2)].string), UFDB_ACL_OUTSIDE ); }
    break;

  case 176:
#line 612 "sg.y"
    { ufdbLogError( "line %d: unsupported logfile context for %s", lineno, (yyvsp[(3) - (3)].string) ); 							      ufdbFree( (yyvsp[(3) - (3)].string) ); }
    break;

  case 177:
#line 613 "sg.y"
    { ufdbLogError( "line %d: unsupported logfile context for %s", lineno, (yyvsp[(3) - (3)].string) ); 							      ufdbFree( (yyvsp[(3) - (3)].string) ); }
    break;

  case 178:
#line 614 "sg.y"
    { ufdbLogError( "line %d: unsupported logfile context for %s", lineno, (yyvsp[(2) - (2)].string) ); 							      ufdbFree( (yyvsp[(2) - (2)].string) ); }
    break;

  case 179:
#line 615 "sg.y"
    { ufdbLogError( "line %d: unsupported logfile context for %s", lineno, (yyvsp[(2) - (2)].string) ); 							      ufdbFree( (yyvsp[(2) - (2)].string) ); }
    break;

  case 180:
#line 620 "sg.y"
    { sgTime( (yyvsp[(2) - (2)].string) ); }
    break;

  case 184:
#line 634 "sg.y"
    { sgTimeElementInit(); }
    break;

  case 185:
#line 634 "sg.y"
    { sgTimeElementAdd((yyvsp[(3) - (3)].string),T_WEEKDAY); }
    break;

  case 187:
#line 635 "sg.y"
    { sgTimeElementInit(); }
    break;

  case 188:
#line 635 "sg.y"
    { sgTimeElementAdd((yyvsp[(3) - (3)].string),T_WEEKLY); }
    break;

  case 190:
#line 636 "sg.y"
    { sgTimeElementInit(); }
    break;

  case 191:
#line 636 "sg.y"
    { sgTimeElementAdd((yyvsp[(3) - (3)].string),T_WEEKLY); }
    break;

  case 193:
#line 637 "sg.y"
    { sgTimeElementInit(); }
    break;

  case 194:
#line 637 "sg.y"
    { sgTimeElementEnd(); }
    break;

  case 195:
#line 638 "sg.y"
    { ufdbLogFatalError( "invalid time specification at line %d", lineno );   }
    break;

  case 196:
#line 642 "sg.y"
    { sgTimeElementClone(); }
    break;

  case 205:
#line 655 "sg.y"
    { sgTimeElementAdd( (yyvsp[(1) - (1)].string), T_DVAL ); }
    break;

  case 206:
#line 658 "sg.y"
    { sgTimeElementAdd( (yyvsp[(1) - (1)].tval), T_TVAL ); }
    break;

  case 207:
#line 661 "sg.y"
    { sgTimeElementAdd( (yyvsp[(1) - (1)].string), T_DVALCRON ); }
    break;

  case 208:
#line 665 "sg.y"
    {  yyerror( "syntax error" );  }
    break;


/* Line 1267 of yacc.c.  */
#line 2918 "y.tab.c"
      default: break;
    }
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;


  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
#else
      {
	YYSIZE_T yysize = yysyntax_error (0, yystate, yychar);
	if (yymsg_alloc < yysize && yymsg_alloc < YYSTACK_ALLOC_MAXIMUM)
	  {
	    YYSIZE_T yyalloc = 2 * yysize;
	    if (! (yysize <= yyalloc && yyalloc <= YYSTACK_ALLOC_MAXIMUM))
	      yyalloc = YYSTACK_ALLOC_MAXIMUM;
	    if (yymsg != yymsgbuf)
	      YYSTACK_FREE (yymsg);
	    yymsg = (char *) YYSTACK_ALLOC (yyalloc);
	    if (yymsg)
	      yymsg_alloc = yyalloc;
	    else
	      {
		yymsg = yymsgbuf;
		yymsg_alloc = sizeof yymsgbuf;
	      }
	  }

	if (0 < yysize && yysize <= yymsg_alloc)
	  {
	    (void) yysyntax_error (yymsg, yystate, yychar);
	    yyerror (yymsg);
	  }
	else
	  {
	    yyerror (YY_("syntax error"));
	    if (yysize != 0)
	      goto yyexhaustedlab;
	  }
      }
#endif
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse look-ahead token after an
	 error, discard it.  */

      if (yychar <= YYEOF)
	{
	  /* Return failure if at end of input.  */
	  if (yychar == YYEOF)
	    YYABORT;
	}
      else
	{
	  yydestruct ("Error: discarding",
		      yytoken, &yylval);
	  yychar = YYEMPTY;
	}
    }

  /* Else will try to reuse look-ahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

  /* Do not reclaim the symbols of the rule which action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;	/* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (yyn != YYPACT_NINF)
	{
	  yyn += YYTERROR;
	  if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
	    {
	      yyn = yytable[yyn];
	      if (0 < yyn)
		break;
	    }
	}

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
	YYABORT;


      yydestruct ("Error: popping",
		  yystos[yystate], yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  *++yyvsp = yylval;


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#ifndef yyoverflow
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEOF && yychar != YYEMPTY)
     yydestruct ("Cleanup: discarding lookahead",
		 yytoken, &yylval);
  /* Do not reclaim the symbols of the rule which action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
		  yystos[*yyssp], yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  /* Make sure YYID is used.  */
  return YYID (yyresult);
}


#line 717 "sg.y"


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
  if (s->ip == NULL  &&  s->domainDb == NULL  &&  s->userDb == NULL)
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
  int                foundip, founduser, founddomain, unblockeduser;
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

    foundip = founduser = founddomain = 0;
    unblockeduser = 1;

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

    if (founduser && foundip && founddomain)
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


