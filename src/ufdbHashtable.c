/*
 * ufdbHashtable.c - URLfilterDB
 *
 * ufdbGuard is copyrighted (C) 2005-2009 by URLfilterDB with all rights reserved.
 *
 * Parts of the ufdbGuard daemon are based on squidGuard.
 * This module is entirely written by URLfilterDB.
 *
 * RCS $Id: ufdbHashtable.c,v 1.1 2011/06/10 13:30:45 root Exp root $
 */

#include "ufdb.h"
#include "ufdblib.h"
#include "ufdbchkport.h"
#include "httpsQueue.h"
#include "ufdbHashtable.h"

#include <strings.h>
#include <string.h>
#include <stdlib.h>


static void expandHashtable(
   struct UFDBhashtable *  ht  )
{
   unsigned int            newTableSize;
   unsigned int            i, j;
   struct UFDBhte *        hte;
   struct UFDBhte **       newtable;

   /* Whenever expeandHashtable is called, the ht->mutex is already locked,
    * so we do not use it in this function.
    */

   newTableSize = ht->tableSize * 2 - 1;
   newtable = ufdbCalloc( sizeof(struct UFDBhte*), newTableSize );
   if (newtable == NULL)
   {
      /* Oh my god, we ran out of memory.
       * Well, we just return without expanding the table
       * since the memory allocation error is in this stage
       * not severe and the application can go on.
       */

      /* horrible workaround to prevent calling expandTable all the time: */
      ht->optimalMaxEntries = (unsigned int) (ht->tableSize * 0.88);

      return;
   }

   for (i = 0; i < ht->tableSize; i++)
   {
      while ((hte = ht->table[i]) != NULL)
      {
	 ht->table[i] = hte->next;

	 /* put the entry in the new table (at the new position) */
	 j = hte->hash % newTableSize;
	 hte->next = newtable[j];
	 newtable[j] = hte;
      }
   }

   ht->tableSize = newTableSize;
   ht->optimalMaxEntries = (unsigned int) (newTableSize * 0.68);

   ufdbFree( ht->table );
   ht->table = newtable;
}

   
struct UFDBhashtable *
UFDBcreateHashtable( 
   unsigned int           tableSize,
   unsigned int           (*hashFunction) (void*),
   int                    (*keyEqFunction) (void*,void*) )
{
   struct UFDBhashtable * ht;

   ht = ufdbMalloc( sizeof( struct UFDBhashtable ) );
   if (ht == NULL)
      return NULL;

   if (tableSize < 23)
      tableSize = 23;
   ht->tableSize = tableSize;
   ht->table = ufdbCalloc( sizeof(struct UFDBhte*), tableSize );
   if (ht->table == NULL)
      return NULL;
   ht->nEntries = 0;
   ht->optimalMaxEntries = (unsigned int) (tableSize * 0.68);
   ht->hashFunction = hashFunction;
   ht->keyEqFunction = keyEqFunction;
   pthread_mutex_init( &(ht->mutex), NULL );

   return ht;
}


void
UFDBinsertHashtable( 
   struct UFDBhashtable * ht,
   void *                 key,
   void *                 value,
   int                    lockSetBySearch )
{
   unsigned int           i;
   struct UFDBhte *       hte;

   hte = ufdbMalloc( sizeof(struct UFDBhte) );
   if (hte == NULL)
      return;
   hte->hash = ht->hashFunction( key );
   hte->key = key;
   hte->value = value;

   if (!lockSetBySearch)
   {
      pthread_mutex_lock( &ht->mutex );
   }

      i = hte->hash % ht->tableSize;
      hte->next = ht->table[i];
      ht->table[i] = hte;

      ht->nEntries++;
      if (ht->nEntries > ht->optimalMaxEntries)
	 expandHashtable( ht );

   pthread_mutex_unlock( &ht->mutex );
}


void UFDBunlockHashtable( 
   struct UFDBhashtable * ht  )
{
   pthread_mutex_unlock( &ht->mutex );
}


void *
UFDBsearchHashtable( 
   struct UFDBhashtable * ht,
   void *                 key,
   int                    keepLockForInsert )
{
   unsigned int  	  hash;
   unsigned int  	  i;
   void *                 retval;
   struct UFDBhte  *      hte;

   retval = NULL;
   hash = ht->hashFunction( key );

   pthread_mutex_lock( &ht->mutex );

      i = hash % ht->tableSize;
      hte = ht->table[i];
      while (hte != NULL)
      {
	 if (hte->hash == hash  &&  ht->keyEqFunction(key,hte->key))
	 {
	    retval = hte->value;
	    break;
	 }
	 hte = hte->next;
      }

   if ( !(retval == NULL  &&  keepLockForInsert) )
   {
      pthread_mutex_unlock( &ht->mutex );
   }

   return retval;
}


void * 
UFDBremoveHashtable( 
   struct UFDBhashtable * ht,
   void *                 key )
{
   unsigned int           hash;
   unsigned int           i;
   struct UFDBhte *       hte;
   struct UFDBhte **      head_hte;
   void *                 retval;

   retval = NULL;
   hash = ht->hashFunction( key );

   pthread_mutex_lock( &ht->mutex );

      i = hash % ht->tableSize;
      head_hte = &( ht->table[i] );
      hte = *head_hte;
      while (hte != NULL)
      {
	 if (hte->hash == hash  &&  ht->keyEqFunction(key,hte->key))
	 {
	    *head_hte = hte->next;
	    retval = hte->value;
	    ufdbFree( hte->key );
	    ufdbFree( hte );
	    ht->nEntries--;
	    break;
	 }
	 head_hte = &hte->next;
	 hte = hte->next;
      }

   pthread_mutex_unlock( &ht->mutex );

   return retval;
}


void
UFDBdestroyHashtable( 
   struct UFDBhashtable * ht,
   int                    freeValues )
{
   unsigned int           i;
   struct UFDBhte *       hte;
   struct UFDBhte *       next;

   /* Hmmm. The table will be destroyed and it is not
    * very useful to use the mutex.
    * But we use the mutex anyway to wait for any
    * current use of the table to terminate.
    */
   pthread_mutex_lock( &ht->mutex );

      for (i = 0; i < ht->tableSize; i++)
      {
	 hte = ht->table[i];
	 while (hte != NULL)
	 {
	    next = hte->next;
	    ufdbFree( hte->key );
	    if (freeValues)
	       ufdbFree( hte->value );
	    ufdbFree( hte );
	    hte = next;
	 }
      }
      ufdbFree( ht->table );

   pthread_mutex_unlock( &ht->mutex );

   /* ht contains the mutex, so it must be freed after the unlock(mutex) */
   ufdbFree( ht );
}

