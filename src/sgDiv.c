/*
 * ufdbGuard is copyrighted (C) 2005-2010 by URLfilterDB with all rights reserved.
 *
 * Parts of the ufdbGuard daemon are based on squidGuard.
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


#include "ufdb.h"
#include "sg.h"

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>
#include <pthread.h>

#include "ufdblib.h"

extern int sig_other;           
extern char ** globalArgv;           
extern char ** globalEnvp;           
extern struct Acl * defaultAcl;



void sgHandlerSigHUP( int s )
{
   sig_hup = s;
}


void sgSignalHandler( int s )
{
   static int num = 0;

   if (sig_other == s)
   {
      num++;
      if (num >= 3)
	 abort();
   }
   else
      num = 0;

   sig_other = s;
}


void sgAlarm( int s )
{
}


#if 0
static void printSquidInfo( struct SquidInfo * s )
{
  fprintf( stderr, "   protocol = %s\n", s->protocol );
  fprintf( stderr, "   port     = %d\n", s->port     );
  fprintf( stderr, "   dot      = %d\n", s->dot      );
  UFDBprintRevURL( s->revUrl );
  fprintf( stderr, "   orig     = %s\n", s->orig         );
  fprintf( stderr, "   url2displ= %s\n", s->url2display  );
  fprintf( stderr, "   url      = %s\n", s->url          );
  fprintf( stderr, "   surl     = %s\n", s->surl         );
  fprintf( stderr, "   domain   = %s\n", s->domain       );
  fprintf( stderr, "   orgdomain= %s\n", s->orig_domain  );
  fprintf( stderr, "   src      = %s\n", s->src          );
  fprintf( stderr, "   srcDom   = %s\n", s->srcDomain    );
  fprintf( stderr, "   ident    = %s\n", s->ident        );
  fprintf( stderr, "   method   = %s\n", s->method       );
  fprintf( stderr, "   urlgroup = %s\n", s->urlgroup     );
}
#endif


static __inline__ int domain_has_tld( char * p )
{
   while (*p != '\0')
   {
      if (*p == '/')
         return 0;
      if (*p == '.')
         return 1;
      p++;
   }
   return 0;
}


static __inline__ int squeeze_html_char( char * p, int * hex )
{
   int length;

   length = 0;
   *hex = 0;
   while (*p != '\0'  &&  isxdigit( (int) *p ))
   {
      int h;
      h = (*p <= '9') ? *p - '0' : *p - 'a' + 10;
      *hex = *hex * 16 + h;
      p++;
      length++;
   }

#if 0
   ufdbLogMessage( "   squeeze_html_char hex=%02x  length=%d  *p=%c", *hex, length, *p );
#endif

   if (*p == ';')
   {
      if (*hex == 0)
         return length;
      if (*hex < 0x20)
      {
         if (*hex != '\t'  &&  *hex != '\n'  &&  *hex != '\r'  &&  *hex != '\f')
	    *hex = ' ';
      }
      else if (*hex == 0x7f  ||  *hex >= 0xff)
      {
         *hex = ' ';
      }
      else if (*hex <= 'Z'  &&  *hex >= 'A')
	 *hex += 'a' - 'A';
      return length;
   }
   
   /* '&#xxx' without trailing ';' is not a valid HTML character */
   return -1;
}


/*
 * parse the squidline:
 * URL ip-address/fqdn ident method
 * The URL is normalised/rewritten.
 * Squid pre2.6: http://www.sex.com 10.1.1.44/- - GET
 * Squid 2.6.x:  http://www.sex.com 10.1.1.44/- - GET -    (urlgroup added)
 */
int parseLine( 
  UFDBthreadAdmin *  admin, 
  char *             line, 
  struct SquidInfo * s )
{
  char * p;
  char * d;
  char * a;
  char * e;
  char * o;
  char * field;
  char * upp;
  int    i; 
  int    obfuscated;
  int    is_ipv6;
  char * lineptr;

  s->revUrl = NULL;
  s->quota = NULL;
  s->dot = 0;
  s->port = 80;
  s->orig[0] = 
    s->url[0] = s->domain[0] = s->orig_domain[0] = s->src[0] = s->ident[0] = s->ident[1] =
    s->protocol[0] = s->method[0] = s->urlgroup[0] = s->srcDomain[0] = s->surl[0] =  
    s->url2display[0] = '\0';

#if 0
  fprintf( stderr, "   line: %s\n", line );
#endif

