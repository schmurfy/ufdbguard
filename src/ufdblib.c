/*
 * ufdblib.c - URLfilterDB
 *
 * ufdbGuard is copyrighted (C) 2005-2011 by URLfilterDB with all rights reserved.
 *
 * Parts of ufdbGuard are based on squidGuard.
 * This module is NOT based on squidGuard.
 *
 * RCS $Id: ufdblib.c,v 1.79 2011/05/09 17:24:43 root Exp root $
 */

/* This module is well tested and stable for a long time AND
 * needs maximum performance, so we remove _FORTIFY_SOURCE.
 */
#undef _FORTIFY_SOURCE

/* to inline string functions with gcc : */
#if defined(__OPTIMIZE__) && defined(__GNUC__) && defined(GCC_INLINE_STRING_FUNCTIONS_ARE_FASTER)
#define __USE_STRING_INLINES
#endif

#include "ufdb.h"
#include "ufdblib.h"

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <pthread.h>
#include <syslog.h>
/* TODO: evaluate use of syslog() */
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/socket.h>
#if HAVE_UNIX_SOCKETS
#include <sys/un.h>
#endif
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>

#include "bzlib.h"

/* #define  UFDB_DEBUG_IPV6 */

/* #define UFDB_DO_DEBUG 1 */

#if UFDB_DO_DEBUG || 0
#define DEBUG(x) fprintf x 
#else
#define DEBUG(x) 
#endif

static struct UFDBtable * tableIndex;
static int                tableIndex_i;


#if 0
static unsigned char never[] =
{
   "UFDB 1.1 never 1 key=FGrH-PE5U-ljDA-8jVV date=20050724.1140 -------- \n\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0never\001never\020"
};
#endif


UFDBthreadAdmin * UFDBallocThreadAdmin( void )
{
   UFDBthreadAdmin * admin;

   admin = (UFDBthreadAdmin *) ufdbCalloc( 1, sizeof(UFDBthreadAdmin) );

   return admin;
}


/* Mutex locks for _allocRevURL() are a thread synchronisation bottleneck */
/* so it is better that each thread has its own array of UFDBrevURL* */

static __inline__ UFDBrevURL * _allocRevURL( 
   UFDBthreadAdmin * admin )
{
   int               i;
   UFDBrevURL *      new;
   
   for (i = 0; i < admin->nAlloced; i++)
   {
      if (admin->myArrayUsage[i] == 0)
      {
         admin->myArrayUsage[i] = 1;
	 new = &(admin->myArray[i]);
	 return new;
      }
   }

   if (admin->nAlloced == MAX_REVURLS)
   {
      new = ufdbMalloc( sizeof(UFDBrevURL) );
   }
   else
   {
      admin->myArrayUsage[admin->nAlloced] = 1;
      new = &(admin->myArray[admin->nAlloced]);
      admin->nAlloced++;
   }

   return new;
}


static __inline__ void _freeRevURL( 
   UFDBthreadAdmin * admin, 
   UFDBrevURL *      old )
{
   int               i;

   for (i = 0; i < admin->nAlloced; i++)
   {
      if (old == &(admin->myArray[i]))
      {
         admin->myArrayUsage[i] = 0;
	 return;
      }
   }

   ufdbFree( old );
}


static UFDBrevURL * parseIPv6URL( 
   UFDBthreadAdmin * admin,
   unsigned char *   URL )
{
   unsigned char *   oldBracket;
   unsigned int      size;
   UFDBrevURL *      newHead;
   UFDBrevURL *      newPath;

#ifdef UFDB_DEBUG_IPV6
   fprintf( stderr, " parseIPv6URL: %s\n", URL );
#endif

   oldBracket = (unsigned char *) strchr( (char *) URL, ']' );
   if (oldBracket == NULL)
   {
      oldBracket = URL;
      while (*oldBracket != '\0'  &&  *(oldBracket+1) != '/')
	 oldBracket++;
      if (oldBracket - URL > sizeof(UFDBurlPart)-2)
         oldBracket = URL + sizeof(UFDBurlPart) - 2;
   }
#if 0
   else
   {
      char normalisedDomain[64];

      *oldBracket = '\0';
      if (UFDBparseIPv6address( URL+1, normalisedDomain ) == NULL)
         *oldBracket = ']';
      else
      {
	 *oldBracket = ']';
	 UFDBnormaliseIPv6Domain( URL, normalisedDomain );
      }
   }
#endif

   newHead = _allocRevURL( admin );
   size = (unsigned char *) oldBracket - URL + 1;
   if (size > sizeof(UFDBurlPart)-2)
      size = sizeof(UFDBurlPart) - 2;
   memccpy( newHead->part, URL, ']', size );
   newHead->part[ size ] = '\0';

   if (*(oldBracket+1) == '/')
   {
      newPath = _allocRevURL( admin );
      newHead->next = newPath;
      memccpy( newPath->part, oldBracket+1, '\0', sizeof(UFDBurlPart)-2 );
      newPath->part[ sizeof(UFDBurlPart)-2 ] = '\0';
      newPath->next = NULL;
   }
   else
      newHead->next = NULL;
   
   return newHead;
}


static UFDBrevURL * parseHostnameRecursive( 
   UFDBthreadAdmin * admin,
   UFDBrevURL *      prevHead, 
   unsigned char *   lastPiece )
{
   char *            dot;
   UFDBrevURL *      newHead;

#if 0
   DEBUG(( stderr, "   parseHostnameRecursive  0x%08x 0x%08x %s\n", prevHead, lastPiece, lastPiece ));  /* */
#endif

   dot = strrchr( (char *) lastPiece, '.' );
   if (dot == NULL)
   {
      newHead = _allocRevURL( admin );

      strcpy( (char *) newHead->part, (char *) lastPiece );
      newHead->next = prevHead;
   }
   else
   {
      *dot = '\0';
      newHead = _allocRevURL( admin );

      strcpy( (char *) newHead->part, dot+1 );
      newHead->next = parseHostnameRecursive( admin, prevHead, lastPiece );
   }

   return newHead;
}


/* Parse the URL and reverse it into a linked list 
 * e.g. my.sex.com becomes "com" -> "sex" -> "my" -> NULL
 *
 * The input parameter, URL, must be a writable array of characters (size = UFDBmaxURLsize).
 */
UFDBrevURL * UFDBgenRevURL( 
   UFDBthreadAdmin * admin,
   unsigned char *   URL )
{
   char *            path;
   UFDBurlPart       URLpart;
   char              newStr[UFDB_MAX_URL_LENGTH];

   URL = (unsigned char *) strcpy( newStr, (char *) URL );

   if (URL[0] == '[')
      return parseIPv6URL( admin, URL );

   URLpart[0] = '\0';
   path = strchr( newStr, '/' );
   if (path != NULL)
   {
      UFDBrevURL * head;

      *path = '\0';
      if (*(path-1) == '.')		/* delete the last dot in the domain www.sex.com./index.html */
         *(path-1) = '\0';
      path++;

      head = parseHostnameRecursive( admin, NULL, URL );

      if (*path != '\0')
      {
	 UFDBrevURL * ptr;

	 ptr = head;
	 while (ptr->next != NULL)
	    ptr = ptr->next;

	 ptr->next = _allocRevURL( admin );

	 ptr = ptr->next;
	 ptr->next = NULL;
	 ptr->part[0] = '/';
	 memccpy( (char *) ptr->part + 1, path, '\0', sizeof(UFDBurlPart)-2 );
	 ptr->part[ sizeof(UFDBurlPart)-2 ] = '\0';
      }
      return head;
   }
   else
   {
      unsigned char * end;

      for (end = URL; *end != '\0'; end++)
         ;
      if (*(end-1) == '.')		/* also match www.sex.com. */
         *(end-1) = '\0';
      return parseHostnameRecursive( admin, NULL, URL );
   }
}


/* DEBUG: print a revURL to stderr
 */
void UFDBprintRevURL( UFDBrevURL * revURL )
{
   fputs( "   P ", stderr );
   while (revURL != NULL)
   {
      fputs( (char *) revURL->part, stderr );
      if (revURL->next != NULL)
         fputs( " . ", stderr );
      revURL = revURL->next;
   }
   fputs( " \n", stderr );
}


__inline__ void UFDBfreeRevURL( 
   UFDBthreadAdmin * admin,
   UFDBrevURL *      revURL )
{
   UFDBrevURL *      next;

   while (revURL != NULL)
   {
      next = revURL->next;
      _freeRevURL( admin, revURL );
      revURL = next;
   }
   admin->nAlloced = 0;
}


#define ROUNDUPBY      4
#define ROUNDUP(i)     ( (i) + (ROUNDUPBY - ((i)%ROUNDUPBY) ) )

#define BIGROUNDUPBY   (4 * ROUNDUPBY)
#define BIGROUNDUP(i)  ( (i) + (BIGROUNDUPBY - ((i)%BIGROUNDUPBY) ) )

__inline__ static struct UFDBtable * _reallocTableArray( 
   struct UFDBtable *  ptr,
   int                 nElem )
{
   if (nElem <= ROUNDUPBY)
   {
      ptr = ufdbRealloc( ptr, (size_t) nElem * sizeof(struct UFDBtable) );
   }
   else if (nElem < 4*BIGROUNDUPBY)
   {
      if (nElem % ROUNDUPBY == 1)
	 ptr = ufdbRealloc( ptr, (size_t) ROUNDUP(nElem) * sizeof(struct UFDBtable) );
   }
   else
   {
      if (nElem % BIGROUNDUPBY == 1)
	 ptr = ufdbRealloc( ptr, (size_t) BIGROUNDUP(nElem) * sizeof(struct UFDBtable) );
   }

   return ptr;
}


void UFDBfreeTableIndex_1_2( struct UFDBtable * t )
{
   int i;

   if (t == NULL)
      return;

   for (i = 0; i < t->nNextLevels; i++)
   {
      UFDBfreeTableIndex_1_2( &(t->nextLevel[i]) );
   }

   ufdbFree( &(t->nextLevel[0]) );
}


