%define name weewm
%define version 0.0.2
%define release 1

Name:      %{name}
Summary:   a fast and ultra-light window manager for XFree
Version:   %{version}
Release:   %{release}
Source:    http://www.weewm.org/download/%{name}-%{version}.tar.gz
URL:       http://www.weewm.org
Group:     Graphical desktop/Other
BuildRoot: %{_tmppath}/%{name}-buildroot
License:   GPL
Requires:  xfree

%description
WeeWM is a fast and ultra-light window manager for XFree.
Everything can be done with a keyboard. It is customizable,
with dock management, smart window placement, virtual desktops, and more.

%prep
rm -rf $RPM_BUILD_ROOT 
%setup

%build
make DESTDIR="$RPM_BUILD_ROOT" LOCALRPM="local"

%install 
make DESTDIR="$RPM_BUILD_ROOT" LOCALRPM="local" install

%clean 
rm -rf $RPM_BUILD_ROOT 

%files 
%defattr(-,root,root,0755) 
%doc AUTHORS BUGS COPYING FAQ FAQ.fr INSTALL NEWS README TODO examples/config.rc examples/keys.rc doc/weewm_fr.texi doc/weewm_en.texi
/usr/share/man/man1/weewm.1*
/usr/local/bin/weewm

%changelog 
* Mon Jul 07 2003 FlashCode <flashcode@flashtux.org> 0.0.2-1
- Released version 0.0.2
* Tue Mar 18 2003 FlashCode <flashcode@flashtux.org> 0.0.1-1
- Released version 0.0.1
