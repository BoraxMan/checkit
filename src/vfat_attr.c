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


static int _ioctl_attrs(char *file, __u32 *attrs, int ioctlnum)
{
  int fd;

  // Interesting, we don't need a read-write handle to call the SET ioctl.
  fd = open(file, O_RDONLY | O_NOATIME);
  if (fd < 0) {
    goto err;
  }

  if (ioctl(fd, ioctlnum, attrs) != 0) {
    goto err;
  }

  close (fd);
  return 0;

  err:
    close (fd);
    return -1;
}


int vfat_attr(char *file)
{

  __u32 attrs = 0;
  
  attrs |= ATTR_HIDDEN;  /* ATTR_{RO,HIDDEN,SYS,VOLUME,DIR,ARCH} */
  
  return _ioctl_attrs(file, &attrs, FAT_IOCTL_SET_ATTRIBUTES);

}