void ufdbDecompressTable( struct UFDBmemTable * memTable )
{
   char * new_mem;
   int    rv;

#if 0
   fprintf( stderr, "decompress table from %ld original size of %d\n", 
            memTable->tableSize, memTable->numEntries );
#endif

   new_mem = ufdbMalloc( sizeof(struct UFDBfileHeader) + memTable->numEntries );
   if (new_mem == NULL)
   {
      fprintf( stderr, "ufdbDecompressTable: cannot allocate %ld bytes memory for table decompression\n", 
               (long) (sizeof(struct UFDBfileHeader) + memTable->numEntries) );
      exit( 1 );
   }
   memcpy( new_mem, memTable->mem, sizeof(struct UFDBfileHeader) );

   rv = BZ2_bzBuffToBuffDecompress( new_mem + sizeof(struct UFDBfileHeader),
                                    (unsigned int *) &(memTable->numEntries),
			            (char *) memTable->mem + sizeof(struct UFDBfileHeader),
			            memTable->tableSize,
			            0,
			            0 );
   if (rv != BZ_OK)
   {
      fprintf( stderr, "ufdbDecompressTable: decompression failed with code %d\n", rv );
      exit( 1 );
   }

   ufdbFree( memTable->mem );
   memTable->mem = new_mem;
}


/* parse a binary table header from a memory table.
 */
int UFDBparseTableHeader( struct UFDBmemTable * memTable )
{
   int  retval = UFDB_API_OK;
   int  n;
   char prefix[32];
   char tableName[32];
   char key[32];
   char date[32];
   char flags[8+1];
   ufdbCrypt uc;
   unsigned char * mem;

   strcpy( date, "nodate" );
   strcpy( flags, "--------" );
   memTable->version[0] = '\0';
   memTable->numEntries = 0;
   memTable->indexSize = 0;
   n = sscanf( memTable->mem, "%5s %7s %20s %11d key=%30s date=%20s %8s %d",
               prefix, &(memTable->version[0]), tableName, &(memTable->numEntries), 
	       key, date, flags, &(memTable->indexSize) );
#if 0
   fprintf( stderr, "      UFDBparseTableHeader: n=%d prefix=%-5.5s version=%s name=%s num=%d key=%s date=%s flags=%s indexsize=%d\n", 
            n, prefix, memTable->version, tableName, memTable->numEntries, key, date, flags, memTable->indexSize );
#endif

   if (n < 5  ||  strcmp(prefix,"UFDB") != 0)
   {
      fprintf( stderr, "invalid UFDB header\n" );
      fprintf( stderr, "contact support@urlfilterdb.com\n" );
      syslog( LOG_ALERT, "table %s has an invalid UFDB header", tableName );
      return UFDB_API_ERR_INVALID_TABLE;
   }

   if (memTable->indexSize < 0)
   {
      fprintf( stderr, "error in UFDB header: indexsize < 0\n" );
      fprintf( stderr, "contact support@urlfilterdb.com\n" );
      syslog( LOG_ALERT, "table %s has an erroneous UFDB header: indexsize < 0", tableName );
      return UFDB_API_ERR_INVALID_TABLE;
   }

   if (strcmp( memTable->version, UFDBdbVersion ) > 0)
   {
      fprintf( stderr, "UFDB file for table \"%s\" has data format version \"%s\" while "
      		       "this program\n"
                       "does not support versions higher than \"%s\"\n", 
		       tableName, memTable->version, UFDBdbVersion );
      fprintf( stderr, "Download/install a new version of this program.\n" );
      fprintf( stderr, "Go to http://www.urlfilterdb.com\n" );
      syslog( LOG_ALERT, "table %s has an unsupported data format (%s)", tableName, memTable->version );
      return UFDB_API_ERR_INVALID_TABLE;
   }

   /* starting with version 2.0 we need the indexSize */
   if (strcmp( "2.0", memTable->version ) >= 0)
   {
      if (n < 7)
      {
         fprintf( stderr, "invalid UFDB2 header\n" );
	 fprintf( stderr, "contact support@urlfilterdb.com\n" );
	 syslog( LOG_ALERT, "table %s has an invalid UFDB2 header", tableName );
	 return UFDB_API_ERR_INVALID_TABLE;
      }
   }

   if (strlen( key ) < 19)
   {
      fprintf( stderr, "UFDB file for table \"%s\" has an invalid key\n", tableName );
      fprintf( stderr, "contact support@urlfilterdb.com\n" );
      return UFDB_API_ERR_INVALID_KEY;
   }

   memTable->key[0] = key[0];
   memTable->key[1] = key[1];
   memTable->key[2] = key[2];
   memTable->key[3] = key[3];
   /* skip '-' */
   memTable->key[4] = key[5];
   memTable->key[5] = key[6];
   memTable->key[6] = key[7];
   memTable->key[7] = key[8];
   /* skip '-' */
   memTable->key[8] = key[10];
   memTable->key[9] = key[11];
   memTable->key[10] = key[12];
   memTable->key[11] = key[13];
   /* skip '-' */
   memTable->key[12] = key[15];
   memTable->key[13] = key[16];
   memTable->key[14] = key[17];
   memTable->key[15] = key[18];

   strncpy( memTable->flags, flags, 8 );

   if (strcmp( date, "nodate" ) != 0)
   {
      struct tm   tm;

      if (5 != sscanf( date, "%4d%2d%2d.%2d%2d", 
                       &tm.tm_year, &tm.tm_mon, &tm.tm_mday, &tm.tm_hour, &tm.tm_min ))
      {
         fprintf( stderr, "table %s has an invalid date (date=%13.13s)\n", tableName, date );
         fprintf( stderr, "contact support@urlfilterdb.com\n" );
	 syslog( LOG_ALERT, "table %s has an invalid date", tableName );
	 return UFDB_API_ERR_INVALID_TABLE;
      }
      else
      {
	 time_t      t_now;
	 time_t      t_db;
	 time_t      diff;

	 tm.tm_year -= 1900;
	 tm.tm_mon  -= 1;
	 tm.tm_isdst = 0;
	 tm.tm_sec   = 0;

	 t_db = mktime( &tm );
	 t_now = time( NULL );
	 diff = t_now - t_db;
	 if (diff < -24 * 60 * 60)	/* allow 1 day back for various time zones */
	 {
	    fprintf( stderr, "table %s has an invalid date (%13.13s)\n", tableName, date );
	    fprintf( stderr, "the difference between current system time and database timestamp is %ld seconds\n", diff );
            fprintf( stderr, "contact support@urlfilterdb.com\n" );
	    syslog( LOG_ALERT, "table %s has an invalid date (%13.13s)", tableName, date );
	    ufdbLogError( "table %s has an invalid date (%13.13s)", tableName, date );
	    retval = UFDB_API_ERR_INVALID_TABLE;
	 }
	 else if (diff > 28 * 24 * 60 * 60  &&  flags[1] == 'P')
	 {
	    fprintf( stderr, "table %s is dated %13.13s and has expired\n", tableName, date );
            fprintf( stderr, "contact support@urlfilterdb.com\n" );
	    syslog( LOG_ALERT, "table %s is too old", tableName );
	    ufdbLogFatalError( "table %s is dated %13.13s and has EXPIRED.  *******************\n"
	                       "Check licenses and cron job for ufdbUpdate",
			       tableName, date );
	    retval = UFDB_API_ERR_INVALID_TABLE;
	    UFDBglobalDatabaseStatus = UFDB_API_STATUS_DATABASE_EXPIRED;
	 }
	 else if (diff >  4 * 24 * 60 * 60  &&  flags[1] == 'P')
	 {
	    int days;
	    days = (int) (diff / (24 * 60 * 60));
	    fprintf( stderr, "table %s is dated %13.13s and is %d days old\n", tableName, date, days );
            fprintf( stderr, "contact support@urlfilterdb.com\n" );
	    syslog( LOG_ALERT, "table %s is %d days old. Check the cron job for ufdbUpdate", 
	            tableName, days );
	    ufdbLogError( "table %s is dated %13.13s and is %d days old.  *******************\n"
	                  "Check cron job for ufdbUpdate",
			  tableName, date, days );
	    if (UFDBglobalDatabaseStatus == UFDB_API_STATUS_DATABASE_OK)
	       UFDBglobalDatabaseStatus = UFDB_API_STATUS_DATABASE_OLD;
	 }
#if 0
	 fprintf( stderr, "t_db  %12ld\n", t_db );
	 fprintf( stderr, "t_now %12ld\n", t_now );
	 fprintf( stderr, "diff  %12ld\n", t_now - t_db );
#endif
      }
   }

   if (retval != UFDB_API_OK)
   {
      memTable->table.tag = (unsigned char *) "NEVER";
      memTable->mem = NULL;
      memTable->tableSize = 0;
      memTable->table.nNextLevels = 0;
      memTable->table.nextLevel = NULL;
      /* ********************
      UFDBparseTable( memTable );
      ************************/
   }
   else
   {
      ufdbLogMessage( "loading URL category %s with creation date %s", tableName, date );

      if (flags[2] == 'Q')
      {
	 mem = (unsigned char *) memTable->mem + sizeof( struct UFDBfileHeader );
	 ufdbCryptInit( &uc, (unsigned char *) memTable->key, 16 );
	 ufdbEncryptText( &uc, mem, mem, memTable->tableSize );
      }

      if (memTable->flags[0] == 'C')
      {
	 ufdbDecompressTable( memTable );
      }
   }

   return retval;
}


#include "strcmpurlpart.static.c"


