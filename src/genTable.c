/*
 * genTable.c - URLfilterDB
 *
 * ufdbGuard is copyrighted (C) 2005-2010 by URLfilterDB with all rights reserved.
 *
 * Parts of the ufdbGuard daemon are based on squidGuard.
 * This module is NOT based on squidGuard.
 *
 * Generate a binary table file (.ufdb) from unordered ASCII files 
 * with domains and urls.
 *
 * usage: ufdbGenTable [-V] [-n] [-C] [-k <key>] -t <tableName> -d <domains> [-u <urls>]
 *
 * RCS $Id: genTable.c,v 1.57 2011/06/03 01:56:38 root Exp root $
 */

#if defined(__OPTIMIZE__) && defined(__GNUC__)  && defined(GCC_INLINE_STRING_FUNCTIONS_ARE_FASTER)
#define __USE_STRING_INLINES
#endif

#include "ufdb.h"
#include "ufdblib.h"

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <time.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>

#include "bzlib.h"


static FILE * fin;
static char * inFileName;
static char * urlsFileName;
static char * tableName;

static struct UFDBtable * table;

#if 0
int    UFDBglobalDebug = 0;
#endif

static int numEntries = 0;
static int numNodes = 0;
static int numLeafNodes = 0;
static int numIndexNodes = 0;
static int doCrypt = 1;
static int doCompress = 0;
static int doProd = 0;
static int doWarnings = 1;
static int doSanityCheck = 1;
static int stripWWW = 0;
static char * format = UFDBdbVersion;

#if UFDB_DO_DEBUG || 0
#define DEBUG(x) fprintf x 
#else
#define DEBUG(x) 
#endif

#define ROUNDUPBY      8
#define ROUNDUP(i)     ( (i) + (ROUNDUPBY - ((i)%ROUNDUPBY) ) )

#define BIGROUNDUPBY   64
#define BIGROUNDUP(i)  ( (i) + (BIGROUNDUPBY - ((i)%BIGROUNDUPBY) ) )

#define ROUNDUPBYCUTOFF BIGROUNDUPBY


#if 0
pthread_mutex_t ufdb_malloc_mutex = UFDB_STATIC_MUTEX_INIT;
#endif



static void usage( void )
{
   fprintf( stderr, "usage: ufdbGenTable [-V] [-n] [-q] [-C] [-k <key>] -t <tableName> -d <domains> [-u <urls>]\n" );
   fprintf( stderr, "flags: -n  no encryption\n" );
   fprintf( stderr, "       -k  16-char encryption key\n" );
   fprintf( stderr, "       -F  1.2|2.0 (default is %s)\n", UFDBdbVersion );
   fprintf( stderr, "       -D  debug\n" );
   fprintf( stderr, "       -q  be quiet (suppress warnings)\n" );
   fprintf( stderr, "       -C  use bz2 compression\n" );
   fprintf( stderr, "       -s  sanity check for domain names (obsolete) (check is always done)\n" );
   fprintf( stderr, "       -W  strip www. from URLs (recommended)\n" );
   fprintf( stderr, "       -V  print version and exit\n" );
   fprintf( stderr, "       -t  tablename\n" );
   fprintf( stderr, "       -d  domains\n" );
   fprintf( stderr, "       -u  urls\n" );
   exit( 1 );
}


void ufdbLogError( char * format, ... )
{
   va_list ap;
   char    msg[UFDB_MAX_URL_LENGTH];

   va_start( ap, format );
   if (vsnprintf(msg, UFDB_MAX_URL_LENGTH-1, format, ap) > (UFDB_MAX_URL_LENGTH - 2))
      msg[UFDB_MAX_URL_LENGTH-1] = '\0';
   va_end( ap );

   fprintf( stderr, "%s\n", msg );
}


void ufdbLogMessage( char * format, ... )
{
   va_list ap;
   char    msg[UFDB_MAX_URL_LENGTH];

   va_start( ap, format );
   if (vsnprintf(msg, UFDB_MAX_URL_LENGTH-1, format, ap) > (UFDB_MAX_URL_LENGTH - 2))
      msg[UFDB_MAX_URL_LENGTH-1] = '\0';
   va_end( ap );

   fprintf( stderr, "%s\n", msg );
}


void ufdbLogFatalError( char * format, ... )
{
   va_list ap;
   char    msg[UFDB_MAX_URL_LENGTH];

   va_start( ap, format );
   if (vsnprintf(msg, UFDB_MAX_URL_LENGTH-1, format, ap) > (UFDB_MAX_URL_LENGTH - 2))
      msg[UFDB_MAX_URL_LENGTH-1] = '\0';
   va_end( ap );

   fprintf( stderr, "%s *****\n", msg );
}


void ufdbSetGlobalErrorLogFile( void )
{
}


#if 1
#define _STRDUP(s) ufdbStrdup(s)
#else
/* 
 *  Small speed optimisation: allocate memory for strdupped strings in large blocks since they are never freed.
 */
static char * _STRDUP( 
   char * s )
{
   static char * freeMem = NULL;
   static char * last = NULL;

   char * p;
   int    size;

   for (p = s; *p != '\0'; p++)
      ;
   size = ((int) (p - s)) + 1;

   if ((int) (last - freeMem) < size)
   {
      freeMem = malloc( 256 * 1024 );
      last = freeMem + 256 * 1024 - 1;
   }

   p = strcpy( freeMem, s );
   freeMem += size;

   return p;
}
#endif


