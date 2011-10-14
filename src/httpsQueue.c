/* 
 * httpsQueue.c
 * 
 * ufdbGuard is copyrighted (C) 2005-2011 by URLfilterDB with all rights reserved.
 *
 * Parts of the ufdbGuard daemon are based on squidGuard.
 * This module is NOT based on squidGuard.
 *
 * communication between the worker and other threads is done via
 * RW-locks and data is passed via a queue.
 *
 * Since this queue is a FIFO we could use a tail queue. But we don't
 * like the malloc() overhead, so therefore we use a static array.
 *
 * RCS $Id: httpsQueue.c,v 1.15 2010/10/07 15:08:08 root Exp root $
 */

#include "ufdb.h"
#include "ufdblib.h"
#include "httpsQueue.h"

#include <unistd.h>
#include <string.h>
#include <pthread.h>

#define UFDB_HTTPS_QUEUE_SIZE	 256

static struct {
   int    portnumber;
   char   hostname[1020];
} q[UFDB_HTTPS_QUEUE_SIZE];

static volatile int    ihead = 0;                    /* array index for the head */
static volatile int    itail = 0;                    /* array index for the tail */
static volatile int    _ufdb_httpsq_n_queued = 0;
static pthread_mutex_t httpsq_lock = UFDB_STATIC_MUTEX_INIT;
static pthread_cond_t  empty = PTHREAD_COND_INITIALIZER;

#if 0
#define UFDB_DEBUG_QUEUE
#endif


/*
 * enqueue a hostname:portnumber for future https tunnel detection.
 */
int ufdbHttpsQueueRequest( 
   char * hostname, 
   int    portnumber )
{
   int    i;
   int    rv;

   rv = pthread_mutex_lock( &httpsq_lock );
#if defined(UFDB_DEBUG) || defined(UFDB_DEBUG_QUEUE)
   if (rv != 0)
      ufdbLogError( "ufdbHttpsQueueRequest: mutex_lock failed rv=%d *****", rv );
#endif

   /*
    * prevent duplicates: check if the request is already queued.
    */
   for (i = ihead; i != itail; i = (i+1) % UFDB_HTTPS_QUEUE_SIZE)
   {
      if (strcmp( hostname, q[i].hostname ) == 0  &&
          q[i].portnumber == portnumber)
      {
	 rv = pthread_mutex_unlock( &httpsq_lock );
#ifdef UFDB_DEBUG
	 if (rv != 0)
	    ufdbLogError( "ufdbHttpsQueueRequest: mutex_unlock failed rv=%d *****", rv );
#endif
#if defined(UFDB_DEBUG) || defined(UFDB_DEBUG_QUEUE)
         ufdbLogMessage( "ufdbHttpsQueueRequest: duplicate at %d: %s:%d", i, hostname, portnumber );
#endif
         return UFDB_API_REQ_QUEUED;
      }
   }

   if (_ufdb_httpsq_n_queued < UFDB_HTTPS_QUEUE_SIZE)
   {
      /*
       * insert the request at the tail
       */
      q[itail].portnumber = portnumber;
      strcpy( q[itail].hostname, hostname );

      _ufdb_httpsq_n_queued++;
#if defined(UFDB_DEBUG) || defined(UFDB_DEBUG_QUEUE)
      ufdbLogMessage( "ufdbHttpsQueueRequest: at %d (%d): %s:%d", itail, _ufdb_httpsq_n_queued, hostname, portnumber );
#endif

      itail = (itail + 1) % UFDB_HTTPS_QUEUE_SIZE;

      /*
       * leave the critical section
       */
      rv = pthread_mutex_unlock( &httpsq_lock );
#ifdef UFDB_DEBUG
      if (rv != 0)
	 ufdbLogError( "ufdbHttpsQueueRequest: mutex_unlock failed rv=%d *****", rv );
#endif

      /*
       * signal anyone who is waiting
       */
      pthread_cond_broadcast( &empty );

#ifdef _POSIX_PRIORITY_SCHEDULING
      sched_yield();
#endif
   } 
   else    
   {
      /* queue is full; just drop the request */
      ufdbLogError( "HTTPS/SSL security detection queue is full. %s:%d is not checked now", 
                    hostname, portnumber );

      rv = pthread_mutex_unlock( &httpsq_lock );
#ifdef UFDB_DEBUG
      if (rv != 0)
	 ufdbLogError( "ufdbHttpsQueueRequest: (full) mutex_unlock failed rv=%d *****", rv );
#endif

#ifdef _POSIX_PRIORITY_SCHEDULING
      sched_yield();
#endif

      return UFDB_API_ERR_FULL;
   }

   return UFDB_API_OK;
}


/*
 * Get a queued HTTPS tunnel check request.
 */
void ufdbGetHttpsRequest( char * hostname, int * portnumber )
{
   int rv;

allover:
   rv = pthread_mutex_lock( &httpsq_lock );
#ifdef UFDB_DEBUG
   if (rv != 0)
      ufdbLogError( "ufdbGetHttpsRequest: mutex_lock failed with rv=%d *****", rv );
#endif

#if defined(UFDB_DEBUG) || defined(UFDB_DEBUG_QUEUE)
   ufdbLogMessage( "ufdbGetHttpsRequest: n_queued: %d", _ufdb_httpsq_n_queued );
#endif

   while (1)
   {
      /*
       *  If there are requests in the queue
       */
      if (_ufdb_httpsq_n_queued > 0)
      {
         _ufdb_httpsq_n_queued--;
         /*
          * get the request at the head
          */
#if defined(UFDB_DEBUG) || defined(UFDB_DEBUG_QUEUE)
	 ufdbLogMessage( "ufdbGetHttpsRequest: ihead=%d (%d)  %s:%d", 
	                 ihead, _ufdb_httpsq_n_queued, q[ihead].hostname, q[ihead].portnumber );
#endif
         *portnumber = q[ihead].portnumber;
	 strcpy( hostname, q[ihead].hostname );
         ihead = (ihead + 1) % UFDB_HTTPS_QUEUE_SIZE;

         pthread_mutex_unlock( &httpsq_lock );

	 return;
      } 
      else   /* otherwise wait until there are fds available */ 
      {
         pthread_cond_wait( &empty, &httpsq_lock );
         /*
          * I have been signaled because the queue
          * is not empty.  Go try again.
	  * usleep() to try to prevent a race with old pthread libraries.
          */
         rv = pthread_mutex_unlock( &httpsq_lock );
#ifdef UFDB_DEBUG
	 if (rv != 0)
	    ufdbLogError( "ufdbGetHttpsRequest: mutex_unlock failed with rv=%d *****", rv );
#endif
	 usleep( ((unsigned long) pthread_self()) % 1025 + 201 );
	 goto allover;
      }
   }
}


int UFDBhttpsVerificationQueued( void )
{
#if 0
   ufdbLogMessage( "   UFDBhttpsVerificationQueued: %d", _ufdb_httpsq_n_queued );
#endif

   return _ufdb_httpsq_n_queued;
}

