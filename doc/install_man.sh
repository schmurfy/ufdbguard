#!/bin/sh
#
# install_man.sh  -  install a man page in /usr/local/share/man or /usr/share/man or /usr/man

if [ $# -ne 2 ]
then
   echo "usage: install_man.sh <manpage> <section>"
   exit 1
fi

MAN="$1"
SECTION="$2"

if [ -d /usr/local/share/man ]
then 
   MANTOP=/usr/local/share/man
else
   if [ -d /usr/share/man ]
   then
      MANTOP=/usr/share/man
   else
      if [ -d /usr/local/man ]
      then
         MANTOP=/usr/local/man
      else
	 if [ -d /usr/man ]
	 then
	    MANTOP=/usr/man
	 else
	    echo "cannot find man installation path: man $MAN cannot be installed"
	    exit 0
	 fi
      fi
   fi
fi

MANDIR="$MANTOP/man$SECTION"
if [ ! -d "$MANDIR" ]
then
   echo "man section $SECTION directory ($MANDIR) does not exist: man $MAN cannot be installed"
   exit 0
fi

if [ ! -w "$MANDIR" ]
then
   echo "man section $SECTION directory ($MANDIR) has no write access: man $MAN cannot be installed"
   exit 0
fi

umask 022
cp $MAN $MANDIR/$MAN
if [ $? -ne 0 ]
then
   echo "installation of man $MAN failed."
   exit 0
fi

chmod 644 $MANDIR/$MAN
exit 0

