/* Sets hidden attribute for NTFS filesystems */

#include <stdlib.h>
#include <stdio.h>
#include <attr/attributes.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>

#define FILE_ATTRIBUTE_HIDDEN 0x2

int ntfs_attr(const char *file)
{
  int fd;
  int32_t checksum_attr;
  int attr_len = sizeof(int32_t);
  
  if (fd = open(file, O_WRONLY) == -1)
    return 1;
  
  if ((attr_get(file, "system.ntfs_attrib", (char *)&checksum_attr, &attr_len, 0)) == -1)
  {
    close(file);
    return 0;
  }
  checksum_attr |= FILE_ATTRIBUTE_HIDDEN;
  
  if ((attr_set(file, "system.ntfs_attrib", (const char *)&checksum_attr, sizeof(attr_len), ATTR_REPLACE)) == -1)
  {
    close(file);
    return 1;
  }
  close(file);
  return 0;
}
