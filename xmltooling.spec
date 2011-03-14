Name:		xmltooling
Version:	1.4.1
Release:	1
Summary:    OpenSAML XMLTooling library
Group:		Development/Libraries/C and C++
Vendor:		Internet2
License:	Apache 2.0
URL:		http://www.opensaml.org/
Source:	    %{name}-%{version}.tar.gz
BuildRoot:	%{_tmppath}/%{name}-%{version}-root
%if 0%{?suse_version} > 1030 && 0%{?suse_version} < 1130
BuildRequires:  libXerces-c-devel >= 2.8.0
%else
BuildRequires:  libxerces-c-devel >= 2.8.0
%endif
BuildRequires:  libxml-security-c-devel >= 1.4.0
%{?_with_log4cpp:BuildRequires: liblog4cpp-devel >= 1.0}
%{!?_with_log4cpp:BuildRequires: liblog4shib-devel}
BuildRequires:	gcc-c++, openssl-devel, curl-devel >= 7.10.6
%if 0%{?suse_version} > 1000
BuildRequires: pkg-config
%endif
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

%package -n libxmltooling5
Summary:    OpenSAML XMLTooling library
Group:      Development/Libraries/C and C++
Provides:   xmltooling = %{version}-%{release}
Obsoletes:  xmltooling < %{version}-%{release}

%description -n libxmltooling5
The XMLTooling library contains generic XML parsing and processing
classes based on the Xerces-C DOM. It adds more powerful facilities
for declaring element- and type-specific API and implementation
classes to add value around the DOM, as well as signing and encryption
support.

This package contains just the shared library.

%package -n libxmltooling-devel
Summary:	XMLTooling development Headers
Group:		Development/Libraries/C and C++
Requires:	libxmltooling5 = %{version}-%{release}
Provides:	xmltooling-devel = %{version}-%{release}
Obsoletes:	xmltooling-devel < %{version}-%{release}
%if 0%{?suse_version} > 1030 && 0%{?suse_version} < 1130
Requires:  libXerces-c-devel >= 2.8.0
%else
Requires:  libxerces-c-devel >= 2.8.0
%endif
Requires: libxml-security-c-devel >= 1.4.0
%{?_with_log4cpp:Requires: liblog4cpp-devel >= 1.0}
%{!?_with_log4cpp:Requires: liblog4shib-devel}
Requires: openssl-devel, curl-devel >= 7.10.6

%description -n libxmltooling-devel
The XMLTooling library contains generic XML parsing and processing
classes based on the Xerces-C DOM. It adds more powerful facilities
for declaring element- and type-specific API and implementation
classes to add value around the DOM, as well as signing and encryption
support.

This package includes files needed for development with XMLTooling.

%package -n xmltooling-schemas
Summary:	XMLTooling schemas and catalog
Group:		Development/Libraries/C and C++

%description -n xmltooling-schemas
The XMLTooling library contains generic XML parsing and processing
classes based on the Xerces-C DOM. It adds more powerful facilities
for declaring element- and type-specific API and implementation
classes to add value around the DOM, as well as signing and encryption
support.

This package includes XML schemas and related files.

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
%post -n libxmltooling5 -p /sbin/ldconfig
%endif

%ifnos solaris2.8 solaris2.9 solaris2.10
%postun -n libxmltooling5 -p /sbin/ldconfig
%endif

%files -n libxmltooling5
%defattr(-,root,root,-)
%{_libdir}/*.so.*

%files -n xmltooling-schemas
%defattr(-,root,root,-)
%dir %{_datadir}/xml/xmltooling
%{_datadir}/xml/xmltooling/*

%files -n libxmltooling-devel
%defattr(-,root,root,-)
%{_includedir}/*
%{_libdir}/*.so
%{_libdir}/pkgconfig/xmltooling.pc
%doc %{pkgdocdir}

%changelog
* Tue Oct 26 2010  Scott Cantor  <cantor.2@osu.edu>  - 1.4-1
- Update version
- Add pkg-config support.
- Sync package names for side by side install.
- Adjust Xerces dependency name and Group setting
- Split out schemas into separate subpackage

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