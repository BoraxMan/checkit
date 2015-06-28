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

#define RESET_TEXT()	printf("\033[0;0m")
#define VERSION "0.3.2"

enum validities
{
  VALID = 0,
  INVALID = 1
};

enum errorTypes
{
    ERROR_CRC_CALC,
    ERROR_REMOVE_XATTR,
    ERROR_STORE_CRC,
    ERROR_OPEN_DIR,
    ERROR_OPEN_FILE,
    ERROR_READ_FILE,
    ERROR_SET_CRC,
    ERROR_REMOVE_HIDDEN,
    ERROR_NO_XATTR,
    ERROR_NO_OVERWRITE,
    ERROR_WRITE_FILE,
    ERROR_FILENAME_OVERFLOW
};

enum characterAttributes
{
  RESET		= 0,
  BRIGHT	= 1,
  DIM		= 2,
  UNDERLINE	= 3,
  BLINK		= 4,
  REVERSE	= 7,
  HIDDEN	= 8
};

enum characterColours
{
  BLACK		= 0,
  RED		= 1,
  GREEN		= 2,
  YELLOW	= 3,
  BLUE		= 4,
  MAGENTA	= 5,
  CYAN		= 6
};

enum flags
{
  VERBOSE	= 0x01,
  STORE	   	= 0x02,
  CHECK		= 0x04,
  DISPLAY	= 0x08,
  REMOVE	= 0x10,
  RECURSE	= 0x20,
  OVERWRITE	= 0x40,
  PRINT		= 0x80,
  EXPORT	= 0x100,
  IMPORT	= 0x200,
  PIPEDFILES	= 0x400,
  SETCRCRO	= 0x800, /* Set CRC to be read only */
  SETCRCRW	= 0x1000 /* set CRC to be read write */
};

enum extendedAttributeTypes
{
  XATTR = 1,
  HIDDEN_ATTR = 2
};

enum checkitOptionsEnum
{
  UPDATEABLE	= 0x01,
  STATIC	= 0x02,
  OPT_ERROR	= 0x04
};


enum fsTypes {
VFAT = 1,
NTFS = 2,
UDF = 3,
XFS = 4,
JFS = 5,
NFS = 6,
SMB = 7,
CIFS = 8
};

static const int LIST_XATTR_BUFFER_SIZE =  2048; /* Statically allocated buffer. */

typedef unsigned long long t_crc64;

char* hiddenCRCFile(const char *file);
t_crc64 FileCRC64(const char *filename);
uint64_t crc64(uint64_t crc, const unsigned char *s, uint64_t l);
void textcolor(int attr, int fg, int bg);
t_crc64 getCRC(const char *filename);
int presentCRC64(const char *file);
int exportCRC(const char *filename, int flags);
int removeCRC(const char *filename);
int importCRC(const char *filename, int flags);
int putCRC(const char *file, int flags);
t_crc64 getCRC(const char *file);
int vfat_attr(char *file);
int ntfs_attr(char *file);
const char* errorMessage(int error);
int getfsType(const char *file);
