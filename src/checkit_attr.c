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

/* Set user customized option for files */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <attr/xattr.h>
#include <stdint.h>

#include "checkit.h"
#include "fsmagic.h"
#include "checkit_attr.h"

extern int failed;
extern int processed;
const char* checkitOptionsName = "user.checkit";

int setCheckitOptions(const char *file, char checkitOptions)
{
  int fstype;
  fstype = getfsType(file);

  if(fstype != VFAT && fstype != UDF && fstype != NFS)
  { /* If not VFAT or UDF, attempt to store attribute/option in extended attribute */
    if ((setxattr(file, checkitOptionsName, (const char *)&checkitOptions, sizeof(checkitOptions), 0)) == -1)
      return ERROR_SET_CRC;
    else
      return 0; /* And we're done here, return to process next file */
  }
  else
  {
    return ERROR_SET_CRC;
  }
  return 0;
}

char getCheckitOptions(const char *file)
{ 
  char buf[LIST_XATTR_BUFFER_SIZE];
  char *current_attr;
  int x;
  char checkitOptions;
  int fstype;
  fstype = getfsType(file);
    

  if(fstype != VFAT && fstype != UDF && fstype != NFS)
  { /* If not VFAT or UDF, attempt to read attribute/option in extended attribute */
    
    x = listxattr(file,buf,LIST_XATTR_BUFFER_SIZE);
    
    current_attr = buf;    
    if (x != -1)
    {
      do
      {
	if (strcmp(current_attr, checkitOptionsName) == 0)
	{
	  if ((getxattr(file, checkitOptionsName, (char *)&checkitOptions, sizeof(checkitOptions)) == -1))
	  {
	    checkitOptions = OPT_ERROR;
	  }
	  break;
	}
	else
	  current_attr += (strlen(current_attr) + 1);
      } while ((current_attr - buf) < x);
    }
  }
  else
  {
    checkitOptions = STATIC; /* If its VFAT/UDF, don't update.  We don't support the option for these filesystems, yet...*/
  }
  return checkitOptions; 
}

int removeCheckitOptions(const char *file)
{
  char buf[LIST_XATTR_BUFFER_SIZE];
  int x;
  int fstype;
  char *current_attr;
  fstype = getfsType(file);
    
  if(fstype != VFAT && fstype != UDF && fstype != NFS)
  { /* If not VFAT or UDF, attempt to read attribute/option in extended attribute */
    
    x = listxattr(file,buf,LIST_XATTR_BUFFER_SIZE);
    
    current_attr = buf;    
    if (x != -1)
    {
      do
      {
	if (strcmp(current_attr, checkitOptionsName) == 0)
	{
	  if ((removexattr(file, checkitOptionsName)) == -1)
	  {
	    return ERROR_REMOVE_XATTR;
	  }
	break;
	}
	else
	  current_attr += (strlen(current_attr) + 1);
      } while ((current_attr - buf) < x);
    }
  }
  return 0;
}