  field = strtok_r( line, "\t ", &lineptr );
  if (field == NULL)
    return 0;

  /* prevent loops with regular expression matching: */
  p = strstr( field, "/URLblocked.cgi" );
  if (p != NULL)
    *(p+15) = '\0';

  strcpy( s->orig, field );
  if (UFDBglobalShowURLdetails)
  {
     strncpy( s->url2display, field, 1000 );
     p = &(s->url2display[1000]);
  }
  else
  {
     strncpy( s->url2display, field, 300 );
     s->url2display[300] = '\0';
     p = strchr( s->url2display, '?' );
     if (p == NULL)
     {
	p = strchr( s->url2display, ';' );
	if (p == NULL)
	{
	   p = strstr( s->url2display, "%3F" );
	   if (p == NULL)
	      p = &(s->url2display[300]);
	}
     }
  }
  strcpy( p, "...." );

#if 0
  ufdbLogMessage( "   parseLine: url2display = %s", s->url2display );
#endif

  for (p = field;  *p != '\0';  p++) 	/* convert URL to lowercase chars */
  {
    if (*p <= 'Z'  &&  *p >= 'A')
      *p += 'a' - 'A';
  }

  /* optimize for speed; trim very long URLs */
  if (p - field > 510)
     field[510] = '\0';

  p = strstr( field, "://" );
  if (p == NULL)			/* no protocol, defaults to http */
  {
    strcpy( s->protocol, "http" );
    p = field;
  }
  else
  {
    i = p - field;
    if (i > 15)
       i = 15;				/* WHOEHA a large protocol name. truncate it */
    strncpy( s->protocol, field, i );
    s->protocol[i] = '\0';
    if (strcmp( s->protocol, "https" ) == 0)
       s->port = 443;
    p += 3;
    field = p;
  }

   /* skip the initial optional username:password@ part */
   /* anticipate:   user:pw@www.domain.com		*/
   /*               user:pw@www.domain.com/path		*/
   /*               www.domain.com			*/
   /*               www.domain.com/path			*/
   i = 0;
   while (*p != '\0')
   {
      if (*p == '/')
         break;
      if (*p == '@')
      {
         p++;
	 i = 1;
	 break;
      }
      p++;
   }
   if (i == 0)
      p = field;

  /* do some character decoding: translate '//' '/./' '&#xx' and '%xx' */

  upp = s->url;
  while(*p != '\0') 
  {
    if (*p == '%')			/* convert '%HH' into character */
    {
      if (isxdigit((int) *(p+1))  &&  isxdigit( (int) *(p+2) ))
      {
	int    hex;
	char   h1, h2;
	/* note that the URL is already converted to lowercase */
	h1 = *(p+1);
	h2 = *(p+2);
	hex  = (h1 <= '9') ? h1 - '0' : h1 - 'a' + 10;
        hex *= 16;
	hex += (h2 <= '9') ? h2 - '0' : h2 - 'a' + 10;
	/* be careful with conversion of control characters */
	if (hex == 0)
	{
	   p += 3;
	   continue;
	}
	else if (hex < 0x20)
	{
	   if (hex != '\t'  &&  hex != '\r'  &&  hex != '\n'  &&  hex != '\f')
	   {
	      hex = ' ';
	   }
	}
	else
	{ 
	   if (hex == 0x7f  ||  hex == 0xff)
	   {
	      hex = ' ';
	   }
	   else if (hex <= 'Z'  &&  hex >= 'A')
	   {
		 hex += 'a' - 'A';
	   }
	}
	*upp++ = hex;
	p += 2;
      }
      else
      { 						/* an erroneous hex code, we ignore it */
        *upp++ = *p;					/* just copy one character */
      }
    }
    else if (*p == '&'  &&  *(p+1) == '#')		/* convert '&#HH;' into character */
    {
       int  hex;
       int  length;

       length = squeeze_html_char( p+2, &hex );
       if (length >= 0)
       {
	  if (hex != 0)
	     *upp++ = hex;
	  p += length + 2;
       }
       else  /* not a valid HTML character code; just copy one character */
       {
          *upp++ = *p;
       }
    }
    else
    {
      while (*p == '/')
      {
         if (*(p+1) == '/')					/* substitute // by / */
	    p++;
	 else if (*(p+1) == '.'  &&  *(p+2) == '/')		/* substitute /./ by / */
	    p += 2;
	 else if (*(p+1) == '.'  &&  *(p+2) == '.'  &&  *(p+3) == '/')    /* substitute /xxxx/../ by / */
	 {
	    /* try to find the previous directory... */
	    char * prevdir;
	    prevdir = upp - 1;
	    while (*prevdir != '/'  &&  prevdir > s->url)
	       prevdir--;
	    if (prevdir > s->url)
	    {
	       upp = prevdir;
	       p += 3;
	    }
	    else
	       break;
	 }
	 else
	    break;
      }
      *upp++ = *p;
    }
    p++;
  }
  *upp++ = *p;
  *upp = '\0';

#if 0
  ufdbLogMessage( "   parseLine: s->url = %s", s->url );
#endif

