/*******************************************/
/*              CHECKIT                    */
/* A file checksummer and integrity tester */
/*    By Dennis Katsonis   March 2014      */
/*         dennisk@netspace.net.au         */
/*                                         */
/*******************************************/

#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/vfs.h>
#include <fcntl.h>
#include <stdio.h>
#include <attr/attributes.h>
#include <sys/statfs.h>
#include <stdint.h>

#include <errno.h>
#include <dirent.h>

#include "crc64.h"
#include "checkit.h"
#include "fsmagic.h"

int flags = 0;
int processed = 0;
int failed = 0;
int fstype = 0;
 


static char* hiddenCRCFile(const char *file)
{
  static char crc_file[MAX_FILENAME_LENGTH] = "\0";

  strcpy(crc_file, ".");
  strncat(crc_file, file, MAX_FILENAME_LENGTH - 8);
  strcat(crc_file, ".crc64");
  return(crc_file);
}

static int fileExists(const char* file) {
  struct stat buf;
  return (stat(file, &buf) == 0);
}

int presentCRC64(const char *file)
{  /* Check if CRC64 attribute is present. Returns XATTR if xattr, HIDDEN if hidden file. */
  char buf[4096];
  
  attrlist_cursor_t cursor;
  attrlist_ent_t *attrbufl;
  int attrcount = 0;
  
  cursor.opaque[0] = 0;
  cursor.opaque[1] = 0;
  cursor.opaque[2] = 0;
  cursor.opaque[3] = 0;

  if (attr_list(file, buf, 4096, 0, &cursor) == 0)
     if (((attrlist_t *) buf)->al_count)
      for (attrcount = 0; attrcount < ((attrlist_t *) buf)->al_count; attrcount++)
      {
	attrbufl = ATTR_ENTRY(buf, attrcount);
	if (!strcmp(attrbufl->a_name, "crc64")) /* We've found an existing attribute */
	  return XATTR;
      }
  /* No attribute?  Lets look for an existing hidden file. */

  if (fileExists(hiddenCRCFile(file)))
    return HIDDEN_ATTR;
  
  return 0;    
}

void textcolor(int attr, int fg, int bg)
{ /*Change textcolour */
	char command[13];

  	/* Command is the control command to the terminal */
	sprintf(command, "%c[%d;%d;%dm", 0x1B, attr, fg + 30, bg + 40);
	printf("%s", command);
} /* end of textcolor() */

int exportCRC(const char *filename)
{
  int file_handle;
  t_crc64 crc64;


  if (presentCRC64(filename) != XATTR)
    return 1;
    
  if (fileExists(hiddenCRCFile(filename)) & (!(flags & OVERWRITE)))
    return 1; /* Don't overwrite attribute unless allowed. */

  if ((file_handle = open(hiddenCRCFile(filename), O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR)) == -1)
    return 1;
  
  crc64 = getCRC(filename);
  
  if (flags & VERBOSE)
    printf("Exporting to %s\r\n", hiddenCRCFile(filename));

  write(file_handle, &crc64, sizeof (t_crc64));
  close(file_handle);


  if ((attr_remove(filename, "crc64",0)) == -1)
    fprintf(stderr, "Removing xattr failed on %s: %s\r\n", filename);

  processed++;
  return 0;
}
  
int removeCRC(const char *filename)
{ /* Removes CRC, either the xattr, hidden file, or both */
  if (presentCRC64(filename) == XATTR)
  {
    if ((attr_remove(filename, "crc64",0)) == -1)
    {
      fprintf(stderr, "Removing xattr failed on %s: ", filename);
      perror(NULL);
      failed++;
      return 1;
    }
    else
    {
      if (flags & VERBOSE)
	printf("Checksum removed\n");
    }
  }
  if (presentCRC64(filename) == HIDDEN_ATTR)
    if ((unlink(hiddenCRCFile(filename)) == -1) && VERBOSE)
      perror("Could not remove hidden checksum file:");

  processed++;
  return 0;
}

