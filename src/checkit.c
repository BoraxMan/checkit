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
#include <fcntl.h>
#include <stdio.h>
#include <attr/attributes.h>
#include <stdint.h>
#include <unistd.h>
#include <errno.h>
#include <dirent.h>
#include "crc64.h"

#define MAX_BUF_LEN  (65536)
#define RESET_TEXT()	printf("\033[0;0m")

#define RESET		0
#define BRIGHT 		1
#define DIM		2
#define UNDERLINE 	3
#define BLINK		4
#define REVERSE		7
#define HIDDEN		8

#define BLACK 		0
#define RED		1
#define GREEN		2
#define YELLOW		3
#define BLUE		4
#define MAGENTA		5
#define CYAN		6

#define VERBOSE 0b1
#define STORE	0b10
#define CHECK	0b100
#define DISPLAY	0b1000
#define REMOVE	0b10000
#define RECURSE	0b100000
#define OVERWRITE	0b1000000
#define PRINT	0b10000000
#define VERSION "0.0.1"


static int proc_dir(const char *dir);
static int proc_file(const char *filename);
unsigned long long FileCRC64(const char *filename);
uint64_t crc64(uint64_t crc, const unsigned char *s, uint64_t l);
void textcolor(int attr, int fg, int bg);
void print_help(void);

int flags = 0;
int processed = 0;
int failed = 0;

void print_help(void)
{
  puts("CHECKIT: A file checksum utility.");
  puts("(C) Dennis Katsonis (2014)");
  puts("");
  puts("CRC64 routine copyright..");
  puts("Copyright (c) 2012, Salvatore Sanfilippo <antirez at gmail dot com>");
  puts("All rights reserved.");
  puts("");
  printf("Version : %s\n",VERSION);
  puts("");
  puts("Checkit stores a checksum (CRC64) as an extended attribute.  Using");
  puts("this program you can easily calculate and store a checksum as");
  puts("a file attribute, and check the file data against the checksum");
  puts("at any time, to determine if there have been any changes to");
  puts("the file.");
  puts("");
  puts("Usage :");
  puts(" -s 	Calculate and store checksum");
  puts(" -c	Check file against stored checksum");
  puts(" -v	Verbose.  Print more information");
  puts(" -p	Display CRC64 checksum");
  puts(" -x	Remove stored CRC64 checksum");
  puts(" -o	Overwrite existing checksum (by default, checkit will not overwrite an existing checksum)");
  puts(" -r	Recurse through directories");
  printf("Version : %s\n",VERSION);
}


void textcolor(int attr, int fg, int bg)
{
	char command[13];

  	/* Command is the control command to the terminal */
	sprintf(command, "%c[%d;%d;%dm", 0x1B, attr, fg + 30, bg + 40);
	printf("%s", command);
} /* end of textcolor() */


unsigned long long FileCRC64(const char *filename)
{
  
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
	puts("Error reading file.");
	return 0;
      }
    temp =  (unsigned long long) crc64(temp, buf, (unsigned int)bufread);
    tot = tot + bufread;
    if (bufread < MAX_BUF_LEN)
      cont = 0;
  }
  if (flags & VERBOSE)
    printf("%llu bytes processed.\n",tot);
  close(fd);
  return temp;
}

