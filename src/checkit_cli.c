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


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/statfs.h>
#include <linux/limits.h>
#include <errno.h>
#include <dirent.h>
#include <libgen.h>

#include "checkit.h"
#include "checkit_attr.h"

extern int failed;
extern int processed;

static int processFile(char *filename, int flags);
static int processDir(char *path, char *dir, int flags);
static void printErrorMessage(int result, const char *filename);

void printErrorMessage(int result, const char *filename)
{
    
  printf("For file %s: %s\n", filename, errorMessage(result));
}

void printHeader(void)
{
  printf("CHECKIT: A file checksum utility.\tVersion : %s\n",VERSION);
  puts("(C) Dennis Katsonis (2014)");
  puts("");
  puts("CRC64 Copyright (c) 2012, Salvatore Sanfilippo <antirez at gmail dot com>");
  puts("All rights reserved.");
  puts("");
}


void printLicence(void)
{
  printHeader();
  puts("License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>.");
  puts("");
  puts("This program is free software: you can redistribute it and/or modify");
  puts("it under the terms of the GNU General Public License as published by");
  puts("the Free Software Foundation, either version 3 of the License, or");
  puts("(at your option) any later version.");
  puts("");
  puts("This program is distributed in the hope that it will be useful,");
  puts("but WITHOUT ANY WARRANTY; without even the implied warranty of");
  puts("MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the");
  puts("GNU General Public License for more details.");
  puts("You should have received a copy of the GNU General Public License");
  puts("along with this program.  If not, see <http://www.gnu.org/licenses/>.");
}

  