static __inline__ unsigned char * parseNextTag( 
   struct UFDBtable * parent, 
   unsigned char *    mem )
{
   unsigned char *    tag;
   int                tagType;
   int                n;

   while (mem != NULL)
   {
      tag = mem;
      while (*mem >= ' ')
	 mem++;

      tagType = *mem;
      *mem++ = '\0';

      switch (tagType)
      {
      case UFDBsubLevel:
	    DEBUG(( stderr, "   parse  tag = %-10s  sub-level\n", tag )); /* */

	    n = parent->nNextLevels;
	    parent->nNextLevels++;
	    parent->nextLevel = _reallocTableArray( parent->nextLevel, parent->nNextLevels );
	    parent->nextLevel[n].tag = tag;
	    parent->nextLevel[n].nNextLevels = 0;
	    parent->nextLevel[n].nextLevel = NULL;

	    mem = parseNextTag( &(parent->nextLevel[n]), mem );
	    break;

      case UFDBsameLevel:
	    DEBUG(( stderr, "   parse  tag = %-10s  same-level\n", tag )); /* */

	    n = parent->nNextLevels;
	    parent->nNextLevels++;
	    parent->nextLevel = _reallocTableArray( parent->nextLevel, parent->nNextLevels );
	    parent->nextLevel[n].tag = tag;
	    parent->nextLevel[n].nNextLevels = 0;
	    parent->nextLevel[n].nextLevel = NULL;

	    break;

      case UFDBprevLevel:
	    if (tag[0] >= ' ')   /* is the code followed by a tag or another code ? */
	    {
	       DEBUG(( stderr, "   parse  tag = %-10s  prev-level\n", tag )); /* */

	       n = parent->nNextLevels;
	       parent->nNextLevels++;
	       parent->nextLevel = _reallocTableArray( parent->nextLevel, parent->nNextLevels );
	       parent->nextLevel[n].tag = tag;
	       parent->nextLevel[n].nNextLevels = 0;
	       parent->nextLevel[n].nextLevel = NULL;
	    }
	    else 
	    {
	       DEBUG(( stderr, "   parse  tag = %-10s  prev-level\n", "*" )); /* */
	       ;
	    }
	    return mem;
	    break;

      case UFDBendTable:
	    DEBUG(( stderr, "   parse  tag = %-10s  end-of-table\n", tag[0] >= ' ' ? (char*) tag : "NONE" )); /* */
	    if (tag[0] >= ' ')   /* is the code followed by a tag or another code ? */
	    {
	       DEBUG(( stderr, "   parse  tag = %-10s  end-of-table\n", tag )); /* */

	       n = parent->nNextLevels;
	       parent->nNextLevels++;
	       parent->nextLevel = _reallocTableArray( parent->nextLevel, parent->nNextLevels );
	       parent->nextLevel[n].tag = tag;
	       parent->nextLevel[n].nNextLevels = 0;
	       parent->nextLevel[n].nextLevel = NULL;
	    }
	    return NULL;
	    break;

      default:
            DEBUG(( stderr, "tagType = %d !\n", tagType ));
	    ;
      }
   }

   return NULL;
}


static __inline__ struct UFDBtable * _allocTableIndex( int num )
{
   struct UFDBtable * t;
   
   DEBUG(( stderr, "      _allocTableIndex( %3d )   i = %d\n", num, tableIndex_i ));

   t = &tableIndex[tableIndex_i];
   tableIndex_i += num;

   return t;
}


static unsigned char * parseNextTag_2_0( 
   struct UFDBtable * parent, 
   unsigned char *    mem )
{
   unsigned char *    tag;
   int                tagType;
   int                n;
   unsigned int       num_children;

   while (mem != NULL)
   {
      tag = mem;
      while (*mem >= ' ')
	 mem++;

      tagType = *mem;
      *mem++ = '\0';

      switch (tagType)
      {
      case UFDBsubLevel:
	    num_children = *mem++;
	    if (num_children == 0)
	    {
	       num_children = *mem  +  (*(mem+1) * 256)  +  (*(mem+2) * 65536);
	       mem += 3;
	    }

	    DEBUG(( stderr, "   parse2 tag = %-18s  sub-level   %d child(ren)\n", 
	            tag, num_children ));   /* */

	    n = parent->nNextLevels;
	    parent->nNextLevels++;
	    if (parent->nextLevel == NULL)
	    {
	       /* only at the top-level node, the array is not pre-allocated ... */
	       parent->nextLevel = _allocTableIndex( 1 );
	    }
	    parent->nextLevel[n].tag = tag;
	    parent->nextLevel[n].nNextLevels = 0;
	    parent->nextLevel[n].nextLevel = _allocTableIndex( num_children );

	    mem = parseNextTag_2_0( &(parent->nextLevel[n]), mem );
	    break;

      case UFDBsameLevel:
	    DEBUG(( stderr, "   parse2 tag = %-18s  same-level\n", tag )); /* */

	    n = parent->nNextLevels;
	    parent->nNextLevels++;
	    parent->nextLevel[n].tag = tag;
	    parent->nextLevel[n].nNextLevels = 0;
	    parent->nextLevel[n].nextLevel = NULL;

	    break;

      case UFDBprevLevel:
	    if (tag[0] >= ' ')   /* is the code followed by a tag or another code ? */
	    {
	       DEBUG(( stderr, "   parse2 tag = %-18s  prev-level\n", tag )); /* */

	       n = parent->nNextLevels;
	       parent->nNextLevels++;
	       parent->nextLevel[n].tag = tag;
	       parent->nextLevel[n].nNextLevels = 0;
	       parent->nextLevel[n].nextLevel = NULL;
	    }
	    else 
	    {
	       DEBUG(( stderr, "   parse2 tag = %-18s  prev-level\n", "*" )); /* */
	       ;
	    }
	    return mem;
	    break;

      case UFDBendTable:
#if 0
	    if (tag[0] >= ' ')   /* is the code followed by a tag or another code ? */
	    {
	       DEBUG(( stderr, "   parse2 tag = %-18s  end-of-table\n", tag )); /* */
	       n = parent->nNextLevels;
	       parent->nNextLevels++;
	       parent->nextLevel[n].tag = tag;
	       parent->nextLevel[n].nNextLevels = 0;
	       parent->nextLevel[n].nextLevel = NULL;
	    }
	    else
#endif
	    {
	       DEBUG(( stderr, "   parse2 tag = %-18s  end-of-table\n", "END" )); /* */
	       ;
	    }
	    return NULL;
	    break;
      }
   }

   return NULL;
}


/* Parse a binary table that is loaded into memory.
 */
void UFDBparseTable( struct UFDBmemTable * memTable )
{
   memTable->table.nNextLevels = 0;
   memTable->table.nextLevel = NULL;

   if (strcmp( memTable->version, "2.0" ) >= 0)
   {
      memTable->index = ufdbCalloc( memTable->indexSize+1, sizeof(struct UFDBtable) );
      tableIndex = memTable->index;
      tableIndex_i = 0;
      (void) parseNextTag_2_0( &(memTable->table),
                               memTable->mem + sizeof(struct UFDBfileHeader) );
#if 0
      fprintf( stderr, "predicted index size: %d   last index: %d\n", memTable->indexSize, tableIndex_i );
#endif
   }
   else
   {
      memTable->index = NULL;
      (void) parseNextTag( &(memTable->table), 
                           (unsigned char *) memTable->mem + sizeof(struct UFDBfileHeader) );
   }
}


void UFDBappInit( void )
{
   ;
}


void UFDBtimerInit( struct tms * t )
{
   (void) times( t );
}


void UFDBtimerStop( struct tms * t )
{
   struct tms te;

   (void) times( &te );

   t->tms_utime  = te.tms_utime  - t->tms_utime;
   t->tms_stime  = te.tms_stime  - t->tms_stime;
   t->tms_cutime = te.tms_cutime - t->tms_cutime;
   t->tms_cstime = te.tms_cstime - t->tms_cstime;
}


void UFDBtimerPrintString( char * line, struct tms * t, char * tag )
{
   double  numTicks;

   numTicks = (double) sysconf( _SC_CLK_TCK );

   if (tag == NULL)
      tag = "UFDB timer";

   sprintf( line, "%s:  %5.2f user  %5.2f sys  %5.2f total", 
	    tag,
            (double) t->tms_utime / numTicks, 
	    (double) t->tms_stime / numTicks, 
	    (double) (t->tms_utime+t->tms_stime) / numTicks );
}


void UFDBtimerPrint( struct tms * t, char * tag )
{
   char    line[256];

   UFDBtimerPrintString( line, t, tag );
   fprintf( stderr, "%s\n", line );
   fflush( stderr );
}


/* perform lookup of revUrl in table t.
 * return 1 iff found, 0 otherwise.
 */
int UFDBlookupRevUrl( 
   struct UFDBtable * t, 
   UFDBrevURL *       revUrl )
{
   int b, e;
#if UFDB_DO_DEBUG
   struct UFDBtable * origtable = t;
#endif

begin:
   DEBUG(( stderr, "    UFDBlookupRevUrl:  table %-14s [%d]  tag %s  :  %s\n", origtable->tag, origtable->nNextLevels, t->tag, revUrl->part ));  /* */

   /* TODO: Random memory access is much slower than sequential memory access.
    * Therefor we choose between sequential search and binary search
    * depending on the number of elements in the array.
    */
   e = t->nNextLevels - 1;
#if 0
   if (e < 4)
   {
      /* sequential search */
      for (b = 0; b <= e; b++)
      {
	 int cmp;
	 cmp = strcmpURLpart( (char *) revUrl->part, (char *) t->nextLevel[b].tag );
	 DEBUG(( stderr, "      strcmpURLpart[%d]  %s  %s  = %d\n", b,
			 revUrl->part, t->nextLevel[b].tag, cmp ));
	 if (cmp == 0)
	 {
	    t = &(t->nextLevel[b]);
	    if (t->nNextLevels == 0)		/* no more levels in table -> MATCH */
	       return 1;
	    revUrl = revUrl->next;
	    if (revUrl == NULL)			/* no more levels in URL -> NO match */
	       return 0;

	    /* optimise the recursive call : */
	    /* return UFDBlookupRevUrl( t, revUrl ); */
	    goto begin;
	 }
      }
      return 0;  /* not found */
   }
   else
#endif
   {
      /* binary search */
      b = 0;
      while (b <= e)
      {
	 int i, cmp;

	 i = (b + e) / 2;
	 cmp = strcmpURLpart( (char *) revUrl->part, (char *) t->nextLevel[i].tag );
	 DEBUG(( stderr, "      strcmpURLpart[%d]  %s  %s  = %d\n", i,
			 revUrl->part, t->nextLevel[i].tag, cmp ));
	 if (cmp == 0)
	 {
	    t = &(t->nextLevel[i]);
	    if (t->nNextLevels == 0)		/* no more levels in table -> MATCH */
	       return 1;
	    revUrl = revUrl->next;
	    if (revUrl == NULL)			/* no more levels in URL -> NO match */
	       return 0;

	    /* optimise the recursive call : */
	    /* return UFDBlookupRevUrl( t, revUrl ); */
	    goto begin;
	 }
	 if (cmp < 0)
	    e = i - 1;
	 else
	    b = i + 1;
      }
   }

   return 0;  /* not found */
}


/*
 *  UFDBlookup: lookup a domain/URL in domain and URL databases.  
 *  return 1 if found.
 */
int UFDBlookup( 
   UFDBthreadAdmin *     admin,
   struct UFDBmemTable * mt, 
   char *                request )
{
   int                   result;
   UFDBrevURL *          revUrl;
   
   if (admin == NULL)
   {
      fprintf( stderr, "UFDBlookup admin=NULL\n" );
      return 0;
   }

   revUrl = UFDBgenRevURL( admin, (unsigned char *) request );
   result = UFDBlookupRevUrl( &(mt->table.nextLevel[0]), revUrl );
   DEBUG(( stderr, "  UFDBlookup( %s, %s ) = %d\n", mt->table.nextLevel[0].tag, request, result ));

   UFDBfreeRevURL( admin, revUrl );

   return result;
}