int importCRC(const char *filename)
{
  int file_handle;
  t_crc64 crc64;
  int ATTRFLAGS;
      
  if (flags & OVERWRITE)
    ATTRFLAGS = 0;
  else
    ATTRFLAGS = ATTR_CREATE;
  
  if ((presentCRC64(filename) != HIDDEN_ATTR) && (flags & OVERWRITE))
    return 1;
  

  if (flags & VERBOSE)
    printf("Importing from %s\r\n", hiddenCRCFile(filename));

  if ((file_handle = open(hiddenCRCFile(filename), O_RDONLY)) == -1)
  {
    fprintf(stderr, "Could not read file.\r\n");
    return 1; 
  }
  crc64 = getCRC(filename);
  read(file_handle, &crc64, sizeof (t_crc64));
  close(file_handle);
  if ((attr_set(filename, "crc64", (const char *)&crc64, sizeof(crc64), ATTRFLAGS)) == -1)
  {
    fprintf(stderr, "Setting xattr failed on %s: ", filename);
    perror(NULL);
    return 1;
  }
  unlink(hiddenCRCFile(filename));
  processed++;
  return 0;
}

t_crc64 FileCRC64(const char *filename)
{ /* Open file and calcuate CRC */
  unsigned char buf[MAX_BUF_LEN];
  size_t bufread = MAX_BUF_LEN;
  int cont = 1;
  int fd;
  uint64_t tot = 0;
  uint64_t temp = 0;
  
  if ((fd = open(filename,O_RDONLY)) == -1)
  {
    perror("Failed to open file: \n");
    return 0;
  }
  
  while (cont)
  {
    bufread = read(fd, buf, bufread);
      if (bufread == -1)
      {
	perror("Error reading file.");
	close(fd);
	return 0;
      }
    temp =  (t_crc64) crc64(temp, buf, (unsigned int)bufread);
    tot = tot + bufread;
    if (bufread < MAX_BUF_LEN)
      cont = 0;
  }
  if (flags & VERBOSE)
    printf("%llu bytes processed.\n",tot);
  close(fd);
  return temp;
}

int processDir(const char *dir)
{ /* Process directory and files within it */	
  DIR *dp;
  struct dirent *entry;
  struct stat statbuf;
  struct statfs sstat;
  
  if((dp = opendir(dir)) == NULL)
  {
    fprintf(stderr, "Cannot open directory: %s\n",dir);
    return(1);
  }
  else
  {
    
    chdir(dir);
    statfs(entry->d_name, &sstat);
    switch (sstat.f_type)
    {
      case MSDOS_SUPER_MAGIC:
	fstype = VFAT;
	break;
      case NTFS_SB_MAGIC:
	fstype = NTFS;
	break;
      default:
	fstype = 0;
	break;
    }
    while((entry = readdir(dp)) != NULL)
    {
      stat(entry->d_name, &statbuf);
      if (S_ISDIR(statbuf.st_mode))
      {
	if(strcmp(".", entry->d_name) == 0 || strcmp("..", entry->d_name) == 0)
	  continue;
	  processDir(entry->d_name);
      }
      else
      {
	processFile(entry->d_name);
	if (flags & VERBOSE)
	{
	  printf("Processsing file %s.\n",entry->d_name);
	}
      }
     }
    chdir("..");
    closedir(dp);
  }
return 0;
}

int putCRC(const char *file)
{     
  t_crc64 checksum_file;

  int file_handle;
  int ATTRFLAGS;
      
  if (flags & OVERWRITE)
    ATTRFLAGS = 0;
  else
    ATTRFLAGS = ATTR_CREATE;
  
  if (flags & VERBOSE)
    printf("Processing %s : ", file);
  
  checksum_file = FileCRC64(file);
  if ((attr_set(file, "crc64", (const char *)&checksum_file, sizeof(checksum_file), ATTRFLAGS)) == -1)
  {
    fprintf(stderr, "Setting xattr failed on %s: \r\n", file);
    perror(NULL);
  }
  else
  {
    processed++;
    return 0;
  }
   
  if ((file_handle = open(hiddenCRCFile(file), O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR)) == -1)
  {
    perror(NULL);
    return 1;
  }
  if (write(file_handle, &checksum_file, sizeof (t_crc64)) == -1)
  {  
    perror(NULL);
    close(file_handle);
    return 1;
  }
  
  close(file_handle);

  processed++;
  return 0;
}