  p = s->url;

  is_ipv6 = 0;
  if (*p == '[')			/* IPv6 address */
  {
     char * end;
     char * oldBracket;

     is_ipv6 = 1;
     s->orig_domain[0] = '\0';
     oldBracket = strchr( p, ']' );
     if (oldBracket != NULL)
        *oldBracket = '\0';
     end = UFDBparseIPv6address( p, s->orig_domain );
     if (oldBracket != NULL)
        *oldBracket = ']';
     if (end != NULL)
     {
        UFDBupdateURLwithNormalisedDomain( p, s->orig_domain );
	/* uh-oh: the normalised domain is usually smaller and our pointers have moved */
     }
     if (end == NULL)		/* invalid IPv6 address */
     {
        end = oldBracket;
        if (end == NULL)
	{
	   end = strchr( p, '/' );
	   if (end == NULL)
	   {
	      /* I give up */
	      d = e = a = NULL;
	      goto parse2;
	   }
        }
     }
     s->dot = 1;			/* set 'dot' for IPv6 also */
     d = strchr( end, '/' );
     e = d;
     a = strchr( end, ':' ); 		/* find port */
  }
  else					/* not IPv6 */
  {
     d = strchr( p, '/' ); 		/* find domain end */
     e = d;
     a = strchr( p, ':' ); 		/* find port */
  }

parse2:
  if (a != NULL  &&  (a < d || d == NULL))
  {
    s->port = 0;
    o = a + 1;
    while (isdigit( (int) *o ))
    {
       s->port = s->port * 10 + (*o - '0');
       o++;
    }
    memmove( a, o, strlen(o)+1 );	/* remove the port number from the URL */
    e = a;
  }

  /* how ugly, Squid strips "https://" from the URL and inserts ":443" */
  if (s->port == 443)
     strcpy( s->protocol, "https" );

  if (!is_ipv6)
  {
     if (e == NULL)
	strcpy( s->orig_domain, p );
     else
     {
	i = e - p;
	strncpy( s->orig_domain, p, i );
	s->orig_domain[i] = '\0';
     }
  }
  
  /* strip prefix www. www8. ftp. ftp5. ... */
  o = p;
  if ((p[0] == 'w' && p[1] == 'w' && p[2] == 'w') ||		/* match "www" */
      (p[0] == 'f' && p[1] == 't' && p[2] == 'p'))  		/* match "ftp" */
  {
    p += 3;
    while (p[0] <= '9'  &&  p[0] >= '0')              		/* match www99. */
      p++;
    								/* skip www_xxx.com and www99.com */
    if (*p != '.'  ||  !domain_has_tld(p+1))
      p = o;  /* not a hostname */
    else
      p++;
  }

  if (e == NULL)
  {
     strcpy( s->domain, p );	

     /* The original URL will have &blah appended when safe-search is ON
      * and therefore we must make sure that URLs that only have the domain name,
      * have a / before the &blah, i.e the result becomes hotbot.com/&adf=on 
      */
     strcat( s->orig, "/" );
  }
  else 
  {
     memcpy( s->domain, p, e - p );	
     *(s->domain + (e - p)) = '\0';
  }

  strcpy( s->surl, p );

#if 0
  ufdbLogMessage( "   parseLine: s->surl = %s", s->surl );
#endif

