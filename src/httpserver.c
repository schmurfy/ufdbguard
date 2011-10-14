/*
 * httpserver.c - URLfilterDB
 *
 * ufdbGuard is copyrighted (C) 2005-2011 by URLfilterDB with all rights reserved.
 *
 * Parts of the ufdbGuard daemon are based on squidGuard.
 * This module is NOT based on squidGuard.
 *
 * serve HTTP GET requests for /cgi-bin/URLblocked.cgi
 *
 * RCS $Id: httpserver.c,v 1.34 2011/06/22 17:57:07 root Exp root $
 */


#include "ufdb.h"
#include "ufdblib.h"
#include "httpserver.h"
#include "version.h"

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <ctype.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>

#define LANG_IND_EN  0
#define LANG_IND_NL  1
#define LANG_IND_DE  2
#define LANG_IND_PL  3
#define LANG_IND_IT  4
#define LANG_IND_ES  5
#define LANG_IND_PT  6
#define LANG_IND_FR  7
#define LANG_IND_TR  8
#define LANG_IND_SV  9


#define LANG_IND_DEFAULT  LANG_IND_EN

static char _fatal_error_text [] =
   "<center>\n"
   "<font color=red><b>"
   "Access to the internet is blocked because<br>\n"
   "the URL filter has a fatal error. <br>\n"
   "Ask you helpdesk or web proxy administrator for help.\n"
   "</b></font>\n"
   "</center>\n";
static char _loading_database_text [] =
   "<center>\n"
   "<font color=red><b>"
   "Access to the internet is temporarily blocked because<br>\n"
   "a new URL database is being loaded by the URL filter. <br>\n"
   "Wait one minute and try again.\n"
   "</b></font>\n"
   "</center>\n";

static char * _title [] =
{
   /* EN */  "403 Forbidden",			
   /* NL */  "403 Geen Toegang",
   /* DE */  "403 Verboten",
   /* PL */  "403 Cenzura, zakaz pobrania",
   /* IT */  "403 Accesso non permesso",
   /* ES */  "403 Ning&uacute;n acces",
   /* PT */  "403 Proibido",
   /* FR */  "403 Interdit",
   /* TR */  "403 Eri&#351;im engellendi",
   /* SV */  "403 F&ouml;rbjuden"
};
static char * _forbidden [] =
{
   "Forbidden",
   "Geen Toegang",
   "Verboten",
   "Cenzura, zakaz pobrania",
   "Accesso non permesso",
   "Ning&uacute;n acces",
   "Proibido",
   "Interdit",
   "Eri&#351;im engellendi",
   "Sidan stoppad enligt landstingets riktlinjer"
};
static char * _explain_1 [] =
{
   "Access is blocked since the URL is in the filter category",
   "De toegang is geblokkeerd omdat de URL in de categorie",
   "Zugriff verweigert weil die URL die Klassifizierung",
   "Nie otworzysz tej strony bo jest ona sklasyfikowana jako",
   "L'accesso &egrave; ostruito poich&eacute; il URL &egrave; considerato come",
   "Se bloquea el acceso puesto que el URL se considera ser",
   "O acesso a este site foi bloqueado porque o conte&uacute;do est&aacute;",
   "L'access est inderdit parce que le site est",
   "Ula&#351;mak istedi&#287;iniz sayfaya eri&#351;im kapal&#305;d&#305;r. S&#305;n&#305;f&#305;:",
   "Access till denna sida &auml;r stoppad:"
};
static char * _explain_2 [] =
{
   ".",
   " valt.",
   " hat.",
   " przez program kontroli ufdbGuard.",
   ".",
   ".",
   ".",
   ".",
   ".",
   "."
};
static char * _moreInfo1 [] =
{
   "More information about ufdbGuard is <a href=\"" UFDB_EXPLAIN_DENY_REASON_URL,
   "Meer informatie over ufdbGuard is <a href=\"" UFDB_EXPLAIN_DENY_REASON_URL,
   "Mehr Informationen &uuml;ber ufdbGuard ist <a href=\"" UFDB_EXPLAIN_DENY_REASON_URL,
   "Informacja o tym programie kontroli jest na <a href=\"" UFDB_EXPLAIN_DENY_REASON_URL,
   "Le pi&ugrave; informazioni di ufdbGuard &egrave; <a href=\"" UFDB_EXPLAIN_DENY_REASON_URL,
   "M&aacute;s informaci&oacute;n sobre ufdbGuard est&aacute; <a href=\"" UFDB_EXPLAIN_DENY_REASON_URL,
   "Mais informa&ccedil;&atilde;o sobre ufdbGuard est&aacute; <a href=\"" UFDB_EXPLAIN_DENY_REASON_URL,
   "Plus d'information de ufdbGuard est <a href=\"" UFDB_EXPLAIN_DENY_REASON_URL,
   "ufdbGuard hakk&#305;nda bilgi i&ccedil;in <a href=\"" UFDB_EXPLAIN_DENY_REASON_URL,
   "Mer information om ufdbGuard &auml;r <a href=\"" UFDB_EXPLAIN_DENY_REASON_URL
};
static char * _moreInfo2 [] =
{
   "\" target=\"_blank\">here</a>.",
   "\" target=\"_blank\">hier</a>.",
   "\" target=\"_blank\">hier</a>.",
   "\" target=\"_blank\">stronie</a>.",
   "\" target=\"_blank\">qui</a>.",
   "\" target=\"_blank\">aqu&iacute;</a>.",
   "\" target=\"_blank\">aqui</a>.",
   "\" target=\"_blank\">ici</a>.",
   "\" target=\"_blank\">t&#305;klay&#305;n&#305;z</a>.",
   "\" target=\"_blank\"h&auml;r</a>."
};
static char * _goBack [] = 
{
   "Click here to go back",
   "Klik hier om terug te gaan",
   "Klicken Sie hier um zur&uuml;ck zu gehen",
   "Wycofaj do poprzedniej strony",
   "andare indietro",
   "ir detr&aacute;s",
   "volte",
   "rentrer",
   "&Ouml;nceki sayfa",
   "Klicka h&auml;r f&ouml;r att komma tillbaks"
};


static void writeBuffer( int fd, char * buffer, int size )
{
   int retval;

try_again:
   retval = write( fd, buffer, size );
   if (retval < 0)
   {
      if (errno == EINTR)
         goto try_again;
      ufdbLogError( "HttpServer: write failed: %s", strerror(errno) );
      return;
   }

   if (retval < size)
   {
      buffer += retval;
      size -= retval;
      goto try_again;
   }
}


static void AnswerHttpTimeout( int fd )
{
   time_t    now_t;
   struct tm t;
   char      header[2048];
   char      content[8192+1200];

   if (UFDBglobalDebug || UFDBglobalDebugHttpd)
      ufdbLogMessage( "AnswerHttpTimeout" );

   now_t = time( NULL );
   gmtime_r( &now_t, &t );
   sprintf( header, 
            "HTTP/1.0 200 OK\r\n"
	    "Date: %3.3s, %02d %3.3s %4d %02d:%02d:%02d GMT\r\n"
	    "Server: ufdbhttpd/" VERSION "\r\n"
	    "Cache-Control: max-age=300\r\n"
	    "Connection: close\r\n"
	    "Content-Type: text/html\r\n"
	    "\r\n",
            &"SunMonTueWedThuFriSat"[t.tm_wday*3],
            t.tm_mday,
            &"JanFebMarAprMayJunJulAugSepOctNovDec"[t.tm_mon*3],
            t.tm_year + 1900,
            t.tm_hour, t.tm_min, t.tm_sec
	    );
   sprintf( content,
            "<html>\r\n"
	    "<head>\r\n"
	    "<title>Timeout</title>\r\n"
	    "</head>\r\n"
	    "<body topmargin=1 leftmargin=1 marginheight=1 marginwidth=1 bgcolor=\"orange\" text=\"black\">\r\n"
	    "<font size=\"+2\">A timeout error occurred. <br> </font>\r\n"
	    "<font size=\"+1\">\r\n"
	    "The HTTP message for ufdbGuard was not received within the timeout period<br>\r\n"
	    "The URL that you are trying to access is blocked.<br>\r\n"
	    "</font>\r\n"
	    "</body>\r\n"
	    "</html>\r\n"
	    );

   writeBuffer( fd, header, strlen(header) );
   writeBuffer( fd, content, strlen(content) );
}


