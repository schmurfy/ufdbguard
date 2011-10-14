#!/usr/bin/perl -wT
#
# URLblocked.cgi - explain to the user that the URL is blocked and by which rule set.
#
# Currently the error messages supports 
# en (English), 
# de (German), 
# pl (Polish)
# sv (Swedisk)
# it (Italian)
# pt (Portuguese)
# fr (French)
# tr (Turkish)
# nl (Dutch).
# You can add a language yourself: search for all occurences of "NEWLANGUAGE"
# and add your language text.

use strict;
use Socket;
use FCGI;		### without FCGI this statement is not necessary

use constant {
   CT_IMAGE  => 1,
   CT_JAVA   => 2,
   CT_HTML   => 3,
   CT_XML    => 4,
   CT_CSS    => 5,
   CT_TEXT   => 6,
   CT_JSON   => 7,
   CT_STREAM => 8
};

use vars qw( $admin $clientaddr $clientname $clientuser $clientgroup $targetgroup );
use vars qw( $color $size $mode $textcolor $bgcolor $titlesize $textsize $url );
use vars qw( @day @month @languages $lang $protocol $address $port $path );

local $admin;
local $clientaddr;
local $clientname;
local $clientuser;
local $clientgroup;
local $targetgroup;
local $color;
local $size;
local $mode;
local $textcolor;
local $bgcolor;
local $titlesize;
local $textsize;
local $url;
local $lang;
local $protocol;
local $address;
local $port;
local $path;
local @day = ("Sunday","Monday","Tuesday","Wednesday","Thursday","Friday","Saturday");
local @month = ("Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec");
local @languages = (
		"de (German),",
		"nl (Dutch),",
		"pl (Polish),",
		"sv (Swedish),",
		"es (Spanish),",
		"it (Italian),",
		"pt (Portuguese),",
		"fr (French),",
		"tr (Turkish),",
		"NEW (NEWLANGUAGE),",
		"en (English),",
	       );

sub init();
sub session_reinit();
sub getPreferedLanguage(@);
sub parseURL($);
sub parseQuery($);


sub init () 
{
   ( $admin, $clientaddr, $clientname, $clientuser, $clientgroup, $targetgroup, $url )  =
      parseQuery( $ENV{"QUERY_STRING"} );
   $lang = getPreferedLanguage( @languages );
}


sub session_reinit ()
{
   $admin = 'unknown';
   $clientaddr = 'unknown';
   $clientname = 'unknown';
   $clientuser = 'unknown';
   $clientgroup = 'unknown';
   $targetgroup = 'unknown';
   $color = 'orange';
   $size = 'normal';
   $mode = 'default';
   $url = 'unknown';
   $lang = 'unknown';
}


#
# Find the first supported language of the client.
#
sub getPreferedLanguage (@) 
{
  my @supported = @_;
  my @clientLanguages = split(/\s*,\s*/,$ENV{"HTTP_ACCEPT_LANGUAGE"}) if(defined($ENV{"HTTP_ACCEPT_LANGUAGE"}));
  my $lang;
  my $supp;

  ### NO!  push(@clientLanguages,$supported[0]);
  for $lang (@clientLanguages) 
  {
    $lang =~ s/\s.*//;
    $lang =~ s/-.*//;
    for $supp (@supported) 
    {
      $supp =~ s/\s.*//;
      return($lang) if ($lang eq $supp);
    }
  }
  return 'en';   # default language is 'en'
}