  /* determine if the domainname is an IP address.
   * At the same time scan for IP address obfuscations, e.g. 066.1.2.3
   */
  obfuscated = (s->domain[0] == '0');
  for (o = s->domain; isdigit( (int) *o) || *o == '.'; o++)
  {
     if (*o == '.'  &&  *(o+1) == '0')
        obfuscated = 1;
  }
  if (*o == '\0')
  {
     s->dot = 1;
     if (obfuscated)
     {
        /* rewrite s->surl to remove the obfuscation */
	d = p = s->surl;
	/* remove obfuscating leading zeroes */
	while (*p == '0'  &&  *(p+1) != '.'  &&  *(p+1) != '/'  &&  *(p+1) != '\0')
	   p++;
        while (*p != '\0'  &&  *p != '/')
	{
	   if (*p == '.'  &&  *(p+1) == '0')
	   {
	      *d++ = '.';
	      p++;
	      /* remove obfuscating zeroes */
	      while (*p == '0'  &&  *(p+1) != '.'  &&  *(p+1) != '/'  &&  *(p+1) != '\0')
		 p++;
	   }
	   else
	   {
	      *d = *p;
	      d++;
	      p++;
	   }
	}
	/* copy the URI */
	while (*p != '\0')
	{
	   *d = *p;
	   d++;
	   p++;
        }
        *d = '\0';
     }
  }

  if ((p = strtok_r(NULL," \t\n",&lineptr)) != NULL)
  {
    o = strchr( p, '/' );
    if (o != NULL) 
    {
      memcpy( s->src, p, o-p );
      s->src[o-p] = '\0';
      strcpy( s->srcDomain, o+1 );
      if (*s->srcDomain == '-')
	s->srcDomain[0] = '\0';
    }
    else
    {
      strcpy( s->src, p );
      s->srcDomain[0] = '\0';	
    }

    if ((p = strtok_r(NULL," \t\n",&lineptr)) != NULL)		/* parse ident */
    {
      if (p[0] != '-'  ||  p[1] != '\0')
      {
	strcpy( s->ident, p );
	for (p = s->ident; *p != '\0'; p++) 			/* convert ident to lowercase chars */
	{
	  if (*p <= 'Z'  &&  *p >= 'A')
	    *p += 'a' - 'A';
        }
      }

      if ((p = strtok_r(NULL," \t\n",&lineptr)) != NULL)	/* parse method, e.g. GET/CONNECT */
      {
         strcpy( s->method, p );
	 if ((p = strtok_r(NULL," \t\n",&lineptr)) != NULL)	/* parse optional urlgroup (new in squid 2.6) */
	    strcpy( s->urlgroup, p );
	 else
	    strcpy( s->urlgroup, "#" );
      }
      else
	 strcpy( s->urlgroup, "#" );
    }
  }
  else
  {
     strcpy( s->urlgroup, "#" );
  }

  if (s->domain[0] == '\0')
    return 0;
  if (s->method[0] == '\0')
    return 0;

  /* s->revUrl is pointing to malloced memory and must be freed at the end of the main loop ! */
  s->revUrl = UFDBgenRevURL( admin, (unsigned char *) s->surl );

#if 0
  printSquidInfo( s );
#endif

  return 1;
}


/*
 * checks the vality of a dotted address. 
 */
ulong * sgConvDot( 
   char *  dot, 
   ulong * ipaddr )
{
   char *  t;
   int     octet;
   int     shift;

   *ipaddr = 0;
   shift = 24;
   t = dot;
   while (*t != '\0')
   {
      if (*t > '9'  ||  *t < '0')
         return NULL;

      octet = 0;
      while (*t >= '0'  &&  *t <= '9') 
      {
	 octet = octet*10 + (*t - '0');
         t++;
      }

      if (*t == '.') 
         t++;
      else if (*t != '\0') 
         return NULL;

      if (shift < 0)
         return NULL;

      if (octet > 255)
         return NULL;

      *ipaddr |= octet << shift;
      shift -= 8;
   }

   return ipaddr;
}


/*
 *  Regexp functions
 */

char * ufdbRegExpSubst( struct ufdbRegExp * regexp, char * pattern, char * newstring )
{
  struct ufdbRegExp * re;
  regmatch_t        pm;
  char *            result = NULL;
  char *            p;
  int               substlen;

  newstring[0] = '\0';
  for (re = regexp; re != NULL; re = re->next)
  {
    if (UFDBregexec(re->compiled, pattern, 1, &pm, 0) != 0) 
    {
      result = NULL;
    }
    else
    {
      substlen = strlen( re->substitute );
      if (re->httpcode != NULL)
	strcpy( newstring, re->httpcode );
      else
	*newstring = '\0';
      p = newstring;
      do {
	if ((p - newstring) + pm.rm_so >= UFDB_MAX_URL_LENGTH)
	  break;
	p = strncat( newstring, pattern, pm.rm_so );
	if ((p - newstring) + substlen >= UFDB_MAX_URL_LENGTH)
	  break;
	p = strcat( newstring, re->substitute );	
	pattern = pattern + pm.rm_eo;
      } while (UFDBregexec(re->compiled, pattern, 1, &pm, REG_NOTBOL) == 0 && re->global);

      if ((p - newstring) + strlen(pattern) <= UFDB_MAX_URL_LENGTH)
	p = strcat( newstring, pattern );
      result = newstring;
      break;
    }
  }

  return result;
}