static int UFDBsanityCheckDomainname( char * url )
{
   char * first_slash;
   char * tld;
   int    retval;
   char * oldBracket;

#if 0
   fprintf( stderr, "UFDBsanityCheckDomainname: %s\n", url );
#endif

   if (*url == '[')			/* IPv6 address */
   {
      char normalisedDomain[64];

      oldBracket = strchr( url, ']' );
      if (oldBracket == NULL)
      {
         fprintf( stderr, "error: IPv6 address has no closing ']': %s\n", url );
	 return 0;
      }
      *oldBracket = '\0';
      if (UFDBparseIPv6address( url+1, normalisedDomain ) == NULL)
      {
	 *oldBracket = ']';
	 return 0;
      }
      else
      {
	 *oldBracket = ']';
	 UFDBupdateURLwithNormalisedDomain( url, normalisedDomain );
         return 1;
      }
   }

   first_slash = strchr( url, '/' );
   if (first_slash != NULL)
      *first_slash = '\0';

   tld = strrchr( url, '.' );
   if (tld == NULL)
      tld = url;
   else
      tld++;

   retval = 1;
   if (*tld >= '0'  &&  *tld <= '9')
      retval = 1;
   else
   if (strcmp( tld, "com" ) != 0  &&
       strcmp( tld, "wpad" ) != 0  &&
       strcmp( tld, "net" ) != 0  &&
       strcmp( tld, "us" ) != 0  &&
       strcmp( tld, "uk" ) != 0  &&
       strcmp( tld, "au" ) != 0  &&
       strcmp( tld, "ca" ) != 0  &&
       strcmp( tld, "hk" ) != 0  &&
       strcmp( tld, "ie" ) != 0  &&
       strcmp( tld, "mc" ) != 0  &&
       strcmp( tld, "mt" ) != 0  &&
       strcmp( tld, "nz" ) != 0  &&
       strcmp( tld, "sg" ) != 0  &&
       strcmp( tld, "vi" ) != 0  &&
       strcmp( tld, "ph" ) != 0  &&
       strcmp( tld, "pg" ) != 0  &&
       strcmp( tld, "nl" ) != 0  &&
       strcmp( tld, "be" ) != 0  &&
       strcmp( tld, "aw" ) != 0  &&
       strcmp( tld, "sr" ) != 0  &&
       strcmp( tld, "za" ) != 0  &&
       strcmp( tld, "es" ) != 0  &&
       strcmp( tld, "ar" ) != 0  &&
       strcmp( tld, "bo" ) != 0  &&
       strcmp( tld, "bs" ) != 0  &&
       strcmp( tld, "cr" ) != 0  &&
       strcmp( tld, "cu" ) != 0  &&
       strcmp( tld, "do" ) != 0  &&
       strcmp( tld, "gi" ) != 0  &&
       strcmp( tld, "gt" ) != 0  &&
       strcmp( tld, "mx" ) != 0  &&
       strcmp( tld, "ni" ) != 0  &&
       strcmp( tld, "ve" ) != 0  &&
       strcmp( tld, "uy" ) != 0  &&
       strcmp( tld, "sv" ) != 0  &&
       strcmp( tld, "pa" ) != 0  &&
       strcmp( tld, "pe" ) != 0  &&
       strcmp( tld, "py" ) != 0  &&
       strcmp( tld, "pr" ) != 0  &&
       strcmp( tld, "pt" ) != 0  &&
       strcmp( tld, "br" ) != 0  &&
       strcmp( tld, "mz" ) != 0  &&
       strcmp( tld, "vn" ) != 0  &&
       strcmp( tld, "vu" ) != 0  &&
       strcmp( tld, "fm" ) != 0  &&
       strcmp( tld, "to" ) != 0  &&
       strcmp( tld, "tv" ) != 0  &&
       strcmp( tld, "tk" ) != 0  &&
       strcmp( tld, "nu" ) != 0  &&
       strcmp( tld, "de" ) != 0  &&
       strcmp( tld, "at" ) != 0  &&
       strcmp( tld, "ch" ) != 0  &&
       strcmp( tld, "it" ) != 0  &&
       strcmp( tld, "dk" ) != 0  &&
       strcmp( tld, "fr" ) != 0  &&
       strcmp( tld, "st" ) != 0  &&
       strcmp( tld, "ws" ) != 0  &&
       strcmp( tld, "ru" ) != 0  &&
       strcmp( tld, "cn" ) != 0  &&
       strcmp( tld, "cc" ) != 0  &&
       strcmp( tld, "biz" ) != 0  &&
       strcmp( tld, "ae" ) != 0  &&
       strcmp( tld, "af" ) != 0  &&
       strcmp( tld, "xxx" ) != 0  &&
       strcmp( tld, "aero" ) != 0  &&
       strcmp( tld, "coop" ) != 0  &&
       strcmp( tld, "edu" ) != 0  &&
       strcmp( tld, "gov" ) != 0  &&
       strcmp( tld, "org" ) != 0  &&
       strcmp( tld, "info" ) != 0  &&
       strcmp( tld, "jobs" ) != 0  &&
       strcmp( tld, "museum" ) != 0  &&
       strcmp( tld, "name" ) != 0  &&
       strcmp( tld, "mil" ) != 0  &&
       strcmp( tld, "pro" ) != 0  &&
       strcmp( tld, "tel" ) != 0  &&
       strcmp( tld, "travel" ) != 0  &&
       strcmp( tld, "eu" ) != 0  &&
       strcmp( tld, "int" ) != 0  &&
       strcmp( tld, "info" ) != 0  &&
       strcmp( tld, "asia" ) != 0  &&
       strcmp( tld, "ag" ) != 0  &&
       strcmp( tld, "ai" ) != 0  &&
       strcmp( tld, "al" ) != 0  &&
       strcmp( tld, "am" ) != 0  &&
       strcmp( tld, "an" ) != 0  &&
       strcmp( tld, "ao" ) != 0  &&
       strcmp( tld, "aq" ) != 0  &&
       strcmp( tld, "arpa" ) != 0  &&
       strcmp( tld, "as" ) != 0  &&
       strcmp( tld, "aw" ) != 0  &&
       strcmp( tld, "ax" ) != 0  &&
       strcmp( tld, "az" ) != 0  &&
       strcmp( tld, "ac" ) != 0  &&
       strcmp( tld, "ad" ) != 0  &&
       strcmp( tld, "ba" ) != 0  &&
       strcmp( tld, "bb" ) != 0  &&
       strcmp( tld, "bd" ) != 0  &&
       strcmp( tld, "bf" ) != 0  &&
       strcmp( tld, "bg" ) != 0  &&
       strcmp( tld, "bh" ) != 0  &&
       strcmp( tld, "bi" ) != 0  &&
       strcmp( tld, "bj" ) != 0  &&
       strcmp( tld, "bm" ) != 0  &&
       strcmp( tld, "bn" ) != 0  &&
       strcmp( tld, "bt" ) != 0  &&
       strcmp( tld, "bv" ) != 0  &&
       strcmp( tld, "bw" ) != 0  &&
       strcmp( tld, "by" ) != 0  &&
       strcmp( tld, "bz" ) != 0  &&
       strcmp( tld, "tt" ) != 0  &&
       strcmp( tld, "is" ) != 0  &&
       strcmp( tld, "gg" ) != 0  &&
       strcmp( tld, "tp" ) != 0  &&
       strcmp( tld, "cat" ) != 0  &&
       strcmp( tld, "cd" ) != 0  &&
       strcmp( tld, "cf" ) != 0  &&
       strcmp( tld, "cg" ) != 0  &&
       strcmp( tld, "ci" ) != 0  &&
       strcmp( tld, "ck" ) != 0  &&
       strcmp( tld, "cl" ) != 0  &&
       strcmp( tld, "cm" ) != 0  &&
       strcmp( tld, "co" ) != 0  &&
       strcmp( tld, "cu" ) != 0  &&
       strcmp( tld, "cv" ) != 0  &&
       strcmp( tld, "cx" ) != 0  &&
       strcmp( tld, "cy" ) != 0  &&
       strcmp( tld, "cz" ) != 0  &&
       strcmp( tld, "dj" ) != 0  &&
       strcmp( tld, "dm" ) != 0  &&
       strcmp( tld, "do" ) != 0  &&
       strcmp( tld, "dz" ) != 0  &&
       strcmp( tld, "ec" ) != 0  &&
       strcmp( tld, "ee" ) != 0  &&
       strcmp( tld, "eg" ) != 0  &&
       strcmp( tld, "er" ) != 0  &&
       strcmp( tld, "es" ) != 0  &&
       strcmp( tld, "et" ) != 0  &&
       strcmp( tld, "fi" ) != 0  &&
       strcmp( tld, "fj" ) != 0  &&
       strcmp( tld, "fk" ) != 0  &&
       strcmp( tld, "fm" ) != 0  &&
       strcmp( tld, "fo" ) != 0  &&
       strcmp( tld, "fr" ) != 0  &&
       strcmp( tld, "ga" ) != 0  &&
       strcmp( tld, "gb" ) != 0  &&
       strcmp( tld, "gd" ) != 0  &&
       strcmp( tld, "ge" ) != 0  &&
       strcmp( tld, "gf" ) != 0  &&
       strcmp( tld, "gg" ) != 0  &&
       strcmp( tld, "gh" ) != 0  &&
       strcmp( tld, "gi" ) != 0  &&
       strcmp( tld, "gl" ) != 0  &&
       strcmp( tld, "gm" ) != 0  &&
       strcmp( tld, "gn" ) != 0  &&
       strcmp( tld, "gp" ) != 0  &&
       strcmp( tld, "gq" ) != 0  &&
       strcmp( tld, "gr" ) != 0  &&
       strcmp( tld, "gs" ) != 0  &&
       strcmp( tld, "gt" ) != 0  &&
       strcmp( tld, "gu" ) != 0  &&
       strcmp( tld, "gw" ) != 0  &&
       strcmp( tld, "gy" ) != 0  &&
       strcmp( tld, "hm" ) != 0  &&
       strcmp( tld, "hn" ) != 0  &&
       strcmp( tld, "hr" ) != 0  &&
       strcmp( tld, "ht" ) != 0  &&
       strcmp( tld, "hu" ) != 0  &&
       strcmp( tld, "id" ) != 0  &&
       strcmp( tld, "il" ) != 0  &&
       strcmp( tld, "im" ) != 0  &&
       strcmp( tld, "in" ) != 0  &&
       strcmp( tld, "io" ) != 0  &&
       strcmp( tld, "iq" ) != 0  &&
       strcmp( tld, "ir" ) != 0  &&
       strcmp( tld, "je" ) != 0  &&
       strcmp( tld, "jm" ) != 0  &&
       strcmp( tld, "jo" ) != 0  &&
       strcmp( tld, "jp" ) != 0  &&
       strcmp( tld, "ke" ) != 0  &&
       strcmp( tld, "kg" ) != 0  &&
       strcmp( tld, "kh" ) != 0  &&
       strcmp( tld, "ki" ) != 0  &&
       strcmp( tld, "km" ) != 0  &&
       strcmp( tld, "kn" ) != 0  &&
       strcmp( tld, "kr" ) != 0  &&
       strcmp( tld, "kw" ) != 0  &&
       strcmp( tld, "ky" ) != 0  &&
       strcmp( tld, "kz" ) != 0  &&
       strcmp( tld, "me" ) != 0  &&
       strcmp( tld, "mobi" ) != 0  &&
       strcmp( tld, "mobile" ) != 0  &&
       strcmp( tld, "la" ) != 0  &&
       strcmp( tld, "lb" ) != 0  &&
       strcmp( tld, "lc" ) != 0  &&
       strcmp( tld, "li" ) != 0  &&
       strcmp( tld, "lk" ) != 0  &&
       strcmp( tld, "lr" ) != 0  &&
       strcmp( tld, "ls" ) != 0  &&
       strcmp( tld, "lt" ) != 0  &&
       strcmp( tld, "lu" ) != 0  &&
       strcmp( tld, "lv" ) != 0  &&
       strcmp( tld, "ly" ) != 0  &&
       strcmp( tld, "ma" ) != 0  &&
       strcmp( tld, "md" ) != 0  &&
       strcmp( tld, "mg" ) != 0  &&
       strcmp( tld, "mh" ) != 0  &&
       strcmp( tld, "mk" ) != 0  &&
       strcmp( tld, "ml" ) != 0  &&
       strcmp( tld, "mm" ) != 0  &&
       strcmp( tld, "mn" ) != 0  &&
       strcmp( tld, "mo" ) != 0  &&
       strcmp( tld, "mp" ) != 0  &&
       strcmp( tld, "mr" ) != 0  &&
       strcmp( tld, "ms" ) != 0  &&
       strcmp( tld, "mt" ) != 0  &&
       strcmp( tld, "mu" ) != 0  &&
       strcmp( tld, "mv" ) != 0  &&
       strcmp( tld, "mw" ) != 0  &&
       strcmp( tld, "my" ) != 0  &&
       strcmp( tld, "mz" ) != 0  &&
       strcmp( tld, "na" ) != 0  &&
       strcmp( tld, "nc" ) != 0  &&
       strcmp( tld, "ne" ) != 0  &&
       strcmp( tld, "nf" ) != 0  &&
       strcmp( tld, "ng" ) != 0  &&
       strcmp( tld, "ni" ) != 0  &&
       strcmp( tld, "no" ) != 0  &&
       strcmp( tld, "np" ) != 0  &&
       strcmp( tld, "nr" ) != 0  &&
       strcmp( tld, "nu" ) != 0  &&
       strcmp( tld, "om" ) != 0  &&
       strcmp( tld, "pa" ) != 0  &&
       strcmp( tld, "pf" ) != 0  &&
       strcmp( tld, "pk" ) != 0  &&
       strcmp( tld, "pl" ) != 0  &&
       strcmp( tld, "pm" ) != 0  &&
       strcmp( tld, "pn" ) != 0  &&
       strcmp( tld, "ps" ) != 0  &&
       strcmp( tld, "pw" ) != 0  &&
       strcmp( tld, "py" ) != 0  &&
       strcmp( tld, "qa" ) != 0  &&
       strcmp( tld, "re" ) != 0  &&
       strcmp( tld, "ro" ) != 0  &&
       strcmp( tld, "rs" ) != 0  &&
       strcmp( tld, "rw" ) != 0  &&
       strcmp( tld, "sa" ) != 0  &&
       strcmp( tld, "sb" ) != 0  &&
       strcmp( tld, "sc" ) != 0  &&
       strcmp( tld, "sd" ) != 0  &&
       strcmp( tld, "se" ) != 0  &&
       strcmp( tld, "sh" ) != 0  &&
       strcmp( tld, "si" ) != 0  &&
       strcmp( tld, "sj" ) != 0  &&
       strcmp( tld, "sk" ) != 0  &&
       strcmp( tld, "sl" ) != 0  &&
       strcmp( tld, "sm" ) != 0  &&
       strcmp( tld, "sn" ) != 0  &&
       strcmp( tld, "so" ) != 0  &&
       strcmp( tld, "su" ) != 0  &&
       strcmp( tld, "sy" ) != 0  &&
       strcmp( tld, "sz" ) != 0  &&
       strcmp( tld, "tc" ) != 0  &&
       strcmp( tld, "td" ) != 0  &&
       strcmp( tld, "tf" ) != 0  &&
       strcmp( tld, "tg" ) != 0  &&
       strcmp( tld, "th" ) != 0  &&
       strcmp( tld, "tj" ) != 0  &&
       strcmp( tld, "tl" ) != 0  &&
       strcmp( tld, "tm" ) != 0  &&
       strcmp( tld, "tn" ) != 0  &&
       strcmp( tld, "tr" ) != 0  &&
       strcmp( tld, "tw" ) != 0  &&
       strcmp( tld, "tz" ) != 0  &&
       strcmp( tld, "ua" ) != 0  &&
       strcmp( tld, "ug" ) != 0  &&
       strcmp( tld, "um" ) != 0  &&
       strcmp( tld, "uz" ) != 0  &&
       strcmp( tld, "va" ) != 0  &&
       strcmp( tld, "vc" ) != 0  &&
       strcmp( tld, "vg" ) != 0  &&
       strcmp( tld, "wf" ) != 0  &&
       strcmp( tld, "ye" ) != 0  &&
       strcmp( tld, "yt" ) != 0  &&
       strcmp( tld, "yu" ) != 0  &&
       strcmp( tld, "zm" ) != 0  &&
       strcmp( tld, "zw" ) != 0)
      retval = 0;

   if (UFDBglobalDebug  &&  retval == 0)
      fprintf( stderr, "TLD '%s' not matched for '%s'\n", tld, url );

   if (first_slash != NULL)
      *first_slash = '/';

   return retval;
}


