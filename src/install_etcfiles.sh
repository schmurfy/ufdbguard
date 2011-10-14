#!/bin/sh
#
# install_etcfiles.sh
#
# $Id: install_etcfiles.sh,v 1.16 2010/10/07 15:08:31 root Exp root $

PATH=/usr/xpg4/bin:$PATH
export PATH

OS=unix

ID=`id -un`
if [ "$ID" != root ]
then
   echo
   echo "***  You must be root but you appear to be $ID. ***"
   echo "The ufdb startup script cannot be installed..."
   echo
   exit 1
fi

CFGDIR="$1"
if [ ! -d "$CFGDIR" ]
then
   echo "install_etcfiles.sh: $CFGDIR is not a directory   **********"
   exit 1
fi

BINDIR="$2"
if [ ! -d "$BINDIR" ]
then
   echo "install_etcfiles.sh: $BINDIR is not a directory   **********"
   exit 1
fi

if [ -d /usr/pkg/etc/rc.d ]			# NetBSD
then
   OS=netbsd
   INITDIR=/usr/pkg/etc/rc.d
else
   if [ -d /usr/local/etc/rc.d ]		# FreeBSD 
   then
      OS=freebsd
      INITDIR=/usr/local/etc/rc.d
   else
      if [ -d /etc/rc.d/init.d ]		# Linux 2.6
      then 
	 INITDIR=/etc/rc.d/init.d
	 if [ -d /etc/rc.d/rc3.d ]
	 then
	    RCROOT=/etc/rc.d
	 else
	    RCROOT=$INITDIR
	 fi
      else
	 if [ -d /etc/init.d ]			# Linux 2.4
	 then 
	    INITDIR=/etc/init.d
	    if [ -d /etc/rc.d/rc3.d ]
	    then
	       RCROOT=/etc/rc.d
	    else
	       RCROOT=$INITDIR 			# Linux 2.4
	    fi
	 else
	    if [ -d /sbin/init.d ]		# HPUX 11.x
	    then
	       INITDIR=/sbin/init.d
	       RCROOT=$INITDIR
	    else
	       if [ -d /etc/rc3.d ]		# Solaris 9, 10
	       then
		  OS=solaris
		  RCROOT=/etc/rc3.d
	       else
		  if [ -f /etc/rc.securelevel ]		# OpenBSD
		  then
		     OS=openbsd
		     RCROOT=$BINDIR
		  else
		     if [ -d /etc/init.d ]
		     then
			INITDIR=/etc/init.d
		     else
			echo
			echo "================================================================="
			echo "Cannot find the system rc init.d directory   *******"
			echo "Please run install_etcfiles.sh manually and specify the directory   ********"
			echo "================================================================="
			echo
			exit 1
		     fi
		  fi
	       fi
	    fi
	 fi
      fi
   fi
fi


install_openbsd () {
   # OpenBSD is different from other *nix systems.
   # On OpenBSD the stop/start script is installed in $BINDIR
   # and called from /etc/rc.local and /etc/rc.shutdown

   rm -f $BINDIR/ufdb.sh
   cp mtserver/ufdb.sh $BINDIR/ufdb.sh
   chmod 755 $BINDIR/ufdb.sh

   ALREADY_DONE=`grep "/ufdb.sh.*start" /etc/rc.local`
   if [ "$ALREADY_DONE" = "" ]
   then
      (
         echo 
	 echo "# start ufdbGuard"
	 echo "$BINDIR/ufdb.sh start"
	 echo
      ) >> /etc/rc.local
      (
         echo 
	 echo "# stop ufdbGuard"
	 echo "$BINDIR/ufdb.sh stop"
	 echo
      ) >> /etc/rc.shutdown

      echo
      echo "======================================================================="
      echo "/etc/rc.local and /etc/rc.shutdown are modified to start/stop ufdbGuard"
      echo "======================================================================="
      echo
   else
      echo
      echo "Notice: /etc/rc.local and /etc/rc.shutdown already start/stop ufdbGuard"
      echo
   fi
}


install_netbsd () {

   if [ ! -w $INITDIR ]
   then
      echo "No permission to write to $INITDIR"
      exit 1
   fi

   rm -f $INITDIR/ufdb.sh
   cp mtserver/ufdb.sh $INITDIR/ufdb.sh
   chmod 755 $INITDIR/ufdb.sh

   echo
   echo "=================================================================="
   echo "ufdb.sh startup script is copied to $INITDIR "
   echo "=================================================================="
   echo
}


