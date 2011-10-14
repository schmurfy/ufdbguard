/*
 * ipv6 address translation tests.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>


int main( int argc, char * argv[] )
{
   char * addr;
   int retval;
   const char * rewritten;
   char   newaddr[INET6_ADDRSTRLEN];
   struct in6_addr ipv6;

   addr = argv[1];
   retval = inet_pton( AF_INET6, addr, (void *) &ipv6 );
   if (retval <= 0)
   {
      printf( "ipv6: \"%s\" cannot be converted to an IPv6 address\n", addr );
      return 1;
   }

   printf( "\"%s\" is %x:%x:%x:%x:%x:%x:%x:%x\n", addr, ipv6.s6_addr16[0], ipv6.s6_addr16[1], ipv6.s6_addr16[2],
           ipv6.s6_addr16[3], ipv6.s6_addr16[4], ipv6.s6_addr16[5], ipv6.s6_addr16[6], ipv6.s6_addr16[7] );

   rewritten = inet_ntop( AF_INET6, (void *) &ipv6, newaddr, INET6_ADDRSTRLEN );
   if (rewritten == NULL)
   {
      printf( "ipv6: problem with converting IPv6 struct into string: %s", strerror(errno) );
      return 1;
   }

   printf( "ipv6: \"%s\" is normalised into \"%s\"\n", addr, newaddr );

   return 0;
}