void initTable( char * tableName )
{
   table = (struct UFDBtable *) malloc( sizeof( struct UFDBtable ) + sizeof(struct UFDBtable*) );
   table->tag = (unsigned char *) _STRDUP( tableName );
   table->nNextLevels = 0;
   table->nextLevel = NULL;

   numIndexNodes = 0;
}


static __inline__ void * _trealloc( void * p, int n )
{
   int nup;

   if (n < ROUNDUPBYCUTOFF)
   {
      nup = ROUNDUP(n);
      if (nup == ROUNDUP(n-1))
         return p;
   }
   else
   {
      nup = BIGROUNDUP(n);
      if (nup == BIGROUNDUP(n-1))
         return p;
   }

   return realloc( p, nup * sizeof(struct UFDBtable) );
}

#include "strcmpurlpart.static.c"

int UFDBinsertURL( struct UFDBtable * t, UFDBrevURL * revURL, UFDBurlType type )
{
   /*
    * find the index where our URL has to be inserted before or is equal to
    * e.g. the level "net" is either "< nl" or "= net".
    */
   int b, e, i;
   int cmp;
   int rv = 0;

   DEBUG(( stderr, "      UFDBinsertURL( 0x%08x, 0x%08x )\n", t, revURL ));

   if (revURL == NULL)
   {
      if (t != NULL)
      {
         DEBUG(( stderr, "        revURL=NULL t: nNextLevels=%d, tag=%s\n", t->nNextLevels, t->tag ));
	 if (t->nNextLevels > 0)
	 {
	    /* interesting... we are trying to insert "xxx.com" while the tree already
	     * has one or more members with subdomains like yyy.xxx.com.
	     * Lets optimise this and get rid of the subdomains !
	     */
	    rv = 1;
	    t->nNextLevels = 0;
	    free( t->nextLevel );	/* TO-DO: should free() a tree ! */
	    t->nextLevel = NULL;
	 }
      }
      else
         DEBUG(( stderr, "        revURL=NULL t=NULL\n" ));
      return rv;
   }

   /* binary search */
   cmp = -999;
   i = b = 0;
   e = t->nNextLevels - 1;
   while (b <= e)
   {
      i = (b + e) / 2;
      cmp = strcmpURLpart( (char *) revURL->part, (char *) t->nextLevel[i].tag );
      /* DEBUG(( stderr, "       %d = strcmp( %s, %s )\n", cmp, revURL->part, t->nextLevel[i]->tag )); */
      if (cmp == 0)
         break;		/* found an exact match */
      if (cmp < 0)
         e = i - 1;
      else
         b = i + 1;
   }

   DEBUG(( stderr, "      UFDBinsertURL after bsearch: part=%s, cmp=%d, i=%d, b=%d, e=%d, nNextLevels=%d\n", 
           (revURL==NULL ? (unsigned char *)"NULL" : revURL->part), cmp, i, b, e, t->nNextLevels ));
   
   /* implemented optimisations: 
    * do not add subdom.abc.com/aurl if abc.com is already in the tree
    * do not add subdom.abc.com if abc.com is already in the tree
    * remove subdom.abc.com from tree if abc.com is being inserted
    */

   if (t->nNextLevels == 0)		/* the very first entry at this level */
   {
      t->nNextLevels = 1;
      t->nextLevel = malloc( ROUNDUP(1) * sizeof(struct UFDBtable) );
      t->nextLevel[0].tag = (unsigned char *) _STRDUP( (char *) revURL->part );
      t->nextLevel[0].nNextLevels = 0;
      t->nextLevel[0].nextLevel = NULL;

      rv = UFDBinsertURL( &(t->nextLevel[0]), revURL->next, type );
   }
   else if (cmp == 0)				/* an exact match at this level */
   {
      /* optimisation: do not add site.com/foo if site.com is in the table */
      if (type == UFDBurl)
      {
         if (t->nextLevel[i].nNextLevels != 0)
	    rv = UFDBinsertURL( &(t->nextLevel[i]), revURL->next, type );
      }
      else
      {
	 rv = UFDBinsertURL( &(t->nextLevel[i]), revURL->next, type );
      }
   }
   else if (cmp < 0)				/* this entry < nextLevel[i] */
   {
      t->nNextLevels++;
      t->nextLevel = _trealloc( t->nextLevel, t->nNextLevels );

      /* make space in the array */
      if (t->nNextLevels >= i)
      {
	 memmove( &(t->nextLevel[i+1]), &(t->nextLevel[i]), (t->nNextLevels-i) * sizeof(struct UFDBtable) );
      }

      /* insert the current revURL into the array */
      t->nextLevel[i].nNextLevels = 0;
      t->nextLevel[i].tag = (unsigned char *) _STRDUP( (char *) revURL->part );
      t->nextLevel[i].nextLevel = NULL;

      /* process the tail of revURL */
      rv = UFDBinsertURL( &(t->nextLevel[i]), revURL->next, type );
   }
   else 					/* this entry > nextLevel[i] */
   {
      i++;
      
      t->nNextLevels++;
      t->nextLevel = _trealloc( t->nextLevel, t->nNextLevels );

      /* make space in the array */
      if (t->nNextLevels > i)
      {
	 memmove( &(t->nextLevel[i+1]), &(t->nextLevel[i]), (t->nNextLevels-i) * sizeof(struct UFDBtable) );
      }

      /* insert the current revURL into the array */
      t->nextLevel[i].nNextLevels = 0;
      t->nextLevel[i].tag = (unsigned char *) _STRDUP( (char *) revURL->part );
      t->nextLevel[i].nextLevel = NULL;

      /* process the tail of revURL */
      rv = UFDBinsertURL( &(t->nextLevel[i]), revURL->next, type );
   }

   return rv;
}


