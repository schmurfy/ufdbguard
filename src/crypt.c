/*
 * crypt.c
 *
 * ufdbGuard is copyrighted (C) 2005-2010 by URLfilterDB with all rights reserved.
 *
 * Parts of the ufdbGuard daemon are based on squidGuard.
 * This module is NOT based on squidGuard.
 *
 * Encryption module
 *
 * $Id: crypt.c,v 1.7 2010/09/09 20:55:24 root Exp root $
 */


#include "ufdb.h"


/* We want agressive optimisations: */
/* These pragmas only work for GCC 4.4 and above */
#pragma GCC optimize (3,"inline-functions")
#pragma GCC optimize (3,"inline-limit=1500")


void ufdbCryptInit( ufdbCrypt * uc, const unsigned char * key, unsigned int keySize )
{
   int             i;
   unsigned int    t, u;
   unsigned int    ki;
   unsigned int    sti;
   unsigned char * state;
      
   state = uc->state;

   uc->x = 0;
   uc->y = 0;
   for (i = 0; i < 256; i++)
      state[i] = i;

   ki = 0;
   sti = 0;
   for (i = 0; i < 256; i++)
   {
      t = state[i];
      sti = (sti + key[ki] + t) & 0x00FF;
      u = state[sti];
      state[sti] = t;
      state[i] = u;
      ki = (ki + 1) % keySize;
   }
}

 
static __inline__ unsigned int _nextByte( ufdbCrypt * uc )
{
   unsigned int x;
   unsigned int y;
   unsigned int sx, sy;
   unsigned char * state;
      
   state = uc->state;

   x = (uc->x + 1) & 0x00FF;
   sx = state[x];

   y = (sx + uc->y) & 0x00FF;
   sy = state[y];

   uc->x = x;
   uc->y = y;
   state[y] = sx;
   state[x] = sy;

   return state[(sx + sy) & 0x00FF];
}


void ufdbEncryptText( 
   ufdbCrypt *           uc, 
   unsigned char *       dest, 
   const unsigned char * src, 
   unsigned int          len )
{
   while (len > 0)
   {
      *dest = *src ^ _nextByte( uc );
      src++;
      dest++;
      len--;
   }
}


