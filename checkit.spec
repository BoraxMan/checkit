Summary: A file integrity checksum tool
Name: checkit
Version: 0.3.4
Release: 1
License: GPL
Group: System
Source: checkit-0.3.4.tar.gz
Source0: http://dennisk.customer.netspace.net.au/checkit/checkit-0.3.4.tar.gz

URL: http://dennisk.customer.netspace.net.au/checkit.html
Distribution: Fedora
Vendor: DK Soft
Packager: Dennis Katsonis <dennisk@netspace.net.au>

%description
Checksum adds additions data assurance
capabilities to filesystems which support
extended attributes.  Checkit allows you
to detect any otherwise undetected data
integrity issues or file changes to any file.
By storing a checksum as an extended attribute,
checkit provides an easy way to detect any
silent data corruption, bit rot or otherwise
undetected error.

%prep
%setup

%build
%_configure --prefix=/usr CFLAGS="-DNDEBUG -O3" --without-gcc-arch
make

%install
rm -rf $RPM_BUILD_ROOT
make install-strip DESTDIR=$RPM_BUILD_ROOT

%files
%defattr(-,root,root,-)
%{_docdir}/*
%{_bindir}/*
%{_mandir}/man1/*
#/usr/share/doc/checkit/README

%clean
rm -rf $RPM_BUILD_ROOT

