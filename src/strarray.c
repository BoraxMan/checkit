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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "strarray.h"
#include "checkit.h"

const int chunkSize = 4096;

int initFileList(fileList *list)
{
  list->files = malloc(chunkSize);
  if (list->files == NULL)
  {
    return ERROR_NO_MEM;
  }
  list->freeSpace = chunkSize;
  list->size = chunkSize;
  list->ptr = list->files;
  return SUCCESS;
}

int appendFileList(fileList *list, const char *basename, const char *filename)
{
  int strlenBasename = strlen(basename);
  int strSize = (strlenBasename + strlen(filename) + 1); /* +1 for the "\n" we add to the end. */

  if (list->files == NULL)
    return ERROR_NO_MEM;
  
  if (list->freeSpace <= (strSize + 4))
  {
    
    /* Save ptr offset */
    size_t offset = list->ptr - list->files;
    
    list->files = realloc(list->files, list->size + chunkSize);
    if (list->files == NULL)
    {
      return ERROR_NO_MEM;
    }
    
    list->size+=chunkSize;
    list->freeSpace+=chunkSize;
    /* Restore ptr offset 
     The ptr offset avoid having to scan the entire string
     for the terminating null on each call of strlen */
    list->ptr = list->files;
    list->ptr += offset;
  }
  if (strlenBasename)
  {
    strcat(list->ptr, basename);
  }
  strcat(list->ptr, filename);
  strcat(list->ptr, "\n");
  list->ptr+=strSize;
  list->freeSpace-= (strSize + strlenBasename - 1);

  return SUCCESS;
}

void freeFileList(fileList *list)
{
  list->ptr = NULL;
  list->freeSpace = 0;
  list->size = 0;
  if (list->files != NULL)
    free(list->files);
  list->files = NULL;
}

const char *getFileList(const fileList *list)
{
  if (list->files != NULL)
    return list->files;
  else
    return NULL;
}