int processFile(char *filename, int flags)
{
  struct stat statbuf;
  int file;
  t_crc64 result;
  t_crc64 resultCRC;
  static char directory[PATH_MAX - 1];

  char *base_filename;
  char *dir_filename;
  char *_filename;
  char checkitAttributes;
 
  /* Seperate filename into directory and filename parts. */
  _filename = strdup(filename);
  base_filename = basename(_filename);
  dir_filename = dirname(_filename);
    
  if ((file = stat (filename, &statbuf)) != 0 )
  {
    printErrorMessage(ERROR_OPEN_FILE, filename);
    return ERROR_OPEN_FILE;
  }
  
  if (S_ISDIR(statbuf.st_mode) && (flags & RECURSE))
  {
    result = processDir(directory, filename, flags);
    if (result)
    {
      printErrorMessage(result, filename);
      return result;
    }
    else
      return 0;
  }
  
  if (base_filename[0] == '.')
    return 0; /* Don't process hidden files */
  
  if (strcmp(dir_filename, ".") != 0)
    sprintf(directory, "%s/", dir_filename);

  
  checkitAttributes = getCheckitOptions(filename);
  
  if (flags & STORE)
  {
    if (!(flags & OVERWRITE) && (checkitAttributes != UPDATEABLE))
    {
      result = presentCRC64(filename);
      if(result)
      {
	printErrorMessage(ERROR_NO_OVERWRITE, filename);
	return ERROR_NO_OVERWRITE;
      }
    }
  } /* We've checked there isn't a CRC we can overwrite and we are allowed to.  Lets continue. */

  if (S_ISREG (statbuf.st_mode))
  {
	
    if (flags & DISPLAY) /* Display CRC64 */
      {
	result = getCRC(filename);
	if(!result)
	{ /* getCRC returns 0 on error, so if 0, print error messsage and exit. */
	  printErrorMessage(result, filename);
	  return -1;
	}
	printf("Checksum for %s: %llx\n", filename, getCRC(filename));
	checkitAttributes = getCheckitOptions(filename);
	if (checkitAttributes == UPDATEABLE)
	  printf("R/W Checksum: Checkit can update this checksum.\n");
	if (checkitAttributes == STATIC)
	  printf("R/O Checksum: Checkit will not update this checksum.\n");
      }
    
    if (flags & SETCRCRO)
    {

      if(flags & VERBOSE)
	printf("Setting CRC for %s to remain static/read only.\n", filename);
      if (setCheckitOptions(filename, STATIC))
      {
	printErrorMessage(result,filename);
	return -1;
      }
    } /* End of set CRC option routine */
    
    if (flags & SETCRCRW)
    {
      if(flags & VERBOSE)
	printf("Setting CRC for %s to allow updates/read-write.\n", filename);

      if (setCheckitOptions(filename, UPDATEABLE))
      {
	printErrorMessage(result,filename);
	return -1;
      }
    } /* End of set CRC option routine */
      

    if (flags & EXPORT) /* Export CRC to file */
    {
      if (flags & VERBOSE)
	printf("Exporting attribute for %s to %s\n", filename, hiddenCRCFile(basename(filename)));
      result = exportCRC(filename, flags);
      if (result)
      { 
	printErrorMessage(result, filename);
	return result;
      }
    } /* End of export routine. */

    if (flags & IMPORT) /* Export CRC to file */
    {
      result = importCRC(filename, flags);
      if (result)
      {
	printErrorMessage(result, filename);
	return result;
      }
      
    } /* End of export routine. */
    
    
    if (flags & STORE) /* Calculate and store CRC64 */
    {
      printf("Storing checksum for file %s\n", filename);
      result = putCRC(filename, OVERWRITE);
      if (result)
      {
	printErrorMessage(result, filename);
	return result;
      }
    } /* End of store routine. */
    
    if (flags & CHECK) /* Check CRC */
    {
      result = FileCRC64(filename);
      if(!result)
      { /* FileCRC64 returns 0 on error, so if 0, print error message (couldn't calculate CRC) and exit. */
	printErrorMessage(ERROR_CRC_CALC, filename);
	return -1;
      }
      
      resultCRC = getCRC(filename);
      if(!resultCRC)
      { /* getCRC returns 0 on error, so if 0, print error messsage (couldn't read file) and exit. */
	printErrorMessage(ERROR_READ_FILE, filename);
	return -1;
      }
      
      if (result == resultCRC)
      {
	printf("%s%-20s\t[", directory, base_filename);
	textcolor(BRIGHT,GREEN,BLACK);
	printf("  OK  ");
	RESET_TEXT();
      }
      else
      {
	printf("%s%-20s\t[", directory, base_filename);
	textcolor(RESET,RED,BLACK);
	printf(" FAILED ");
	failed++;
	RESET_TEXT();
      }

    printf("]\n");
    } /* End of Check CRC routine */

    if (flags & REMOVE)
    {
      if (flags & VERBOSE)
	puts("Removing checksum.");
      
      result = removeCRC(filename);
      result |= removeCheckitOptions(filename);
      
      if (result)
      {
	printErrorMessage(result, filename);
	return result;
      }
    } /* End of Remove CRC routine */


  } /* End of file processing regime */
  free(_filename);
  ++processed;
  return 0;
}


int processDir(char *path, char *dir, int flags)
{ /* Process directory and files within it */	
  DIR *dp;
  char *dirend;
  struct dirent *entry;
  struct stat statbuf;
  struct statfs sstat;
  
  if((dp = opendir(dir)) == NULL)
    return ERROR_OPEN_DIR;

  chdir(dir);
  strcat(path, dir);
  strcat(path, "/"); /* Assemble directory name. */

  while((entry = readdir(dp)) != NULL)
  {
    stat(entry->d_name, &statbuf);
    statfs(entry->d_name, &sstat);

    if (S_ISDIR(statbuf.st_mode))
    {
    if(strcmp(".", entry->d_name) == 0 || strcmp("..", entry->d_name) == 0)
      continue;
      processDir(path, entry->d_name, flags);
    }
    else
    {
      processFile(entry->d_name, flags);
      if (flags & VERBOSE)
      {
	printf("Processing file %s.\n",entry->d_name);
      }
    }
  } /* End while */
  chdir("..");
  /* As we go up a directory, we remove the directory name from
   * the path by setting the '/' character prior to the directory name
   * to NULL, to truncate the string. */
  dirend = strrchr(path,'/'); /* The '/' at the end of the path. */
  if (dirend != NULL)
    *dirend = 0;
  dirend = strrchr(path,'/'); /* The '/' at the start of the path. */
  if (dirend != NULL) /* Make it null, to terminate the string here. */
    *++dirend = 0;
  closedir(dp);
  return 0;
}