static void AnswerHttpEmpty( int fd, char * url )
{
   time_t    now_t;
   struct tm t;
   char      header[2048];
   char      content[8192+1200];

   if (UFDBglobalDebug || UFDBglobalDebugHttpd)
      ufdbLogMessage( "AnswerHttpEmpty" );

   now_t = time( NULL );
   gmtime_r( &now_t, &t );
   sprintf( header, 
            "HTTP/1.0 200 OK\r\n"
	    "Date: %3.3s, %02d %3.3s %4d %02d:%02d:%02d GMT\r\n"
	    "Server: ufdbhttpd/" VERSION "\r\n"
	    "Cache-Control: max-age=300\r\n"
	    "Connection: close\r\n"
	    "Content-Type: text/html\r\n"
	    "\r\n",
            &"SunMonTueWedThuFriSat"[t.tm_wday*3],
            t.tm_mday,
            &"JanFebMarAprMayJunJulAugSepOctNovDec"[t.tm_mon*3],
            t.tm_year + 1900,
            t.tm_hour, t.tm_min, t.tm_sec
	    );
   sprintf( content,
            "<html>\r\n"
	    "<head>\r\n"
	    "<title>Error</title>\r\n"
	    "</head>\r\n"
	    "<body topmargin=1 leftmargin=1 marginheight=1 marginwidth=1 bgcolor=\"orange\" text=\"black\">\r\n"
	    "<font size=\"+2\">An error occurred. <br> </font>\r\n"
	    "<font size=\"+1\">\r\n"
	    "This http server can only serve URL requests for ufdbGuard <br>\r\n"
	    "redirection messages and does not understand the URL. <br>\r\n"
	    "URL: <tt>%s</tt> <br>\r\n"
	    "Most likely the configuration of \"redirect\" statements is incorrect.  It should include \"/cgi-bin/URLblocked.cgi\". <br>\r\n"
	    "</font>\r\n"
	    "</body>\r\n"
	    "</html>\r\n",
	    url
	    );

   writeBuffer( fd, header, strlen(header) );
   writeBuffer( fd, content, strlen(content) );
}


static void AnswerHttpNotFound( int fd )
{
   time_t    now_t;
   struct tm t;
   char      header[2048];

   if (UFDBglobalDebug || UFDBglobalDebugHttpd)
      ufdbLogMessage( "AnswerHttpNotFound" );

   now_t = time( NULL );
   gmtime_r( &now_t, &t );
   sprintf( header, 
            "HTTP/1.0 404 Not Found\r\n"
	    "Date: %3.3s, %02d %3.3s %4d %02d:%02d:%02d GMT\r\n"
	    "Server: ufdbhttpd/" VERSION "\r\n"
	    "Cache-Control: max-age=300\r\n"
	    "Connection: close\r\n"
	    "Content-Type: text/html\r\n"
	    "\r\n",
            &"SunMonTueWedThuFriSat"[t.tm_wday*3],
            t.tm_mday,
            &"JanFebMarAprMayJunJulAugSepOctNovDec"[t.tm_mon*3],
            t.tm_year + 1900,
            t.tm_hour, t.tm_min, t.tm_sec
	    );

   writeBuffer( fd, header, strlen(header) );
}


static void AnswerHttpHead( int fd )
{
   time_t    now_t;
   struct tm t;
   char      header[2048];

   if (UFDBglobalDebug || UFDBglobalDebugHttpd)
      ufdbLogMessage( "AnswerHttpHead" );

   now_t = time( NULL );
   gmtime_r( &now_t, &t );
   sprintf( header, 
            "HTTP/1.0 503 HEAD command not supported\r\n"
	    "Date: %3.3s, %02d %3.3s %4d %02d:%02d:%02d GMT\r\n"
	    "Server: ufdbhttpd/" VERSION "\r\n"
	    "Cache-Control: max-age=300\r\n"
	    "Connection: close\r\n"
	    "Content-Type: text/html\r\n"
	    "\r\n",
            &"SunMonTueWedThuFriSat"[t.tm_wday*3],
            t.tm_mday,
            &"JanFebMarAprMayJunJulAugSepOctNovDec"[t.tm_mon*3],
            t.tm_year + 1900,
            t.tm_hour, t.tm_min, t.tm_sec
	    );

   writeBuffer( fd, header, strlen(header) );
}


static void convertHTTPcodes( 
   char * new_text,
   char * text )
{
   char * b;
   char * orig_text;

   orig_text = text;
   b = new_text;
   while (*text != '\0')
   {
      if (*text == '%')
      {
	 int u1, u2, ascii;
	 text++;
again:
	 if (*text == '\0' || *text+1 == '\0')
	 {
	    *b++ = '%';
	    *b = '\0';
	    return;
	 }
	 u1 = *text++;
	 if (u1 == '%')
	 {
	    *b++ = '%';
	    continue;
	 }
	 if (u1 >= 'A'  &&  u1 <= 'F')
	    u1 = u1 - 'A' + 10;
	 else if (u1 >= 'a'  &&  u1 <= 'f')
	    u1 = u1 - 'a' + 10;
	 else
	    u1 = u1 - '0';
	 u2 = *text++;
	 if (u2 >= 'A'  &&  u2 <= 'F')
	    u2 = u2 - 'A' + 10;
	 else if (u2 >= 'a'  &&  u2 <= 'f')
	    u2 = u2 - 'a' + 10;
	 else
	    u2 = u2 - '0';
         ascii = u1 * 16 + u2;
	 if (ascii < 9 || ascii > 127)
	    ascii = 32;
	 if (ascii == '%')
	    goto again;
	 *b++ = tolower( ascii );
      }
      else
      {
         *b++ = tolower( *text );
	 text++;
      }
   }
   *b = '\0';
}


#define CONTENTTYPE_IMAGE  0
#define CONTENTTYPE_JAVA   1
#define CONTENTTYPE_HTML   2
#define CONTENTTYPE_XML    3
#define CONTENTTYPE_CSS    4
#define CONTENTTYPE_TEXT   5
#define CONTENTTYPE_JSON   6
#define CONTENTTYPE_STREAM 7

static char * contentType2String( 
   int contentType )
{
   switch (contentType)
   {
      case CONTENTTYPE_IMAGE:    return "image";
      case CONTENTTYPE_JAVA:     return "java";
      case CONTENTTYPE_HTML:     return "html";
      case CONTENTTYPE_XML:      return "xml";
      case CONTENTTYPE_CSS:      return "css";
      case CONTENTTYPE_TEXT:     return "text";
      case CONTENTTYPE_JSON:     return "json";
      case CONTENTTYPE_STREAM:   return "stream";
   }
   return "error";
}


/* If the URL has no file suffix, we cannot know the content type.
 * Historically it was guessed that the file type was HTML.
 * Now we try to guess the file type looking at the URL because sometimes there are clues.
 * If there are no clues, we return the default_type.
 */
