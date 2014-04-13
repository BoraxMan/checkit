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

#define VERBOSE 0x01
#define STORE	0x02
#define CHECK	0x04
#define DISPLAY	0x08
#define REMOVE	0x10
#define RECURSE	0x20
#define OVERWRITE	0x40
#define PRINT	0x80
#define EXPORT  0x100
#define IMPORT  0x200
#define STDIN   0x400

#define VERSION "0.2.0"

#define XATTR 1
#define HIDDEN_ATTR 2

#define VFAT 1
#define NTFS 2

#define MAX_FILENAME_LENGTH 256



typedef unsigned long long t_crc64;

int processDir(char *directory, char *dir);
int processFile(char *filename);
t_crc64 FileCRC64(const char *filename);
uint64_t crc64(uint64_t crc, const unsigned char *s, uint64_t l);
void textcolor(int attr, int fg, int bg);
t_crc64 getCRC(const char *filename);
int presentCRC64(const char *file);
int exportCRC(const char *filename);
int removeCRC(const char *filename);
int importCRC(const char *filename);
int putCRC(const char *file);
t_crc64 getCRC(const char *file);
int vfat_attr(char *file);
int ntfs_attr(char *file);

