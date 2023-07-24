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
  int hlinks;
  int size;
  
  const char* name;
  const char* group;
  const char* user;
  char* path;
  char* perm;
  char* date;

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

int f_comp(const void* fa, const void* fb);
int d_comp(const void* da, const void* db);
int s_comp(const void* sa, const void* sb); 

char* cat_path(const char* base, const char* end);
char* cat_date(time_t t);
char* cat_perm(mode_t m);

void ls_dir(Directory* dir, char* path);
void ls_fdir(const char* fpath);
void print_dir(File f);
void init_dir(Directory* dir);

void mem_free(void* ptr);

#endif
