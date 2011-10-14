/*
 * main.c - old daemon, not used any more
 *
 * ufdbGuard is copyrighted (C) 2005-2009 by URLfilterDB with all rights reserved.
 * ufdbGuard is inspired by squidGuard.
 *
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

/* to inline strcpy() with gcc : */
/* #define __USE_STRING_INLINES */

#include "ufdb.h"
#include "sg.h"
#include "ufdblib.h"

#include <signal.h>
#include <pthread.h>

extern char **environ;


int main( int argc, char ** argv )
{
   return ufdbguard_main( argc, argv, environ );
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

int pthread_sigmask(int how, const sigset_t *newmask, sigset_t *oldmask)
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

