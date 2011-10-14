/*
 * ufdb-api-private.h - URLfilterDB
 *
 * ufdbGuard API is copyrighted (C) 2007-2010 by URLfilterDB with all rights reserved.
 *
 * ufdbGuard API is used to integrate the functionality of ufdbGuard into
 * programs of 3rd parties.
 *
 * RCS $Id: ufdb-api-private.h,v 1.1 2010/08/27 22:15:50 root Exp root $
 */

#ifndef UFDB_UFDBAPI_PRIVATE_H_INCLUDED
#define UFDB_UFDBAPI_PRIVATE_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

extern volatile unsigned long UFDB_API_num_url_lookups;
extern volatile unsigned long UFDB_API_num_url_matches;
extern volatile unsigned long UFDB_API_num_safesearch;
extern volatile unsigned long UFDB_API_upload_seqno;

#ifdef __cplusplus
}
#endif

#endif

