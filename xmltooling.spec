Name:		xmltooling
Summary:	Open source XMLTooling library
Version:	1.2
Release:	1
Group:		System Environment/Libraries
Vendor:		Internet2
License:	Apache 2.0
URL:		http://www.opensaml.org/
Source0:	%{name}-%{version}.tar.gz
BuildRoot:	%{_tmppath}/%{name}-%{version}-root
%if 0%{?suse_version} > 1030
BuildRequires:  libXerces-c-devel >= 2.8.0
%else
BuildRequires:  xerces%{?xercesver}-c-devel >= 2.8.0
%endif
BuildRequires:	xml-security-c-devel >= 1.4.0
BuildRequires:	openssl-devel, curl-devel >= 7.10.6
%{?_with_log4cpp:BuildRequires:	log4cpp-devel >= 1.0}
%{!?_with_log4cpp:BuildRequires: log4shib-devel}

%description
The XMLTooling library contains generic XML parsing and processing
classes based on the Xerces-C DOM. It adds more powerful facilities
for declaring element- and type-specific API and implementation
classes to add value around the DOM, as well as signing and encryption
support.

This package contains the xmltooling runtime library.

%package devel
Summary: XMLTooling development Headers
Group: Development/Libraries
Requires: %{name} = %{version}
%if 0%{?suse_version} > 1030
Requires: libXerces-c-devel >= 2.8.0
%else
Requires: xerces%{?xercesver}-c-devel >= 2.8.0
%endif
Requires: xml-security-c-devel >= 1.4.0
Requires: openssl-devel, curl-devel >= 7.10.6
%{?_with_log4cpp:Requires: log4cpp-devel >= 1.0}
%{!?_with_log4cpp:Requires: log4shib-devel}

%description devel
The XMLTooling library contains generic XML parsing and processing
classes based on the Xerces-C DOM. It adds more powerful facilities
for declaring element- and type-specific API and implementation
classes to add value around the DOM, as well as signing and encryption
support.

This package contains the headers and other necessary files to build
applications or libraries that use or extend the xmltooling library.

%package docs
Summary: XMLTooling API Documentation
Group: Development/Libraries
Requires: %{name} = %{version}

%description docs
XMLTooling Library API documentation generated by doxygen.

%prep
%setup -q

%build
%configure %{?xmltooling_options}
%{__make}

%install
[ "$RPM_BUILD_ROOT" != "/" ] && %{__rm} -rf $RPM_BUILD_ROOT
%{__make} install DESTDIR=$RPM_BUILD_ROOT
find $RPM_BUILD_ROOT/%{_bindir} -type f |
  %{__sed} -e "s|$RPM_BUILD_ROOT||" | sort > rpm.binlist

%check
%{__make} check

%clean
[ "$RPM_BUILD_ROOT" != "/" ] && %{__rm} -rf $RPM_BUILD_ROOT

%ifnos solaris2.8 solaris2.9 solaris2.10 
%post -p /sbin/ldconfig
%endif

%ifnos solaris2.8 solaris2.9 solaris2.10 
%postun -p /sbin/ldconfig
%endif

%files -f rpm.binlist
%defattr(-,root,root,-)
%{_libdir}/libxmltooling.so.*
%{_libdir}/libxmltooling-lite.so.*
%dir %{_datadir}/xml/xmltooling
%{_datadir}/xml/xmltooling
%docdir %{_datadir}/doc/xmltooling
%{_datadir}/doc/xmltooling/README.txt
%{_datadir}/doc/xmltooling/LICENSE.txt
%{_datadir}/doc/xmltooling/NOTICE.txt
%{_datadir}/doc/xmltooling/CURL.LICENSE
%{_datadir}/doc/xmltooling/LOG4CPP.LICENSE
%{_datadir}/doc/xmltooling/OPENSSL.LICENSE

%files devel
%defattr(-,root,root,-)
%{_includedir}
%{_libdir}/libxmltooling.so
%{_libdir}/libxmltooling-lite.so

%files docs
%defattr(644,root,root,755)
%doc %{_datadir}/doc/xmltooling/api

%changelog
* Wed Dec 3 2008  Scott Cantor  <cantor.2@osu.edu>  - 1.2-1
- Bumping for minor update.
- Fixing SUSE Xerces dependency name.

* Tue Jul 1 2008  Scott Cantor  <cantor.2@osu.edu>  - 1.1-1
- Bumping for minor update.

* Mon Mar 17 2008  Scott Cantor  <cantor.2@osu.edu>  - 1.0-6
- Official release.

* Fri Jan 18 2008  Scott Cantor  <cantor.2@osu.edu>  - 1.0-5
- Release candidate 1.

* Thu Nov 08 2007  Scott Cantor  <cantor.2@osu.edu>  - 1.0-4
- Second public beta.

* Thu Aug 16 2007  Scott Cantor  <cantor.2@osu.edu>  - 1.0-3
- First public beta.

* Fri Jul 13 2007  Scott Cantor  <cantor.2@osu.edu>  - 1.0-2
- Second alpha.

* Wed Apr 12 2006  Scott Cantor  <cantor.2@osu.edu>  - 1.0-1
- First SPEC file based on various versions in existence.