static pthread_mutex_t mutex_url_history = UFDB_STATIC_MUTEX_INIT;
static char *          url_history = NULL;
static volatile int    url_history_index = 0;


/* Store the name of a webserver in the url_history buffer.
 * Return 1 if the buffer is full, 0 otherwise.
 */
int ufdbRegisterUnknownURL( 
   char * webserver,
   int    portnumber )
{
   int    length;
   char   b[252];

   if (url_history_index >= URL_HIST_SIZE - 8)
      return 1;

   /* webserver names that do not have a dot (.) have no domain name          */
   /* and can never be incorporated in the URL database and are skipped here. */
   if (webserver == NULL  ||  strchr( webserver, '.' ) == NULL)
      return 0;

   length = strlen( webserver );

   pthread_mutex_lock( &mutex_url_history );

   if (url_history == NULL)
   {
      url_history = ufdbMalloc( URL_HIST_SIZE );
      url_history[0] = '\0';
   }

   /* check for buffer overflow */
   if (url_history_index + length >= URL_HIST_SIZE - 8)
   {
      url_history_index = URL_HIST_SIZE;
      pthread_mutex_unlock( &mutex_url_history );
      return 1;
   }

   /* almost perfect optimisation for duplicates */
   if (length <= 240)
   {
      if (portnumber == 80)
	 sprintf( b, "|%s|", webserver );
      else
	 sprintf( b, "|%s:%d|", webserver, portnumber );
      if (strstr( url_history, b ) != NULL)
      {
	 pthread_mutex_unlock( &mutex_url_history );
	 return 0;
      }
   }

   strcpy( &(url_history[url_history_index]), webserver );
   url_history_index += length;
   if (portnumber != 80)
   {
      url_history_index += sprintf( &(url_history[url_history_index]), ":%d", portnumber );
   }
   url_history[url_history_index] = '|';
   url_history_index++;
   url_history[url_history_index] = '\0';

   pthread_mutex_unlock( &mutex_url_history );

   return 0;
}


/*
 * Retrieve all registered uncategorised URLs.
 * This must be followed by a ufdbResetUnknownURLs().
 */
char * ufdbGetUnknownURLs( void )
{
   pthread_mutex_lock( &mutex_url_history );
   url_history_index = URL_HIST_SIZE + 2;		/* prevent additions until reset */
   pthread_mutex_unlock( &mutex_url_history );

   return url_history;
}


void ufdbResetUnknownURLs( void )
{
   pthread_mutex_lock( &mutex_url_history );

   if (url_history == NULL)
      url_history = ufdbMalloc( URL_HIST_SIZE );
   url_history[0] = '\0';
   url_history_index = 0;

   pthread_mutex_unlock( &mutex_url_history );
}


int UFDBopenSocket( char * serverName, int port )
{
   int                 s;
   int                 ret;
   struct sockaddr_in  addr;
   int                 sock_parm;
   struct timeval      tv;
#if HAVE_GETADDRINFO==0  &&  defined(__linux__) && HAVE_GETHOSTBYNAME_R>0
   char                buffer[1024];
   struct hostent *    result;
#endif
#if HAVE_GETADDRINFO
   struct addrinfo     addrinfo_hints;
   struct addrinfo *   addrinfo;
#endif

#if HAVE_GETADDRINFO
   addrinfo = NULL;
   addrinfo_hints.ai_flags = 0;
   addrinfo_hints.ai_family = AF_INET;
   addrinfo_hints.ai_socktype = SOCK_STREAM;
   addrinfo_hints.ai_protocol = IPPROTO_TCP;
   addrinfo_hints.ai_addrlen = 0;
   addrinfo_hints.ai_addr = NULL;
   addrinfo_hints.ai_canonname = NULL;
   addrinfo_hints.ai_next = NULL;

   ret = getaddrinfo( serverName, NULL, &addrinfo_hints, &addrinfo );
   if (ret != 0)
   {
      ufdbLogError( "cannot resolve hostname %s: %s", serverName, gai_strerror(ret) );
      return -1;
   }

   memcpy( (void *) &addr, (void *) addrinfo->ai_addr, sizeof(addr) );  
   addr.sin_family = AF_INET;
   addr.sin_port = htons( port );

   freeaddrinfo( addrinfo );

#else

#if defined(__linux__) && HAVE_GETHOSTBYNAME_R>0
   {
      struct hostent      server;
      int                 my_errno;

      /* 
       * check the server name.
       */
      my_errno = 0;
      ret = gethostbyname_r( serverName, &server, buffer, sizeof(buffer)-1, &result, &my_errno );
      if (ret != 0)
      {
	 ufdbLogError( "cannot resolve hostname %s: %s", serverName, strerror(my_errno) );
	 return -1;
      }

      addr.sin_family = AF_INET;
      memcpy( (void *) &addr.sin_addr, (void *) server.h_addr_list[0], sizeof(addr.sin_addr) );  
      addr.sin_port = htons( port );
   }

#else

#if HAVE_GETIPNODEBYNAME
   {
      struct hostent *    server_ptr;
      int                 my_errno;

      my_errno = 0;
      server_ptr = getipnodebyname( serverName, AF_INET, 0, &my_errno );
      if (server_ptr == NULL)
      {
	 ufdbLogError( "cannot resolve hostname %s: %s", serverName, strerror(my_errno) );
	 return -1;
      }

      addr.sin_family = AF_INET;
      memcpy( (void *) &addr.sin_addr, (void *) server_ptr->h_addr_list[0], sizeof(addr.sin_addr) );  
      addr.sin_port = htons( port );

      freehostent( server_ptr );
      /* #error *TEST* HAVE_GETIPNODEBYNAME   TODO */
   }

#else

   {
      struct hostent *       server_ptr;
      static pthread_mutex_t gethostbyname_mutex = UFDB_STATIC_MUTEX_INIT;

      ret = pthread_mutex_lock( &gethostbyname_mutex );
#ifdef UFDB_DEBUG
      if (ret != 0)
	 ufdbLogError( "UFDBopenSocket: mutex_lock failed" );
#endif

      /*
       * check the server name.
       */
      errno = 0;
      server_ptr = gethostbyname( serverName );
      if (server_ptr == NULL)
      {
	 ufdbLogError( "cannot resolve hostname %s: %s", serverName, hstrerror(h_errno) );
	 ret = pthread_mutex_unlock( &gethostbyname_mutex );
#ifdef UFDB_DEBUG
	 if (ret != 0)
	    ufdbLogError( "UFDBopenSocket: mutex_unlock failed" );
#endif
	 return -1;
      }

      addr.sin_family = AF_INET;
      memcpy( (void *) &addr.sin_addr, (void *) server_ptr->h_addr_list[0], sizeof(addr.sin_addr) );  
      addr.sin_port = htons( port );

      ret = pthread_mutex_unlock( &gethostbyname_mutex );
#ifdef UFDB_DEBUG
      if (ret != 0)
	 ufdbLogError( "UFDBopenSocket: mutex_unlock failed" );
#endif
   }

#endif
#endif
#endif

   /*
    * create the socket to connect to the daemon.
    */
   s = socket( AF_INET, SOCK_STREAM, 0 );
   if (s < 0)
      return -1;

   /*
    *  Prevent that the connect takes ages.  Use an aggressive timeout of 5 seconds.
    */
   tv.tv_sec = 5;
   tv.tv_usec = 0;
   setsockopt( s, SOL_SOCKET, SO_RCVTIMEO, (void *) &tv, sizeof(tv) );
   tv.tv_sec = 5;
   tv.tv_usec = 0;
   setsockopt( s, SOL_SOCKET, SO_SNDTIMEO, (void *) &tv, sizeof(tv) );

   sock_parm = 64 * 1024;
   setsockopt( s, SOL_SOCKET, SO_SNDBUF, (void *) &sock_parm, sizeof(sock_parm) );

   if (connect( s, (struct sockaddr *) &addr, sizeof(addr) ) < 0)
   {
      if (errno == EINPROGRESS)
         errno = EAGAIN;
      ufdbLogError( "cannot connect to %s port %d: %s", serverName, port, strerror(errno) );
      close( s );
      s = -1;
   }
   else
   {
#if 0
      struct linger  linger;
      linger.l_onoff = 1;
      linger.l_linger = 2;
      setsockopt( s, SOL_SOCKET, SO_LINGER, (void *) &linger, sizeof(linger) );
#endif

#if 0
      /*
       * Turn off NAGLE.
       */
      sock_parm = 1;
      setsockopt( s, IPPROTO_TCP, TCP_NODELAY, (void *) &sock_parm, sizeof(sock_parm) );
#endif

      /*
       *  Prevent long blocking on communication with the daemon.
       */
      tv.tv_sec = 20;
      tv.tv_usec = 0;
      setsockopt( s, SOL_SOCKET, SO_RCVTIMEO, (void *) &tv, sizeof(tv) );

      tv.tv_sec = 20;
      tv.tv_usec = 0;
      setsockopt( s, SOL_SOCKET, SO_SNDTIMEO, (void *) &tv, sizeof(tv) );
   }

   return s;
}


static int UFDBnormaliseIPv6( 
   const char * address,
   char *       normalised )
{
   struct in6_addr ipv6;

   *normalised = '\0';

   if (inet_pton( AF_INET6, address, (void *) &ipv6 ) <= 0)
      return 0;

   if (inet_ntop( AF_INET6, (void *) &ipv6, normalised, INET6_ADDRSTRLEN ) == NULL)
      return 0;

#ifdef UFDB_DEBUG_IPV6
   fprintf( stderr, "      UFDBnormaliseIPv6  %s  ->  %s\n", address, normalised );
#endif

   return 1;	/* successful */
}


