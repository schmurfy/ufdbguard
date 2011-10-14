/* 
 * strcmpurlpart.static.c - URLfilterDB
 *
 * ufdbGuard is copyrighted (C) 2005-2010 by URLfilterDB with all rights reserved.
 *
 * Parts of the ufdbGuard daemon are based on squidGuard.
 * This module is NOT based on squidGuard.
 *
 * $Id: strcmpurlpart.static.c,v 1.4 2010/04/22 16:28:49 root Exp root $
 */

__inline__ static int strcmpURLpart( char * url, char * table )
{
   if (*table != '/')
   {
#if defined(__USE_STRING_INLINES) && defined(GCC_INLINE_STRING_FUNCTIONS_ARE_FASTER)
      return strcmp( url, table );
#else
      /* inlined: return strcmp( url, table ); */
      while (*url != '\0'  &&  (*url - *table) == 0)
      {
         url++;
	 table++;
      }
      return *url - *table;
#endif
   }

   while (*table != '\0')
   {
      if ((*url - *table) != 0)
         return *url - *table;
      url++;
      table++;
   }

   return 0;
}