char * sgRewriteExpression( struct sgRewrite * rewrite, char * subst, char * newstring )
{
  char * result;

  result = ufdbRegExpSubst( rewrite->rewrite, subst, newstring );
  return result;
}


/*
 *  Expand a redirection URL string: fill in the %i, %n etc.
 */
char * sgParseRedirect( 
  char *             redirect,
  struct SquidInfo * req,
  struct Acl *       acl, 
  struct AclDest *   aclpass,
  char *             buf,
  int                force_302 )
{
  char * p;
  char * q;
  char * t;
  char * d;

#if UFDB_USERQUOTA_SUPPORT
  struct Source * s = req->lastActiveSource;
#endif

  if (aclpass == NULL)
    aclpass = defaultAcl->pass;

  q = p = redirect;
  buf[0] = '\0';

  if (force_302  &&  strncmp(redirect,"302:",4) != 0)
  {
     strcpy( buf, "302:" );
  }

  while ((p = strchr(p,'%')) != NULL)
  {
    strncat( buf, q,  p - q );

    p++;
    switch (*p)
    {
    case 'a': 			/* Source Address */
      strcat( buf, req->src );
      break;

    case 'A':
       t = ufdbSettingGetValue( "administrator" );
       if (t == NULL)
          t = "your system administrator or helpdesk";
       d = buf + strlen( buf );
       while (*t != '\0')
       {
          if (*t == ' ' || *t == '=' || *t == '&' || *t == '\\' || *t == '"')
	  {
	     char h;

	     *d++ = '%';
	     h = *t / 16;
	     *d++ = (h >= 10) ? ('A' - 10 + h) : ('0' + h);
	     h = *t % 16;
	     *d++ = (h >= 10) ? ('A' - 10 + h) : ('0' + h);
	  }
	  else
	     *d++ = *t;
	  t++;
       }
       *d = '\0';
       break;

    case 'i': 			/* Source User Ident */
      if ((req->ident[0] == '\0')  ||  (req->ident[0] == '-' && req->ident[1] == '\0'))
      {
	strcat( buf, "unknown" );
      }
      else
      {
	strcat( buf, req->ident );
      }
      break;

    case 'q': 				/* userquota info */
#if UFDB_USERQUOTA_SUPPORT
      if (s != NULL  && 
          s->userquota.seconds != 0  &&  
	  strcmp(req->ident, "-") != 0) 
      {
	struct UserQuotaInfo * userquota;
	if (sgDbLookup( s->userDb, req->ident, (char **) (char *) &userquota ) == 1) 
	{
	  char qbuf[150];
	  sprintf( qbuf, "%d-%d-%d-%d-%d-%d", 
		   s->userquota.renew, (int) s->userquota.seconds, userquota->status,
		   (int) userquota->time, (int) userquota->last, userquota->consumed );
	  strcat(buf, qbuf);
	}
	else
	{
	  strcat( buf, "noquota" );
	}
      }
      else
      {
	strcat( buf, "noquota" );
      }
#else
      strcat( buf, "noquota" );
#endif  /* UFDB_USERQUOTA_SUPPORT */
      break;

    case 'n': 		/* Source Domain Name */
      if ((req->srcDomain[0] == '\0')  ||  (req->srcDomain[0] == '-' && req->srcDomain[1] == '\0'))
      {
	strcat(buf, "unknown" );
      }
      else
      {
	strcat( buf, req->srcDomain );
      }
      break;

    case 'p': 		/* The url path */
      if ((t = strstr(req->orig,"://")) != NULL)
      {
        t += 3;
        if ((t = strchr(t,'/')) != NULL)
	{
	  t++;
	  strcat( buf, t );
        }
      }
      break;

    case 'f': 		/* The url file */
      if ((t = strrchr(req->orig,'/')) != NULL)
      {
        t++;
        strcat( buf, t );
      }
      break;

    case 's': 		/* Source Class Matched */
      if (acl->source == NULL  ||  acl->source->name == NULL)
	strcat( buf, "default" );
      else
      {
	if (acl->within == UFDB_ACL_NONE)
	   strcat( buf, acl->name );
	else
	{
	   struct Acl * acl0;

	   acl0 = acl;
	   strcat( buf, acl0->name );
	   if (acl0->within == UFDB_ACL_ELSE)
	   {
	      strcat( buf, "-not" );  
	      acl0 = sgAclFindName( acl0->name );
	      if (acl0 == NULL)
	      {
	         strcat( buf, "-ERROR-find-acl" );
		 acl0 = acl;
	      }
	   }
	   switch (acl0->within)
	   {
	   case UFDB_ACL_WITHIN:     
	      strcat( buf, "-within-" );  
	      strcat( buf, acl0->time->name );
	      break;
	   case UFDB_ACL_OUTSIDE:     
	      strcat( buf, "-outside-" );  
	      strcat( buf, acl0->time->name );
	      break;
	   }
	}
      }
      break;

    case 't': 		/* Target Class Matched */
      if (aclpass == NULL)
	strcat( buf, "unknown" );
      else if (aclpass->name == NULL)
	if (aclpass->type == ACL_TYPE_INADDR)
	  strcat( buf, "in-addr" );
	else if (aclpass->type == ACL_TYPE_TERMINATOR)
	  strcat( buf, "none" );
	else
	  strcat( buf, "unknown" );
      else
	strcat( buf, aclpass->name );
      break;

    case 'u': 		/* Requested URL */
      					/* TODO: strcat( buf, req->url2display ); */
      d = buf + strlen( buf );
      t = req->orig;
      while (*t != '\0')
      {
         if (*t == '&'  ||  *t == '?'  ||  *t == ';')
	 {
	    *d++ = *t;
	    *d++ = '.';
	    *d++ = '.';
	    *d++ = '.';
	    break;
	 }
         *d++ = *t++;
      }
      *d = '\0';
      break;

    case 'U': 		/* parsed URL */
      d = buf + strlen( buf );
      t = req->surl;
      while (*t != '\0')
      {
         if (*t == '&'  ||  *t == '?'  ||  *t == ';')
	 {
	    *d++ = *t;
	    *d++ = '.';
	    *d++ = '.';
	    *d++ = '.';
	    break;
	 }
         *d++ = *t++;
      }
      *d = '\0';
      break;

    default:
      strcat( buf, "%" );
    }
    p++;
    q = p;
  }

  if (q == redirect)   	/* no '%' found in redirect string */
  {
    if (force_302  &&  strncmp( redirect, "302:", 4 ) != 0)
    {
       strcpy( buf, "302:" );
       strcat( buf, redirect );
    }
    else
       strcpy( buf, redirect );
  }

  return buf;
}