/* Generate a binary table file format v1.2
 */
void writeTableToFile_1_2( struct UFDBtable * t, FILE * output )
{
   int i;

   fputs( (char *) t->tag, output );
   if (t->nNextLevels > 0)
   {
      fputc( UFDBsubLevel, output );
   }

   for (i = 0; i < t->nNextLevels; i++)
   {
      writeTableToFile_1_2( &(t->nextLevel[i]), output );
      if (t->nextLevel[i].nNextLevels == 0)
      {
	 if (i < t->nNextLevels - 1)
	    fputc( UFDBsameLevel, output );
      }
      else
	 fputc( UFDBprevLevel, output );
   }
}


static void calcIndexSize( struct UFDBtable * t )
{
   int i;

   numNodes++;
   if (t->nNextLevels == 0)
      numLeafNodes++;

   for (i = 0; i < t->nNextLevels; i++)
      calcIndexSize( &(t->nextLevel[i]) );
}


/* generate a binary table file, database table format 2.0
 */
void writeTableToFile_2_0( struct UFDBtable * t, FILE * output )
{
   int i;

   fputs( (char *) t->tag, output );

   if (t->nNextLevels > 0)
   {
      fputc( UFDBsubLevel, output );

      /* write the number of child nodes in a 1-byte or 4-byte code */
      if (t->nNextLevels <= 255)
         fputc( t->nNextLevels, output );
      else
      {
         fputc( 0, output );
         i = t->nNextLevels;
         fputc( i % 256, output );
	 i = i / 256;
         fputc( i % 256, output );
	 i = i / 256;
         fputc( i % 256, output );
	 if (i > 32)
	    fprintf( stderr, "**** OVERFLOW in number of child nodes: %d\n", t->nNextLevels );
      }
      DEBUG(( stderr, "      tag = %-18s sub-level   %d child(ren)\n", 
                      t->tag, t->nNextLevels ));
   }
   else
   {
      numLeafNodes++;
      DEBUG(( stderr, "      tag = %-18s leaf \n", t->tag ));
   }

   for (i = 0; i < t->nNextLevels; i++)
   {
      writeTableToFile_2_0( &(t->nextLevel[i]), output );
      if (t->nextLevel[i].nNextLevels == 0)
      {
	 if (i < t->nNextLevels - 1)
	    fputc( UFDBsameLevel, output );
      }
      else
	 fputc( UFDBprevLevel, output );
   }
}


