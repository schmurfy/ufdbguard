/*
 * inline.h
 *
 * determine if __inline__ is supported.
 * If not, use #define  to prevent compiler errors.
 *
 * RCS: $Id: inline.h,v 1.2 2009/09/29 13:47:20 root Exp root $
 */


#ifndef UFDB_INLINE_H_INCLUDED
#define UFDB_INLINE_H_INCLUDED

#if __IBM_GCC__INLINE__ || __GNUC__
/* __inline__ is supported */
#else
#define __inline__   /**/
#endif

#endif