void textcolor(int attr, int fg, int bg)
{ /*Change textcolour */
  char command[13];

  /* Command is the control command to the terminal */
  sprintf(command, "%c[%d;%d;%dm", 0x1B, attr, fg + 30, bg + 40);
  printf("%s", command);
} /* end of textcolor() */



void printHelp(void)
{
  printHeader();
  puts("Checkit stores a checksum (CRC64) as an extended attribute.  Using");
  puts("this program you can easily calculate and store a checksum as");
  puts("a file attribute, and check the file data against the checksum");
  puts("at any time, to determine if there have been any changes to");
  puts("the file.");
  puts("");
  puts("Options :");
  puts(" -s  Calculate and store checksum\t-c   Check file against stored checksum");
  puts(" -v  Verbose.  Print more information\t-p   Display CRC64 checksum and status");
  puts(" -x  Remove stored CRC64 checksum\t-o   Overwrite existing checksum");
  puts(" -r  Recurse through directories\t-i   Import CRC from hidden file");
  puts(" -e  Export CRC to hidden file  \t-f   Read list of files from stdin");
  puts(" -u  Allow CRC on this file to be updated (for files you intend to change)");
  puts(" -d  Disallow updating of CRC on this file (for files you do not intend to change)");
  puts(" -V  Print licence");
}


int main(int argc, char *argv[])
{
  int optch;
  char *line = NULL;
  size_t size;
  ssize_t read;
  char *ptr;
  int flags = 0;
  
  while ((optch = getopt(argc, argv,"hscvVudexirfop")) != -1)
    switch (optch)
    {
      case 'h' :
	printHelp();
	return 0;
	break;
      case 'V' :
	printLicence();
	return 0;
	break;
      case 's' :
	flags |= STORE;
	break;
      case 'c' :
	flags |= CHECK;
	if ((flags & CHECK) && (flags & STORE))
	{
	  puts("Cannot store and check CRC at same time.");
	  return 1;
	}
	break;

      case 'v' :
	flags |= VERBOSE;
	break;
      case 'x' :
	flags |= REMOVE;
	if (flags & CHECK)
	{
	  puts("Cannot remove and check CRC at same time.");
	  return 1;
	}
	if (flags & STORE)
	{
	  puts("Cannot remove and store CRC at same time.");
	  return 1;
	}
	break;
      case 'o' :
	flags |= OVERWRITE;
	break;
      case 'r' :
	flags |= RECURSE;
	break;
      case 'i' :
        flags |= IMPORT;
	break;
      case 'u' :
	flags |= SETCRCRW;
	if (flags & SETCRCRO)
	{
	  puts("Cannot disallow and allow changes to CRC at the same time!");
	  return 1;
	}
	break;
      case 'd' :
	flags |= SETCRCRO;
	if (flags & SETCRCRW)
	{
	  puts("Cannot disallow and allow changes to CRC at the same time!");
	  return 1;
	}
	break;
      case 'e' :
	flags |= EXPORT;
	if (flags & IMPORT)
	{
	  puts("Cannot import and export at the same time.");
	  return 1;
	}
	break;
      case 'f' :
	flags |= PIPEDFILES;
	break;
      case 'p' :
	flags |= DISPLAY;
	break;
	
      case '?' :
	printf("Unknown option.\n");
	printHelp();
	break;
  }

  if (argc <=1)
  {
    printHelp();
    return(0);
  }
  if (flags & PIPEDFILES)
  {
    while ((read = getline(&line, &size, stdin) != -1))
    {
    ptr = strrchr(line, '\n');
    if (ptr != NULL)
      *ptr = 0;
    processFile(line, flags);
    }
  free(line);
  }

  optch = optind;

  if (optch < argc)
  {
    do
    {
      processFile(argv[optch], flags);
    }
    while ( ++optch < argc);
  }  

  printf("%d file(s) processed.\n", processed);
  if (failed && processed)
    {
    printf("%d file(s) failed.\n", failed);
    return(failed);
    } /* Return the number of failed checks if any errors. */
  else if (processed && (flags & CHECK))
    printf("All file(s) OK.\n");
  
  return 0;
}