static __inline__ void addDomain( 
   UFDBthreadAdmin * admin,
   unsigned char *   domain, 
   UFDBurlType       type )
{
   unsigned char *   end;
   UFDBrevURL *      revUrl;
   int               rv;
   
   /* strip starting and trailing spaces */
   while (*domain == ' ')
      domain++;
   for (end = domain;  *end != '\0' && *end != '\n'; end++)
      ;
   end--;
   while (end > domain  &&  (*end == ' ' || *end == '\t'))
   {
      *end = '\0';
      end--;
   }
   if (*domain == '\0')
      return;

   /* DEBUG: print function entry of addDomain to stderr */
#if 0
   fprintf( stderr, "addDomain( %s )\n", domain ); /* */
#endif
   numEntries++;

   revUrl = UFDBgenRevURL( admin, domain );

#if 0
   UFDBprintRevURL( revUrl ); 
#endif

   /* first do a lookup of the domain, it is already matches, it should
    * not be added !
    */
   rv = UFDBlookupRevUrl( table, revUrl );
   if (rv)
   {
      if (doWarnings)
	 fprintf( stderr, "url/domain %s is not added because it was already matched.\n", domain );
   }
   else
   {
      rv = UFDBinsertURL( table, revUrl, type );
      if (rv)
      {
	 if (doWarnings)
	    fprintf( stderr, "domain %s has optimised subdomains.\n", domain );
      }
   }

   UFDBfreeRevURL( admin, revUrl );
}