int proc_dir(const char *dir)
{
  
  DIR *dp;
  struct dirent *entry;
  struct stat statbuf;
 
  if((dp = opendir(dir)) == NULL)
  {
    fprintf(stderr, "Cannot open directory: %s\n",dir);
    return(1);
  }
  else
  {
    
    chdir(dir);
     while((entry = readdir(dp)) != NULL)
      {
	stat(entry->d_name, &statbuf);
	if (S_ISDIR(statbuf.st_mode))
	{
	  if(strcmp(".", entry->d_name) == 0 || strcmp("..", entry->d_name) == 0)
	     continue;
	  proc_dir(entry->d_name);
	}
	else
	{
	  proc_file(entry->d_name);
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
      
static int proc_file(const char *filename)
{ 
  struct stat statbuf;
  unsigned long long checksum_attr;
  unsigned long long checksum_file;
  int attr_len = sizeof(unsigned long long);
  int file;
  int ATTRFLAGS;
  int attrcount = 0;
  char buf[4096];
  attrlist_cursor_t cursor;
  attrlist_ent_t *attrbufl;

  if (flags & OVERWRITE)
    ATTRFLAGS = 0;
  else
    ATTRFLAGS = ATTR_CREATE;
  
  if ((file = stat (filename, &statbuf)) != 0 )
  {
    perror("failed to open file: \n");
    return 1;
  }
  
  if (flags & STORE)
  {
    if (!(flags & OVERWRITE))
    {
      if(flags  & VERBOSE)
	printf("Checking for existing xattrs in %s\n",filename);

      cursor.opaque[0] = 0;
      cursor.opaque[1] = 0;
      cursor.opaque[2] = 0;
      cursor.opaque[3] = 0;
      
      if (attr_list(filename, buf, 4096, 0, &cursor) != 0)
      {
	puts("Could not check attributes.");
	return 1;
      }
     if (((attrlist_t *) buf)->al_count)
     for (attrcount = 0; attrcount < ((attrlist_t *) buf)->al_count; attrcount++)
      {
	attrbufl = ATTR_ENTRY(buf, attrcount);
	if (!strcmp(attrbufl->a_name, "crc64")) /* We've found an existing attribute */
	{
	  puts("CRC64 attribute already exists");
	  return 1;
	}
      } /* End for loop */
    }
  }
  

  if (S_ISREG (statbuf.st_mode))
  {

    if(flags & VERBOSE)
      printf("Checksumming file %s.\t", filename);
      checksum_file = FileCRC64(filename);

      if (checksum_file == 0)
	printf("Could not process file %s or file only contains nulls.\n", filename);
        if (flags & REMOVE)
	{
	  if ((attr_remove(filename, "crc64",0)) == -1)
	  {
	    printf("%s\n",filename);
	    perror("Removing xattr failed");
	  }
	  else
	  {
	    processed++;
	    if (flags & VERBOSE)
	      printf("Checksum removed\n");
	  }
	    
	}
	
      if (flags & DISPLAY) /* Display CRC64 */
	printf("Checksum for %s: %llx\n", filename, checksum_file);
	
      if (flags & STORE) /* Calculate and store CRC64 */
	{
	  if ((attr_set(filename, "crc64", (const char *)&checksum_file, attr_len, ATTRFLAGS)) == -1)
	  {
	    printf("%s\n",filename);
	    perror("Setting xattr failed");
	  }
	  else
	  {
	    processed++;
	    if (flags & DISPLAY)
	      printf("Checksum for %s: %llx\n", filename, checksum_file);
	  }
	}
	
	if (flags & CHECK) /* Check CRC */
	{
	  if ((attr_get(filename, "crc64", (char *)&checksum_attr, &attr_len, 0)) == -1)
	  {
	    printf("%s\n",filename);
	    perror("Retrieving xattr failed");
	    return 1;
	  }
	  else
	  {
	    printf("%s\t[",filename);
	    if (checksum_attr == checksum_file)	
	    {
	      textcolor(BRIGHT,GREEN,BLACK);
	      printf("  OK  ");
	      RESET_TEXT();
	    }
	    else
	    {
	    textcolor(RESET,RED,BLACK);
	    printf(" FAILED ");
	    failed++;
	    RESET_TEXT();
	    }
	  processed++;
	  }
 
	printf("]\n");
	}
  }
    
  if (S_ISDIR(statbuf.st_mode) && (flags & RECURSE))
    if (proc_dir(filename))
      return 1;
  return 0;
}


int main(int argc, char *argv[])
{

  int optch;
   
  while ((optch = getopt(argc, argv,"hscvxrop")) != -1)
    switch (optch)
    {
      case 'h' :
	print_help();
	return 0;
	break;
      case 's' :
	flags |= STORE;
	break;
      case 'c' :
	flags |= CHECK;
	break;
      case 'v' :
	flags |= VERBOSE;
	break;
      case 'x' :
	flags |= REMOVE;
	break;
      case 'o' :
	flags |= OVERWRITE;
	break;
      case 'r' :
	flags |= RECURSE;
	break;
      case 'p' :
	flags |= DISPLAY;

	break;
	
      case '?' :
	puts("Unknown option.");
	puts("");
	print_help();
	break;
  }
  
  if (argc <=1)
  {
    print_help();
    return(0);
  }
  
 
    /* Check for conflicting options */
    if ((flags & CHECK) && (flags & STORE))
    {
      puts("Cannot store and check CRC at same time");
      return 1;
    }
    
   if ((flags & STORE) && (flags & REMOVE))
    {
      puts("Cannot remove and store CRC at same time");
      return 1;
    }
    
    if ((flags & CHECK) && (flags & REMOVE))
    {
      puts("Cannot remove and check CRC at same time");
      return 1;
    }
    
  optch = optind;

  if (optch < argc)
    {
    do
    {
      proc_file(argv[optch]);
    }
    while ( ++optch < argc);
    }  

  printf("%d file(s) processed.\n", processed);
  if (failed && processed)
    printf("%d file(s) failed.\n", failed);
  else if (processed)
    printf("All file(s) OK.\n");
  
  return 0;
}



   