static int guessContentType( 
   char * url,
   int    default_type )
{
   char * path;
   char * r;

   /* strip http://  https://   ftp://  */
   r = strstr( url, "://" );
   if (r != NULL  &&  r-url < 7)
      url = r + 3;

   path = strchr( url, '/' );
   if (path == NULL)
      return CONTENTTYPE_HTML;

   if (strncmp( url, "googleadservices.com/pagead/conversion/", 39 ) == 0)
      return CONTENTTYPE_IMAGE;

   /* object is xml, html, java or video ...  :-)  so we choose text because it
    * sends an empty object.
    */
   if (strncmp( url, "googleads.g.doubleclick.net/pagead/ads", 38 ) == 0)
      return CONTENTTYPE_TEXT;

   if (strcmp( url, "a.analytics.yahoo.com/fpc.pl" ) == 0)
      return CONTENTTYPE_JAVA;
   if (strcmp( url, "a.analytics.yahoo.com/p.pl" ) == 0)
      return CONTENTTYPE_IMAGE;

   if (strcmp( url, "ping.chartbeat.net/ping" ) == 0)
      return CONTENTTYPE_IMAGE;

   if (strncmp( url, "www.youtube.com/cp/", 15 ) == 0  ||
       strncmp( url, "www.youtube.com/p/", 14 ) == 0   ||
       strncmp( url, "www.youtube.com/v/", 14 ) == 0   ||
       strncmp( url, "www.youtube.com/videoplayback", 29 ) == 0)
      return CONTENTTYPE_STREAM;

   if (strncmp( url, "b.scorecardresearch.com/", 24 ) == 0)
      return CONTENTTYPE_IMAGE;

   if (strcmp( url, "ad.yieldmanager.com/imp" ) == 0  ||  strcmp( url, "ad.yieldmanager.com/st" ) == 0)
      return CONTENTTYPE_JAVA;

   if (strncmp( url, "adserver.adtech.de/addyn", 24 ) == 0)
      return CONTENTTYPE_JAVA;

   if (strcmp( url, "pagead2.googlesyndication.com/pagead/imgad" ) == 0)	/* gif or swf */
      return CONTENTTYPE_STREAM;

   if (strstr( url, ".overture.com/js" ) != NULL)
      return CONTENTTYPE_JAVA;

   if (strstr( url, ".doubleclick.net/adj/" ) != NULL  ||
       strstr( url, ".doubleclick.net/pfadj/" ) != NULL)
      return CONTENTTYPE_JAVA;

   if (strstr( url, ".doubleclick.net/imp" ) != NULL)
      return CONTENTTYPE_IMAGE;

   if (strncmp( url, "view.atdmt.com/", 15 ) == 0  && path != NULL)
   {
      if (strncmp( path, "/action/", 8 ) == 0)
         return CONTENTTYPE_IMAGE;
      if (strstr( path, "/jview/" ) != NULL)
	 return CONTENTTYPE_JAVA;
   }

   if (strncmp( url, "static.ak.connect.facebook.com/connect.php", 42 ) == 0)
      return CONTENTTYPE_JAVA;

   if (strcmp( url, "secure-us.imrworldwide.com/cgi-bin/m" ) == 0)
      return CONTENTTYPE_IMAGE;

   if (strncmp( url, "ftjcfx.com/image-", 17 ) == 0)
      return CONTENTTYPE_IMAGE;
   if (strncmp( url, "lduhtrp.net/image-", 18 ) == 0)
      return CONTENTTYPE_IMAGE;
   if (strstr( url, "img.pheedo.com/img.phdo" ) != NULL)
      return CONTENTTYPE_IMAGE;

   if (strstr( path, "/realmedia/ads/" ) != NULL)
   {
      if (strstr( path, "/adstream_jx" ) != NULL  ||
          strstr( path, "/adstream_mjx" ) != NULL)
	 return CONTENTTYPE_JAVA;

      if (strstr( path, "/adstream_lx" ) != NULL  ||
          strstr( path, "/adstream_nx" ) != NULL)
         return CONTENTTYPE_IMAGE;

      if (strstr( path, "/ads/cap.cgi" ) != NULL)		/* /adstream.cap */
         return CONTENTTYPE_IMAGE;
   }

   if (strstr( url, "overture.com/ls_js_" ) != NULL)
      return CONTENTTYPE_JAVA;

   if (strstr( path, "/scripts/beacon.dll" ) != NULL  ||
       strstr( path, "/scripts/beacon2.dll" ) != NULL)
      return CONTENTTYPE_IMAGE;

   if (strstr( path, "/javascript/" ) != NULL  ||
       strstr( path, "/ajaxpro/" ) != NULL)
      return CONTENTTYPE_JAVA;

   r = strstr( path, "/js.php" );
   if (r != NULL  &&  strlen(r) == 7)
      return CONTENTTYPE_JAVA;
   r = strstr( path, "/javascript.php" );
   if (r != NULL  &&  strlen(r) == 15)
      return CONTENTTYPE_JAVA;

   r = strstr( path, "/css.php" );
   if (r != NULL  &&  strlen(r) == 8)
      return CONTENTTYPE_CSS;

   r = strstr( path, "/image.php" );
   if (r != NULL  &&  strlen(r) == 10)
      return CONTENTTYPE_IMAGE;
   if (strstr( path, "/image.php/" ) != NULL)
      return CONTENTTYPE_IMAGE;

   if (strstr( path, "/js.ng/" ) != NULL  ||
       strstr( path, "/js/" ) != NULL)
      return CONTENTTYPE_JAVA;

   if (strstr( path, "/scripts/" ) != NULL  ||
       strstr( path, "/script/" ) != NULL)
      return CONTENTTYPE_JAVA;

   if (strncmp( url, "xml.", 4 ) == 0)
      return CONTENTTYPE_XML;

   /* SiteCatalyst beacon */
   if (strncmp( path, "/b/ss/", 6 ) == 0  &&
       (strstr( path, "/fas" ) != NULL  ||
        strstr( path, "/h." ) != NULL   ||
	strstr( path, "/g." ) != NULL ))
      return CONTENTTYPE_IMAGE;

   if (strstr( url, ".channel.facebook.com/x/" ) != NULL)
      return CONTENTTYPE_JAVA;
   if (strstr( url, ".channel.facebook.com/p" ) != NULL)
      return CONTENTTYPE_TEXT;

   if (strstr( path, "/xml-rpc" ) != NULL)
      return CONTENTTYPE_XML;

   if (strcmp( path, "/open/1" ) == 0)
      return CONTENTTYPE_STREAM;

#if 0
   if (strstr( path, "/video/" ) != NULL)
      return CONTENTTYPE_STREAM;
#endif

   if (strncmp( url, "pixel.", 6 ) == 0)
      return CONTENTTYPE_IMAGE;

   r = strstr( path, "/pixel" );
   if (r != NULL  &&  strlen(r) == 6)
      return CONTENTTYPE_IMAGE;

   return default_type;
}