static char randomChar( void )
{
   static char * a = "1234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
   return a[random() % 62];
}


static void generateRandomKey( char * encryptKey )
{
   srandom( getpid() * time(NULL) );

   encryptKey[0]  = randomChar();
   encryptKey[1]  = randomChar();
   encryptKey[2]  = randomChar();
   encryptKey[3]  = randomChar();
   encryptKey[4]  = randomChar();
   encryptKey[5]  = randomChar();
   encryptKey[6]  = randomChar();
   encryptKey[7]  = randomChar();
   encryptKey[8]  = randomChar();
   encryptKey[9]  = randomChar();
   encryptKey[10] = randomChar();
   encryptKey[11] = randomChar();
   encryptKey[12] = randomChar();
   encryptKey[13] = randomChar();
   encryptKey[14] = randomChar();
   encryptKey[15] = randomChar();
   encryptKey[16] = '\0';
}


static void copyKey( char * key, char * encryptKey )
{
   key[0]  = encryptKey[0];
   key[1]  = encryptKey[1];
   key[2]  = encryptKey[2];
   key[3]  = encryptKey[3];
   key[4]  = '-';
   key[5]  = encryptKey[4];
   key[6]  = encryptKey[5];
   key[7]  = encryptKey[6];
   key[8]  = encryptKey[7];
   key[9]  = '-';
   key[10] = encryptKey[8];
   key[11] = encryptKey[9];
   key[12] = encryptKey[10];
   key[13] = encryptKey[11];
   key[14] = '-';
   key[15] = encryptKey[12];
   key[16] = encryptKey[13];
   key[17] = encryptKey[14];
   key[18] = encryptKey[15];
   key[19] = '\0';
}


#if 0
static void encryptFile( FILE * f, unsigned char * key )
{
   ufdbCrypt uc;
   long pos;
   int n;
#define N 8192
   unsigned char iblock[N];
   unsigned char oblock[N];

   ufdbCryptInit( &uc, key, 16 );
   
   while (!feof(f))
   {
      pos = ftell( f );
      n = fread( iblock, 1, N, f );
      ufdbEncryptText( &uc, oblock, iblock, n );
      fseek( f, pos, SEEK_SET );
      fwrite( oblock, 1, n, f );
      if (n < N)
         break;
   }
}
#endif


static void encryptMemory( char * to, char * from, long n, unsigned char * key )
{
   ufdbCrypt uc;

#if 0
   fprintf( stderr, "encryptMemory( %08x %08x %ld %16.16s )\n", (unsigned int)to, (unsigned int)from, n, key );
#endif

   ufdbCryptInit( &uc, key, 16 );
   ufdbEncryptText( &uc, (unsigned char *) to, (unsigned char *) from, n );
}


static long compressMemory( char * to, char * from, long size )
{
   unsigned int new_size;

   new_size = (unsigned int) (size + 2048);
   if (BZ_OK != BZ2_bzBuffToBuffCompress( to, &new_size, from, size, 7, 0, 40 ))
   {
      fprintf( stderr, "compression failed.\n" );
      exit( 1 );
   }

#if 0
   fprintf( stderr, "compressMemory from size %ld to %d\n", size, new_size );
#endif

   return new_size;
}


static void updateSizeInHeader( 
   FILE * f, 
   long   size )
{
   int    n;
   int    _indexSize;
   char   version[8];
   char   prefix[32];
   char   tableName[32];
   char   key[32];
   char   date[32];
   char   flags[8+1];
   struct UFDBfileHeader header;

#if 0
   fprintf( stderr, "updateSizeInHeader %ld\n", size );
#endif

   fseek( f, 0, SEEK_SET );
   if (8 != fscanf( f, "%5s %7s %20s %11d key=%30s date=%20s %8s %d",
                    prefix, version, tableName, &n, key, date, flags, &_indexSize ))
   {
      return;
   }

   fseek( f, 0, SEEK_SET );
   sprintf( header.string, "%s %s %s %ld key=%s date=%s %8s %d \n",
            "UFDB", version, tableName, size, key, date, flags, _indexSize );
   fprintf( f, "%s", header.string );
   for (n = sizeof(header.string) - strlen(header.string); n > 0; n--)
      fputc( '\0', f );
   fflush( f );

   fseek( f, 0, SEEK_END );
}


static void doCryptCompress( 
   FILE * f, 
   char * encryptKey )
{
   long   size;
   long   orig_size;
   char * buffer1;
   char * buffer2;

   fflush( f );
   fseek( f, 0, SEEK_END );
   orig_size = size = ftell( f ) - sizeof(struct UFDBfileHeader);

#if 0
   fprintf( stderr, "doCryptCompress orig_size %ld bytes\n", orig_size );
#endif

   buffer1 = malloc( size + 2048 );
   buffer2 = malloc( size + 2048 );
   if (buffer1 == NULL || buffer2 == NULL)
   {
      fprintf( stderr, "cannot allocate memory for encryption and/or compression (size=%ld)\n", size );
      exit( 1 );
   }

   /* read file into buffer1 */
   fseek( f, sizeof(struct UFDBfileHeader), SEEK_SET );
   if (1 != fread( buffer1, size, 1, f ))
   {
      fprintf( stderr, "cannot read file for encryption and/or compression.\n" );
      exit( 1 );
   }

   /* make sure the 'result' is in buffer2 */
   if (doCompress)
      size = compressMemory( buffer2, buffer1, size );
   else
      memcpy( buffer2, buffer1, size );

   /* crypt from buffer2 into buffer1 */
   if (doCrypt)
      encryptMemory( buffer1, buffer2, size, (unsigned char *) encryptKey );
   else
      memcpy( buffer1, buffer2, size );

   /* rewrite the file header and put the original size in there for the decompression function */
   updateSizeInHeader( f, orig_size );

   /* write buffer1 to the file */
   fseek( f, sizeof(struct UFDBfileHeader), SEEK_SET );

   if (1 != fwrite( buffer1, size, 1, f ))
   {
      fprintf( stderr, "fwrite failed.\n" );
      exit( 1 );
   }
   fflush( f );

   /* truncate the file (if we did compression) */
   if (doCompress  &&  size < orig_size)
   {
      if (ftruncate( fileno(f), size + sizeof(struct UFDBfileHeader) ))
         ;
   }

   free( buffer1 );
   free( buffer2 );
}


