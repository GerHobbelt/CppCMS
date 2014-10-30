Name:           cppcms
Version:        1.0.5
Release:        1
Summary:        CppCMS - C++ Web Framework
Group:          Development/Libraries/C and C++
License:        LGPLv3
URL:            http://cppcms.com
Source:         http://downloads.sourceforge.net/project/cppcms/cppcms/%{version}/%{name}-%{version}.tar.bz2 
BuildRoot:      %{_tmppath}/%{name}-%{version}-build

BuildRequires:  cmake zlib-devel pcre-devel libicu-devel gcc-c++ gcc libgcrypt-devel make python

%description
A Free C++ Web Development Framework (not a CMS) aimed for Rapid Web Application Development. 
It differs from most of other web development frameworks like: Python Django, Java Servlets or C++ Wt in following:

- It is designed and tuned to handle extremely high loads.
- It uses modern C++ as primary development language in order to achieve first goal.
- It is aimed on development of Web Sites rather then "GUI Like" web applications.
- It is available under open source LGPLv3 license and alternative Commercial License for users who needs an alternative license for proprietary software development.


%package -n libcppcms
Summary:        CppCMS - C++ Web Framework
Group:          Development/Libraries/C and C++
License:        LGPLv3

%description -n libcppcms
A Free C++ Web Development Framework (not a CMS) aimed for Rapid Web Application Development. 
It differs from most of other web development frameworks like: Python Django, Java Servlets or C++ Wt in following:

- It is designed and tuned to handle extremely high loads.
- It uses modern C++ as primary development language in order to achieve first goal.
- It is aimed on development of Web Sites rather then "GUI Like" web applications.
- It is available under open source LGPLv3 license and alternative Commercial License for users who needs an alternative license for proprietary software development.

%package -n libcppcms-devel  
Summary:        Development libraries for %{name}  
Group:          Development/Libraries/C and C++  
Requires:       lib%{name} = %{version}, cppcms-tools = %{version}
  
%description -n libcppcms-devel  
Development files for %{name} 

%package -n cppcms-tools
Summary:        Various tools for %{name}  
Group:          Development/Libraries/C and C++  
Requires:       lib%{name} = %{version}  
  
%description -n cppcms-tools
Tools for %{name} 


%prep
%setup -q 

%build
cmake	-DCMAKE_INSTALL_PREFIX=%{_prefix} \
	-DLIBDIR=%{_libdir} \
	.
 
make %{?_smp_mflags}     

%install
%make_install

%clean
rm -rf %{buildroot}

%post -n libcppcms -p /sbin/ldconfig
%postun -n libcppcms -p /sbin/ldconfig

%files -n cppcms-tools 
%defattr(-,root,root)
%{_bindir}/cppcms_*

%files -n libcppcms
%defattr(-,root,root)
%{_libdir}/libcppcms.so.*
%{_libdir}/libbooster.so.*

%files -n libcppcms-devel 
%defattr(-,root,root)
%{_libdir}/libcppcms.so
%{_libdir}/libcppcms.a
%{_libdir}/libbooster.so
%{_libdir}/libbooster.a
%{_includedir}/cppcms
%{_includedir}/booster

%changelog

