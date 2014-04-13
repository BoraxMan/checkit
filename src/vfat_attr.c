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

  __u32 attrs = 0;
  int fd;
  attrs |= ATTR_HIDDEN; 
 
 fd = open(file, O_WRONLY | O_NOATIME);
  if (fd < 0) {
    goto err;
  }

  if (ioctl(fd, FAT_IOCTL_SET_ATTRIBUTES, &attrs) != 0) {
    goto err;
  }

  close (fd);
  return 0;

  err:
    close (fd);
    perror("");
    return -1;

}