void convertSpecialCharacters( unsigned char * domain )
{
   unsigned char * s;
   unsigned char * d;

   for (s = domain, d = domain; *s != '\0'; s++)
   {
      if (*s == '%')
      {
         unsigned int hex;
	 unsigned int h1, h2;

	 h1 = *(s+1);
	 h2 = *(s+2);
	 if (isxdigit(h1) && isxdigit(h2))
	 {
	    hex  = (h1 <= '9') ? h1 - '0' : h1 - 'a' + 10;
	    hex *= 16;
	    hex += (h2 <= '9') ? h2 - '0' : h2 - 'a' + 10;
	    if (hex == 0)
	    {
	       s += 2;
	       continue;
	    }
	    else if (hex <= 0x20)
	    {
	       if (hex != '\t'  &&  hex != '\r'  &&  hex != '\n'  &&  hex != '\f')
		  hex = ' ';
	    }
	    else
	    {
	       if (hex == 0x7f  ||  hex == 0xff)
		  hex = ' ';
	       else
		  if (hex <= 'Z'  &&  hex >= 'A')
		     hex += 'a' - 'A';
	    }
	    *d++ = hex;
	    s += 2;
	 }
	 else
	    *d++ = *s;
      }
      else
      {
         *d++ = *s;
      }
   }
   *d = '\0';
}


