/*
 * ufdbHashtable.h - URLfilterDB
 *
 * ufdbGuard is copyrighted (C) 2005-2009 by URLfilterDB with all rights reserved.
 *
 * Parts of the ufdbGuard daemon are based on squidGuard.
 * This module is entirely written by URLfilterDB.
 *
 * RCS $Id: ufdbHashtable.h,v 1.1 2011/06/10 13:31:16 root Exp root $
 */

#ifndef UFDB_UFDBHASHTABLE_H_INCLUDED
#define UFDB_UFDBHASHTABLE_H_INCLUDED

#include <pthread.h>


struct UFDBhte
{
   void *            key;
   void *            value;
   unsigned int      hash;
   struct UFDBhte *  next;
};

struct UFDBhashtable
{
   pthread_mutex_t   mutex;
   unsigned int      tableSize;
   struct UFDBhte ** table;
   unsigned int      nEntries;
   unsigned int      optimalMaxEntries;
   unsigned int      (*hashFunction)( void * );
   int               (*keyEqFunction)( void *, void * );
};


struct UFDBhashtable *
UFDBcreateHashtable( 
   unsigned int           tableSize,
   unsigned int           (*hashFunction)( void * ),
   int                    (*keyEqFunction)( void *, void * ) );

void
UFDBinsertHashtable( 
   struct UFDBhashtable * ht, 
   void *                 key, 
   void *                 value,
   int                    lockSetBySearch );

void *
UFDBsearchHashtable( 
   struct UFDBhashtable * ht,
   void *                 key,
   int                    keepLockForInsert );


void * 
UFDBremoveHashtable( 
   struct UFDBhashtable * ht,
   void *                 key );


void
UFDBdestroyHashtable( 
   struct UFDBhashtable * ht,
   int                    freeValues );

void
UFDBunlockHashtable( 
   struct UFDBhashtable * ht  );

#endif 

