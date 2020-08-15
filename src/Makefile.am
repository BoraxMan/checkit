#AM_CFLAGS = -Wall -O3 -s
#AM_LDFLAGS = 
AM_CFLAGS =  '-DDATADIR="$(datadir)"' -I$(srcdir)/../include -I$(srcdir)

bin_PROGRAMS = checkit
checkit_SOURCES = checkit_cli.c checkit.c checkit_attr.c crc64.c vfat_attr.c ntfs_attr.c strarray.c
