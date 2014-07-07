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
