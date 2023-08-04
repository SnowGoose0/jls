#include <ctype.h>
#include <sys/stat.h>
#include <unistd.h>

#include "utils.h"

char* month[12] = {"Jan", "Feb", "Mar", "Apr",
		  "May", "Jun", "Jul", "Aug",
		  "Sep", "Oct", "Nov", "Dec"};

size_t* file_max_length(const File* files, size_t length) {
  size_t* ret_array = (size_t*) malloc(sizeof(size_t) * 9); // 9 fields in the struct

  size_t max_ino = 0;
  size_t max_hlinks = 0;
  size_t max_size = 0;
  size_t max_name = 0;
  size_t max_group = 0;
  size_t max_user = 0;
  size_t max_path = 0;
  size_t max_perm = 0;
  size_t max_date = 0;

  for (size_t i = 0; i < length; i++) {
    size_t len_ino = snprintf(NULL, 0, "%d", files[i].ino);
    size_t len_hlinks = snprintf(NULL, 0, "%d", files[i].hlinks);
    size_t len_size = snprintf(NULL, 0, "%d", files[i].size);
    size_t len_name = strlen(files[i].name);
    size_t len_group = strlen(files[i].group);
    size_t len_user = strlen(files[i].user);
    size_t len_path = strlen(files[i].path);
    size_t len_perm = strlen(files[i].perm);
    size_t len_date = strlen(files[i].date);

    if (len_ino > max_ino) max_ino = len_ino;
    if (len_hlinks > max_hlinks) max_hlinks = len_hlinks;
    if (len_size > max_size) max_size = len_size;
    if (len_name > max_name) max_name = len_name;
    if (len_group > max_group) max_group = len_group;
    if (len_user > max_user) max_user = len_user;
    if (len_path > max_path) max_path = len_path;
    if (len_perm > max_perm) max_perm = len_perm;
    if (len_date > max_date) max_date = len_date;
  }

  ret_array[0] = max_ino;
  ret_array[1] = max_hlinks;
  ret_array[2] = max_size;
  ret_array[3] = max_name;
  ret_array[4] = max_group;
  ret_array[5] = max_user;
  ret_array[6] = max_path;
  ret_array[7] = max_perm;
  ret_array[8] = max_date;

  return ret_array;
}

void print_blank(int n) {
  for (int i = 0; i < n; ++i) putchar(' ');
}


void print_dir(Directory* dir, int i_flag, int l_flag) {
  size_t* align_max = file_max_length(dir->files, dir->file_count);
  
  for (int i = 0; i < dir->file_count; i++) {
    print_fdir(dir->files[i], align_max, i_flag, l_flag);
  }

  free(align_max);
}

void print_fdir(File f, size_t* entry_max, int i_flag, int l_flag) { 
  if (i_flag) {
    print_blank(entry_max[INO] - snprintf(NULL, 0, "%d", f.ino));
    printf("%d ", f.ino);
  }

  if (l_flag) {
    printf("%s ", f.perm);
    
    print_blank(entry_max[HLINKS] - snprintf(NULL, 0, "%d", f.hlinks));    
    printf("%d ", f.hlinks);
    
    print_blank(entry_max[GROUP] - strlen(f.group));    
    printf("%s ", f.group);
    
    print_blank(entry_max[USER] - strlen(f.user));
    printf("%s ", f.user);
    
    print_blank(entry_max[SIZE] - snprintf(NULL, 0, "%d", f.size));
    printf("%d ", f.size);
    
    printf("%s ", f.date);
  }

  printf("%s", f.name);

  if (f.slink != NULL && l_flag)
    printf(" -> %s", f.slink);

  putchar('\n');
}

void init_dir(Directory* dir) {
  dir->dir_stream = NULL;  dir->dir_entry = NULL;
  dir->files = NULL;
  dir->child_dir = NULL;
  dir->file_count = 0;
  dir->child_dir_count = 0;
}

char* cat_path(const char* base, const char* end) {
  char* path;
  int base_len = (int) strlen(base);
  int end_len = (int) strlen(end);

  path = (char*) calloc(base_len + end_len + 2, sizeof(char));
  strncpy(path, base, base_len);

  char* cat_offset = path + base_len;
  
  if (path[base_len - 1] != '/') {
    cat_offset = path + base_len + 1;
    path[base_len] = '/';
  }
  
  strncpy(cat_offset, end, end_len);

  return path;
}

