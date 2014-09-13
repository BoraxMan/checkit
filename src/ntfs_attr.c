/*  CHECKIT  
    A file checksummer and integrity tester 
    Copyright (C) 2014 Dennis Katsonis

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


/* Sets hidden attribute for NTFS filesystems.
It seems NTFS can support extended attributes with Linux, at 
least with NTFS-3G so this probably won't be necessary in most cases.*/


#include <stdlib.h>
#include <stdio.h>
#include <attr/attributes.h>
#include <attr/xattr.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>

#define FILE_ATTRIBUTE_HIDDEN 0x2

int ntfs_attr(const char *file)
{
  int fd;
  int32_t checksum_attr;
  int attr_len = sizeof(int32_t);
  
  if ((fd = open(file, O_WRONLY)) == -1)
    return 1;
  
  if ((getxattr(file, "system.ntfs_attrib", (char *)&checksum_attr, sizeof(int32_t))) == -1)
  {
    close(fd);
    return 1;
  }
  checksum_attr |= FILE_ATTRIBUTE_HIDDEN;
  
  if ((setxattr(file, "system.ntfs_attrib", (const char *)&checksum_attr, sizeof(attr_len), XATTR_REPLACE)) == -1)
  {
    close(fd);
    return 1;
  }
  close(fd);
  return 0;
}