static void AnswerHttpUrlBlocked( 
   int       fd, 
   int       lang, 
   char *    parameters, 
   const char *  imagesDirectory )
{
   char *    p;
   char *    pptr;
   time_t    now_t;
   struct tm t;
   char *    mode = "default";
   char *    color = "orange";
   char *    size = "normal";
   char *    clientaddr = "";
   char *    clientname = "";
   char *    clientuser = "";
   char *    source = "";
   char *    category = "unknown";
   char *    textcolor = "white";
   char *    bgcolor = "black";
   char *    titlesize = "+0";
   char *    textsize = "+0";
   int       contentLength;
   int       contentType;
   char      header[2048];
   char      admin[2048];
   char      url[8192];
   char      whyblocked[8192];
   char      text[8192];
   char      content[8192+8192+1200];
   char      moreInfoParams[8192+1200];

#if 0
   if (UFDBglobalDebug || UFDBglobalDebugHttpd)
      ufdbLogMessage( "AnswerHttpUrlBlocked: %d %s %s", lang, parameters, imagesDirectory );
#endif

   if (*parameters == '&')  /* Apache mod_rewrite adds an '&' */
      parameters++;
   content[0] = '\0';
   contentLength = 0;
   strcpy( admin, "The network administrator" );
   strcpy( url, "unknown" );

   /*
    * valid parameters are:
    * admin
    * mode
    * color
    * size
    * clientaddr 
    * clientname
    * clientuser/clientident
    * clientgroup/source
    * targetgroup/category
    * url
    */
   p = strtok_r( parameters, "&", &pptr );
   while (p != NULL)
   {
      char * value = strchr( p, '=' );
      if (value == NULL)
      {
         ufdbLogError( "AnswerHttpUrlBlocked: parameter %s has no value", p );
	 p = strtok_r( NULL, "&", &pptr );
	 continue;
      }
      *value = '\0';
      value++;
      if (strcmp( p, "admin" ) == 0)
         convertHTTPcodes( admin, value );
      else if (strcmp( p, "mode" ) == 0)
         mode = value;
      else if (strcmp( p, "color" ) == 0  ||  strcmp( p, "colour" ) == 0)
         color = value;
      else if (strcmp( p, "size" ) == 0)
         size = value;
      else if (strcmp( p, "clientaddr" ) == 0)
         clientaddr = value;
      else if (strcmp( p, "clientname" ) == 0)
         clientname = value;
      else if (strcmp( p, "clientuser" ) == 0  ||  strcmp( p, "clientident" ) == 0)
         clientuser = value;
      else if (strcmp( p, "clientgroup" ) == 0  ||  strcmp( p, "source" ) == 0  ||  strcmp( p, "srcclass" ) == 0)
         source = value;
      else if (strcmp( p, "category" ) == 0  ||  strcmp( p, "targetgroup" ) == 0  ||  strcmp( p, "targetclass" ) == 0)
         category = value;
      else if (strcmp( p, "url" ) == 0)
      {
         convertHTTPcodes( url, value );

	 /* strip the URL */
	 p = strchr( url, '?' );
	 if (p != NULL)
	    *p = '\0';
	 p = strchr( url, ';' );
	 if (p != NULL)
	    *p = '\0';
	 p = strchr( url, '&' );
	 if (p != NULL)
	    *p = '\0';

	 /* url is the last parameter so stop parsing */
	 break;
      }
      else 
         ufdbLogError( "AnswerHttpUrlBlocked: unknown parameter '%s'", p );

      p = strtok_r( NULL, "&", &pptr );
   }

   content[0] = '\0';

   p = strstr( url, "://" );
   if (p == NULL)
      pptr = strchr( url, '/' );
   else
      pptr = strchr( p+3, '/' );

   if (pptr == NULL)
   {
      contentType = guessContentType( url, CONTENTTYPE_HTML );
      p = NULL;
   }
   else
   {
      p = strrchr( pptr, '.' );
      if (p == NULL  || strlen(p) > 6)
      {
	 p = NULL;
	 contentType = guessContentType( url, CONTENTTYPE_HTML );
      }
      else
      {
	 char * ch;

	 p++;
	 ch = p;
	 while (*ch != '\0')
	 {
	    if (isupper(*ch))
	       *ch = tolower(*ch);
	    ch++;
	 }

	 if (strcmp( p, "bmp" ) == 0  ||
	     strcmp( p, "gif" ) == 0  ||
	     strcmp( p, "ico" ) == 0  ||
	     strcmp( p, "img" ) == 0  ||
	     strcmp( p, "jpg" ) == 0  ||
	     strcmp( p, "jpe" ) == 0  ||
	     strcmp( p, "jpeg" ) == 0  ||
	     strcmp( p, "png" ) == 0  ||
	     strcmp( p, "tiff" ) == 0)
	 {
	    contentType = CONTENTTYPE_IMAGE;
	 }
	 else if (strcmp( p, "css" ) == 0)
	 {
	    contentType = CONTENTTYPE_CSS;
	 }
	 else if (strcmp( p, "js" ) == 0  ||
		  strcmp( p, "jar" ) == 0)
	 {
	    contentType = CONTENTTYPE_JAVA;
	 }
	 else if (strcmp( p, "xml" ) == 0  ||
		  strcmp( p, "sxml" ) == 0  ||
		  strcmp( p, "rss" ) == 0)
	 {
	    contentType = CONTENTTYPE_XML;
	 }
	 else if (strcmp( p, "cab" ) == 0   ||
		  strcmp( p, "class" ) == 0  ||
		  strcmp( p, "divx" ) == 0  ||
		  strcmp( p, "h264" ) == 0  ||
		  strcmp( p, "mpg" ) == 0  ||
		  strcmp( p, "mpeg" ) == 0  ||
		  strcmp( p, "ogv" ) == 0  ||
		  strcmp( p, "qt" ) == 0  ||
		  strcmp( p, "ra" ) == 0  ||
		  strcmp( p, "ram" ) == 0  ||
		  strcmp( p, "rv" ) == 0  ||
		  strcmp( p, "wmv" ) == 0  ||
		  strcmp( p, "avi" ) == 0  ||
		  strcmp( p, "mov" ) == 0  ||
		  strcmp( p, "swf" ) == 0  ||
		  strcmp( p, "mp4" ) == 0  ||
		  strcmp( p, "m4v" ) == 0  ||
		  strcmp( p, "flv" ) == 0  ||
		  strcmp( p, "bz2" ) == 0   ||
		  strcmp( p, "dat" ) == 0  ||
		  strcmp( p, "doc" ) == 0  ||
		  strcmp( p, "gz" ) == 0   ||
		  strcmp( p, "mp3" ) == 0  ||
		  strcmp( p, "msi" ) == 0  ||
		  strcmp( p, "mst" ) == 0  ||
		  strcmp( p, "ppt" ) == 0  ||
		  strcmp( p, "pdf" ) == 0  ||
		  strcmp( p, "rar" ) == 0  ||
		  strcmp( p, "tar" ) == 0  ||
		  strcmp( p, "ttf" ) == 0  ||
		  strcmp( p, "xls" ) == 0  ||
		  strcmp( p, "zip" ) == 0   ||
		  strcmp( p, "bin" ) == 0)
	 {
	    contentType = CONTENTTYPE_STREAM;
	 }
	 else if (strcmp( p, "txt" ) == 0  ||
		  strcmp( p, "csv" ) == 0)
	 {
	    contentType = CONTENTTYPE_TEXT;
	 }
	 else if (strcmp( p, "json" ) == 0)
	 {
	    contentType = CONTENTTYPE_JSON;
	 }
	 else if (strcmp( p, "htm" ) == 0  ||
		  strcmp( p, "shtml" ) == 0  ||
		  strcmp( p, "dhtml" ) == 0  ||
		  strcmp( p, "html" ) == 0)
	 {
	    contentType = CONTENTTYPE_HTML;
	 }
	 else
	 {
	    contentType = guessContentType( url, CONTENTTYPE_HTML );
	 }
      }
   }

#if 1
   /*  transparent  grey  ads  xml  The network administrator  http://adnet.bluebillywig.com/crossdomain.xml  */
   if (UFDBglobalDebug || UFDBglobalDebugHttpd)
      ufdbLogMessage( "AnswerHttpUrlBlocked:  %s  %s  %s  %s  \"%s\"  \"%s\"  %s", mode, color, category, 
                      contentType2String(contentType), p==NULL ? "" : p, admin, url );
#endif

   if (strcmp( color, "orange" ) == 0)
   {
      textcolor = "white";
      bgcolor = "#ee8811";
   }
   else if (strcmp( color, "white" ) == 0)
   {
      textcolor = "#3f003f";
      bgcolor = "white";
   }
   else if (strcmp( color, "black" ) == 0)
   {
      textcolor = "#f0f0f0";
      bgcolor = "black";
   }
   else if (strcmp( color, "red" ) == 0)
   {
      textcolor = "#f0f0f0";
      bgcolor = "red";
   }
   else if (strcmp( color, "grey" ) == 0  ||  strcmp( color, "gray" ) == 0)
   {
      textcolor = "#111111";
      bgcolor = "#c2c2c2";
   }
   else
   {
      textcolor = "white";
      bgcolor = "#ee8811";
   }

   if (strcmp( "size", "normal" ) == 0)
   {
      titlesize = "+2";
      textsize = "+0";
   }
   else if (strcmp( "size", "small" ) == 0)
   {
      titlesize = "+1";
      textsize = "-1";
   }
   else if (strcmp( "size", "large" ) == 0)
   {
      titlesize = "+3";
      textsize = "+1";
   }
   else
   {
      titlesize = "+2";
      textsize = "+0";
   }

   now_t = time( NULL );
   gmtime_r( &now_t, &t );

   sprintf( moreInfoParams, "ufdbcat=%s&ufdbsrc=%s&ufdburl=%s", category, source, url );

   /*
    * Check for graphics and send a bitmap.
    */
   if (contentType == CONTENTTYPE_IMAGE)
   {
      int  ifd;
      char file[1024];

      /* Send an image for URLs that have one of the following suffixes:
       * bmp, gif, jpg, jpeg, png, ico.
       * with special png file for the category ads.
       */
      sprintf( header, 
	       "HTTP/1.0 200 OK\r\n"
	       "Date: %3.3s, %02d %3.3s %4d %02d:%02d:%02d GMT\r\n"
	       "Server: ufdbhttpd/" VERSION "\r\n"
	       "Cache-Control: max-age=300\r\n"
	       "Connection: close\r\n"
	       "Content-Type: image/png\r\n"
	       "\r\n",
	       &"SunMonTueWedThuFriSat"[t.tm_wday*3],
	       t.tm_mday,
	       &"JanFebMarAprMayJunJulAugSepOctNovDec"[t.tm_mon*3],
	       t.tm_year + 1900,
	       t.tm_hour, t.tm_min, t.tm_sec
	       );
      if (strcmp( category, "ads" ) == 0  ||
          strcmp( category, "always-block" ) == 0  ||
	  strcmp( category, "alwaysblock" ) == 0 )
      {
         if (strcmp( mode, "noads" ) == 0)
	    p = "no-ads.png";
	 else if (strcmp( mode, "cross" ) == 0)
	    p = "smallcross.png";
	 else if (strcmp( mode, "square" ) == 0)
	    p = "square.png";
         else if (strcmp( mode, "simple-red" ) == 0)
	    p = "transparent.png";
	 else  /* transparent */
	    p = "transparent.png";
	 sprintf( file, "%s/%s", imagesDirectory, p );
	 ifd = open( file, O_RDONLY );
	 if (ifd < 0)
	 {
	    ufdbLogError( "cannot open image file %s: %s *****", file, strerror(errno) );
	    contentLength = 0;
	 }
	 else
	 {
	    contentLength = read( ifd, content, sizeof(content)-1 );
	    if (contentLength < 0)
	       ufdbLogError( "cannot read image file %s: %s *****", file, strerror(errno) );
	    close( ifd );
	 }
      }
      else   /* send bitmap; category is NOT "ads" */
      {
	 char * langStr;

	 switch (lang)
	 {
	    case LANG_IND_NL:	langStr = "nl";  break;
	    case LANG_IND_DE:	langStr = "de";  break;
	    case LANG_IND_PL:	langStr = "pl";  break;
	    case LANG_IND_IT:	langStr = "it";  break;
	    case LANG_IND_ES:	langStr = "es";  break;
	    case LANG_IND_PT:	langStr = "pt";  break;
	    case LANG_IND_FR:	langStr = "fr";  break;
	    case LANG_IND_TR:	langStr = "tr";  break;
	    case LANG_IND_SV:	langStr = "sv";  break;
	    case LANG_IND_EN:
	    default: 		langStr = "en";
	 }
	 if (strcmp( mode, "cross" ) == 0)
	    sprintf( file, "%s/smallcross.png", imagesDirectory );
	 else if (strcmp( mode, "square" ) == 0)
	    sprintf( file, "%s/square.png", imagesDirectory );
         else if (strcmp( mode, "simple-red" ) == 0)
	    sprintf( file, "%s/transparent.png", imagesDirectory );
	 else if (strcmp( mode, "transparent" ) == 0  ||  strcmp( mode, "transparant" ) == 0)
	    sprintf( file, "%s/transparent.png", imagesDirectory );
	 else
	    sprintf( file, "%s/forbidden-normal-%s.png", imagesDirectory, langStr );
	 ifd = open( file, O_RDONLY );
	 if (ifd < 0)
	 {
	    ufdbLogError( "cannot open image file %s: %s *****", file, strerror(errno) );
	    contentLength = 0;
	 }
	 else
	 {
	    contentLength = read( ifd, content, sizeof(content)-1 );
	    if (contentLength < 0)
	       ufdbLogError( "cannot read image file %s: %s *****", file, strerror(errno) );
	    close( ifd );
	 }
      }
   }
   else if (contentType == CONTENTTYPE_JAVA)
   {
      /*
       * send empty reply for URLs that have one of the following suffixes:
       * js, jar
       */
      sprintf( header, 
	       "HTTP/1.0 200 OK\r\n"
	       "Date: %3.3s, %02d %3.3s %4d %02d:%02d:%02d GMT\r\n"
	       "Server: ufdbhttpd/" VERSION "\r\n"
	       "Cache-Control: max-age=300\r\n"
	       "Connection: close\r\n"
	       "Content-Type: application/x-javascript\r\n"
	       "\r\n",
	       &"SunMonTueWedThuFriSat"[t.tm_wday*3],
	       t.tm_mday,
	       &"JanFebMarAprMayJunJulAugSepOctNovDec"[t.tm_mon*3],
	       t.tm_year + 1900,
	       t.tm_hour, t.tm_min, t.tm_sec
	       );
      strcpy( content, "\r\n" );
      contentLength = 2;
   }
   else if (contentType == CONTENTTYPE_STREAM)
   {
      /*
       * send empty reply for URLs that have one of the following suffixes:
       * rar, tar, zip, mp3, ...
       */
      sprintf( header, 
	       "HTTP/1.0 200 OK\r\n"
	       "Date: %3.3s, %02d %3.3s %4d %02d:%02d:%02d GMT\r\n"
	       "Server: ufdbhttpd/" VERSION "\r\n"
	       "Cache-Control: max-age=300\r\n"
	       "Connection: close\r\n"
	       "Content-Type: application/octet-stream\r\n"
	       "\r\n",
	       &"SunMonTueWedThuFriSat"[t.tm_wday*3],
	       t.tm_mday,
	       &"JanFebMarAprMayJunJulAugSepOctNovDec"[t.tm_mon*3],
	       t.tm_year + 1900,
	       t.tm_hour, t.tm_min, t.tm_sec
	       );
      content[0] = '\0';
      contentLength = 0;
   }
   else if (contentType == CONTENTTYPE_JSON)
   {
      /*
       * send empty reply for URLs that have one of the following suffixes:
       * json
       */
      sprintf( header, 
	       "HTTP/1.0 200 OK\r\n"
	       "Date: %3.3s, %02d %3.3s %4d %02d:%02d:%02d GMT\r\n"
	       "Server: ufdbhttpd/" VERSION "\r\n"
	       "Cache-Control: max-age=300\r\n"
	       "Connection: close\r\n"
	       "Content-Type: application/json\r\n"
	       "\r\n",
	       &"SunMonTueWedThuFriSat"[t.tm_wday*3],
	       t.tm_mday,
	       &"JanFebMarAprMayJunJulAugSepOctNovDec"[t.tm_mon*3],
	       t.tm_year + 1900,
	       t.tm_hour, t.tm_min, t.tm_sec
	       );
      strcpy( content, "\r\n" );
      contentLength = 2;
   }
   else if (contentType == CONTENTTYPE_CSS)
   {
      /*
       * send empty reply for URLs that have one of the following suffixes:
       * css
       */
      sprintf( header, 
	       "HTTP/1.0 200 OK\r\n"
	       "Date: %3.3s, %02d %3.3s %4d %02d:%02d:%02d GMT\r\n"
	       "Server: ufdbhttpd/" VERSION "\r\n"
	       "Cache-Control: max-age=300\r\n"
	       "Connection: close\r\n"
	       "Content-Type: text/css\r\n"
	       "\r\n",
	       &"SunMonTueWedThuFriSat"[t.tm_wday*3],
	       t.tm_mday,
	       &"JanFebMarAprMayJunJulAugSepOctNovDec"[t.tm_mon*3],
	       t.tm_year + 1900,
	       t.tm_hour, t.tm_min, t.tm_sec
	       );
      strcpy( content, "\r\n" );
      contentLength = 2;
   }
   else if (contentType == CONTENTTYPE_TEXT)
   {
      /*
       * send empty reply for URLs that have one of the following suffixes:
       * txt
       */
      sprintf( header, 
	       "HTTP/1.0 200 OK\r\n"
	       "Date: %3.3s, %02d %3.3s %4d %02d:%02d:%02d GMT\r\n"
	       "Server: ufdbhttpd/" VERSION "\r\n"
	       "Cache-Control: max-age=300\r\n"
	       "Connection: close\r\n"
	       "Content-Type: text/plain\r\n"
	       "\r\n",
	       &"SunMonTueWedThuFriSat"[t.tm_wday*3],
	       t.tm_mday,
	       &"JanFebMarAprMayJunJulAugSepOctNovDec"[t.tm_mon*3],
	       t.tm_year + 1900,
	       t.tm_hour, t.tm_min, t.tm_sec
	       );
      content[0] = '\0';
      contentLength = 0;
   }
   else if (contentType == CONTENTTYPE_XML)
   {
      sprintf( header, 
	       "HTTP/1.0 200 OK\r\n"
	       "Date: %3.3s, %02d %3.3s %4d %02d:%02d:%02d GMT\r\n"
	       "Server: ufdbhttpd/" VERSION "\r\n"
	       "Cache-Control: max-age=300\r\n"
	       "Content-Type: text/xml\r\n"
	       "Connection: close\r\n"
	       "\r\n",
	       &"SunMonTueWedThuFriSat"[t.tm_wday*3],
	       t.tm_mday,
	       &"JanFebMarAprMayJunJulAugSepOctNovDec"[t.tm_mon*3],
	       t.tm_year + 1900,
	       t.tm_hour, t.tm_min, t.tm_sec
	       );
#if 0
      p = strstr( url, "/crossdomain.xml" );
      if (p != NULL)
      {
	 strcpy( content, "<?xml version=\"1.0\"?>\r\n"
			  "<cross-domain-policy>\r\n"
			  "   <allow-access-from domain=\"*\" />\r\n"
			  "</cross-domain-policy>\r\n"  );
      }
      else
#endif
      {
	 strcpy( content, "<?xml version=\"1.0\" encoding=\"utf-8\"?>\r\n"
			  "<ufdbguardd>\r\n"
			  "   <dummy value=\"0\" />\r\n"
			  "</ufdbguardd>\r\n"  );
      }
      contentLength = strlen( content );
   }
   else		/**********************************************  contentType == CONTENTTYPE_HTML  */
   {
      sprintf( header, 
	       "HTTP/1.0 200 OK\r\n"
	       "Date: %3.3s, %02d %3.3s %4d %02d:%02d:%02d GMT\r\n"
	       "Server: ufdbhttpd/" VERSION "\r\n"
	       "Cache-Control: max-age=300\r\n"
	       "Connection: close\r\n"
	       "Content-Type: text/html\r\n"
	       "\r\n",
	       &"SunMonTueWedThuFriSat"[t.tm_wday*3],
	       t.tm_mday,
	       &"JanFebMarAprMayJunJulAugSepOctNovDec"[t.tm_mon*3],
	       t.tm_year + 1900,
	       t.tm_hour, t.tm_min, t.tm_sec
	       );

      if (strcmp( category, "fatal-error" ) == 0)		/* HTML, fatal-error */
      {
	 strcpy( text, _fatal_error_text );
	 sprintf( content,
		  "<html>\r\n"
		  "<head>\r\n"
		  "<title>%s</title>\r\n"
		  "</head>\r\n"
		  "<body topmargin=1 leftmargin=1 marginheight=1 marginwidth=1 bgcolor=\"%s\" text=\"%s\">\r\n"
		  "%s\r\n"
		  "</body>\r\n"
		  "</html>\r\n"
		  "<!-- lang:%d %s %s %s %s %s -->",
		  _title[lang],
		  bgcolor, textcolor,
		  text,
		  lang, color, mode, category, contentType2String(contentType), url );
	 contentLength = strlen( content );
      }
      else if (strcmp( category, "loading-database" ) == 0)	/* HTML, loading-database */
      {
	 strcpy( text, _loading_database_text );
	 sprintf( content,
		  "<html>\r\n"
		  "<head>\r\n"
		  "<title>%s</title>\r\n"
		  "</head>\r\n"
		  "<body topmargin=1 leftmargin=1 marginheight=1 marginwidth=1 bgcolor=\"%s\" text=\"%s\">\r\n"
		  "%s\r\n"
		  "</body>\r\n"
		  "</html>\r\n"
		  "<!-- lang:%d %s %s %s %s %s -->",
		  _title[lang],
		  bgcolor, textcolor,
		  text,
		  lang, color, mode, category, contentType2String(contentType), url );
	 contentLength = strlen( content );
      }
      else							/* HTML */
      {
	 sprintf( whyblocked, "blocked by ufdbGuard: url=%s", url );
	 if (category[0] != '\0')
	 {
	    strcat( whyblocked, " category=" );
	    strcat( whyblocked, category );
	 }
	 if (source[0] != '\0')
	 {
	    strcat( whyblocked, " source=" );
	    strcat( whyblocked, source );
	 }

	 /*
	  * send ads-specific reply for the ads category.
	  */
	 if (strcmp( category, "ads" ) == 0  ||			/* HTML, ADS */
	     strcmp( category, "always-block" ) == 0  ||
	     strcmp( category, "alwaysblock" ) == 0 )
	 {
	    if (strcmp( mode, "noads" ) == 0)
	       sprintf( text, " <a title=\"%s\" target=\"_blank\">no ads</a> ", whyblocked );
	    else if (strcmp( mode, "square" ) == 0)
	       sprintf( text, " <a title=\"%s\" target=\"_blank\">[]</a> ", whyblocked );
	    else if (strcmp( mode, "cross" ) == 0)
	       sprintf( text, " <a title=\"%s\" target=\"_blank\">x</a> ", whyblocked );
	    else if (strcmp( mode, "simple-red" ) == 0)
	       sprintf( text, " <font color=red><i><a title=\"%s\" target=\"_blank\">%s</a></i></font> ", whyblocked, category );
	    else  if (strcmp( mode, "transparent" ) == 0  ||  strcmp( mode, "transparant" ) == 0)
	       strcpy( text, " " );
	    else  /* default */
	    {
	       sprintf( text, " <font color=\"%s\"><i><a title=\"%s\" target=\"_blank\">%s %s%s</a></i></font> ", 
	                textcolor, whyblocked, _explain_1[lang], category, _explain_2[lang] );
	    }

	    if (strcmp( mode, "transparent" ) == 0  ||  strcmp( mode, "transparant" ) == 0)
	    {
	       /* no bgcolor and font color */
	       sprintf( content,
			"<html>\r\n"
			"<head>\r\n"
			"<title>%s</title>\r\n"
			"</head>\r\n"
			"<body topmargin=1 leftmargin=1 marginheight=1 marginwidth=1>\r\n"
			"<center>%s</center>\r\n"
			"</body>\r\n"
			"</html>\r\n"
			"<!-- lang:%d %s %s %s %s %s -->",
			_title[lang],
			text,
			lang, color, mode, category, contentType2String(contentType), url );
	    }
	    else					/* HTML, ADS, NOT transparent */
	    {
	       char infotext[1024];

	       if (strcmp( mode, "simple-red" ) == 0)
	       {
	          infotext[0] = '\0';
		  bgcolor = "ffcccc";
		  textcolor = "red";
	       }
	       else
	       {
		  int  n;
		  n = 0;
		  infotext[n] = '\0';
		  if (source[0] != '\0'  &&  strcmp( source, "unknown" ) != 0)
		     n += sprintf( &infotext[n], "source=%s &nbsp; ", source );
		  if (clientuser[0] != '\0'  &&  strcmp( clientuser, "unknown" ) != 0)
		     n += sprintf( &infotext[n], "user=%s &nbsp; ", clientuser );
		  if (clientaddr[0] != '\0'  &&  strcmp( clientaddr, "unknown" ) != 0)
		     n += sprintf( &infotext[n], "client=%s &nbsp; ", clientaddr );
		  if (clientname[0] != '\0')
		     n += sprintf( &infotext[n], "clientname=%s &nbsp; ", clientname );
	       }

	       sprintf( content,
			"<html>\r\n"
			"<head>\r\n"
			"<title>%s</title>\r\n"
			"</head>\r\n"
			"<body topmargin=1 leftmargin=1 marginheight=1 marginwidth=1 bgcolor=\"%s\" text=\"%s\">\r\n"
			"<center>\r\n"
			"<font size=\"%s\">%s</font>\r\n"
			"<br>\r\n&nbsp;<p />\r\n"
			"<font size=\"-3\">%s</font>\r\n"
			"</center>\r\n"
			"</body>\r\n"
			"</html>\r\n"
			"<!-- lang:%d %s %s %s %s %s -->",
			_title[lang],
			bgcolor, textcolor,
			textsize, text, 
			infotext,
			lang, color, mode, category, contentType2String(contentType), url );
	    }
	    contentLength = strlen( content );
	 }
	 else					/* HTML, NO ADS */
	 {
	    if (strcmp( mode, "transparent" ) == 0  ||  strcmp( mode, "transparant" ) == 0)
	    {
	       sprintf( content,
			"<html>\r\n"
			"<head>\r\n"
			"<title>%s</title>\r\n"
			"</head>\r\n"
			"<body topmargin=1 leftmargin=1 marginheight=1 marginwidth=1>\r\n"
			"<center>\r\n"
			"<i>%s</i><p />\r\n"
			"<font size=\"%s\">\r\n"
			"%s <i>%s</i>%s <br>\r\n"
			"URL: <tt>%s</tt> <br>\r\n"
			"<br>\r\n"
			"<a href=\"javascript:history.go(-1);\">%s</a>. <br>\r\n"
			"<br>\r\n"
			"%s?%s%s\r\n"
			"</center>\r\n"
			"</font>\r\n"
			"</body>\r\n"
			"</html>\r\n"
			"<!-- lang:%d %s %s %s %s %s -->",
			_title[lang],
			_forbidden[lang],
			textsize,
			_explain_1[lang], category, _explain_2[lang],
			url,
			_goBack[lang],
			_moreInfo1[lang], moreInfoParams, _moreInfo2[lang],
			lang, color, mode, category, contentType2String(contentType), url );
	    }
	    else if (strcmp( mode, "simple-red" ) == 0)		/* HTML, NO ADS, simple-red */
	    {
	       sprintf( content,
			"<html>\r\n"
			"<head>\r\n"
			"<title>%s</title>\r\n"
			"</head>\r\n"
			"<body topmargin=1 leftmargin=1 marginheight=1 marginwidth=1 bgcolor=\"ffcccc\" link=\"red\" alink=\"red\" vlink=\"red\" text=\"red\">\r\n"
			"<center>\r\n"
			"<a href=\"%s?%s\" title=\"%s\" target=\"_blank\"> "
			   "%s<br>\r\n"
			   "<i>%s</i></a>\r\n"
			"</center>\r\n"
			"</body>\r\n"
			"</html>\r\n"
			"<!-- lang:%d %s %s %s %s %s -->",
			_title[lang],
			UFDB_EXPLAIN_DENY_REASON_URL, moreInfoParams, whyblocked,
			_forbidden[lang], 
			category,
			lang, color, mode, category, contentType2String(contentType), url );
	    }
	    else					/* HTML, NO ADS, MODE NOT transparent/simple-red */
	    {
	       sprintf( content,
			"<html>\r\n"
			"<head>\r\n"
			"<title>%s</title>\r\n"
			"</head>\r\n"
			"<body topmargin=1 leftmargin=1 marginheight=1 marginwidth=1 bgcolor=\"%s\" link=\"%s\" alink=\"%s\" vlink=\"%s\" text=\"%s\">\r\n"
			"<center>\r\n"
			"<a href=\"%s?%s\" title=\"%s\" target=\"_blank\"> "
			   "<font size=\"%s\">%s</font></a><p />\r\n"
			"<font size=\"%s\">\r\n"
			"%s <i>%s</i>%s</a><br>\r\n"
			"URL: <tt>%s</tt> <br>\r\n"
			"<p>\r\n"
			"<a title=\"%s\" href=\"javascript:history.go(-1);\">%s</a>. <br>\r\n"
			"%s\r\n"
			"<p>\r\n"
			"%s?%s%s\r\n"
			"</font>\r\n"
			"</center>\r\n"
			"</body>\r\n"
			"</html>\r\n"
			"<!-- lang:%d %s %s %s %s %s -->",
			_title[lang],
			bgcolor, textcolor, textcolor, textcolor, textcolor,			/* body */
			UFDB_EXPLAIN_DENY_REASON_URL, moreInfoParams, whyblocked,
			titlesize, _forbidden[lang],		/* font */
			textsize,				/* font */
			_explain_1[lang], category, _explain_2[lang],
			url,
			whyblocked, _goBack[lang],
			admin,
			_moreInfo1[lang], moreInfoParams, _moreInfo2[lang],
			lang, color, mode, category, contentType2String(contentType), url );
	    }
	    contentLength = strlen( content );
	 }
      }
   }

#if 0
   if (UFDBglobalDebug)
      ufdbLogMessage( "AnswerHttpUrlBlocked: strlen(header): %d  contentLength: %d", strlen(header), contentLength );
#endif

   writeBuffer( fd, header, strlen(header) );
   if (contentLength > 0)
      writeBuffer( fd, content, contentLength );
}