int main( int argc, char * argv[] )
{
   int    n;
   int    opt;
   time_t t;
   struct tm * tm;
   char   encryptKey[16+1];
   char   key[16+3+1];
   char   flags[8+1];
   FILE * fout;
   char * fout_buffer;
   struct UFDBfileHeader header;
   UFDBthreadAdmin * admin;
   char   date[64];
   char   outFileName[512];
   unsigned char   domain[4096];

   UFDBappInit();
   admin = UFDBallocThreadAdmin();
   inFileName = NULL;
   urlsFileName = NULL;
   tableName = "defaulttable";
   encryptKey[0] = '\0';

   while ((opt = getopt( argc, argv, "DF:k:t:d:u:nCqPsVW?" )) > 0)
   {
      switch (opt)
      {
      case 'D':
      	 UFDBglobalDebug = 1;
	 break;
      case 'F':
         format = optarg;
	 if (strcmp( format, "1.2" ) != 0  &&  strcmp( format, "2.0" ) != 0)
	 {
	    fprintf( stderr, "-F option only accepts 1.2 and 2.0 as format specifiers\n" );
	    usage();
	 }
	 break;
      case 't':
         tableName = optarg;
	 break;
      case 'd':
         inFileName = optarg;
	 break;
      case 's':
      	 doSanityCheck = 1;
	 break;
      case 'u':
         urlsFileName = optarg;
	 break;
      case 'k':
         strncpy( encryptKey, optarg, 16 );
	 encryptKey[16] = '\0';
	 if (strlen( encryptKey ) != 16)
	 {
	    fprintf( stderr, "key \"%s\" is not valid.\n", encryptKey );
	    usage();
	 }
	 break;
      case 'n':
         doCrypt = 0;
	 break;
      case 'P':
         doProd = 1;
	 break;
      case 'C':
         doCompress = 1;
	 break;
      case 'q':
         doWarnings = 0;
	 break;
      case 'V':
         printf( "ufdbGenTable version " VERSION "\n" );
	 exit( 0 );
      case 'W':
         stripWWW = 1;
	 break;
      case '?':
	 fprintf( stderr, "help:\n" );
         usage();
	 break;
      default:
	 fprintf( stderr, "internal error: getopt returned \"%c\"\n", opt );
         usage();
	 break;
      }
   }

   if (strlen(tableName) > 15)
   {
      fprintf( stderr, "the tableName must be shorter than 16 characters\n" );
      usage();
   }

   if (inFileName == NULL)
   {
      fprintf( stderr, "the input file name is not specified: use the -d option\n" );
      usage();
   }

   fin = fopen( inFileName, "r" );
   if (fin == NULL)
   {
      fprintf( stderr, "cannot read from \"%s\": %s\n", inFileName, strerror(errno) );
      usage();
   }

   strcpy( outFileName, inFileName );
   strcat( outFileName, UFDBfileSuffix );
   fout = fopen( outFileName, "w+" );
   if (fout == NULL)
   {
      fprintf( stderr, "cannot write to \"%s\": %s\n", outFileName, strerror(errno) );
      usage();
   }
   fout_buffer = malloc( 16384 );
   setvbuf( fout, fout_buffer, _IOFBF, 16384 );

   /* setlinebuf( stderr ); */
   initTable( tableName );


   /* process the domains ********************************************/
   n = 0;
readdomains:
   while (!feof(fin))
   {
      unsigned char * ptr;
      ptr = domain;

      while ((*ptr = fgetc(fin)) != '\n')
      {
	 /* check for a last line without \n */
         if (feof(fin))
	 {
	    if (ptr != domain)
	       break;
	    goto eof;
	 }
	 if (*ptr >= 'A'  &&  *ptr <= 'Z')
	    *ptr = *ptr + 'a' - 'A';
	 if (*ptr < ' ')			/* illegal control character in URL */
	 {
	    fprintf( stderr, "illegal control character in URL: %s\n", domain );
	    *ptr = '\0';
	    while (!feof(fin) && fgetc(fin) != '\n')
	       ;
	    break;
	 }
	 if (*ptr != '\r')			/* Skip '\r' */
	    ptr++;
	 if (ptr > &domain[4090])
	 {
	    *ptr = '\0';
	    fprintf( stderr, "URL too long: %s\n", domain );
	    while (!feof(fin) && fgetc(fin) != '\n')
	       ;
	    goto readdomains;
	 }
      }
      *ptr = '\0';

      if (domain[0] != '#')
      {
	 char * first_slash;

	 if (strncmp( (char *) domain, "www.", 4 ) == 0)
	 {
	    if (stripWWW)
	    {
	       memmove( domain, domain+4, strlen((char*) domain)-4+1 );
	       if (doWarnings)
		  fprintf( stderr, "notice: \"www.\" is stripped for %s\n", domain );
	    }
	    else if (doWarnings)
	    {
	       fprintf( stderr, "warning: domain name starts with \"www.\": %s (use -W option ?)\n", domain );
	    }
	 }

	 if (doWarnings)
	 {
	    if (ptr - domain > 66)
	       fprintf( stderr, "warning: long domain name: %s\n", domain );
	 }

	 first_slash = strchr( (char *) domain, '/' );
	 if (first_slash != NULL)
	    fprintf( stderr, "warning: domain name (%s) has a '/' ?!?!\n", domain );
	 if (doWarnings  &&  !UFDBsanityCheckDomainname( (char *) domain ))
	    fprintf( stderr, "warning: illegal domain name: %s\n", domain );
	 addDomain( admin, domain, UFDBdomain );
      }
   }
eof:
   fclose( fin );

   /* process the urls ***********************************************/
   if (urlsFileName != NULL)
   {
      fin = fopen( urlsFileName, "r" );
      if (fin == NULL)
      {
	 fprintf( stderr, "cannot read from \"%s\": %s\n", urlsFileName, strerror(errno) );
	 usage();
      }

readurls:
      while (!feof(fin))
      {
	 unsigned char * ptr;
	 unsigned char * first_slash;

	 ptr = domain;

	 while ((*ptr = fgetc(fin)) != '\n')
	 {
	    /* check for a last line without \n */
	    if (feof(fin))
	    {
	       if (ptr != domain)
		  break;
	       goto eof2;
	    }
	    if (*ptr >= 'A'  &&  *ptr <= 'Z')
	       *ptr = *ptr + 'a' - 'A';
	    if (*ptr < ' ')			/* illegal control character in URL */
	    {
	       fprintf( stderr, "illegal control character in URL: %s\n", domain );
	       *ptr = '\0';
	       while (!feof(fin) && fgetc(fin) != '\n')
		  ;
	       break;
	    }
	    if (*ptr != '\r')			/* Skip '\r' */
	       ptr++;
	    if (ptr > &domain[4090])
	    {
	       *ptr = '\0';
	       fprintf( stderr, "URL too long: %s\n", domain );
	       while (!feof(fin) && fgetc(fin) != '\n')
		  ;
	       goto readurls;
	    }
	 }
	 *ptr = '\0';

	 if (domain[0] != '#')
	 {
	    convertSpecialCharacters( domain );

	    first_slash = (unsigned char *) strchr( (char *) domain, '/' );
	    if (first_slash == NULL)
	    {
	       if (doWarnings) 
	       {
	          fprintf( stderr, "warning: URL has no '/': %s\n", domain );
		  if (strlen( (char *) domain ) > 66)
		     fprintf( stderr, "warning: long URL: %s\n", domain );
	       }
	    }
	    else
	    {
	       int pathlen;

	       pathlen = strlen( (char *) first_slash );
	       if (doWarnings) 
		  if (first_slash - &domain[0] > 66  ||  pathlen > 66)
		     fprintf( stderr, "warning: long URL: %s\n", domain );
	       if (pathlen >= sizeof(UFDBurlPart))
	       {
		  fprintf( stderr, "   *** very long URL: %s\n", domain );
		  fprintf( stderr, "   *** the long URL is being truncated (max path length = %d) !\n", (int) sizeof(UFDBurlPart)-1 );
		  *( first_slash + sizeof(UFDBurlPart) ) = '\0';
	       }
	    }

	    if (doWarnings  &&  strncmp( (char *) domain, "www.", 4 ) == 0)
	       fprintf( stderr, "warning: URL starts with \"www.\": %s (strip www. ?)\n", domain );

	    if (doWarnings  &&  !UFDBsanityCheckDomainname( (char *) domain ))
	       fprintf( stderr, "warning: illegal domain name: %s\n", domain );
	    addDomain( admin, domain, UFDBurl );
	 }
      }
eof2:
      fclose( fin );
   }

   if (encryptKey[0] == '\0')
      generateRandomKey( encryptKey );
   copyKey( key, encryptKey );

   if (format[0] != '1')
   {
      calcIndexSize( table );
      numIndexNodes = numNodes - numLeafNodes;
      if (UFDBglobalDebug)
	 fprintf( stderr, "#nodes: %9d   #leafs: %9d   #index: %9d\n",
		  numNodes, numLeafNodes, numIndexNodes );
   }

   /* write the table header to the output file */
   strcpy( flags, "--------" );
   if (doCompress)
      flags[0] = 'C';
   if (doProd)
      flags[1] = 'P';
   if (doCrypt)
      flags[2] = 'Q';
   t = time( NULL );
   tm = gmtime( &t );
   sprintf( date, "%4d%02d%02d.%02d%02d", 
            tm->tm_year+1900, tm->tm_mon+1, tm->tm_mday, tm->tm_hour, tm->tm_min );
   sprintf( header.string, "%s %s %s %d key=%s date=%s %8s %d \n",
            "UFDB", format, tableName, numEntries, key, date, flags, numNodes );
   fprintf( fout, "%s", header.string );
   for (n = sizeof(header.string) - strlen(header.string); n > 0; n--)
      fputc( '\0', fout );

   /* write the table in binary format to the output file */
   if (strcmp( format, "1.2" ) == 0)
      writeTableToFile_1_2( table, fout );
   else
      writeTableToFile_2_0( table, fout );
   fputc( UFDBendTable, fout );

   /* encrypt and compress the table: rewind, read, compress, crypt and write */
   if (doCrypt || doCompress)
   {
      doCryptCompress( fout, encryptKey );
   }

   fclose( fout );
   free( fout_buffer );

   return 0;
}


/* since ufdbguard (single-threaded) and ufdbguardd (multi-threaded)
 * share source code, we put some pthread dummys here since we don't need/want pthreads.
 */
int pthread_mutex_lock( pthread_mutex_t * mutex )
{
   return 0;
}

int pthread_mutex_trylock( pthread_mutex_t * mutex )
{
   return 0;
}

int pthread_mutex_unlock( pthread_mutex_t * mutex )
{
   return 0;
}


int pthread_cond_signal(pthread_cond_t *cond)
{
   return 0;
}

int pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex)
{
   return 0;
}