char* cat_slink(const char* path) {
  struct stat fstat;

  if (lstat(path, &fstat) == 0 && S_ISLNK(fstat.st_mode)) {
    char* slink;
    char slink_tmp[4096];
    ssize_t slink_len = 0;

    slink_len = readlink(path, slink_tmp, fstat.st_size);

    if (slink_len == -1) {
      return NULL;
    }

    slink = (char*) calloc(slink_len + 5, sizeof(char));
    memcpy(slink, slink_tmp, slink_len);

    return slink;
  }

  return NULL;
}

char* cat_date(time_t t) {
  char* t_format;
  struct tm tm_format;

  tm_format = *localtime(&t);
  t_format = (char*) calloc(50, sizeof(char));

  strftime(t_format, 50, "%b %d %Y %H:%M", &tm_format);

  if (tm_format.tm_mday < 10) {
    t_format[4] = ' ';
  }
  
  return t_format;
}

char* cat_perm(mode_t m) {
  char* m_format = (char*) calloc(11, sizeof(char));
  
  m_format[0] = S_ISDIR(m) ? 'd' : '-';

  if (m_format[0] == '-')
    m_format[0] = S_ISLNK(m) ? 'l' : '-';
  
  m_format[1] = (m & S_IRUSR) ? 'r' : '-';
  m_format[2] = (m & S_IWUSR) ? 'w' : '-';
  m_format[3] = (m & S_IXUSR) ? 'x' : '-';
  m_format[4] = (m & S_IRGRP) ? 'r' : '-';
  m_format[5] = (m & S_IWGRP) ? 'w' : '-';
  m_format[6] = (m & S_IXGRP) ? 'x' : '-';
  m_format[7] = (m & S_IROTH) ? 'r' : '-';
  m_format[8] = (m & S_IWOTH) ? 'w' : '-';
  m_format[9] = (m & S_IXOTH) ? 'x' : '-';
  
  return m_format;
}

int alphacmp(const char* s1, const char* s2, int type) {
  const char* ss1 = s1;
  const char* ss2 = s2;
  
  while (*s1 && *s2) {
    int char_diff;

    while (!isalnum(*s1) && *s1) s1++; 
    while (!isalnum(*s2) && *s2) s2++;

    if ((char_diff = tolower(*s1) - tolower(*s2)) != 0)
      return char_diff;

    s1++;
    s2++;
  }

  if (tolower(*s1) - tolower(*s2) == 0)
    return alphacasecmp(ss1, ss2, type);
  
  return type * (tolower(*s1) - tolower(*s2));
}

int alphacasecmp(const char* s1, const char* s2, int type) {  
  while (*s1 && *s2) {
    int diff;

    while (!isalnum(*s1) && *s1) s1++;
    while (!isalnum(*s2) && *s2) s2++;

    if ((diff = *s1 - *s2) != 0)
      return diff;

    s1++;
    s2++;
  }
  
  return type * (*s1 - *s2);
}

int f_comp(const void* fa, const void* fb) {
  char* fa_s = (char*) ((File*) fa)->name;
  char* fb_s = (char*) ((File*) fb)->name;
  
  return alphacmp(fa_s, fb_s, CMP_FIL_T);
}

int d_comp(const void* da, const void* db) {
  char* da_s = (char*) ((Directory*) da)->base_path;
  char* db_s = (char*) ((Directory*) db)->base_path;

  return alphacmp(da_s, db_s, CMP_DIR_T);
}

int s_comp(const void* sa, const void* sb) {
  return alphacmp(*((const char**) sa), *((const char**) sb), CMP_DIR_T);
}

void ssort(void* base, size_t count, size_t size, int (*compar)(const void*, const void*)) {
  if (count == 0) return;
  
  char* arr = (char*) base;
  char* temp = (char*) malloc(size);
  size_t min_idx;

  for (size_t i = 0; i < count - 1; ++i) {
    min_idx = i;
	
    for (size_t j = i + 1; j < count; ++j) {
      if (compar(&arr[j * size], &arr[min_idx * size]) < 0) {
	min_idx = j;
      }
    }

    if (min_idx != i) {
      memcpy(temp, &arr[i * size], size);
      memcpy(&arr[i * size], &arr[min_idx * size], size);
      memcpy(&arr[min_idx * size], temp, size);
    }
  }

  free(temp);
}

void mem_free(void* ptr) {
  if (ptr != NULL) {
    free(ptr);
    ptr = NULL;
  }
}

