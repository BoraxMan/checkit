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

#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/vfs.h>
#include <fcntl.h>
#include <stdio.h>
#include <attr/xattr.h>
#include <linux/limits.h>
#include <sys/statfs.h>
#include <stdint.h>
#include <unistd.h>
#include <errno.h>
#include <libgen.h>


#include "crc64.h"
#include "checkit.h"
#include "fsmagic.h"

const int MAX_BUF_LEN  = 65536;
int processed = 0;
int failed = 0;

const char* attributeName = "user.crc64";

const char* errorMessage(int error)
{ /* Standardised error messages. */
  char *_error[] = {
    "Failed to calculate CRC from file.",
    "Failed to remove extended attribute.",
    "Could not store CRC.",
    "Could not open directory.",
    "Could not open file.",
    "Could not read file.",
    "Setting CRC failed.  Could not write attribute.",
    "Could not remove hidden checksum file.",
    "No extended attribute to export.",
    "Can not overwrite existing checksum.",
    "Could not write to file.",
    "Filename too long."
  };
  return _error[error];
}

char* hiddenCRCFile(const char *file)
{ /* Returns a string with the filename of the hidden CRC file */
  static char crc_file[PATH_MAX - 1] = "\0";
  char *base_filename;
  char *dir_filename;
  char *_filename;
   
  _filename = strdup(file);
  
  base_filename = basename(_filename);
  dir_filename = dirname(_filename);  
  sprintf(crc_file, "%s//.%s.crc64", dir_filename, base_filename);

  free(_filename); /* It seems basename() and dirname() refer to this string,
		    * so we cannot free it until we are done with the strings
		    * it provides. */
  return(crc_file);
}

static int fileExists(const char* file) {
  struct stat buf;
  return (stat(file, &buf) == 0);
}

int presentCRC64(const char *file)
{  /* Check if CRC64 attribute is present. Returns XATTR if xattr, HIDDEN if hidden file. */
  char buf[LIST_XATTR_BUFFER_SIZE];
  char *current_attr;
  int x;

  x = listxattr(file,buf,LIST_XATTR_BUFFER_SIZE);
  current_attr = buf;    
  if (x != -1)
  {
  do {
    if (strcmp(current_attr, attributeName) == 0)
      return XATTR;

      else
      current_attr += (strlen(current_attr) + 1);
    } while ((current_attr - buf) < x);
  }
  /* No attribute?  Lets look for an existing hidden file. */

  if (fileExists(hiddenCRCFile(file)))
    return HIDDEN_ATTR;

  errno = 0; /* Clear errno from any previous issue. We will be printing
	      * an error message, but it is not related to any previous error
	      * encountered (such as not finding the hidden CRC file. */
  return 0;    
}


int exportCRC(const char *filename, int flags)
{
  int file_handle;
  t_crc64 crc64;

  if (presentCRC64(filename) != XATTR)
    return ERROR_NO_XATTR; /* No extended attribute to export. */
    
  if (fileExists(hiddenCRCFile(filename)) && (!(flags & OVERWRITE)))
    return ERROR_NO_OVERWRITE; /* Don't overwrite attribute unless allowed. */

  if ((file_handle = open(hiddenCRCFile(filename), O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR)) == -1)
    return ERROR_OPEN_FILE;
  
  crc64 = getCRC(filename);
  if(!crc64) /* If 0 returned (error), return with error being we couldn't read the file.
	      * perror will print more detail. */
    return ERROR_READ_FILE;
  
  write(file_handle, &crc64, sizeof (t_crc64));
  close(file_handle);

  if ((removexattr(filename, attributeName)) == -1)
    return ERROR_REMOVE_XATTR;
  
  //++processed;
  return 0;
}
  
int removeCRC(const char *filename)
{ /* Removes CRC, either the xattr, hidden file, or both */
  if (presentCRC64(filename) == XATTR)
  {
    if ((removexattr(filename, attributeName)) == -1)
      return ERROR_REMOVE_XATTR;
  }
  if (presentCRC64(filename) == HIDDEN_ATTR)
    if ((unlink(hiddenCRCFile(filename)) == -1) && VERBOSE)
      return ERROR_REMOVE_HIDDEN;

  //++processed;
  return 0;
}

int importCRC(const char *filename, int flags)
{
  int file_handle;
  t_crc64 crc64;
  int ATTRFLAGS;
      
  ATTRFLAGS = (flags & OVERWRITE) ? 0 : XATTR_CREATE;
  
  if ((presentCRC64(filename) != HIDDEN_ATTR) && (flags & OVERWRITE))
    return ERROR_NO_OVERWRITE;
  
  if ((file_handle = open(hiddenCRCFile(filename), O_RDONLY)) == -1)
    return ERROR_OPEN_FILE;
  
  /*crc64 = getCRC(filename);*/
  read(file_handle, &crc64, sizeof (t_crc64));
  close(file_handle);
  if ((setxattr(filename, attributeName, (const char *)&crc64, sizeof(crc64), ATTRFLAGS)) == -1)
    return ERROR_SET_CRC;

  unlink(hiddenCRCFile(filename));
  //++processed;

  return 0;
}

