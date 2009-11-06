Name:		xmltooling
Version:	1.3.1
Release:	1
Summary:    OpenSAML XMLTooling library
Group:		System Environment/Libraries
Vendor:		Internet2
License:	Apache 2.0
URL:		http://www.opensaml.org/
Source:	    %{name}-%{version}.tar.gz
BuildRoot:	%{_tmppath}/%{name}-%{version}-root
%if 0%{?suse_version} > 1030
BuildRequires:  libXerces-c-devel >= 2.8.0
BuildRequires:  libxml-security-c-devel >= 1.4.0
%{?_with_log4cpp:BuildRequires: liblog4cpp-devel >= 1.0}
%{!?_with_log4cpp:BuildRequires: liblog4shib-devel}
%else
BuildRequires:  xerces%{?xercesver}-c-devel >= 2.8.0
BuildRequires:  xml-security-c-devel >= 1.4.0
%{?_with_log4cpp:BuildRequires: log4cpp-devel >= 1.0}
%{!?_with_log4cpp:BuildRequires: log4shib-devel}
%endif
BuildRequires:	gcc-c++, openssl-devel, curl-devel >= 7.10.6
%{!?_without_doxygen:BuildRequires: doxygen}
%if "%{_vendor}" == "redhat"
BuildRequires: redhat-rpm-config
%endif

%if "%{_vendor}" == "suse"
%define pkgdocdir %{_docdir}/%{name}
%else
%define pkgdocdir %{_docdir}/%{name}-%{version}
%endif

%description
The XMLTooling library contains generic XML parsing and processing
classes based on the Xerces-C DOM. It adds more powerful facilities
for declaring element- and type-specific API and implementation
classes to add value around the DOM, as well as signing and encryption
support.

%if 0%{?suse_version} > 1030
%package -n libxmltooling4
Summary:    OpenSAML XMLTooling library
Group:      Development/Libraries
Provides:   xmltooling = %{version}
Obsoletes:  xmltooling

%description -n libxmltooling4
The XMLTooling library contains generic XML parsing and processing
classes based on the Xerces-C DOM. It adds more powerful facilities
for declaring element- and type-specific API and implementation
classes to add value around the DOM, as well as signing and encryption
support.

This package contains just the shared library.
%endif

%if 0%{?suse_version} > 1030
%package -n libxmltooling-devel
Requires:   libxmltooling4 = %version
Obsoletes:  xmltooling-devel
%else
%package devel
Requires:   %name = %version
%endif
Summary: XMLTooling development Headers
Group: Development/Libraries
%if 0%{?suse_version} > 1030
Requires: libXerces-c-devel >= 2.8.0
Requires: libxml-security-c-devel >= 1.4.0
%{?_with_log4cpp:Requires: liblog4cpp-devel >= 1.0}
%{!?_with_log4cpp:Requires: liblog4shib-devel}
%else
Requires: xerces%{?xercesver}-c-devel >= 2.8.0
Requires: xml-security-c-devel >= 1.4.0
%{?_with_log4cpp:Requires: log4cpp-devel >= 1.0}
%{!?_with_log4cpp:Requires: log4shib-devel}
%endif
Requires: openssl-devel, curl-devel >= 7.10.6

%if 0%{?suse_version} > 1030
%description -n libxmltooling-devel
%else
%description devel
%endif
The XMLTooling library contains generic XML parsing and processing
classes based on the Xerces-C DOM. It adds more powerful facilities
for declaring element- and type-specific API and implementation
classes to add value around the DOM, as well as signing and encryption
support.

This package includes files needed for development with XMLTooling.

%prep
%setup -q

%build
%configure %{?xmltooling_options}
%{__make}

%install
%{__make} install DESTDIR=$RPM_BUILD_ROOT pkgdocdir=%{pkgdocdir}
# Don't package unit tester if present.
%{__rm} -f $RPM_BUILD_ROOT/%{_bindir}/xmltoolingtest

%check
%{__make} check

%clean
[ "$RPM_BUILD_ROOT" != "/" ] && %{__rm} -rf $RPM_BUILD_ROOT

%ifnos solaris2.8 solaris2.9 solaris2.10
%if 0%{?suse_version} > 1030
%post -n libxmltooling4 -p /sbin/ldconfig
%else
%post -p /sbin/ldconfig
%endif
%endif

%ifnos solaris2.8 solaris2.9 solaris2.10
%if 0%{?suse_version} > 1030
%postun -n libxmltooling4 -p /sbin/ldconfig
%else
%postun -p /sbin/ldconfig
%endif
%endif

%if 0%{?suse_version} > 1030
%files -n libxmltooling4
%else
%files
%endif
%defattr(-,root,root,-)
%{_libdir}/*.so.*
%dir %{_datadir}/xml/xmltooling
%{_datadir}/xml/xmltooling/*

%if 0%{?suse_version} > 1030
%files -n libxmltooling-devel
%else
%files devel
%endif
%defattr(-,root,root,-)
%{_includedir}/*
%{_libdir}/*.so
%doc %{pkgdocdir}

%changelog
* Mon Aug 31 2009  Scott Cantor  <cantor.2@osu.edu>  - 1.3-1
- Bump soname for SUSE packaging.

* Thu Aug 6 2009  Scott Cantor  <cantor.2@osu.edu>  - 1.2.1-1
- SuSE conventions
- Stop packaging unit tester

* Wed Dec 3 2008  Scott Cantor  <cantor.2@osu.edu>  - 1.2-1
- Bumping for minor update.
- Fixing SuSE Xerces dependency name.

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
