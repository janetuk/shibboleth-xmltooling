%define tarversion 2_7_1
%define barename xerces-c

# threads
# values: pthreads, none
%define threads pthreads

Summary:	Xerces-C++ validating XML parser
Name:		xerces271-c
Version:	2.7.1
Release:	1
URL:		http://shibboleth.internet2.edu/downloads/
Source0:	%{barename}-src_%{tarversion}.tar.gz
License:	Apache
Group:			Libraries
BuildRoot:	%{_tmppath}/%{name}-root
Prefix:		/usr
Obsoletes:	%{barename} <= %{version}

%description
Xerces-C++ is a validating XML parser written in a portable subset of C++.
Xerces-C++ makes it easy to give your application the ability to read and
write XML data. A shared library is provided for parsing, generating,
manipulating, and validating XML documents.

The parser provides high performance, modularity, and scalability. Source
code, samples and API documentation are provided with the parser. For
portability, care has been taken to make minimal use of templates, no RTTI,
and minimal use of #ifdefs.

%package        samples
Summary:        Sample applications using Xerces-C++
Group:          Applications/Text
Requires:       %{name} = %{version}-%{release}
Obsoletes:	%{barename}-samples <= %{version}

%description    samples
Sample applications using Xerces-C++.

%package devel
Requires:	%{name} = %{version}-%{release}
Obsoletes:	%{barename}-devel <= %{version}
Group:		Development/Libraries
Summary:	Header files for Xerces-C++ validating XML parser

%description devel
Header files you can use to develop XML applications with Xerces-C++.

Xerces-C++ is a validating XML parser written in a portable subset of C++.
Xerces-C++ makes it easy to give your application the ability to read and
write XML data. A shared library is provided for parsing, generating,
manipulating, and validating XML documents.

%package doc
Obsoletes:	%{barename}-doc <= %{version}
Group:		Documentation
Summary:	Documentation for Xerces-C++ validating XML parser

%description doc
Documentation for Xerces-C++.

Xerces-C++ is a validating XML parser written in a portable subset of C++.
Xerces-C++ makes it easy to give your application the ability to read and
write XML data. A shared library is provided for parsing, generating,
manipulating, and validating XML documents.

%prep
%setup -q -n %{barename}-src_%{tarversion}

%build
export CC=%{__cc}
export CXX=%{__cxx}

%ifarch alpha ppc64 s390x sparc64 x86_64 ia64
  %define rcopts -b 64 -p %{_target_os}
%else
  %define rcopts -b 32 -p %{_target_os}
%endif

export XERCESCROOT=`pwd`
cd $XERCESCROOT/src/xercesc
./runConfigure %{rcopts} -C --libdir="%{_libdir}" -minmem -nsocket -tnative -r%{threads} -P%{prefix}
make
cd $XERCESCROOT/samples
./runConfigure %{rcopts}
make

%install
rm -rf $RPM_BUILD_ROOT
make -C src/xercesc install XERCESCROOT=`pwd` DESTDIR=$RPM_BUILD_ROOT
if [ ! -e $RPM_BUILD_ROOT%{_prefix}/%{_lib} ]; then
	mv $RPM_BUILD_ROOT%{_prefix}/lib $RPM_BUILD_ROOT%{_prefix}/%{_lib} 
fi
# Samples
mv bin/obj __obj
install -dm 755 $RPM_BUILD_ROOT%{_bindir}
install -pm 755 bin/* $RPM_BUILD_ROOT%{_bindir}
mv __obj bin/obj

%clean
rm -rf $RPM_BUILD_ROOT

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig

%files
%defattr(-,root,root,-)
%doc LICENSE* NOTICE STATUS credits.txt
%{_libdir}/libxerces-*.so.*

%files samples
%defattr(-,root,root,-)
%{_bindir}/*

%files devel
%defattr(-,root,root,-)
%{_includedir}/xercesc
%{_libdir}/libxerces-*.so

%files doc
%defattr(644,root,root,755)
%doc doc/html/*

%changelog
* Wed Jun  6 2007 Scott Cantor <cantor.2@osu.edu>
- reapply improvements from our spec file for 2.6.0

* Fri Jun  6 2003 Tuan Hoang <tqhoang@bigfoot.com>
- updated for new Xerces-C filename and directory format
- fixed date format in changelog section

* Fri Mar 14 2003 Tinny Ng <tng@ca.ibm.com>
- changed to 2.3

* Wed Dec 18 2002 Albert Strasheim <albert@stonethree.com>
- added symlink to libxerces-c.so in lib directory

* Fri Dec 13 2002 Albert Strasheim <albert@stonethree.com>
- added seperate doc package
- major cleanups

* Tue Sep 03 2002  <thomas@linux.de>
- fixed missing DESTDIR in Makefile.util.submodule

* Mon Sep 02 2002  <thomas@linux.de>
- Initial build.