char * UFDBparseIPv6address( 
   char * url,
   char * domain )
{
   char * url_start;
   char * d;
   char   normalisedAddress[INET6_ADDRSTRLEN];

#ifdef UFDB_DEBUG_IPV6
   fprintf( stderr, "   UFDBparseIPv6address: url: %s\n", url );
#endif

   url_start = url;
   *domain = '\0';
   d = domain;
   if (*url == '[')
   {
      url++;
   }
   
   while (*url != '\0')
   {
      if (*url == ']')
      {
	 *d++ = ']';
	 *d = '\0';
	 if (UFDBnormaliseIPv6( domain, normalisedAddress ))
	    strcpy( domain, normalisedAddress );
	 else
	 {
	    if (UFDBglobalDebug)
	        ufdbLogMessage( "URL has invalid IPv6 address: %s", *url_start=='[' ? url_start+1 : url_start );
	    return NULL;
	 }
	 /* TODO: handle IPv4 in IPv6 addresses e.g.  ::127.0.0.1 */
	 return url;
      }

      if (*url == ':'  ||  *url == '.'  ||  isxdigit(*url))
      {
         *d++ = *url++;
      }
      else	/* URL address error */
      {
	 *d = '\0';
	 if (UFDBglobalDebug)
	     ufdbLogMessage( "URL has invalid IPv6 address: %s", *url_start=='[' ? url_start+1 : url_start );
         return NULL;
      }
   }
   *d = '\0';

#ifdef UFDB_DEBUG_IPV6
   fprintf( stderr, "   UFDBparseIPv6address: domain: %s\n", domain );
#endif

   if (UFDBnormaliseIPv6( domain, normalisedAddress ))
   {
#ifdef UFDB_DEBUG_IPV6
      fprintf( stderr, "      IPv6 domain '%s' normalised to '%s'\n", domain, normalisedAddress );
#endif
      strcpy( domain, normalisedAddress );
   }
   else
   {
      if (UFDBglobalDebug)
	  ufdbLogMessage( "URL has invalid IPv6 address: %s", *url_start=='[' ? url_start+1 : url_start );
      return NULL;  /* address error */
   }

   /* TODO: handle IPv4 in IPv6 addresses e.g.  ::127.0.0.1 */
   return url;
}


void UFDBupdateURLwithNormalisedDomain(
   char * url,
   char * newDomain )
{
   char * oldURL;
   char * oldEnd;
   int    n;
   int    nbytes;

   oldURL = url;
#ifdef UFDB_DEBUG_IPV6
   fprintf( stderr, "      UFDBupdateURLwithNormalisedDomain: %s\n", url );
#endif

   if (*url != '[')
   {
      ufdbLogError( "UFDBupdateURLwithNormalisedDomain: URL does not start with '[': %s", url );
      return;
   }
   url++;

   oldEnd = strchr( url, ']' );
   if (oldEnd == NULL)
   {
      ufdbLogError( "UFDBupdateURLwithNormalisedDomain: URL does not have a ']': %s", url );
      return;
   }

   while (1)
   {
      if (*url == ']')
      {
         if (*newDomain == '\0')	/* the normalised domain name has equal length */
	    return;
	 /* the newDomain string is longer than the original */
	 n = strlen( newDomain );
	 nbytes = strlen( url ) + 1;
	 memmove( url+n, url, nbytes );
	 while (*newDomain != '\0')
	    *url++ = *newDomain++;
	 return;
      }

      if (*newDomain == '\0')
      {
         /* the newDomain string is shorter than the original */
	 nbytes = strlen( oldEnd ) + 1;
	 memmove( url, oldEnd, nbytes );
#ifdef UFDB_DEBUG_IPV6
         fprintf( stderr, "      UFDBupdateURLwithNormalisedDomain: %s\n", oldURL );
#endif
	 return;
      }

      *url++ = *newDomain++;
   }
}


/*
 * UFDBaddSafeSearch - modify a URL for a search which requires SafeSearch
 *
 * return UFDB_API_OK for unmodified URLs and UFDB_API_MODIFIED_FOR_SAFESEARCH
 *
 * parameters: domain - the domainname
 *             strippedURL - the stripped URL including the domainname
 *             originalURL - the unmodified user-supplied URL
 *	       The originalURL must be of type char[UFDB_MAX_URL_LENGTH]
 *	       and may be modified to force SafeSearch.
 */
