Name:           log4cpp
Version:        0.3.5rc1
Release:        1
Summary:        C++ logging library

Group:          System Environment/Libraries
License:        LGPL
URL:            http://log4cpp.sourceforge.net/
Source0:        http://wayf.internet2.edu/shibboleth/%{name}-%{version}.tar.gz
Source1:	http://wayf.internet2.edu/shibboleth/%{name}-%{version}.tar.gz.asc
BuildRoot:      %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)

BuildRequires:  doxygen

%description
Log4cpp is library of C++ classes for flexible logging to files,
syslog, IDSA and other destinations. It is modeled after the Log4j
Java library, staying as close to their API as is reasonable.

%package        devel
Summary:        Development files for %{name}
Group:          Development/Libraries
Requires:       %{name} = %{version}-%{release}

%description    devel
Log4cpp is library of C++ classes for flexible logging to files,
syslog, IDSA and other destinations. It is modeled after the Log4j
Java library, staying as close to their API as is reasonable.

This package contains the development files for log4cpp.

%package        docs
Summary:        Developer documentation for %{name}
Group:          Documentation

%description    docs
Log4cpp is library of C++ classes for flexible logging to files,
syslog, IDSA and other destinations. It is modeled after the Log4j
Java library, staying as close to their API as is reasonable.

This package contains the documentation files for log4cpp.

%prep
%setup0 -q

%build
# TODO: --with-idsa
%configure
make %{?_smp_mflags}

%install
rm -rf $RPM_BUILD_ROOT _docs
make install DESTDIR=$RPM_BUILD_ROOT docdir=/_docs
mv $RPM_BUILD_ROOT/_docs .

%check || :
make check

%clean
rm -rf $RPM_BUILD_ROOT

%post -p /sbin/ldconfig
%postun -p /sbin/ldconfig

%files
%defattr(-,root,root,-)
%doc AUTHORS ChangeLog COPYING NEWS README THANKS TODO
%{_libdir}/*.so.*

%files devel
%defattr(-,root,root,-)
%{_bindir}/log4cpp-config
%{_datadir}/aclocal/log4cpp.m4
%{_includedir}/log4cpp
%{_mandir}/man3/*
%{_libdir}/*.so
%{_libdir}/*.a
%{_libdir}/pkgconfig/log4cpp.pc
%exclude %{_libdir}/*.la

%files docs
%defattr(-,root,root,-)
%doc _docs/*

%changelog
* Tue Oct 19 2004   Derek Atkins <derek@ihtfp.com> - 0.3.5-1
- First build for Shib.
