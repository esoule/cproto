Summary:  cproto - generate C function prototypes and convert function definitions
%define AppProgram cproto
%define AppVersion 4.7l.1.1
# $XTermId: cproto.spec,v 1.5 2014/01/01 15:30:54 tom Exp $
Name: %{AppProgram}
Version: %{AppVersion}
Release: 1
License: Public Domain
Group: Applications/Development
URL: ftp://invisible-island.net/%{AppProgram}
Source0: %{AppProgram}-%{AppVersion}.tgz
Packager: Thomas Dickey <dickey@invisible-island.net>

%description
Cproto generates function prototypes for functions defined in the specified C
source files to the standard output.  The function definitions may be in the
old style or ANSI C style.  Optionally, cproto also outputs declarations for
variables defined in the files.

%prep

%setup -q -n %{AppProgram}-%{AppVersion}

%build

INSTALL_PROGRAM='${INSTALL}' \
	./configure \
		--target %{_target_platform} \
		--prefix=%{_prefix} \
		--bindir=%{_bindir} \
		--libdir=%{_libdir} \
		--mandir=%{_mandir}

make

%install
[ "$RPM_BUILD_ROOT" != "/" ] && rm -rf $RPM_BUILD_ROOT

make install                    DESTDIR=$RPM_BUILD_ROOT

strip $RPM_BUILD_ROOT%{_bindir}/%{AppProgram}

%clean
[ "$RPM_BUILD_ROOT" != "/" ] && rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root)
%{_prefix}/bin/%{AppProgram}
%{_mandir}/man1/%{AppProgram}.*

%changelog
# each patch should add its ChangeLog entries here

* Sun Jan 02 2011 Thomas Dickey
- cproto 4.7j

* Wed Jul 14 2010 Thomas Dickey
- initial version
