#include <openssl/ssl.h>
#include <openssl/opensslv.h>
#include <openssl/crypto.h>
#include <string.h>

int main()
{
   char * version;

   version = (char *) SSLeay_version( SSLEAY_CFLAGS );

   if (strstr( version, "-DOPENSSL_THREADS" ) != NULL)
      exit( 0 );

   exit( 1 );
}