t_crc64 getCRC(const char *file)
{ /* This retreives the CRC, first by checking for an extended attribute
    then by looking for a hidden file.  Returns 0 if unsuccessful, otherwise
    return the checksum.*/
  int attribute_format;
  t_crc64 checksum_attr;
  int file_handle;
  int attr_len = sizeof(t_crc64);
    
  attribute_format = presentCRC64(file);
  
  if (!attribute_format)
    return 0;
  
  if (attribute_format == XATTR)
  {
    if ((attr_get(file, "crc64", (char *)&checksum_attr, &attr_len, 0)) == -1)
    {
      return 0;
    }
    else
      return checksum_attr;
  }
  if (attribute_format == HIDDEN_ATTR)
  {
    if ((file_handle = open(hiddenCRCFile(file), O_RDONLY)) == -1)
      if (flags & VERBOSE)
      {
	fprintf(stderr, "Couldn't open file %s : ", file);
	perror(NULL);
      }
    read(file_handle, &checksum_attr, sizeof(t_crc64));
    return checksum_attr;
  }
  return 0;
}


int processFile(const char *filename)
{
  struct stat statbuf;
  int file;

  if (filename[0] == '.')
    return 0; /* Don't process hidden files */

  if ((file = stat (filename, &statbuf)) != 0 )
  {
    perror("Failed to open file: \n");
    return 1;
  }
  
  if (flags & STORE)
  {
    if (!(flags & OVERWRITE))
    {
      if(flags  & VERBOSE)
	printf("Checking for existing xattrs in %s\n",filename);
	if(presentCRC64(filename))
	{
	  fprintf(stderr,"Cannot overwrite existing CRC.\r\n");
	  return(1);
	}
      } 
    } /* We've checked there isn't a CRC we can overwrite.  Lets continue. */

  if (S_ISREG (statbuf.st_mode))
  {
	
    if (flags & DISPLAY) /* Display CRC64 */
      printf("Checksum for %s: %llx\n", filename, getCRC(filename));
	
    if (flags & EXPORT) /* Export CRC to file */
    {
      if (exportCRC(filename))
	return 1;
    } /* End of export routine. */

    if (flags & IMPORT) /* Export CRC to file */
    {
      if(importCRC(filename))
	return 1;
    } /* End of export routine. */
    
    
    if (flags & STORE) /* Calculate and store CRC64 */
    {
      if (putCRC(filename))
      {
	fprintf(stderr, "Failed to store CRC.\r\n");
	return 1;
      }
    } /* End of store routine. */
    
    if (flags & CHECK) /* Check CRC */
    {
      if (FileCRC64(filename) == getCRC(filename))
      {
	printf("%-20s\t[", filename);
	textcolor(BRIGHT,GREEN,BLACK);
	printf("  OK  ");
	RESET_TEXT();
      }
      else
      {
	printf("%-20s\t[", filename);
	textcolor(RESET,RED,BLACK);
	printf(" FAILED ");
	failed++;
	RESET_TEXT();
      }
    processed++;
    printf("]\n");
    } /* End of Check CRC routine */

    if (flags & REMOVE)
    {
      if (removeCRC(filename))
	return 1;
    } /* End of Remove CRC routine */


  } /* End of file processing regime */
    
  if (S_ISDIR(statbuf.st_mode) && (flags & RECURSE))
    if (processDir(filename))
      return 1;
  return 0;
}

   

