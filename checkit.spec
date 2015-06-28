Summary: A file integrity checksum tool
Name: checkit
Version: 0.3.2
Release: 1
License: GPL
Group: System
Source: http://dennisk.customer.netspace.net.au/checkit/checkit-0.3.2.tar.gz
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
%_configure --prefix=/usr
make

%install
rm -rf $RPM_BUILD_ROOT
make install DESTDIR=$RPM_BUILD_ROOT

%files
%defattr(-,root,root,-)
%doc README
%{_bindir}/*
%{_mandir}/man1/*
/usr/share/doc/checkit/README

%clean
rm -rf $RPM_BUILD_ROOT