static int FindLanguageIndex( char * language )
{
   if (strcmp( language, "en" ) == 0)
      return LANG_IND_EN;
   else if (strcmp( language, "nl" ) == 0)
      return LANG_IND_NL;
   else if (strcmp( language, "de" ) == 0)
      return LANG_IND_DE;
   else if (strcmp( language, "pl" ) == 0)
      return LANG_IND_PL;
   else if (strcmp( language, "it" ) == 0)
      return LANG_IND_IT;
   else if (strcmp( language, "pt" ) == 0)
      return LANG_IND_PT;
   else if (strcmp( language, "fr" ) == 0)
      return LANG_IND_FR;
   else if (strcmp( language, "tr" ) == 0)
      return LANG_IND_TR;
   else if (strcmp( language, "sv" ) == 0)
      return LANG_IND_SV;
   else
      return -1;
}


static int FindLanguage( char * headers )
{
   char * l;
   char * end;
   int    ind;
   int    length;
   char   language[64];

   l = strstr( headers, "Accept-Language:" );
   if (l == NULL)
      return LANG_IND_EN;
   l += sizeof("Accept-Language:") - 1;

   while (*l != '\0')
   {
      while (isspace(*l))
	 l++;
      end = l;
      while (isalpha(*end))
         end++;
      length = end - l;
      if (length > 63)
         length = 63;
      strncpy( language, l, length );
      language[length] = '\0';
      ind = FindLanguageIndex( language );
      if (ind >= 0)
         return ind;
      /* try the next language */
      l = end + 1;
      while (!isalpha(*l)  &&  *l != '\0')
      {
         if (*l == '\r'  ||  *l == '\n')
	    return LANG_IND_EN;
         l++;
      }
   }
   return LANG_IND_EN;
}