sub parseQuery ($) 
{
  my $query       = shift;
  my $admin       = 'The system administrator.';
  my $clientaddr  = '';
  my $clientname  = '';
  my $clientuser  = '';
  my $clientgroup = '';
  my $targetgroup = 'undefined';
  my $url         = 'undefined';

  if (defined($query))
  {
    while ($query =~ /^\&?([^\&=]+)=\"([^\"]*)\"(.*)/  || 
           $query =~ /^\&?([^\&=]+)=([^\&=]*)(.*)/)
    {
      my $key = $1;
      my $value = $2;
      $value = '??' unless(defined($value) && $value && $value ne '??');
      $query = $3;

      if ($key =~ /^(admin|clientaddr|clientname|clientuser|clientident|clientgroup|category|targetgroup|color|size|source|srcclass|targetclass|mode|url)$/) 
      {
	$value =~ s/%20/ /g;
	$value =~ s/%22/"/g;
	$value =~ s/%23/#/g;
	$value =~ s/%26/\&/g;
	$value =~ s/%2F/\//ig;
	$value =~ s/%3A/:/ig;
	$value =~ s/%3B/;/ig;
	$value =~ s/%3C/</ig;
	$value =~ s/%3D/=/ig;
	$value =~ s/%3E/>/ig;
	$value =~ s/%3F/?/ig;
	$value =~ s/%5C/\\/ig;
	$key = 'clientgroup' if ($key eq 'source'  ||  $key eq 'srcclass');
	$key = 'clientuser' if ($key eq 'clientident');
	$key = 'targetgroup' if ($key eq 'category'  ||  $key eq 'targetclass');
	eval "\$$key = \$value";
      }

      if ($query =~ /^url=(.*)/) 
      {
	$url = $1;
	last;
      }
    }
  }

  return ( $admin, $clientaddr, $clientname, $clientuser, $clientgroup, $targetgroup, $url );
}


sub parseURL ($) 
{
  my $url      = shift;
  my $protocol = "";
  my $address  = "";
  my $port     = "";
  my $path     = "";

  $url =~ /^([^\/:]+):\/\/([^\/:]+)(:\d*)?(.*)/;
  $protocol = $1 if(defined($1));
  $address  = $2 if(defined($2));
  $port     = $3 if(defined($3));
  $path     = $4 if(defined($4));

  return ( $protocol, $address, $port, $path );
}


