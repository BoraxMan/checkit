#define RESET_TEXT()	printf("\033[0;0m")
#define VERSION "0.2.1"

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
    ERROR_WRITE_FILE
};

enum attributes
{
  RESET		= 0,
  BRIGHT	= 1,
  DIM		= 2,
  UNDERLINE	= 3,
  BLINK		= 4,
  REVERSE	= 7,
  HIDDEN	= 8
};

enum colours
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
  PIPEDFILES	= 0x400
};

enum extendedAttributeTypes
{
  XATTR = 1,
  HIDDEN_ATTR = 2
};

enum fsTypes {
VFAT = 1,
NTFS = 2,
UDF = 3
};


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