/* A typical request looks like this:
 *
 * GET /cgi-bin/URLblocked?mode=normal&... HTTP/1.1
 * Host: www.myserver.com
 * Accept: text/html
 * Accept-Encoding: compress
 * Connection: Keep-Alive
 *
 * OR
 *
 * GET /cgi-bin/URLblocked?mode=normal&... HTTP/1.0
 * User-Agent: Wget/1.8.2
 * Accept: text/html
 * Accept-Encoding: compress
 * Connection: Keep-Alive
 *
 */
static void ServeHttpClient( 
   int            fd,
   const char *   imagesDirectory )
{
   int            start, maxbytes;
   int            nbytes;
   int            ntrials;
   time_t	  start_time, now;
   char *         p;
   char *         command;
   char *         reqptr;
   char           request[16384];

   start_time = time( NULL );
   if (UFDBglobalDebug)
      ufdbLogMessage( "ServeHttpClient %3d %d", start_time%1000, fd );

   ntrials = 20;
   start = 0;
try_again:
   errno = 0;
   maxbytes = 16380 - start;
   nbytes = read( fd, &request[start], maxbytes );
   now = time( NULL );
   if (UFDBglobalDebug)
      ufdbLogMessage( "ServeHttpClient %3d %d  read %d bytes", now%1000, fd, nbytes );
   if (nbytes < 0)
   {
      if (now - start_time >= 4)
      {
	 ufdbLogError( "ServeHttpClient: timeout with %d bytes received", start );
	 AnswerHttpTimeout( fd );
	 return;
      }
      if (errno == EINTR  ||  errno == EAGAIN)
         goto try_again;
      else
      {
         ufdbLogError( "ServeHttpClient: unrecoverable error: %s", strerror(errno) );
	 AnswerHttpEmpty( fd, "unknown" );
	 return;
      }
   }
   start += nbytes;
   request[start] = '\0';
   if (strstr( request, "\r\n\r\n" ) == NULL)
   {
      if (--ntrials > 0)
      {
	 if (now - start_time >= 4)
	 {
	    ufdbLogError( "ServeHttpClient: timeout with %d bytes received", start );
	    AnswerHttpTimeout( fd );
	    return;
	 }
	 usleep( 5001 );
	 goto try_again;
      }
      else
      {
         ufdbLogError( "ServeHttpClient: did not get a whole HTTP request within the time limit\n"
         	       "I got: <<%s>>", 
		       request );
	 AnswerHttpEmpty( fd, "unknown" );
	 return;
      }
   }

   /* We got the full header. Now we can parse it and send an answer. */
   p = strtok_r( request, " \t", &reqptr );
   if (p == NULL  ||  
       (strcmp( p, "GET" ) != 0  &&  strcmp( p, "POST" ) != 0  &&  strcmp( p, "HEAD" ) != 0))
   {
      ufdbLogError( "ServeHttpClient: we got an unsupported message that is not a http GET/HEAD/POST but '%s'", 
                    p==NULL ? "NULL" : p );
      AnswerHttpNotFound( fd );
      return;
   }
   command = p;

   p = strtok_r( NULL, "? \t", &reqptr );
   if (p == NULL)
   {
      ufdbLogError( "ServeHttpClient: received GET/HEAD/POST command without URL" );
      AnswerHttpNotFound( fd );
      return;
   }
   /* p points to a URL, 
    * HTTP/1.0 has http://hostname/blah
    * HTTP/1.1 has /blah
    * and we only want "/blah".
    */
   if (strncmp( p, "http://", 7 ) == 0)
   {
      char * p7;
      p7 = strchr( p+7, '/' );
      if (p7 == NULL)
      {
	 ufdbLogError( "ServeHttpClient: unsupported URL for GET/POST: %s", p );
	 AnswerHttpEmpty( fd, p );
	 return;
      }
      p = p7;
   }

   if (UFDBglobalDebug)
      ufdbLogMessage( "ServeHttpClient: %s %s", command, p );

   if (strcmp( command, "HEAD" ) == 0)
   {
      AnswerHttpHead( fd );
      return;
   }

   if (strcmp( p, "/cgi-bin/URLblocked.cgi" ) == 0)
   {
      int lang; 

      p = strtok_r( NULL, " \t", &reqptr );
      lang = FindLanguage( reqptr );
      AnswerHttpUrlBlocked( fd, lang, p, imagesDirectory );
   }
   else
   {
      if (strcmp( p, "/favicon.ico" ) == 0  ||  strcmp( p, "/robots.txt" ) == 0)
         AnswerHttpNotFound( fd );
      else
      {
	 ufdbLogError( "ServeHttpClient: unsupported URL for GET/POST: \"%s\"", p );
	 AnswerHttpEmpty( fd, p );
      }
   }
}