t_crc64 FileCRC64(const char *filename)
{ /* Open file and calcuate CRC.  Returns 0 on failure, otherwise returns the CRC. */
  unsigned char buf[MAX_BUF_LEN];
  size_t bufread = MAX_BUF_LEN;
  int cont = 1;
  int fd;
  uint64_t tot = 0;
  uint64_t temp = 0;
  
  if ((fd = open(filename,O_RDONLY)) == -1)
    return ERROR_CRC_CALC; /* ERROR_CRC_CALC is defined as being 0. */
  
  while (cont)
  {
    bufread = read(fd, buf, bufread);
      if (bufread == -1)
      {
	close(fd);
	return ERROR_CRC_CALC;
      }
    temp =  (t_crc64) crc64(temp, buf, (unsigned int)bufread);
    tot = tot + bufread;
    if (bufread < MAX_BUF_LEN)
      cont = 0;
  }

  close(fd);
  return temp;
}

int putCRC(const char *file, int flags)
{     
  t_crc64 checksum_file;
  t_crc64 oldCRC = 0;

  int file_handle;
  int ATTRFLAGS;
  int fstype;
  fstype = getfsType(file);
      
  ATTRFLAGS = (flags & OVERWRITE) ? 0 : XATTR_CREATE;
  
  oldCRC = getCRC(file);
  checksum_file = FileCRC64(file);
  if ((checksum_file != oldCRC) && (oldCRC != 0) && (checksum_file != 0))
  {
    printf("File %s has been changed!\n", file);
  }
  
  if(fstype != VFAT && fstype != UDF && fstype != NFS)
  { /* If not VFAT or UDF, attempt to store CRC in extended attribute */
    if ((setxattr(file, attributeName, (const char *)&checksum_file, sizeof(checksum_file), ATTRFLAGS)) == -1)
      return ERROR_SET_CRC;
    else
    {
/* ++processed; */ 
      return 0; /* And we're done here, return to process next file */
    }
  } 

  if ((file_handle = open(hiddenCRCFile(file), O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR)) == -1)
    return ERROR_OPEN_FILE;
  if (write(file_handle, &checksum_file, sizeof (t_crc64)) == -1)
    return ERROR_WRITE_FILE;

  close(file_handle);
  if(fstype == VFAT) /* Set hidden flag for VFAT */
    vfat_attr(hiddenCRCFile(file));
  else if (fstype == NTFS) /* or NTFS */
    ntfs_attr(hiddenCRCFile(file));

  //++processed;
  return 0;
}

t_crc64 getCRC(const char *file)
{ /* This retreives the CRC, first by checking for an extended attribute
    then by looking for a hidden file.  Returns 0 if unsuccessful, otherwise
    return the checksum.*/
  int attribute_format;
  t_crc64 checksum_attr;
  int file_handle;
    
  attribute_format = presentCRC64(file);
  
  if (!attribute_format)
    return ERROR_CRC_CALC;
    
  if (attribute_format == XATTR)
  {
    if ((getxattr(file, attributeName, (char *)&checksum_attr, sizeof(t_crc64)) == -1))
    {
      return ERROR_CRC_CALC;
    }
    else
    {
    /* ++processed; */
      return checksum_attr;
    }
  }
  if (attribute_format == HIDDEN_ATTR)
  {
    if ((file_handle = open(hiddenCRCFile(file), O_RDONLY)) == -1)
      return ERROR_CRC_CALC;
    read(file_handle, &checksum_attr, sizeof(t_crc64));
    {
    /* ++processed; */
      return checksum_attr;
    }
  }
  return ERROR_CRC_CALC;
}

int getfsType(const char *file)
{
  int fstype;
  struct statfs sstat;
  
  statfs(file, &sstat);
  switch (sstat.f_type)
    {
    case MSDOS_SUPER_MAGIC:
      fstype = VFAT;
      break;
    case NTFS_SB_MAGIC:
      fstype = NTFS;
      break;
    case UDF_SUPER_MAGIC:
      fstype = UDF;
      break;
    case XFS_SUPER_MAGIC:
      fstype = XFS;
      break;
    case JFS_SUPER_MAGIC:
      fstype = JFS;
      break;
    case NFS_SUPER_MAGIC:
      fstype = NFS;
      break;
    case SMB_SUPER_MAGIC:
      fstype = SMB;
      break;
    case CIFS_MAGIC_NUMBER:
      fstype = CIFS;
      break;
    default:
      fstype = 0;
      break;
    }
  return fstype;
}