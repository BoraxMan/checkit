#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>

#include "checkit.h"

extern int flags;
extern int failed;
extern int processed;

void print_help(void)
{
  printf("CHECKIT: A file checksum utility.\tVersion : %s\n",VERSION);
  puts("(C) Dennis Katsonis (2014)");
  puts("");
  puts("CRC64 Copyright (c) 2012, Salvatore Sanfilippo <antirez at gmail dot com>");
  puts("All rights reserved.");
  puts("");
  puts("Checkit stores a checksum (CRC64) as an extended attribute.  Using");
  puts("this program you can easily calculate and store a checksum as");
  puts("a file attribute, and check the file data against the checksum");
  puts("at any time, to determine if there have been any changes to");
  puts("the file.");
  puts("");
  puts("Options :");
  puts(" -s  Calculate and store checksum\t-c   Check file against stored checksum");
  puts(" -v  Verbose.  Print more information\t-p   Display CRC64 checksum");
  puts(" -x  Remove stored CRC64 checksum\t-o   Overwrite existing checksum");
  puts(" -r  Recurse through directories\t-i   Import CRC from hidden file");
  puts(" -e  Export CRC to hidden file  \t-f    Read list if files from stdin");
}


int main(int argc, char *argv[])
{
  int optch;
  char *line = NULL;
  size_t size;
  ssize_t read;
  char *ptr;
   
  while ((optch = getopt(argc, argv,"hscvexirfop")) != -1)
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
      case 'i' :
        flags |= IMPORT;
	break;
      case 'e' :
	flags |= EXPORT;
	break;
      case 'f' :
	flags |= STDIN;
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
      puts("Cannot store and check CRC at same time.");
      return 1;
    }
    
   if ((flags & STORE) && (flags & REMOVE))
    {
      puts("Cannot remove and store CRC at same time.");
      return 1;
    }
    
    if ((flags & CHECK) && (flags & REMOVE))
    {
      puts("Cannot remove and check CRC at same time.");
      return 1;
    }
    
    if ((flags & EXPORT) && (flags & IMPORT))
    {
      puts("Cannot import and export at the same time.");
      return 1;
    }
    if (flags & STDIN)
    {
      while ((read = getline(&line, &size, stdin) != -1))
      {
	ptr = strrchr(line, '\n');
	if (ptr != NULL)
	  *ptr = 0;
	processFile(line);
      }
    free(line);
    }
    
  optch = optind;

  if (optch < argc)
    {
    do
    {
      processFile(argv[optch]);
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