int UFDBaddSafeSearch(
   char * domain,
   char * strippedURL,
   char * originalURL  )
{
   char * slash;

   originalURL[UFDB_MAX_URL_LENGTH-28] = '\0';

   slash = strchr( strippedURL, '/' );
   if (slash == NULL)
      strippedURL = "";
   else
      strippedURL = slash;

#if 0
   printf( "   SS: %s %s\n", domain, strippedURL );
#endif

   if (strstr( domain, "similar-images.googlelabs." ) != NULL  &&	/* Google images */
       strstr( strippedURL, "q=") != NULL)
   {
      strcat( originalURL, "&safe=active&safeui=on" );
      UFDB_API_num_safesearch++;
      return UFDB_API_MODIFIED_FOR_SAFESEARCH;
   }
   else
   if (strstr( domain, "images.google." ) != NULL  &&	/* Google images */
       strstr( strippedURL, "q=") != NULL)
   {
      strcat( originalURL, "&safe=active&safeui=on" );
      UFDB_API_num_safesearch++;
      return UFDB_API_MODIFIED_FOR_SAFESEARCH;
   }
   else
   if ((domain[0] <= '9' && domain[0] >= '0')  &&		/* google-related sites like www.google-tr.info */
       strstr( strippedURL, "cx=partner" ) != NULL  &&
       strstr( strippedURL, "/cse" ) != NULL  &&
       strstr( strippedURL, "q=" ) != NULL)
   {
      strcat( originalURL, "&safe=active" );
      UFDB_API_num_safesearch++;
      return UFDB_API_MODIFIED_FOR_SAFESEARCH;
   }
   else
   if ((strstr( domain, "google." ) != NULL  ||
        strstr( domain, "googleusercontent.com" ) != NULL)  &&		/* Google */
       strstr( strippedURL, "q=" ) != NULL  &&
       (strstr( strippedURL, "/search" ) != NULL ||
        strstr( strippedURL, "/uds/afs" ) != NULL ||
        strstr( strippedURL, "/uds/gwebsearch" ) != NULL ||
        strstr( strippedURL, "/uds/gvideosearch" ) != NULL ||
        strstr( strippedURL, "/uds/gimagesearch" ) != NULL ||
        strstr( strippedURL, "/uds/gblogsearch" ) != NULL ||
        strstr( strippedURL, "/videosearch" ) != NULL ||
        strstr( strippedURL, "/blogsearch" ) != NULL ||
        strstr( strippedURL, "/gwebsearch" ) != NULL ||
        strstr( strippedURL, "/groups" ) != NULL ||
        strstr( strippedURL, "/cse" ) != NULL ||
        strstr( strippedURL, "/products" ) != NULL ||
        strstr( strippedURL, "/images" ) != NULL ||
        strstr( strippedURL, "/custom" ) != NULL) )
   {
      char * safe;
      /* search for 'safe=off' and replace by 'safe=active' */
      safe = strstr( originalURL, "&safe=off" );
      if (safe != NULL)
      {
         safe += 6;
	 *safe++ = 'a';		/* 'o' */
	 *safe++ = 'c';		/* 'f' */
	 *safe++ = 't';		/* 'f' */
	 (void) memmove( safe+3, safe, strlen(safe)+1 );
	 *safe++ = 'i';
	 *safe++ = 'v';
	 *safe   = 'e';
      }
      strcat( originalURL, "&safe=active&safeui=on" );
      UFDB_API_num_safesearch++;
      return UFDB_API_MODIFIED_FOR_SAFESEARCH;
   }
   else
   if (strstr( domain, "webmenu.com" ) != NULL  &&		/* webmenu */
       (strstr( strippedURL, "q_or=") != NULL  ||
        strstr( strippedURL, "q_and=") != NULL  ||
        strstr( strippedURL, "ss=") != NULL  ||
        strstr( strippedURL, "keyword=") != NULL  ||
	strstr( strippedURL, "query=") != NULL) )
   {
      char * p;
      /* TODO: fix problem of cookie override; a user can set preferences to turn the filter OFF
       * in the user preferences.
       */
      while ((p = strstr( originalURL, "&ss=n" )) != NULL)
         *(p+4) = 'y';
      strcat( originalURL, "&ss=y" );
      UFDB_API_num_safesearch++;
      return UFDB_API_MODIFIED_FOR_SAFESEARCH;
   }
   else
   if (strstr( domain, "zapmeta." ) != NULL  &&			/* zapmeta */
       strstr( strippedURL, "vid=" ) != NULL  &&
       strstr( strippedURL, "q=" ) != NULL)
   {
      strcat( originalURL, "&ss=y" );
      UFDB_API_num_safesearch++;
      return UFDB_API_MODIFIED_FOR_SAFESEARCH;
   }
   else
   if (strstr( domain, "trovator.com" ) != NULL  &&		/* trovator */
       strstr( strippedURL, "q=" ) != NULL)
   {
      strcat( originalURL, "&fil=si" );
      UFDB_API_num_safesearch++;
      return UFDB_API_MODIFIED_FOR_SAFESEARCH;
   }
   else
   if (strstr( domain, ".yauba.com" ) != NULL  &&		/* yauba */
       strstr( strippedURL, "query=") != NULL)
   {
      strcat( originalURL, "&ss=y" );
      UFDB_API_num_safesearch++;
      return UFDB_API_MODIFIED_FOR_SAFESEARCH;
   }
   else
   if (strstr( domain, "forestle.org" ) != NULL  &&		/* forestle */
       (strstr( strippedURL, "settings") != NULL  ||
        strstr( strippedURL, "q=") != NULL))
   {
      strcat( originalURL, "&adultfilter=noadult" );
      UFDB_API_num_safesearch++;
      return UFDB_API_MODIFIED_FOR_SAFESEARCH;
   }
   else
   if (strstr( domain, "zombol.com" ) != NULL  &&		/* zombol */
       strstr( strippedURL, "/results") != NULL  &&
       strstr( strippedURL, "q=") != NULL)
   {
      strcat( originalURL, "&safe=active" );
      UFDB_API_num_safesearch++;
      return UFDB_API_MODIFIED_FOR_SAFESEARCH;
   }
   else
   if (strstr( domain, "kalooga.com" ) != NULL  &&		/* kalooga */
       strstr( strippedURL, "search") != NULL  &&
       strstr( strippedURL, "query=") != NULL)
   {
      strcat( originalURL, "&filter=default" );
      UFDB_API_num_safesearch++;
      return UFDB_API_MODIFIED_FOR_SAFESEARCH;
   }
   else
   if (strstr( domain, "muuler.com" ) != NULL  &&		/* muuler */
       strstr( strippedURL, "/result") != NULL  &&
       strstr( strippedURL, "q=") != NULL)
   {
      strcat( originalURL, "&safe=active" );
      UFDB_API_num_safesearch++;
      return UFDB_API_MODIFIED_FOR_SAFESEARCH;
   }
   else
   if (strstr( domain, "foozir.com" ) != NULL  &&		/* foozir */
       strstr( strippedURL, "/result") != NULL  &&
       strstr( strippedURL, "q=") != NULL)
   {
      strcat( originalURL, "&safe=active" );
      UFDB_API_num_safesearch++;
      return UFDB_API_MODIFIED_FOR_SAFESEARCH;
   }
   else
   if (strstr( domain, "moons.it" ) != NULL  &&			/* moons */
       strstr( strippedURL, "/ricerca") != NULL  &&
       strstr( strippedURL, "q=") != NULL)
   {
      strcat( originalURL, "&safe=active" );
      UFDB_API_num_safesearch++;
      return UFDB_API_MODIFIED_FOR_SAFESEARCH;
   }
   else
   if (strstr( domain, "wotbox.com" ) != NULL  &&		/* wotbox */
       (strstr( strippedURL, "q=") != NULL  ||
        strstr( strippedURL, "op0=") != NULL) )
   {
      strcat( originalURL, "&a=true" );
      UFDB_API_num_safesearch++;
      return UFDB_API_MODIFIED_FOR_SAFESEARCH;
   }
   else
   if (strstr( domain, "ripple.org" ) != NULL  &&		/* ripple */
       strstr( strippedURL, "q=") != NULL)
   {
      strcat( originalURL, "&safe=active" );
      UFDB_API_num_safesearch++;
      return UFDB_API_MODIFIED_FOR_SAFESEARCH;
   }
   else
   if (strstr( domain, "ant.com" ) != NULL  &&			/* TODO ant */
       strstr( strippedURL, "antq=") != NULL)
   {
      strcat( originalURL, "&safe=1" );
      UFDB_API_num_safesearch++;
      return UFDB_API_MODIFIED_FOR_SAFESEARCH;
   }
   else
   if (strstr( domain, "duckduckgo.org" ) != NULL  &&		/* duckduckgo */
       strstr( strippedURL, "q=") != NULL)
   {
      strcat( originalURL, "&kp=1" );
      UFDB_API_num_safesearch++;
      return UFDB_API_MODIFIED_FOR_SAFESEARCH;
   }
   else
   if (strstr( domain, "zoower.com" ) != NULL  &&		/* zoower */
       strstr( strippedURL, "q=") != NULL)
   {
      strcat( originalURL, "&safe=active" );
      UFDB_API_num_safesearch++;
      return UFDB_API_MODIFIED_FOR_SAFESEARCH;
   }
   else
   if ((strstr( domain, "qbyrd.com" ) != NULL  ||  strstr( domain, "search-results.com" ) != NULL)  &&		/* qbyrd */
       strstr( strippedURL, "q=") != NULL)
   {
      char * adt;
      adt = strstr( originalURL, "adt=1" );
      if (adt != NULL)
         *(adt+4) = '0';
      else
	 strcat( originalURL, "&adt=0" );
      UFDB_API_num_safesearch++;
      return UFDB_API_MODIFIED_FOR_SAFESEARCH;
   }
   else
   if (strstr( domain, "easysearch.org.uk" ) != NULL  &&
       strstr( strippedURL, "search") != NULL)
   {
      strcat( originalURL, "&safe=on" );
      return UFDB_API_MODIFIED_FOR_SAFESEARCH;
   }
   else
   if (strstr( domain, "dly.net" ) != NULL  &&		/* Google clone: dly.net */
       (strstr( strippedURL, "/search?") != NULL  ||  strstr( strippedURL, "/custom?") != NULL)  &&
       strstr( strippedURL, "q=") != NULL)
   {
      strcat( originalURL, "&safe=active" );
      return UFDB_API_MODIFIED_FOR_SAFESEARCH;
   }
   else
   if (strstr( domain, "bing.com" ) != NULL  &&
       strstr( strippedURL, "q=" ) != NULL)    			/* bing */
   {
      strcat( originalURL, "&ADLT=STRICT&filt=all" );
      return UFDB_API_MODIFIED_FOR_SAFESEARCH;
   }
   else
   if (strstr( domain, "ask.com" ) != NULL  &&
       strchr( strippedURL, '?' ) != NULL)    			/* ask */
   {
      strcat( originalURL, "&adt=0" );
      return UFDB_API_MODIFIED_FOR_SAFESEARCH;
   }
   else
   if (strncmp( domain, "api.search.yahoo.", 17 ) == 0  &&	/* Yahoo API */
       strstr( strippedURL, "query=" ) != NULL)
   {
      strcat( originalURL, "&adult_ok=0" );				
      return UFDB_API_MODIFIED_FOR_SAFESEARCH;
   }
   else
   if ((strstr( domain, ".terra." ) != NULL  &&  
        strstr( domain, "busca" ) != NULL)  &&	  /* terra.com */
       (strstr( strippedURL, "query=" ) != NULL  ||  
        strstr( strippedURL, "source=" ) != NULL) )	  /* .ar .br .cl .co .ec .es */
   {
      strcat( originalURL, "&npl=%26safe%3dhigh" );				
      return UFDB_API_MODIFIED_FOR_SAFESEARCH;
   }
   else
   if (strstr( domain, "searchalot.com" ) != NULL  &&	/* Searchalot */
       strstr( strippedURL, "q=" ) != NULL)
   {
      strcat( originalURL, "&safesearch=high" );				
      return UFDB_API_MODIFIED_FOR_SAFESEARCH;
   }
   else
   if (strstr( domain, "alltheinternet.com" ) != NULL  &&	/* Alltheinternet */
       strstr( strippedURL, "q=" ) != NULL)
   {
      strcat( originalURL, "&safesearch=high" );				
      return UFDB_API_MODIFIED_FOR_SAFESEARCH;
   }
   else
   if (strstr( domain, "search.yahoo." ) != NULL  &&	/* Yahoo */
       strstr( strippedURL, "p=" ) != NULL)
   {
      strcat( originalURL, "&vm=r" );				
      /* TODO: investigate http://www.yahoo.com/r/sx/ *-http://search.yahoo.com/search */
      return UFDB_API_MODIFIED_FOR_SAFESEARCH;
   }
   else
   if (strstr( domain, "excite." ) != NULL  &&
       strstr( strippedURL, "search" ) != NULL  &&
       strchr( strippedURL, '?' ) != NULL)  			/* Excite */
   {
      strcat( originalURL, "&familyfilter=1&splash=filtered" );
      return UFDB_API_MODIFIED_FOR_SAFESEARCH;
   }
   else
   if (strncmp( domain, "search.msn.", 11 ) == 0)		/* MSN */
   {
      if (slash == NULL)
         strcat( originalURL, "/" );
      strcat( originalURL, "&adlt=strict" );
      UFDB_API_num_safesearch++;
      return UFDB_API_MODIFIED_FOR_SAFESEARCH;
   }
   else
   if (strncmp( domain, "search.live.", 12 ) == 0  &&	/* Live */
       strstr( strippedURL, "q=" ) != NULL)
   {
      strcat( originalURL, "&adlt=strict" );
      UFDB_API_num_safesearch++;
      return UFDB_API_MODIFIED_FOR_SAFESEARCH;
   }
   else
   if (strcmp( domain, "api.search.live.net" ) == 0  &&	/* Live API */
       strstr( strippedURL, "sources=" ) != NULL)
   {
      strcat( originalURL, "&adlt=strict" );
      UFDB_API_num_safesearch++;
      return UFDB_API_MODIFIED_FOR_SAFESEARCH;
   }
   else
   if (strstr( domain, "yauba.co.in" ) != NULL  &&		/* yauba.co.in */
       strstr( strippedURL, "query=" ) != NULL)
   {
      strcat( originalURL, "&ss=y" );
      UFDB_API_num_safesearch++;
      return UFDB_API_MODIFIED_FOR_SAFESEARCH;
   }
   else
   if (strstr( domain, "blinkx.com" ) != NULL  &&		/* blinkx.com */
       strchr( strippedURL, '?' ) != NULL)
   {
      strcat( originalURL, "&safefilter=on" );
      UFDB_API_num_safesearch++;
      return UFDB_API_MODIFIED_FOR_SAFESEARCH;
   }
   else
   if (strncmp( domain, "busca.ya.", 9 ) == 0  &&		/* ya.com */
       strstr( strippedURL, "buscar=" ) != NULL)
   {
      strcat( originalURL, "&filtrofamiliar=Activado&safe=active" );
      UFDB_API_num_safesearch++;
      return UFDB_API_MODIFIED_FOR_SAFESEARCH;
   }
   else
   if (strncmp( domain, "search.lycos.", 13 ) == 0)		/* Lycos .com */
   {
      if (slash == NULL)
         strcat( originalURL, "/" );
      strcat( originalURL, "&contentFilter=strict&family=on" );
      UFDB_API_num_safesearch++;
      return UFDB_API_MODIFIED_FOR_SAFESEARCH;
   }
   else
   if (strncmp( domain, "suche.lycos.", 12 ) == 0)		/* Lycos .de .at  .ch */
   {
      if (slash == NULL)
         strcat( originalURL, "/" );
      strcat( originalURL, "&family=on" );
      UFDB_API_num_safesearch++;
      return UFDB_API_MODIFIED_FOR_SAFESEARCH;
   }
   else
   if (strcmp( domain, "buscador.lycos.es" ) == 0)		/* Lycos .es */
   {
      if (slash == NULL)
         strcat( originalURL, "/" );
      strcat( originalURL, "&family=on" );
      UFDB_API_num_safesearch++;
      return UFDB_API_MODIFIED_FOR_SAFESEARCH;
   }
   else
   if (strcmp( domain, "vachercher.lycos.fr" ) == 0)	/* Lycos .fr */
   {
      if (slash == NULL)
         strcat( originalURL, "/" );
      strcat( originalURL, "&family=on" );
      UFDB_API_num_safesearch++;
      return UFDB_API_MODIFIED_FOR_SAFESEARCH;
   }
   else
   if (strcmp( domain, "cerca.lycos.it" ) == 0)		/* Lycos .it */
   {
      if (slash == NULL)
         strcat( originalURL, "/" );
      strcat( originalURL, "&family=on" );
      UFDB_API_num_safesearch++;
      return UFDB_API_MODIFIED_FOR_SAFESEARCH;
   }
   else
   if (strstr( domain, "webpile.it" ) != NULL  &&		/* webpile.it */
       strncmp( strippedURL, "search", 6 ) == 0)
   {
      strcat( originalURL, "&filtro=4" );
      UFDB_API_num_safesearch++;
      return UFDB_API_MODIFIED_FOR_SAFESEARCH;
   }
   else
   if (strcmp( domain, "alltheweb.com" ) == 0)		/* alltheweb */
   {
      if (slash == NULL)
         strcat( originalURL, "/" );
      strcat( originalURL, "&copt_offensive=on&nooc=on" );
      UFDB_API_num_safesearch++;
      return UFDB_API_MODIFIED_FOR_SAFESEARCH;
   }
   else
   if (strstr( domain, "dogpile.com" ) != NULL  ||
       strstr( domain, "dogpile.co.uk" ) != NULL)		/* dogpile */
   {
      if (slash == NULL)
         strcat( originalURL, "/" );
      strcat( originalURL, "&adultfilter=heavy" );
      UFDB_API_num_safesearch++;
      return UFDB_API_MODIFIED_FOR_SAFESEARCH;
   }
   else
   if (strcmp( domain, "a9.com" ) == 0)			/* A9 */
   {
      if (slash == NULL)
         strcat( originalURL, "/" );
      strcat( originalURL, "&qsafe=high" );
      UFDB_API_num_safesearch++;
      return UFDB_API_MODIFIED_FOR_SAFESEARCH;
   }
   else
   if (strcmp( domain, "hotbot.com" ) == 0)			/* hotbot */
   {
      if (slash == NULL)
         strcat( originalURL, "/" );
      strcat( originalURL, "&adf=on&family=on" );
      UFDB_API_num_safesearch++;
      return UFDB_API_MODIFIED_FOR_SAFESEARCH;
   }
   else
   if (strstr( domain, "infospace.com" ) != NULL)		/* infospace */
   {
      if (slash == NULL)
         strcat( originalURL, "/" );
      strcat( originalURL, "&familyfilter=1" );
      UFDB_API_num_safesearch++;
      return UFDB_API_MODIFIED_FOR_SAFESEARCH;
   }
   else
   if (strstr( domain, "metacrawler.com" ) != NULL)		/* metacrawler */
   {
      if (slash == NULL)
         strcat( originalURL, "/" );
      strcat( originalURL, "&familyfilter=1" );
      UFDB_API_num_safesearch++;
      return UFDB_API_MODIFIED_FOR_SAFESEARCH;
   }
   else
   if (strstr( domain, "metaspy.com" ) != NULL)		/* metaspy */
   {
      if (slash == NULL)
         strcat( originalURL, "/" );
      strcat( originalURL, "&familyfilter=1" );
      UFDB_API_num_safesearch++;
      return UFDB_API_MODIFIED_FOR_SAFESEARCH;
   }
   else
   if (strstr( domain, "webfetch.com" ) != NULL  ||		/* webfetch */
       strstr( domain, "webfetch.co.uk" ) != NULL)
   {
      if (slash == NULL)
         strcat( originalURL, "/" );
      strcat( originalURL, "&familyfilter=1" );
      UFDB_API_num_safesearch++;
      return UFDB_API_MODIFIED_FOR_SAFESEARCH;
   }
   else
   if (strstr( domain, "webcrawler.com" ) != NULL)		/* webcrawler */
   {
      if (slash == NULL)
         strcat( originalURL, "/" );
      strcat( originalURL, "&familyfilter=1" );
      UFDB_API_num_safesearch++;
      return UFDB_API_MODIFIED_FOR_SAFESEARCH;
   }
   else
   if (strstr( domain, "buscamundo.com" ) != NULL  &&	/* buscamundo */
       strstr( strippedURL, "qu=") != NULL)
   {
      strcat( originalURL, "&filter=on" );
      UFDB_API_num_safesearch++;
      return UFDB_API_MODIFIED_FOR_SAFESEARCH;
   }
   else
   if (strncmp( domain, "search", 6 ) == 0  &&
       strstr( domain, "foxnews.com" ) != NULL)		/* foxnews */
   {
      if (slash == NULL)
         strcat( originalURL, "/" );
      strcat( originalURL, "&familyfilter=1" );
      UFDB_API_num_safesearch++;
      return UFDB_API_MODIFIED_FOR_SAFESEARCH;
   }
   else
   if (strstr( domain, "altavista.com" ) != NULL  &&	/* altavista */
       (strstr( strippedURL, "q=" ) != NULL  ||  strstr( strippedURL, "aqa=" ) != NULL))
   {
      char * dis;
      dis = strstr( strippedURL, "dis=1" );
      if (dis != NULL)						/* replace dis=1 by dis=0 */
         *(dis+4) = '0';
      else
	 strcat( originalURL, "&dis=0" );				/* OR add "dis=0" */
      UFDB_API_num_safesearch++;
      return UFDB_API_MODIFIED_FOR_SAFESEARCH;
   }
   /* TODO: investigate altavista filter */

   return UFDB_API_OK;
}