install_freebsd () {

   if [ ! -w $INITDIR ]
   then
      echo "install_etcfiles.sh: No permission to write to $INITDIR   ***"
      exit 1
   fi

   rm -f $INITDIR/ufdb.sh
   cp mtserver/ufdb.sh $INITDIR/ufdb.sh
   chmod 755 $INITDIR/ufdb.sh

   echo
   echo "=================================================================="
   echo "ufdb.sh startup script is copied to $INITDIR "
   echo "=================================================================="
   echo
}


install_solaris () {

   if [ ! -w "$RCROOT" ]
   then
      echo "No permission to write to $RCROOT"
      exit 1
   fi

   for file in /etc/rc0.d/K15ufdb /etc/rc1.d/K15ufdb /etc/rc2.d/K15ufdb /etc/rc3.d/S40ufdb \
	       /etc/rc4.d/K15ufdb /etc/rc5.d/S40ufdb /etc/rcS.d/K15ufdb
   do
      if [ -d `dirname $file` ]
      then
	 rm -f $file
	 cp mtserver/ufdb.sh $file
	 chmod 755 $file
      fi
   done

   echo
   echo "=================================================================="
   echo "ufdb startup script is copied to /etc/rc*.d/K15ufdb and /etc/rc*.d/S40ufdb"
   echo "=================================================================="
   echo
}


install_unix () {

   if [ ! -w $INITDIR ]
   then
      echo "No permission to write to $INITDIR"
      exit 1
   fi

   rm -f $INITDIR/ufdb
   cp mtserver/ufdb.sh $INITDIR/ufdb
   chmod 755 $INITDIR/ufdb

   # some old installers made a /etc/rc3.d/S40ufdb which was not a softlink, remove the old beasties
   rm -f $RCROOT/rc3.d/S40ufdb
   rm -f $RCROOT/rc5.d/S40ufdb

   if [ ! -f $RCROOT/rc0.d/K02ufdb ]		# rc0
   then
      ln -s $INITDIR/ufdb $RCROOT/rc0.d/K02ufdb
   fi

   if [ ! -f $RCROOT/rc3.d/K02ufdb ]		# rc3
   then
      ln -s $INITDIR/ufdb $RCROOT/rc3.d/K02ufdb
   fi
   if [ ! -f $RCROOT/rc3.d/S50ufdb ]
   then
      ln -s $INITDIR/ufdb $RCROOT/rc3.d/S50ufdb
   fi

   if [ ! -f $RCROOT/rc5.d/K02ufdb ]		# rc5
   then
      ln -s $INITDIR/ufdb $RCROOT/rc5.d/K02ufdb
   fi
   if [ ! -f $RCROOT/rc5.d/S50ufdb ]
   then
      ln -s $INITDIR/ufdb $RCROOT/rc5.d/S50ufdb
   fi

   if [ ! -f $RCROOT/rc6.d/K02ufdb ]		# rc6
   then
      ln -s $INITDIR/ufdb $RCROOT/rc6.d/K02ufdb
   fi

   echo
   echo "=================================================================="
   echo "ufdb startup script is copied to $INITDIR/ufdb"
   echo "=================================================================="
   echo
}


# Since version 1.14 "su" is not used any more
check_su () {

   eval `grep ^RUNAS= mtserver/ufdb.sh `
   if [ "$RUNAS" != ""  -a  "$RUNAS" != root ]
   then
      SUTEST=`su $RUNAS -c "echo sutest" | tail -1`
      if [ "$SUTEST" != sutest ]
      then
         echo
	 echo "***** \"su $RUNAS\" failed.  Make sure that user $RUNAS has a valid login shell.  *****"
	 echo
      fi
   fi
}

case $OS in
   netbsd)      install_netbsd ;;
   freebsd)     install_freebsd ;;
   openbsd)     install_openbsd ;;
   solaris)     install_solaris ;;
   *) 		install_unix ;;
esac


if [ -d /etc/sysconfig ]
then
   if [ -f /etc/sysconfig/ufdbguard ]
   then
      cp mtserver/sysconfig.ufdbguard /etc/sysconfig/ufdbguard.latest
      echo "/etc/sysconfig/ufdbguard exists.  a new version is in /etc/sysconfig/ufdbguard.latest"
   else
      cp mtserver/sysconfig.ufdbguard /etc/sysconfig/ufdbguard
   fi
fi

SERVICE=`grep ufdbguardd /etc/services`
if [ "$SERVICE" = "" ]
then
   (
      echo ""
      echo "# for URLfilterDB daemon : "
      echo "ufdbguardd      3977/tcp"
   ) >> /etc/services
fi

