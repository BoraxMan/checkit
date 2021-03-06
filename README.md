CHECKIT
=======

Checkit is a file integrity tool for Linux and Unix systems.

Checkit Copyright (C) 2014 Dennis Katsonis

LICENCE INFO
------------

Checkit is free software distributed under the GNU GPL.  See below for
copyright for CRC64.

WHAT THIS PROGRAM DOES:
-----------------------

Checksum adds additions data assurance capabilities to filesystems which
support extended attributes.  Checkit allows you to detect any otherwise
undetected data integrity issues or file changes to any file.  By storing
a checksum as an extended attribute, checkit provides an easy way to
detect any silent data corruption, bit rot or otherwise modified error.

This was inspired by the checksumming that is performed by filesystems
like BTRFS and ZFS.  These filesystems ensure data integrity by storing
a checksum (CRC) of data and checking read data against the checksum.
With mirroring of data, they can silently heal the data should an error
be found.

This program does not duplicate this ability, but offers rudimentary
checksum abilities for other filesystems.  It simply calculates a
checksum and stores the checksum with the file.  It can then be later
used to verify the checksum against the data.  Any data corruption of
file changes would result in a failed check.

WHY IT WAS CREATED:
-------------------

Moving data from disk to disk, or leaving data on the disk, leaves a
very small possibility of silent data corruption.  While rare, the large
amounts of data being handled by drives make silent corruption a real
possibility.  While BTRFS and ZFS can handle this, other filesystems
can't.  This program was created to add an ability to detect (but not
fix) issues.  With the ability to detect, you can easily find out whether
a copy or extract operation occured perfectly, or whether there has been
bit rot in the file.

Backups provide point of reference, but comparison isn't very efficient,
as it involves reading two files.  Using a CRC, you only need to read
the file once, even after a copy operation to determine whether the file
is OK.  Also, should the file be different from the backup, which copy
is OK, the backup or the original?  Checkit will let you know whether
it is the backup or original which has changed.

There are other ways to do this.  You can use a cryptographic hash
and store a SHA-1 or MD5 or SHA-256 value in a seperate file, or even
use GPG and digitally sign the file.  The problem is, that the value
is stored in a seperate file, and doing directory recursion, or all
files of a particular type (i.e., all JPG's isn't as straighforward.
With checkit, the CRC is stored as an extended attribute.  It remains
as part of the file, and can be copied or archived automatically with
the file.  No need for seperate files to store the hash/checksum.

Checkit also has the ability to export this to a hidden file, and import
it back into an extended attribute.

HOW TO USE:
-----------

Checkit calculates and stores the CRC as an extended attribute
(user.crc64) or as a hidden file.  The file must reside on a filesystem
which supports extended attributes (XFS, JFS, EXT2, EXT3, EXT4, BTRFS,
ReiserFS among others) to use the extended attribute (recommended).

First, run checkit to store the CRC as an attribute, then at any time,
you can run checkit to check that the file data still matches the CRC.
If any of the files don't match, or the CRC is missing, checkit will
report it.  If you pass the "verbose" option, checkit will summarise
the files that didn't pass the checksum test at the end of the run.

Checkit allows you to determine whether the checksum is 'read only'
or 'read write', using the -d and -u options.  A 'read only' checksum
will not be updated, unless the '-o' overwrite option is used.  This
way, the checksum is not inadvertantly updated when rerunning checkit
over it.  A 'read write' checksum can be updated, and can be used in
those cases where you want to rerun checkit over the file, and
recompute the checksum because of intended changes.  For the most 
part, checkit is intended for files you want to remain static,
such as your precious family photos and movies, archives and such.



OPERATION:
----------

checkit [OPTION] [FILE]

Options:

-V	Print licence
-s	Calcuates and stores the checksum.
-c	Check file against stored checksum.
-v	Verbose.  Print more information.
-p	Display CRC64 checksum.
-x	Remove stored checksum.  This simply deletes the extended
attribute.
-o	Overwite existing checksum.  By default, checkit does not
overwrite an existing checkum.	This option allows you to update the
checksum, should the file be deliberately altered).
-r	Recurse through subdirectories.
-e	Export CRC to a hidden file.
-i	Import CRC from a hidden file.
-d	Disallow updating of CRC on this file
(for files you do not intend to change)
-u	Allow CRC on this file to be updated (for files you intend
to change)
[FILE] can include wildcards.

Examples:

checkit -s -o picture.jpg	;Calculates checksum of picture.jpg and
overwrites old CRC64
checkit -s -r .			;Processes current directory and all
sub-directories and files.
checkit -c -r pictures/		;Check the enture pictures
directory. Checkit will report whether all files are OK or not.

checkit \-d  dissertation.txt	;Sets the CRC as read only.
Checkit will NOT update the CRC if you try to store the checksum again.

checkit \-u dissertation.txt	;Setc the CRC as read write.
Checkit will update the checksum if you run it with the -s option.

LIMITATIONS:
------------

As checkit doesn't repair files, you need to ensure that you have backups
of important data.  Checkit by default stores the CRC in an extended
attribute.

If the checksum is stored as an extended attribute, which is the default,
it will be lost if you transfer the file to a filesystem which does not
support extended attributes, or the method of copying does not preserve
them.  If you export the CRC, then the CRC can travel with the file as long
as you copy the CRC hidden file as well.

Checkit cannot detect changes and recompute the checksum without being run
manually, because it doesn't run in the background.  While having a 'checkit'
daemon, which can automatically update the checksum should the file be
changed is potentially useful, this is better done by migrating to BTRFS
or ZFS, which handles this more efficiently than checkit ever could.

TODO:
-----

* Intergration with file managers.

CRC64 routine.

The checksum routine is the crc-64-jones created by Salvatore Sanfilippo.