static __inline__ int squeeze_html_char( 
   char * p,  
   int *  hex )
{
   int    length;

   length = 0;
   *hex = 0;
   while (*p != '\0'  &&  isxdigit( (int) *p ))
   {
      int h;
      h = (*p <= '9') ? *p - '0' : *p - 'a' + 10;
      *hex = *hex * 16 + h;
      p++;
      length++;
   }

#if 0
   fprintf( stderr, "   squeeze_html_char hex=%04x  length=%d  *p=%c\n", *hex, length, *p );
#endif

   if (*p != ';')
      return -1;		/* '&#xxx' without trailing ';' is not a valid HTML character */

   if (*hex == 0)
      return length;

   if (*hex < 0x0020)
   {
      if (*hex != '\t'  &&  *hex != '\n'  &&  *hex != '\r'  &&  *hex != '\f')
	 *hex = ' ';
   }
   else if (*hex == 0x007f  ||  *hex >= 0x00ff)
   {
      *hex = ' ';
   }
   else if (*hex <= 'Z'  &&  *hex >= 'A')
   {
      *hex += 'a' - 'A';
   }

   return length;
}


/*
 * strip a URL:
 * remove http:// prefix, 
 * remove www[0-9*]. prefix,
 * remove port number, 
 * remove username and password,
 * remove IP address obfuscations (numbers with leading zeroes)
 * convert hex codes (%61 = 'a') to characters,
 * convert HTML character codes (&#61; = 'a') to characters,
 * convert special characters to void or space,
 * convert characters to lower case.
 * substitute // by / in a URL
 * substitute /./ by / in a URL
 * substitute /foo/../bar by /bar in a URL
 */
