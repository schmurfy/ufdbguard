###########################################################################################
# TODO: install rpmdev and run rpmdev-newspec to create a new spec file based on a template
###########################################################################################

Summary: ufdbGuard is a URL filter for Squid
Name: ufdbGuard

%define version 1.23
Version: %{version}
Release: 1

License: GNU General Public License v2.0 only
Group: Internet/Proxy
Prefix: /usr/local/ufdbguard
Provides: ufdbguardd

URL: http://www.urlfilterdb.com/
# The sources for all versions of ufdbGuard are on sourceforge.net
# The latest version can also be downloaded from URLfilterDB (Source1)
Source: %{name}-%{version}.tar.gz
Source0: http://downloads.sourceforge.net/%{name}/%{name}-%{version}.tar.gz
Source1: http://www.urlfilterdb.com/en/downloads/urlfiltersoftware.html

Buildroot: /local/src/ufdbGuard-%{version}

Requires: wget >= 1.11
Requires: openssl >= 0.9.7a-1
Requires: bzip2-libs >= 1.0.0
Buildrequires: openssl-devel-0.9.7a-1
Buildrequires: bzip2-devel-1.0.0

# TODO: %_initddir is macro for /etc/rc.d/init.d
Requires(post): chkconfig
Requires(preun): chkconfig
Requires(preun): initscripts

%description
ufdbGuard is a free URL filter for Squid with additional features like
SafeSearch enforcement for a large number of search engines, safer HTTPS 
visits and dynamic detection of proxies (URL filter circumventors).

ufdbGuard supports free and commercial URL databases that can be
downloaded from various sites and vendors.
You can even make your own URL database for ufdbGuard.

%post
# This adds the proper /etc/rc*.d links for the script
/sbin/chkconfig --add <script>

%preun
if [ $1 = 0 ] ; then
   /sbin/service <script> stop >/dev/null 2>&1
   /sbin/chkconfig --del <script>
fi

%files
%defattr(-,root,root,-)


# for pre-F13:
%clean
[ "$RPM_BUILD_ROOT" != "/" ] && rm -rf ${RPM_BUILD_ROOT}

# ufdbGuard is installed with user ufdb and group ufdb
Requires(pre): shadow-utils
%pre
getent group ufdb >/dev/null || groupadd -r ufdb
getent passwd ufdb >/dev/null || \
useradd -r -g ufdb -d /usr/local/ufdbguard -s /sbin/nologin \
-c "ufdbGuard URL filter" ufdb
exit 0

%install
make DESTDIR=$RPM_BUILD_ROOT install

