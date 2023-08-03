#ifndef MY_LS
#define MY_LS

#include <time.h>
#include <dirent.h>

#define ARG_DELIM                '-'
#define ARG_RECUR                'R'
#define ARG_LFORM                'l'
#define ARG_INDEX                'i'

#define ERR_FDNE        "Error: cannot access '%s': No such file or directory\n"
#define ERR_ODNE        "Error: invalid option -- '%c'\n"

typedef struct {
  int ino;
  int hlinks;
  int size;
  
  const char* name;
  const char* group;
  const char* user;
  char* path;
  char* perm;
  char* date;
  char* slink;

  unsigned char type;
} File;

typedef struct dir {
  struct dirent* dir_entry;
  DIR* dir_stream;
  char* base_path;

  int file_count;
  File* files; /* Directories will also be accounted as files */

  int child_dir_count;
  struct dir* child_dir;
} Directory;

void ls_dir(Directory* dir, char* path);
void ls_fdir(const char* fpath);

#endif