/*
 *  converts yyyy.mm.ddTHH:MM:SS to seconds since EPOC
 */

time_t iso2sec( char * date )
{
  struct tm *t;
  int y,m,d,H,M,S;

  t = (struct tm *) ufdbCalloc(1,sizeof(struct tm));
  sscanf(date,"%4d%*[.-]%2d%*[.-]%2d%*[T]%2d%*[:-]%2d%*[:-]%2d",
	 &y,&m,&d,&H,&M,&S);
  m--; 
  y = y - 1900;
  if (y < 0 || m < 0 || m > 11 || d < 1 || d > 31 || H < 0 || H > 23 
     || M < 0 || M > 59 || S < 0 || S > 59)
    return (time_t) -1;
  t->tm_year = y;
  t->tm_mon =  m;
  t->tm_mday = d;
  t->tm_hour = H;
  t->tm_min = M;
  t->tm_sec = S;

  return (time_t) mktime(t);
}

/*
 *  converts yyyy.mm.dd to seconds since EPOC
 */
time_t date2sec( char * date )
{
  struct tm *t;
  int y,m,d;

  t = (struct tm *) ufdbCalloc(1,sizeof(struct tm));
  sscanf(date,"%4d%*[.-]%2d%*[.-]%2d",&y,&m,&d);
  m--; 
  y = y - 1900;
  if (y < 0 || m < 0 || m > 11 || d < 1 || d > 31)
    return (time_t) -1;
  t->tm_year = y;
  t->tm_mon =  m;
  t->tm_mday = d;

  return (time_t) mktime(t);
}


struct UserQuotaInfo * setuserquota( struct UserQuotaInfo * uq )
{
  uq->time = 0; 
  uq->last = 0; 
  uq->consumed = 0; 
  uq->status = UDFB_Q_STAT_FIRSTTIME; 

  return uq;
}