sub getContentType( $ )
{
   my $url = shift;
   my $suffix;
   my $path;

   $url =~ s/[;\?\&].*//;
   $url =~ s/^(ftp|http|https):\/\///;

   $path = $url;
   $path =~ s/^[^\/]*//;

   $suffix = $path;
   $suffix =~ s/.*\././;

   return CT_IMAGE  if ($suffix =~ /\.(bmp|gif|ico|jpg|jpeg|jpe|png|tiff)$/i);
   return CT_CSS    if ($suffix =~ /\.css$/i);
   return CT_JSON   if ($suffix =~ /\.json$/i);
   return CT_JAVA   if ($suffix =~ /\.(js|jar)$/i);
   return CT_TEXT   if ($suffix =~ /\.(csv|txt)$/i);
   return CT_HTML   if ($suffix =~ /\.(htm|html|dhtml|shtml)$/i);
   return CT_XML    if ($suffix =~ /\.(xml|sxml|rss)$/i);
   return CT_STREAM if ($suffix =~ /\.(bin|bz2|cab|class|dat|doc|gz|h264|mp3|mpg|mpeg|msi|mst|ppt|pdf|rar|tar|ttf|xls|zip|ogv|divx|xvid|qt|ra|ram|rv|wmv|avi|mov|swf|mp4|mv4|flv)$/i);

   # no suffix found, now we start with the guesswork


   return CT_HTML   if ($path eq '/' );
   return CT_IMAGE  if ($url =~ /^googleadservices\.com\/pagead\/conversion\// );
   return CT_TEXT   if ($url =~ /^googleads\.g\.doubleclick\.net\/pagead\/ads\// );
   return CT_JAVA   if ($url eq 'a.analytics.yahoo.com/fpc.pl' );
   return CT_IMAGE  if ($url eq 'a.analytics.yahoo.com/p.pl' );
   return CT_IMAGE  if ($url eq 'ping.chartbeat.net/ping' );

   if ($url =~ '^www\.youtube\.com')
   {
      return CT_STREAM  if ($path =~ /^\/cp\//  ||  $path =~ /^\/p\//  ||
                            $path =~ /^\/v\//   ||  $path =~ /^\/videoplayback/ );
   }

   return CT_IMAGE  if ($url =~ /^b\.scorecardresearch\.com\// );

   return CT_JAVA   if ($url =~ /\.doubleclick\.net\/adj\//   ||
                        $url =~ /\.doubleclick\.net\/pfadj\// );
   return CT_IMAGE  if ($url =~ /\.doubleclick\.net\/imp/ );

   if ($url =~ /^view\.atdmt\.com\//)
   {
      return CT_IMAGE  if ($path =~ /^\/action\// );
      return CT_JAVA   if ($path =~ /^\/jview\// );
   }

   return CT_JAVA   if ($url eq 'static.ak.connect.facebook.com/connect.php' );

   return CT_IMAGE  if ($url eq 'secure-us.imrworldwide.com/cgi-bin/m' );

   return CT_IMAGE  if ($url =~ /ftjcfx\.com\/image-/ );
   return CT_IMAGE  if ($url =~ /lduhtrp\.net\/image-/ );
   return CT_IMAGE  if ($url =~ /img\.pheedo\.com\/img\.phdo/ );
      
   if ($path =~ /\/realmedia\/ads\//i )
   {
      return CT_JAVA   if ($path =~ /\/adstream_jx\//  ||  $path =~ /\/adstream_mjx\// );
      return CT_IMAGE  if ($path =~ /\/adstream_lx\//  ||  $path =~ /\/adstream_nx\// );
      return CT_IMAGE  if ($path =~ /\/ads\/cap\.cgi/  );
   }

   return CT_JAVA   if ($url =~ /overture\.com\/ls_js_/ );

   return CT_IMAGE  if ($path =~ /\/scripts\/beacon\.dll/  ||  $path =~ /\/scripts\/beacon2\.dll/ );

   return CT_JAVA   if ($path =~ /\/javascript\//  ||  $path =~ /\/ajaxpro\// );

   return CT_JAVA   if ($path =~ /\/js\.php$/  ||  $path =~ /\/javascript\.php$/ );

   return CT_CSS    if ($path =~ /\/css\.php$/ );

   return CT_IMAGE  if ($path =~ /\/image\.php$/  || $path =~ /\/image\.php\// );

   return CT_JAVA   if ($path =~ /\/js\.ng\//  ||  $path =~ /\/js\// );

   return CT_JAVA   if ($path =~ /\/scripts\//  ||  $path =~ /\/script\// );

   return CT_XML    if ($url =~ /^xml\./ );

   if ($path =~ /\/b\/ss\// )
   {
      return CT_IMAGE  if ($path =~ /\/FAS/i  ||  $path =~ /\/H\./i  ||  $path =~ /\/G\./i );
   }
  
   return CT_JAVA   if ($url =~ /\.channel\.facebook\.com\/x\// );
   return CT_TEXT   if ($url =~ /\.channel\.facebook\.com\/p/ );

   return CT_XML    if ($path =~ /\/xml-rpc/ );

   return CT_STREAM if ($path eq 'open/1');

   return CT_IMAGE  if ($url =~ /^pixel\./  ||  $path =~ /\/pixel$/ );

   return CT_HTML;
}


while (FCGI::accept() >= 0)
{
   my $time = time;
   my ($sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst);
   my $root;
   my $buffer;

   session_reinit();
   init();

   # the default is english
   my $forbidden = 'no access';
   my $title = $forbidden;
   my $explanation_prefix = 'URL blocked because it is';
   my $explanation_suffix = '';
   my $go_back = 'back';
   my $more_info = 'More information about URLfilterDB is <a href="http://www.urlfilterdb.com">here</a>.';

   my $contentType = getContentType( $url );

   if ($contentType == CT_IMAGE)
   {
      print "Content-Type: image/png\n";
      ($sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst) = gmtime( $time + 180 );
      printf "Expires: %s, %02d-%s-%04d %02d:%02d:%02d GMT\n", 
	     $day[$wday], $mday, $month[$mon], $year+1900, $hour, $min, $sec;
      print "\n";

      my $imgfile;

      if ($targetgroup eq 'ads')
      {
         $imgfile = "transparent.png";
         $imgfile = "no-ads.png"      if ($mode eq 'noads');
         $imgfile = "smallcross.png"  if ($mode eq 'cross');
         $imgfile = "square.png"      if ($mode eq 'square');
      }
      else
      {
	 if ($mode eq 'cross') {
	    $imgfile = "smallcross.png"  if ($mode eq 'cross');
	 }
	 elsif ($mode eq 'square') {
	    $imgfile = "square.png"      if ($mode eq 'square');
	 }
	 elsif ($mode eq 'simple-red'  ||  $mode eq 'transparent'  ||  $mode eq 'transparant') {
	    $imgfile = "transparent.png";
	 }
	 else  {
	    $imgfile = "forbidden-normal-" . $lang . ".png";
	 }
      }
      $root = $ENV{'DOCUMENT_ROOT'};
      open( BLOCKEDPNG, "$root/images/$imgfile" )  ||  print "failed to open $root/images/$imgfile\n";
      print $buffer while (read (BLOCKEDPNG,$buffer,8192));
      close( BLOCKEDPNG );
   }
   elsif ($contentType == CT_STREAM)
   {
      print "Content-Type: application/octet-stream\n";
      ($sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst) = gmtime( $time + 180 );
      printf "Expires: %s, %02d-%s-%04d %02d:%02d:%02d GMT\n", 
	     $day[$wday], $mday, $month[$mon], $year+1900, $hour, $min, $sec;
      print "\n";
   }
   elsif ($contentType == CT_JAVA)
   {
      print "Content-Type: application/x-javascript\n";
      ($sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst) = gmtime( $time + 180 );
      printf "Expires: %s, %02d-%s-%04d %02d:%02d:%02d GMT\n", 
	     $day[$wday], $mday, $month[$mon], $year+1900, $hour, $min, $sec;
      print "\n";

      print "\n";
   }
   elsif ($contentType == CT_JSON)
   {
      print "Content-Type: application/json\n";
      ($sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst) = gmtime( $time + 180 );
      printf "Expires: %s, %02d-%s-%04d %02d:%02d:%02d GMT\n", 
	     $day[$wday], $mday, $month[$mon], $year+1900, $hour, $min, $sec;
      print "\n";

      print "\n";
   }
   elsif ($contentType == CT_CSS)
   {
      print "Content-Type: text/css\n";
      ($sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst) = gmtime( $time + 180 );
      printf "Expires: %s, %02d-%s-%04d %02d:%02d:%02d GMT\n", 
	     $day[$wday], $mday, $month[$mon], $year+1900, $hour, $min, $sec;
      print "\n";

      print "\n";
   }
   elsif ($contentType == CT_TEXT)
   {
      print "Content-Type: text/plain\n";
      ($sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst) = gmtime( $time + 180 );
      printf "Expires: %s, %02d-%s-%04d %02d:%02d:%02d GMT\n", 
	     $day[$wday], $mday, $month[$mon], $year+1900, $hour, $min, $sec;
      print "\n";

      print "\n";
   }
   elsif ($contentType == CT_XML)
   {
      print "Content-Type: text/xml\n";
      ($sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst) = gmtime( $time + 180 );
      printf "Expires: %s, %02d-%s-%04d %02d:%02d:%02d GMT\n", 
	     $day[$wday], $mday, $month[$mon], $year+1900, $hour, $min, $sec;
      print "\n";

      print "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n";
      print "<ufdbguardd>\n";
      print "   <dummy value=\"0\" />\n";
      print "</ufdbguardd>\n";
   }
   else 	# CT_HTML
   {
      if ($targetgroup eq 'fatal-error')
      {
	 print "Content-Type: text/html\n";
	 ($sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst) = gmtime( $time + 180 );
	 printf "Expires: %s, %02d-%s-%04d %02d:%02d:%02d GMT\n", 
		$day[$wday], $mday, $month[$mon], $year+1900, $hour, $min, $sec;
	 print "Content-Language: $lang\n";
	 print "\n";

	 print "<html>";
	 print "<head><title>The URL filter has a fatal error</title></head>\n";
	 print "<body bgcolor=\"#e0e0e0\">\n";
	 print "<center>\n";
	 print "<font color=red><b>\n" .
	       "Access to the internet is blocked because<br>\n" .
	       "the URL filter has a fatal error. <br>\n" . 
	       "Ask your helpdesk or web proxy administrator for help." .
	       "</b></font>\n";
	 print "</center>\n";
	 print "</body>\n";
	 print "</html>\n";
      }
      elsif ($targetgroup eq 'loading-database')
      {
	 print "Content-Type: text/html\n";
	 ($sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst) = gmtime( $time + 180 );
	 printf "Expires: %s, %02d-%s-%04d %02d:%02d:%02d GMT\n", 
		$day[$wday], $mday, $month[$mon], $year+1900, $hour, $min, $sec;
	 print "Content-Language: $lang\n";
	 print "\n";

	 print "<html>";
	 print "<head><title>a new URL database is being loaded</title></head>\n";
	 print "<body bgcolor=\"#e0e0e0\">\n";
	 print "<center>\n";
	 print "<font color=red><b>\n" .
	       "Access to the internet is temporarily blocked because<br>\n" .
	       "a new URL database is being loaded by the URL filter. <br>\n" . 
	       "Wait one minute and try again." .
	       "</b></font>\n";
	 print "</center>\n";
	 print "</body>\n";
	 print "</html>\n";
      }
      else
      {
	 if ($lang eq 'de') {
	    $forbidden = 'Verboten';
	    $title = "zugriff verweigert ($targetgroup)";
	    $explanation_prefix = 'Zugriff verweigert weil die URL die Klassifizierung';
	    $explanation_suffix = 'hat.';
	    $more_info = 'Mehr Informationen &uuml;ber URLfilterDB ist <a href="http://www.urlfilterdb.com/blocking/">hier</a>.';
	    $go_back = 'Klicken Sie hier um zur&uuml;ck zu gehen.';
	 }
	 elsif ($lang eq 'pl') {
	    $forbidden = 'Pobranie tej strony jest zabronione!';
	    $title = "Cenzura, zakaz pobrania ($targetgroup)";
	    $explanation_prefix = 'Nie otworzysz tej strony bo jest ona sklasyfikowana jako';
	    $explanation_suffix = 'przez program kontroli URLfilterDB';
	    $more_info = 'Informacja (po angielsku) o tym programie kontroli jest na <a href="http://www.urlfilterdb.com/blocking/">stronie</a>.';
	    $go_back = 'Wycofaj do poprzedniej strony';
	 }
	 elsif ($lang eq 'sv') {
	    $forbidden = 'Sidan stoppad enligt landstingets riktlinjer';
	    $title = "F&ouml;rbjuden ($targetgroup)";
	    $explanation_prefix = 'Access till denna sida &auml;r stoppad:';
	    $explanation_suffix = '';
	    $more_info = 'Mer information om URLfilterDB &auml;r <a href="http://www.urlfilterdb.com/blocking/">h&auml;r</a>.';
	    $go_back = 'Klicka h&auml;r f&ouml;r att komma tillbaks';
	 }
	 elsif ($lang eq 'nl') {
	    $forbidden = 'Geen Toegang';
	    $title = "geen toegang ($targetgroup)";
	    $explanation_prefix = 'De toegang is geblokkeerd omdat de URL in de categorie';
	    $explanation_suffix = 'valt.';
	    $more_info = 'Meer informatie over URLfilterDB is <a href="http://www.urlfilterdb.com/blocking/">hier</a>.';
	    $go_back = 'Klik hier om terug te gaan';
	 }
	 elsif ($lang eq 'es') {
	    $forbidden = 'Ning&uacute;n acceso';
	    $title = "ning&uacute;n acceso ($targetgroup)";
	    $explanation_prefix = 'Se bloquea el acceso puesto que el URL se considera ser';
	    $explanation_suffix = '';
	    $more_info = 'M&aacute;s informaci&oacute;n sobre URLfilterDB est&aacute; <a href="http://www.urlfilterdb.com/blocking/">aqu&iacute;</a>.';
	    $go_back = 'ir detr&aacute;s';
	 }
	 elsif ($lang eq 'it') {
	    $forbidden = 'Accesso non permesso';
	    $title = "accesso non permesso ($targetgroup)";
	    $explanation_prefix = "L'accesso &egrave; ostruito poich&eacute; il URL &egrave; considerato come";
	    $explanation_suffix = '';
	    $more_info = 'Le pi&ugrave; informazioni su URLfilterDB sono <a href="http://www.urlfilterdb.com/blocking">qui</a>.';
	    $go_back = 'andare indietro';
	 }
	 elsif ($lang eq 'pt') {
	    $forbidden = 'Proibido';
	    $title = "Proibido ($targetgroup)";
	    $explanation_prefix = "O acesso a este site foi bloqueado porque o conte&uacute;do est&aacute;";
	    $explanation_suffix = '';
	    $more_info = 'Mais informa&ccedil;&atilde;o sobre URLfilterDB est&aacute; <a href="http://www.urlfilterdb.com/blocking">aqui</a>.';
	    $go_back = 'volte';
	 }
	 elsif ($lang eq 'fr') {
	    $forbidden = 'Interdit';
	    $title = "Interdit ($targetgroup)";
	    $explanation_prefix = "L'access est inderdit parce que le site est";
	    $explanation_suffix = '';
	    $more_info = "Plus d'information de URLfilterDB est <a href=\"http://www.urlfilterdb.com/blocking\">ici</a>.";
	    $go_back = 'rentrer';
	 }
	 elsif ($lang eq 'tr') {
	    $forbidden = 'Eri&#351;im engellendi';
	    $title = "Eri&#351;im engellendi ($targetgroup)";
	    $explanation_prefix = "Ula&#351;mak istedi&#287;iniz sayfaya eri&#351;im kapal&#305;d&#305;r. S&#305;n&#305;f&#305;:";
	    $explanation_suffix = '';
	    $more_info = "URLfilterDB hakk&#305;nda bilgi i&ccedil;in <a href=\"http://www.urlfilterdb.com/blocking\">t&#305;klay&#305;n&#305;z</a>.";
	    $go_back = '&Ouml;nceki sayfa';
	 }
	 elsif ($lang eq 'NEWLANGUAGE') {
	    $forbidden = 'Forbidden';
	    $title = "Forbidden ($targetgroup)";
	    $explanation_prefix = 'Access is blocked since the URL is considered to be';
	    $explanation_suffix = '';
	    $more_info = 'More information about URLfilterDB is <a href="http://www.urlfilterdb.com/blocking">here</a>.';
	    $go_back = 'Click here to go back';
	 }
	 else {   # default (matches 'en')
	    $forbidden = 'Forbidden';
	    $title = "Forbidden ($targetgroup)";
	    $explanation_prefix = 'Access is blocked since the URL is considered to be';
	    $explanation_suffix = '';
	    $more_info = 'More information about URLfilterDB is <a href="http://www.urlfilterdb.com/blocking">here</a>.';
	    $go_back = 'Click here to go back';
	    $lang = 'en';
	 }

	 if ($color eq 'orange')
	 {
	    $textcolor = 'white';
	    $bgcolor = '#ee8811';
	 }
	 elsif ($color eq 'white')
	 {
	    $textcolor = '#3f003f';
	    $bgcolor = 'white';
	 }
	 elsif ($color eq 'black')
	 {
	    $textcolor = '#f0f0f0';
	    $bgcolor = 'black';
	 }
	 elsif ($color eq 'red')
	 {
	    $textcolor = '#f0f0f0';
	    $bgcolor = 'red';
	 }
	 elsif ($color eq 'grey'  ||  $color eq 'gray')
	 {
	    $textcolor = '#111111';
	    $bgcolor = '#c2c2c2';
	 }
	 else	# default color: orange
	 {
	    $textcolor = 'white';
	    $bgcolor = '#ee8811';
	 }

	 if ($size eq 'normal')
	 {
	    $titlesize = '+2';
	    $textsize = '+0';
	 }
	 elsif ($size eq 'small')
	 {
	    $titlesize = '+1';
	    $textsize = '-1';
	 }
	 elsif ($size eq 'large')
	 {
	    $titlesize = '+3';
	    $textsize = '+1';
	 }
	 else    # default size: normal
	 {
	    $titlesize = '+2';
	    $textsize = '+0';
	    $size = 'normal';
	 }

	 $url =~ s/[?;&].*//;

	 print "Content-Type: text/html\n";
	 ($sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst) = gmtime( $time + 180 );
	 printf "Expires: %s, %02d-%s-%04d %02d:%02d:%02d GMT\n", 
		$day[$wday], $mday, $month[$mon], $year+1900, $hour, $min, $sec;
	 print "Content-Language: $lang\n";
	 print "\n";

	 if ($targetgroup eq 'ads')
	 {
	    my $text;
	    $text = " ";	 # transparent
	    $text = " no ads " if $mode eq 'noads';
	    $text = " [] "     if $mode eq 'square';
	    $text = " x "      if $mode eq 'cross';
	    $text = "<font color=red><i>ads</i></font>"  if $mode eq 'simple-red';

	    print "<html>\n";
	    print "<head><title>$title</title></head>\n";
	    print "<body>\n";
	    print "<font size=\"$textsize\">$text</font>\n";
	    print "</body>\n";
	    print "</html>\n";
	 }
	 else
	 {
	    if ($mode eq 'simple-red')
	    {
	       my $whyblocked = "$explanation_prefix $targetgroup$explanation_suffix.  $url";
	       print "<html>";
	       print "<head><title>$title</title></head>\n";
	       print "<body bgcolor=\"ffcccc\" link=\"red\" alink=\"red\" vlink=\"red\" text=\"red\">\n";
	       print "<p align=center>\n";
	       print "<a title=\"$whyblocked\">$forbidden<br><i>$targetgroup</i></a>\n";
	       print "</body>\n";
	       print "</html>\n";
	    }
	    else
	    {
	       print "<html>";
	       print "<head> <title>$title</title> </head>\n";
	       print "<body bgcolor=\"$bgcolor\" text=\"$textcolor\">\n";
	       print "<font size=\"$titlesize\">$forbidden</font> <br>\n";
	       print "<font size=\"$textsize\">\n";
	       print "$explanation_prefix <i>$targetgroup</i> $explanation_suffix <br>\n";
	       print "URL: $url <br>\n";
	       print "<p>\n";
	       print "<a href=\"javascript:history.go(-1);\">$go_back</a>. <br>\n";
	       print "$admin\n";
	       print "<p>\n";
	       print "$more_info\n";
	       print "</font>\n";
	       print "<br>\n&nbsp;<p />\n";
	       print "<font size=\"-3\">";
	       print "user=$clientuser &nbsp; "      if (defined($clientuser)  &&  length($clientuser)>0);
	       print "client=$clientaddr &nbsp; "    if (defined($clientaddr)  &&  length($clientaddr)>0);
	       print "group=$clientgroup &nbsp; "    if (defined($clientgroup)  &&  length($clientgroup)>0);
	       print "source=$clientname &nbsp; "    if (defined($clientname)  &&  length($clientname)>0);
	       print "</font>\n";
	       print "</body>\n";
	       print "</html>\n";
	    }
	 }
      }
   }
}

# TODO: log this event

exit 0;


