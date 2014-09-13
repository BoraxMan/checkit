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


/* Sets hidden attribute for FAT filesystems */

#define _GNU_SOURCE

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <inttypes.h>
#include <linux/msdos_fs.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>


int vfat_attr(char *file)
{
  __u32 attrs;
  int fd;

  fd = open(file, O_WRONLY | O_NOATIME);
  if (fd < 0)
  {
    return 1;
  }  
  if (ioctl(fd, FAT_IOCTL_GET_ATTRIBUTES, &attrs) != 0)
  {
    close(fd);
    return 1;
  }
  attrs |= ATTR_HIDDEN;
  
  if (ioctl(fd, FAT_IOCTL_SET_ATTRIBUTES, &attrs) != 0)
  {
    close(fd);
    return 1;
  }

  close (fd);
  return 0;

}

