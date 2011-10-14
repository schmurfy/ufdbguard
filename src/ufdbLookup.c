/*
 * ufdbLookup.c - URLfilterDB
 *
 * ufdbGuard is copyrighted (C) 2005-2009 by URLfilterDB with all rights reserved.
 * 
 * Parts of the ufdbGuard daemon are based on squidGuard.
 * squidGuard is copyrighted (C) 1998 by
 * ElTele Øst AS, Oslo, Norway, with all rights reserved.
 *
 * $Id: ufdbLookup.c,v 1.14 2011/01/20 12:49:07 root Exp root $
 */

#include "sg.h"
#include "ufdb.h"
#include "ufdblib.h"

#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

#define DATABASE        NULL

extern int     globalUpdate;
extern char *  globalCreateDb;
extern char ** globalArgv;

/*
 * Initialise a DB (open and/or create a .ufdb file) 
 */
void sgDbInit( 
   struct sgDb *         Db, 
   char *                file )
{
   struct UFDBmemTable * tab;
   int                   n;
   int                   in;
   struct stat           fileStat;
   struct stat           basefileStat;
   char                  f[1024];

   if (file == NULL)
   {
      ufdbLogError( "  sgDbInit( file = NULL )" );
      Db->dbcp = NULL;
      Db->entries = -1;
      return;
   }

#if defined(UFDB_DEBUG)
   ufdbLogError( "  sgDbInit( 0x%08x, %s )", Db, file );
#endif

   sprintf( f, "%s%s", file, UFDBfileSuffix );
   in = open( f, O_RDONLY );
   if (in < 0)
   {
      ufdbLogFatalError( "cannot read from \"%s\" (read-only): %s", f, strerror(errno) );
      Db->dbcp = NULL;
      Db->entries = -1;
      return;
   }

   if (fstat(in,&fileStat) < 0)
   {
      ufdbLogFatalError( "fstat failed on \"%s\": %s", file, strerror(errno) );
   }

   /*
    * Sanity check on date of .ufdb file
    */
   if (stat( file, &basefileStat ) == 0)
   {
      if (basefileStat.st_mtime > fileStat.st_mtime)
         ufdbLogMessage( "WARNING: %s is newer than %s. You must run ufdbGenTable *****", file, f );
   }

   tab = ufdbMalloc( sizeof(struct UFDBmemTable) );
   if (tab == NULL)
   {
      fprintf( stderr, "cannot allocate %ld bytes memory for table header *****\n", (long) sizeof(struct UFDBmemTable) );
      exit( 1 );
   }
   tab->table.tag = (unsigned char *) "UNIVERSE";
   tab->table.nNextLevels = 0;
   tab->table.nextLevel = NULL;
   tab->mem = ufdbMalloc( fileStat.st_size + 1 );
   if (tab == NULL)
   {
      fprintf( stderr, "cannot allocate %ld bytes memory for table %s *****\n", (long) fileStat.st_size+1, file );
      exit( 1 );
   }
   tab->tableSize = fileStat.st_size - sizeof(struct UFDBfileHeader);
   n = read( in, tab->mem, fileStat.st_size );
   if (n != fileStat.st_size)
   {
      fprintf( stderr, "read failed on \"%s\" n=%d st_size=%ld %s\n", file, n, (long) fileStat.st_size, strerror(errno) );
      exit( 1 );
   }
   close( in );

   n = UFDBparseTableHeader( tab );
   if (n == UFDB_API_OK)
      UFDBparseTable( tab );
   else
   {
      fprintf( stderr, "table \"%s\" could not be parsed. error code = %d\n", file, n );
      ufdbLogFatalError( "table \"%s\" could not be parsed. error code = %d\n", file, n );
      /* TODO: do not use exit() */
      exit( 1 );
   }

   Db->dbcp = (void *) tab;
   Db->entries = tab->numEntries;
}