void UFDBstripURL( 
   char * URL, 			/* input URL string */
   char * strippedUrl,  	/* output char array (must be UFDB_MAX_URL_LENGTH bytes) */
   char * domain,       	/* output char array (must be 1024 bytes) */
   char * protocol,		/* output char array (must be 16 bytes) */
   int  * portnumber )		/* output integer */
{
   char * up;
   char * p;
   char * tmp;
   char * domain_start;
   char * domain_end;
   char * optional_token;
   char * origStrippedUrl;
   int    is_ip_address;
   int    is_ipv6;
   int    obfuscated;
   char   buffer[UFDB_MAX_URL_LENGTH];

   *portnumber = 80;
   is_ipv6 = 0;

   UFDB_API_num_url_lookups++;

   /* strip http: and ftp: protocol header */
   p = strstr( URL, "://" );
   if (p != NULL)
   {
      int n;
      n = p - URL;
      if (n > 15)
      {
	 /* WHOEHA a very large protocol name... truncate it! */
         n = 15;
      }
      memcpy( protocol, URL, n );
      protocol[n] = '\0';
      if (n == 5  &&  strcasecmp( protocol, "https" ) == 0)
         *portnumber = 443;
      p += 3;
   }
   else 
   {
      p = URL;
      strcpy( protocol, "http" );
   }

   domain_end = strchr( p, '/' );

   optional_token = strchr( p, '@' );		/* strip user:password@ */
   if (optional_token != NULL)
   {
      if (optional_token < domain_end  ||  domain_end == NULL)
         p = optional_token + 1;
   }

   domain_start = p;

#if 0
   fprintf( stderr, "   UFDBstripURL: p: %s\n", p );
#endif

   if (*p == '[')			/* IPv6 URL: http://[::1]:80/index.html */
   {
      char * end;
      char * oldBracket;

      is_ipv6 = 1;
      oldBracket = strchr( p, ']' );
      if (oldBracket != NULL)
         *oldBracket = '\0';
      end = UFDBparseIPv6address( p, domain );
      if (oldBracket != NULL)
         *oldBracket = ']';
      if (end != NULL)
      {
	 UFDBupdateURLwithNormalisedDomain( p, domain );
	 /* uh-oh: the normalised domain is usually smaller and our pointers have moved */
	 domain_end = strchr( p, '/' );
	 oldBracket = strchr( p, ']' );
	 if (oldBracket == NULL)
	    oldBracket = domain_end - 1;

	 optional_token = strchr( oldBracket, ':' );		
#if 0
         fprintf( stderr, "    UFDBstripURL:  domain_end: %08x oldBracket: %08x  token: %08x\n", 
	          domain_end, oldBracket, optional_token );
#endif
      }
      else
	 optional_token = NULL;
   }
   else
   { 					/* strip www[0-9]. */
      if ((p[0] == 'w' && p[1] == 'w' && p[2] == 'w')  ||           /* match "www" */
	  (p[0] == 'f' && p[1] == 't' && p[2] == 'p'))              /* match "ftp" */
      {
	 tmp = p + 3;
	 while (*tmp <= '9' && *tmp >= '0')
	    tmp++;
	 if (*tmp == '.'  &&  strchr( tmp+1, '.' ) != NULL)
	    p = tmp + 1;
      }
      optional_token = strchr( p, ':' );
   }

   						/* parse&strip :<portnum> */
   if (optional_token != NULL  &&
       (domain_end == NULL  ||  optional_token < domain_end))
   {
      tmp = buffer;				/* copy domain name */
      while (p < optional_token)
         *tmp++ = *p++;
      *tmp = '\0';

      p++;					/* parse :<portnum> */
      *portnumber = 0;
      while (*p <= '9' && *p >= '0')
      {
	 *portnumber = *portnumber * 10 + (*p - '0');
         p++;
      }

      memccpy( tmp, p, '\0', UFDB_MAX_URL_LENGTH-16-6-1 );	/* copy rest of the URL */
   }
   else
   {
      memccpy( buffer, p, '\0', UFDB_MAX_URL_LENGTH-16-1 );
   }
   buffer[UFDB_MAX_URL_LENGTH-1] = '\0';	/* maximise the length of the URL */

   if (!is_ipv6)
   {
      						/* save the original domainname */
      if (domain_end == NULL)			
      {
	 if (optional_token == NULL)
	    strcpy( domain, domain_start );
	 else
	 {
	    int n;
	    n = optional_token - domain_start;
	    memcpy( domain, domain_start, n );
	    domain[n] = '\0';
	 }
      }
      else
      {
	 int n;
	 if (optional_token != NULL)
	    domain_end = optional_token;
	 n = domain_end - domain_start;
	 memcpy( domain, domain_start, n );
	 domain[n] = '\0';
      }
   }

   /*
    * Now a temporary URL is in the buffer.
    * The temporary URL has no protocol, portnum, username/password.
    */
   up = buffer;
   while (*up != '\0')				/* convert URL to lower case */
   {
      if (*up <= 'Z'  &&  *up >= 'A')
	 *up += 'a' - 'A';
      up++;
   }
   *up++ = '\0';				/* prevent problems with % at end of URL */
   *up = '\0';
   
   /* scan for IP address obfuscations */
   obfuscated = (buffer[0] == '0');
   is_ip_address = 1;
   for (tmp = buffer;  *tmp != '\0' && *tmp != '/';  tmp++)
   {
      if (*tmp != '.'  &&  !(*tmp >= '0'  && *tmp <= '9'))
      {
         is_ip_address = 0;
	 break;
      }
      if (*tmp == '.'  &&  *(tmp+1) == '0')
         obfuscated = 1;
   }
   if (is_ip_address  &&  obfuscated)
   {
      char * d;
      char * s;

      /* rewrite the URL to remove the obfuscation */
      d = s = buffer;
      /* remove obfuscating leading zeroes */
      while (*s == '0'  &&  *(s+1) != '.'  &&  *(s+1) != '/'  &&  *(s+1) != '\0')
         s++;
      while (*s != '\0'  &&  *s != '/')
      {
         if (*s == '.'  &&  *(s+1) == '0')
	 {
	    *d++ = '.';
	    s++;
	    /* remove obfuscating zeroes */
	    while (*s == '0'  &&  *(s+1) != '.'  &&  *(s+1) != '/'  &&  *(s+1) != '\0')
	       s++;
	 }
	 else
	 {
	    *d = *s;
	    d++;
	    s++;
	 }
      }
      /* copy the URI */
      while (*s != '\0')
      {
         *d = *s;
	 d++;
	 s++;
      }
      *d = '\0';
   }

   /*
    *  Copy the buffer to strippedUrl, while converting hex codes to characters.
    */
   origStrippedUrl = strippedUrl;
   p = buffer;
   while (*p != '\0')
   {
      if (*p == '%')				/* start of a HEX code */
      {
         if (isxdigit(*(p+1)) && isxdigit(*(p+2)))
	 {
	    char   h;
	    int    hex;

	    h = *(p+1);
	    hex  = (h <= '9') ? h - '0' : h - 'a' + 10;
	    hex *= 16;
	    h = *(p+2);
	    hex += (h <= '9') ? h - '0' : h - 'a' + 10;
	    /* be careful with control characters */
	    if (hex < 0x20)
	    {
	       if (hex == 0)
	       {
	          p += 3;
		  continue;
	       }
	       if (hex != '\t'  &&  hex != '\r'  &&  hex != '\n'  &&  hex != '\f')
	          hex = ' ';
	       *strippedUrl++ = hex;
	       p += 3;
	    }
	    else
	    {
	       if (hex <= 'Z'  &&  hex >= 'A')
		  hex += 'a' - 'A';
	       else if (hex == 0x7f  ||  hex == 0xff)
	          hex = ' ';

	       *strippedUrl++ = hex;
	       p += 3;
	    }
	 }
	 else 					/* erroneous code */
	 {
	    *strippedUrl++ = *p++;		/* just copy one character */
	 }
      }
      else if (*p == '&'  &&  *(p+1) == '#')	/* start of HTML character code */
      {
         int  hex;
	 int  length;

	 length = squeeze_html_char( p+2, &hex );
	 if (length >= 0)
	 {
	    if (hex != 0)
	       *strippedUrl++ = hex;
	    p += length + 3;
	 }
	 else					/* not a valid HTML character code */
	 {
	    *strippedUrl++ = *p++;              /* just copy one character */
	 }
      }
      else					/* plain character */
      {
	 while (*p == '/')
	 {
	    if (*(p+1) == '/')					/* substitute // by / */
	       p++;
	    else if (*(p+1) == '.'  && *(p+2) == '/')		/* substitute /./ by / */
	       p += 2;
	    else if (*(p+1) == '.'  &&  *(p+2) == '.'  &&  *(p+3) == '/')    /* substitute /xxxx/../ by / */
	    {
	       /* try to find the previous directory... */
	       char * tmp;
	       tmp = strippedUrl - 1;
	       while (*tmp != '/'  &&  tmp > origStrippedUrl)
		  tmp--;
	       if (tmp > origStrippedUrl)
	       {
		  strippedUrl = tmp;
		  p += 3;
	       }
	       else
		  break;
	    }
	    else
	       break;
	 }
         *strippedUrl++ = *p++;
      }
   }
   *strippedUrl = '\0';
}


char * UFDBprintable( char * string )
{
   char * p;

   if (string == NULL)
      return "NULL";

   p = string;
   while (*p != '\0')
   {
      if (*p < 32  ||  *p > 126)
         *p = '?';
      p++;
   }

   return string;
}


char * UFDBfgets( 
   char * requestBuffer, 
   int    bufferSize, 
   FILE * fp )
{
   char * b;
   int    ch;
   int    size;

   b = requestBuffer;
   size = 1;

   while ((ch = getc_unlocked(fp)) != EOF)
   {
      *b++ = ch;
      if (ch == '\n')
         goto end;
      if (++size == bufferSize)
         goto end;
   }

   if (b == requestBuffer  &&  (feof(fp) || ferror(fp)))
      return NULL;

end:
   *b = '\0';
   return requestBuffer;
}


/*
 * Setting functions
 */
void ufdbSetting( 
   char * name, 
   char * value )
{
   struct ufdbSetting * sp;

   if (strcmp( name, "administrator" ) == 0)
   {
      char * p;

      while ((p = strchr( value, '?' )) != NULL)
         *p = '_';
      while ((p = strchr( value, '&' )) != NULL)
         *p = '_';
   }

   if (strcmp( name, "port" ) == 0)
   {
      UFDBglobalPortNum = atoi( value );
      ufdbFree( value );
      if (UFDBglobalPortNum <= 0)
      {
         ufdbLogError( "port number must be > 0, using default port %d", UFDB_DAEMON_PORT );
	 UFDBglobalPortNum = UFDB_DAEMON_PORT;
      }
      return;
   }

   if (Setting != NULL)
   {
      if (ufdbSettingFindName(name) != NULL)
      {
         ufdbLogFatalError( "%s: setting %s is defined multiple times in configuration file %s",
		            progname, name, UFDBglobalConfigFile );
	 ufdbFree( value );
         return;
      }
   }

   sp = (struct ufdbSetting *) ufdbMalloc( sizeof(struct ufdbSetting) );
   sp->name = ufdbStrdup( name );
   sp->value = value;
   sp->next = NULL;

   if (Setting == NULL) 
   {
      Setting = sp;
      lastSetting = sp;
   }
   else
   {
      lastSetting->next = sp;
      lastSetting = sp;
   }

   if (strcmp(name,"dbhome") == 0) 
   {
      struct stat dirbuf;

      if (stat( value, &dirbuf ) != 0)
      {
         ufdbLogError( "dbhome: directory does not exist or "
	               "access rights are insufficient (value is \"%s\")", 
		       value );
      }
      else
      {
         if (!S_ISDIR(dirbuf.st_mode))
	    ufdbLogError( "dbhome: %s is not a directory", value );
      }
   }

   if (strcmp(name,"logdir") == 0) 
   {
      UFDBglobalLogDir = ufdbStrdup( value );
      ufdbSetGlobalErrorLogFile();
      ufdbLogMessage( "configuration file: %s", UFDBglobalConfigFile );
   }
}


struct ufdbSetting * ufdbSettingFindName( 
   char * name )
{
   struct ufdbSetting * p;

   for (p = Setting; p != NULL; p = p->next)
   {
      if (strcmp(name,p->name) == 0)
         return p;
   }
   return NULL;
}


char * ufdbSettingGetValue( 
   char * name )
{
   struct ufdbSetting * p;

   p = ufdbSettingFindName( name );
   if (p != NULL)
      return p->value;
   return NULL;
}


void ufdbGetSysInfo( 
   struct utsname * si )
{
   if (uname( si ) != 0)
   {
      strcpy( si->machine, "M?" );
      strcpy( si->release, "R?" );
      strcpy( si->nodename, "unknown" );
      strcpy( si->sysname, "sysname" );
   }
   else
   {
      si->machine[ sizeof(si->machine)-1 ] = '\0';
      si->release[ sizeof(si->release)-1 ] = '\0';
      si->nodename[ sizeof(si->nodename)-1 ] = '\0';
      si->sysname[ sizeof(si->sysname)-1 ] = '\0';
   }
}


long ufdbGetNumCPUs( void ) 
{
   long num_cpus;

#if defined(_SC_NPROCESSORS_ONLN)
   num_cpus = sysconf( _SC_NPROCESSORS_ONLN );

#elif defined(__NR_sched_getaffinity)
   /* sched_setaffinity() is buggy on linux 2.4.x so we use syscall() instead */
   cpu = syscall( __NR_sched_getaffinity, getpid(), 4, &cpu_mask );
   /* printf( "sched_getaffinity returned %d %08lx\n", cpu, cpu_mask ); */
   if (cpu >= 0)
   {
      num_cpus = 0;
      for (cpu = 0; cpu < 32; cpu++)
         if (cpu_mask & (1 << cpu))
            num_cpus++;
      /* printf( "   found %d CPUs in the cpu mask\n", num_cpus ); */
   }
   else
#else
      num_cpus = 0;
#endif

   return num_cpus;
}

