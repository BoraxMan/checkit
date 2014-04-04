/* Sets hidden attribute for FAT filesystems */

#include <inttypes.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <linux/msdos_fs.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int vfat_attr(const char *file)
{
  int fd;
  __u32 attrs = ATTR_HIDDEN;
  if (fd = open(file, O_WRONLY) == -1)
    return 1;
  ioctl(fd, FAT_IOCTL_SET_ATTRIBUTES, &attrs);
  close(file);
  return 0;
}
