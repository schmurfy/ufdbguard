#!/bin/sh
#
# ufdbanalyse - analyse a Squid log file
#
# read a Squid log file and produce a report with a table showing
# percentages for categories.
#
# This script is meant to be used as a tool to find out what types
# of websites are visited.

usage () {
   echo "usage: ufdbanalyse <squid-log-file> <mydomainname> <email-address>"
   echo "The logfile is the file specified in squid.conf by the"
   echo "cache_access_log parameter (often set to access.log)."
   echo "mydomainname is your own local domain name, e.g. example.com"
   echo
   echo "The logfile is stripped and URLs are uploaded to URLfilterDB."
   echo "The output is sent by email."
   exit 1
}

if [ $# -ne 3 ]
then
   usage
fi

logfile="$1"
if [ ! -r "$logfile" ]
then
   echo "cannot read file \"$logfile\""
   usage
fi
mydomain="$2"
email="$3"

# adjust PATH for Solaris awk
PATH=/usr/xpg4/bin:$PATH
export PATH

TMPFILE=/tmp/ufdbanalyse.$$

echo "$mydomain" > $TMPFILE
echo "$email" >> $TMPFILE

awk '{ 
        code=$4; nbytes=$5; command=$6; url=$7;
	sub( '.*/', '', code );
	sub( '?.*', '' url );
     }' < "$logfile" >> $TMPFILE

 
