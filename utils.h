#ifndef UTILS
#define UTILS

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "myls.h"
#include <sys/stat.h>
#include <sys/types.h>

#define CMP_DIR_T  1
#define CMP_FIL_T  1

typedef enum {
  INO = 0,
  HLINKS = 1,
  SIZE = 2,
  NAME = 3,
  GROUP = 4,
  USER = 5,
  PATH = 6,
  PERM = 7,
  DATE = 8,
  TYPE = 9
} FileField;

size_t* file_max_length(const File* files, size_t length);
void print_blank(int n);
void print_dir(Directory* dir, int i_flag, int j_flag);
void print_fdir(File f, size_t* entry_max, int i_flag, int j_flag);
void init_dir(Directory* dir);

int alphacmp(const char* s1, const char* s2, int type);
int alphacasecmp(const char* s1, const char* s2, int type);
int f_comp(const void* fa, const void* fb);
int d_comp(const void* da, const void* db);
int s_comp(const void* sa, const void* sb);
void ssort(void* base, size_t count, size_t size, int (*compar)(const void*, const void*));

char* cat_path(const char* base, const char* end);
char* cat_date(time_t t);
char* cat_perm(mode_t m);

void mem_free(void* ptr);

#endif
