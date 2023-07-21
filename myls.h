#ifndef MY_LS
#define MY_LS

#include <time.h>
#include <dirent.h>

#define ARG_DELIM                '-'
#define ARG_RECUR                'R'
#define ARG_LFORM                'l'
#define ARG_INDEX                'i'

char* month[12] = {"Jan", "Feb", "Mar", "Apr",
		  "May", "Jun", "Jul", "Aug",
		  "Sep", "Oct", "Nov", "Dec"};

typedef struct {
  int ino;
  int size;
  
  const char* name;
  const char* path;
  const char* perms;
  const char* group;
  const char* user;
  const char* date;

  unsigned char type;
} File;

typedef struct dir {
  DIR* dir_stream;
  struct dirent* dir_entry;
  char* base_path;

  int file_count;
  File* files; /* Directories will also be accounted as files */

  int child_dir_count;
  struct dir* child_dir;
} Directory;

char* cat_path(const char* base, const char* end);
char* cat_date(time_t t);
void ls_dir(Directory* dir, const char* path);
void mem_free(void* ptr);

#endif
