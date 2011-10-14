/*
 * test IP counter functions.
 */

#include "ufdblib.h"
#include <stdio.h>

int main()
{
   int i;

   UFDBinitializeIPcounters();

   printf( "#IP: %lu (0)\n", UFDBgetNumberOfRegisteredIPs() );

   UFDBregisterCountedIP( "10.1.0.8" );
   UFDBregisterCountedIP( "10.1.0.8" );
   UFDBregisterCountedIP( "10.1.0.8" );
   UFDBregisterCountedIP( "10.1.0.8" );

   printf( "#IP: %lu (1)\n", UFDBgetNumberOfRegisteredIPs() );

   UFDBregisterCountedIP( "10.1.0.7" );
   UFDBregisterCountedIP( "10.1.0.8" );
   UFDBregisterCountedIP( "10.1.0.9" );

   printf( "#IP: %lu (3)\n", UFDBgetNumberOfRegisteredIPs() );

   UFDBregisterCountedIP( "10.2.0.7" );
   UFDBregisterCountedIP( "10.3.0.7" );
   UFDBregisterCountedIP( "10.4.0.7" );

   printf( "#IP: %lu (6)\n", UFDBgetNumberOfRegisteredIPs() );

   UFDBregisterCountedIP( "10.3.1.7" );
   UFDBregisterCountedIP( "10.3.2.7" );
   UFDBregisterCountedIP( "10.3.3.7" );

   printf( "#IP: %lu (9)\n", UFDBgetNumberOfRegisteredIPs() );

   UFDBregisterCountedIP( "10.222.83.107" );
   UFDBregisterCountedIP( "10.222.83.108" );
   UFDBregisterCountedIP( "254.252.231.244" );
   UFDBregisterCountedIP( "254.253.231.244" );

   printf( "#IP: %lu (13)\n", UFDBgetNumberOfRegisteredIPs() );

   UFDBregisterCountedIP( "pc01.neverseenbeforeextremelyverylonginternalmydomain01.net" );
   UFDBregisterCountedIP( "pc02.neverseenbeforeextremelyverylonginternalmydomain01.net" );
   UFDBregisterCountedIP( "pc03.neverseenbeforeextremelyverylonginternalmydomain01.net" );

   /* time iptst takes 0.40 sec for 10 million+ calls on old Xeon 2.8 GHz 512 KB cache */
   for (i = 10000000; i>0; i--)
      UFDBregisterCountedIP( "10.222.83.107" );

   printf( "#IP: %lu (16)\n", UFDBgetNumberOfRegisteredIPs() );

   UFDBregisterCountedIP( "10.0.0.14" );
   UFDBregisterCountedIP( "10.0.0.15" );
   UFDBregisterCountedIP( "10.0.0.16" );
   UFDBregisterCountedIP( "10.0.0.17" );
   UFDBregisterCountedIP( "10.0.0.18" );
   UFDBregisterCountedIP( "10.0.0.19" );
   UFDBregisterCountedIP( "10.0.0.20" );
   UFDBregisterCountedIP( "10.0.0.21" );
   UFDBregisterCountedIP( "10.0.0.22" );
   UFDBregisterCountedIP( "10.0.0.23" );
   UFDBregisterCountedIP( "10.0.0.24" );
   UFDBregisterCountedIP( "10.0.0.25" );
   UFDBregisterCountedIP( "10.0.0.26" );
   UFDBregisterCountedIP( "10.0.0.27" );
   UFDBregisterCountedIP( "10.0.0.28" );
   UFDBregisterCountedIP( "10.0.0.29" );
   UFDBregisterCountedIP( "10.0.0.30" );
   UFDBregisterCountedIP( "10.0.0.31" );
   UFDBregisterCountedIP( "10.0.0.32" );
   UFDBregisterCountedIP( "10.0.0.33" );
   UFDBregisterCountedIP( "10.0.0.34" );
   UFDBregisterCountedIP( "10.0.0.35" );
   UFDBregisterCountedIP( "10.0.0.36" );
   UFDBregisterCountedIP( "10.0.0.37" );
   UFDBregisterCountedIP( "10.0.0.38" );
   UFDBregisterCountedIP( "10.0.0.39" );
   UFDBregisterCountedIP( "10.0.0.40" );
   UFDBregisterCountedIP( "10.0.0.41" );
   UFDBregisterCountedIP( "10.0.0.42" );
   UFDBregisterCountedIP( "10.0.0.43" );
   UFDBregisterCountedIP( "10.0.0.44" );
   UFDBregisterCountedIP( "10.0.0.45" );

   printf( "#IP: %lu (48)\n", UFDBgetNumberOfRegisteredIPs() );

   return 0;
}

