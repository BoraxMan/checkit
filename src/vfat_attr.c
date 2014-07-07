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
