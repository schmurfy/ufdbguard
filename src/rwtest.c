/* code borrowed from redhat bugzilla report  4979 */

#define _XOPEN_SOURCE  600

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <pthread.h>

#define BUG 1
// #define BUG2
// #define FILLTEST 1
// #define LOCKDUMP 1

long workercount = 64;
long writers = 1;
long workerid;

long writersleep = 100000;
long readersleep = 10000;
long spincount = 10000000;
long spinout = 10000;
long procsleep = 100000;

pthread_rwlock_t mylock;
pthread_rwlockattr_t mylock_attr;


void *
lockdump (pthread_rwlock_t *thelock) {
    int *mptr = (int *) thelock;
    int i;
    for (i=0; i < (sizeof(pthread_rwlock_t) / 4); i++) {
        printf ("lock %x + %d = %x\n", (void *)thelock, i*4, *(mptr+i));
    }
}

void *
attrdump (pthread_rwlockattr_t *theattr) {
    int *mptr = (int *) theattr;
    int i;
    for (i=0; i < (sizeof(pthread_rwlockattr_t) / 4); i++) {
        printf ("attr %x + %d = %x\n", (void *)theattr, i*4, *(mptr+i));
    }
}

void *
lockclear (pthread_rwlock_t *thelock) {
    int *mptr = (int *) thelock;
    int i;
    for (i=0; i < (sizeof(pthread_rwlock_t) / 4); i++) {
        *(mptr+i) = 0;
    }
}

void *
attrclear (pthread_rwlockattr_t *theattr) {
    int *mptr = (int *) theattr;
    int i;
    for (i=0; i < (sizeof(pthread_rwlockattr_t) / 4); i++) {
        *(mptr+i) = 0;
    }
}

void *
lockfill (pthread_rwlock_t *thelock) {
    int *mptr = (int *) thelock;
    int i;
    for (i=0; i < (sizeof(pthread_rwlock_t) / 4); i++) {
        *(mptr+i) = 0xffffffff;
    }
}

void *
attrfill (pthread_rwlockattr_t *theattr) {
    int *mptr = (int *) theattr;
    int i;
    for (i=0; i < (sizeof(pthread_rwlockattr_t) / 4); i++) {
        *(mptr+i) = 0xffffffff;
    }
}

void *
reader (long workid, long rwtype) 
{
    while (1) 
    {
        int spn = spincount, spc = spincount;

        printf ("reader %d going for lock\n", workid);
        pthread_rwlock_rdlock(&mylock);
        printf ("reader %d obtained the lock\n", workid);

        while (spn != 0) { spn = spc--; }
#ifdef LOCKDUMP
        lockdump (&mylock);
#endif
        pthread_rwlock_unlock(&mylock);
        printf ("reader %d released the lock at %d\n", workid, spc);

        spn = spinout, spc = spinout;
        while (spn != 0) { spn = spc--; }
#ifdef LOCKDUMP
        lockdump (&mylock);
#endif
        // printf ("reader %d going yield at %d\n", workid, spc);
        pthread_yield();
        usleep(readersleep);
    }
}


void *
writer (long workid, long rwtype) 
{
    struct timeval tv;
    int nc = 5;

    sleep( 1 );
    while (--nc >= 0) 
    {
        int spn = spincount, spc = spincount;

	gettimeofday( &tv, NULL );
        printf ("%04d.%06d  writer %d going for lock\n", tv.tv_sec%10000, tv.tv_usec, workid);
        pthread_rwlock_wrlock(&mylock);
#ifdef LOCKDUMP
        lockdump (&mylock);
#endif
	gettimeofday( &tv, NULL );
        printf ("%04d.%06d  writer %d obtained the lock\n", tv.tv_sec%10000, tv.tv_usec, workid);

        while (spn != 0) { spn = spc--; }
        pthread_rwlock_unlock(&mylock);
        printf ("writer %d released the lock at %d\n", workid, spc);

#ifdef LOCKDUMP
        lockdump (&mylock);
#endif
        // printf ("writer %d going yield at %d\n", workid, spc);
        // pthread_yield();
        usleep( writersleep );
	// sleep( 4 );
    }

    pthread_exit( NULL );
}

int
main () {

#ifdef FILLTEST
    printf ("size of lock struct is %d\n", sizeof(pthread_rwlock_t));
    printf ("address of lock struct is %x\n", &mylock);
    lockdump (&mylock);
    attrdump (&mylock_attr);
    lockfill (&mylock);
    attrfill (&mylock_attr);
    lockdump (&mylock);
    attrdump (&mylock_attr);
#endif

    pthread_rwlockattr_init (&mylock_attr);
    pthread_rwlockattr_setkind_np (&mylock_attr,
#if 0
                       PTHREAD_RWLOCK_PREFER_WRITER_NP);
#else                                  
                       PTHREAD_RWLOCK_PREFER_WRITER_NONRECURSIVE_NP);
#endif
    pthread_rwlock_init(&mylock, &mylock_attr);

#ifdef LOCKDUMP
    printf ("lock has been initialized\n");
    lockdump (&mylock);
#endif

    pthread_t thrd[workercount];

    pthread_attr_t simple_attr;
    pthread_attr_init(&simple_attr);
    pthread_attr_setinheritsched(&simple_attr, PTHREAD_INHERIT_SCHED);

    long res;

    for (workerid = 0; workerid < writers; workerid++) {
        if (res = pthread_create(&thrd[workerid], &simple_attr,
                                 (void *)writer, (void *)workerid)) {
            printf ("failed to start worker %d, ret %d\n", workerid, res);
        }
    }
    for (workerid = 0; workerid < (workercount - writers); workerid++) {
        if (res = pthread_create(&thrd[writers+workerid], &simple_attr,
                                 (void *)reader, (void *)workerid)) {
            printf ("failed to start worker %d, ret %d\n", workerid, res);
        }
    }

    pthread_join( thrd[0], 0 );

}