void removePidFile( void );

static void ServeHttpConnections( 
   int             s, 
   const char *    imagesDirectory )
{
   int             newfd;
   int             n;
   fd_set          fds;
   struct timeval  tv;

   while (1)
   {
      FD_ZERO( &fds );
      FD_SET( s, &fds );
      tv.tv_sec = 0;
      tv.tv_usec = 750000;
      errno = 0;
      /* select() is used to enable signals to be received by this (non-threaded) process */
      n = select( s+1, &fds, (fd_set *) NULL, (fd_set *) NULL, &tv );
#if 0
      ufdbLogMessage( " select returns %d   errno is %d", n, errno );
#endif
      if (n < 0  && errno == EINTR)
      {
         ufdbLogError( "signal received. exiting..." );
	 removePidFile();
	 exit( 0 );
      }
      if (n == 0)			/* timeout */
         continue;
      newfd = accept( s, NULL, NULL );
      if (newfd < 0)
      {
         if (errno == EINTR)
	    continue;
	 if (errno == EAGAIN)
	    continue;
	 ufdbLogError( "SimulateHttpServer: \"accept\" returns error: %s", strerror(errno) );
	 continue;
      }

      tv.tv_sec = 3;
      tv.tv_usec = 0;
      setsockopt( newfd, SOL_SOCKET, SO_RCVTIMEO, (void *) &tv, sizeof(tv) );
      tv.tv_sec = 3;
      tv.tv_usec = 0;
      setsockopt( newfd, SOL_SOCKET, SO_SNDTIMEO, (void *) &tv, sizeof(tv) );
#if 0
      int sock_parm;
      sock_parm = 16384;
      setsockopt( newfd, SOL_SOCKET, SO_SNDBUF, (void *) &sock_parm, sizeof(sock_parm) );
      sock_parm = 16384;
      setsockopt( newfd, SOL_SOCKET, SO_RCVBUF, (void *) &sock_parm, sizeof(sock_parm) );
#endif

      ServeHttpClient( newfd, imagesDirectory );
      close( newfd );
   }
}


