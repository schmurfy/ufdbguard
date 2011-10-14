
#define XX_POSIX_SOURCE
#define _XOPEN_SOURCE 600

#include <sys/capability.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>


void printids( char * info )
{
   pid_t pid = getpid();
   uid_t ruid;
   uid_t euid;
   uid_t suid;

   getresuid( &ruid, &euid, &suid );

   printf ( "%s: pid = %d  uid = %d  euid = %d  suid = %d\n", info, pid, ruid, euid, suid );

}


void catchhup( int s )
{
   printf( "catchhup: caught signal %d\n", s );
   if (seteuid( 0 ) != 0)
      printf( "catchhup: seteuid(0) failed: %s\n", strerror(errno) );
   else
      printf( "catchhup: I think I am root: uid = %d  euid = %d\n", getuid(), geteuid() );
}


void tryprivilegedthings( void )
{
   int f;

   system( "whoami" );

   if (unlink( "/tmp/dtest" ) != 0)
      printf( "cannot unlink /tmp/dtest: %s\n", strerror(errno) );

   f = open( "/tmp/dtest", O_CREAT|O_WRONLY, 0644 );
   if (f < 0)
      printf( "cannot write to /tmp/dtest: %s\n", strerror(errno) );
   else
   {
      write( f, "booh\n", 5 );
      close( f );
   }
}


int main()
{
   cap_t caps;

   printids( "virgin" );

#ifdef TEST_CAPS
   caps = cap_get_proc();
   if (caps == NULL)
      printf( "dtest: cap_get_proc failed\n" );
   else
   {
      ssize_t y = 0;
      printf( "capabilities: %s\n", cap_to_text(caps, &y) );

      cap_set_flag( caps, CAP_SETUID
      if (cap_set_proc( caps ) == 0)
	 printf( "dtest: cap_set_proc failed for new capabilities: %s\n", strerror(errno) );
   }
#endif

   setresuid( 1007, 1007, -1 );

   printids( "after setreuid 1007" );

   signal( SIGHUP, catchhup );

   tryprivilegedthings();

   while (1)
      sleep( 1 );

   return 0;
}