/*
 *  sgDbLookup: lookup an item in a in-memory database.
 */
int sgDbLookup( struct sgDb * Db, char * request, char ** retval )
{
#if 0
  ufdbLogError( "  sgDbLookup( 0x%08x, %s, 0x%08x )", Db, request, retval );
#endif

  return UFDBlookup( NULL, (struct UFDBmemTable *) Db->dbcp, request );
}


/* 
 * update a database (INSERT a new key).
 */
struct UFDBmemDB * UFDBmemDBinit( void )
{
   struct UFDBmemDB * m;

   m = ufdbMalloc( sizeof(struct UFDBmemDB) );
   m->key = NULL;
   m->value = NULL;
   m->next = NULL;

   return m;
}


void UFDBmemDBinsert( 
   struct UFDBmemDB * db, 
   char * key, 
   char * value, 
   int    length )
{
   struct UFDBmemDB * prev;

#if 0
   ufdbLogError( "  UFDBmemDBinsert( db=%08x, %s, XXX, %d )", db, key, length );
#endif

   if (key == NULL)
   {
      ufdbLogError( "UFDBmemDBinsert: key is NULL. value is %s  *****", value==NULL ? "NULL" : value );
      return;
   }
   if (*key == '\0')
   {
      ufdbLogError( "UFDBmemDBinsert: key is empty. value is %s  *****", value==NULL ? "NULL" : value );
      return;
   }

   if (db->key == NULL)		/* is the list empty ? */
   {
      db->key = ufdbStrdup( key );
      if (value == NULL || length == 0)
      {
         db->value = NULL;
	 db->length = 0;
      }
      else
      {
	 db->value = ufdbMalloc( length );
	 db->length = length;
	 memcpy( db->value, (void *) value, length );
      }
   }
   else
   {				/* search end of list or key */
      prev = NULL;		
      while (db != NULL  &&  strcmp( db->key, key ) != 0)
      {
	 prev = db;
         db = db->next;
      }

      if (db == NULL)		/* we are at the end, key was not found */
      {
         db = ufdbMalloc( sizeof(struct UFDBmemDB) );
	 db->key = ufdbStrdup( key );
	 if (value == NULL || length == 0)
	 {
	    db->value = NULL;
	    db->length = 0;
	 }
	 else
	 {
	    db->value = ufdbMalloc( length );
	    db->length = length;
	    memcpy( db->value, (void *) value, length );
	 }
	 db->next = NULL;
	 prev->next = db;
      }
      else			/* key was found, overwrite value */
      {
	 if (value == NULL || length == 0)
	 {
	    ufdbFree( db->value );
	    db->value = NULL;
	    db->length = 0;
	 }
	 else
	 {
	    if (db->length != length)
	       db->value = ufdbRealloc( db->value, length );
	    db->length = length;
	    memcpy( db->value, (void *) value, length );
	 }
      }
   }
}


int UFDBmemDBfind( 
   struct UFDBmemDB * db,
   char *  key,
   char ** value  )
{
#if 0
   ufdbLogError( "  UFDBmemDBfind( db=%08x, %s )", db, key );
#endif

   if (db == NULL || db->key == NULL)
      return 0;

   while (db != NULL)
   {
      if (strcmp( db->key, key ) == 0)
      {
	 if (*value != NULL)
	    *value = db->value;
         return 1;
      }

      db = db->next;
   }

   return 0;
}


void UFDBmemDBdeleteDB( 
   struct UFDBmemDB * db  )
{
   struct UFDBmemDB * tmp;

   while (db != NULL)
   {
      ufdbFree( db->key );
      ufdbFree( db->value );
      tmp = db->next;
      ufdbFree( db );
      db = tmp;
   }
}



void UFDBmemDBprintUserDB(
   struct UFDBmemDB * db  )
{
   while (db != NULL)
   {
      ufdbLogMessage( "   ++ %s ++", db->key );
      db = db->next;
   }
}