#ifndef HAVE_INET_ATON

int inet_aton( 
   const char *     cp, 
   struct in_addr * inp )
{
  unsigned int      byte_result = 0;
  unsigned long     result = 0;
  char              c = '.'; 		/* mark c to indicate invalid IP in case length is 0 */
  int               dot_count = 0;

  if (cp == NULL)
    return 0;

  while (*cp != '\0')
  {
    int digit;

    c = *cp++;
    digit = (int) (c - '0');	
    if (digit >= 0  &&  digit <= 9)
    {
      byte_result = byte_result * 10 + digit;
      if (byte_result > 255)
	return 0;
    }
    else if (c == '.')
    {
      dot_count++;
      result = (result << 8) + (unsigned long) byte_result;
      byte_result = 0;
    }
    else
      return 0;
  }

  if (c != '.')					/* IP number can't end on '.' */
  {
    /*
      Handle short-forms addresses according to standard. Examples:
      127		-> 0.0.0.127
      127.1		-> 127.0.0.1
      127.2.1		-> 127.2.0.1
    */
    switch (dot_count) {
    case 1: result <<= 8;   /* Fall through */
    case 2: result <<= 8;   /* Fall through */
    }
    inp->s_addr = (result << 8) + (unsigned long) byte_result;
    return 1;
  }

err:
  return 0;
}

#endif


void ufdbSimulateHttpServer( 
   const char *        interface, 
   int                 port, 
   const char *        username,
   const char *        imagesDirectory,
   int                 flags )
{
   int                 s;
   int                 sock_parm;
   struct sockaddr_in  addr;
   struct timeval      tv;
#if 0
   struct linger       linger;
#endif

   errno = 0;
   s = socket( AF_INET, SOCK_STREAM, 0 );
   if (s < 0)
   {
      ufdbLogError( "SimulateHttpServer: cannot create socket: %s", strerror(errno) );
      return;
   }

   addr.sin_family = AF_INET;
   addr.sin_port = htons( port );
   if (interface == NULL  ||  strcmp(interface,"all")==0)
      addr.sin_addr.s_addr = htonl( INADDR_ANY );
   else
   {
      struct in_addr iaddr;
      if (inet_aton( interface, &iaddr ) == 0)
      {
	 addr.sin_addr.s_addr = htonl( INADDR_ANY );
	 ufdbLogError( "interface parameter '%s' is invalid. I will listen on port %d on ALL interfaces.", 
	               interface, port );
      }
      else
	 addr.sin_addr.s_addr = iaddr.s_addr;
   }

   /*
    * Allow server-side addresses to be reused (don't have to wait for timeout).
    */
   sock_parm = 1;
   setsockopt( s, SOL_SOCKET, SO_REUSEADDR, (void *) &sock_parm, sizeof(sock_parm) );

   /*
    * This http server has very little data to receive and send...
    */
   sock_parm = 12 * 1024;
   setsockopt( s, SOL_SOCKET, SO_SNDBUF, (void *) &sock_parm, sizeof(sock_parm) );
   sock_parm = 12 * 1024;
   setsockopt( s, SOL_SOCKET, SO_RCVBUF, (void *) &sock_parm, sizeof(sock_parm) );

   tv.tv_sec = 6;
   tv.tv_usec = 0;
   setsockopt( s, SOL_SOCKET, SO_RCVTIMEO, (void *) &tv, sizeof(tv) );
   tv.tv_sec = 6;
   tv.tv_usec = 0;
   setsockopt( s, SOL_SOCKET, SO_SNDTIMEO, (void *) &tv, sizeof(tv) );

   if (bind( s, (struct sockaddr *) &addr, sizeof(addr) ) < 0)
   {
      ufdbLogError( "SimulateHttpServer: cannot bind socket: %s\n"
      		    "Check for other processes using port %d  uid=%d euid=%d", 
		    strerror(errno), port, getuid(), geteuid() );
      close( s );
      return;
   }
   
   /* Now that the socket is bound, we can drop root privileges */
   if (username != NULL && username[0] != '\0')
   {
      UFDBdropPrivileges( username );
   }

   /*
    * According to comment in the Apache httpd source code, these socket
    * options should only be set after a successful bind....
    */
   sock_parm = 1;
   setsockopt( s, SOL_SOCKET, SO_KEEPALIVE, (void *) &sock_parm, sizeof(sock_parm) );

#if 0
   linger.l_onoff = 1;
   linger.l_linger = 2;
   setsockopt( s, SOL_SOCKET, SO_LINGER, (void *) &linger, sizeof(linger) );
#endif

#if 0
   sock_parm = 1;
   setsockopt( s, IPPROTO_TCP, TCP_NODELAY, (void *) &sock_parm, sizeof(sock_parm) );
#endif

   if (listen( s, 128 ) < 0)
   {
      ufdbLogError( "SimulateHttpServer: cannot listen on socket: %s", strerror(errno) );
      close( s );
      return;
   }

   ufdbLogMessage( "SimulateHttpServer: listening on port %d", port );
    
   ServeHttpConnections( s, imagesDirectory );
}

