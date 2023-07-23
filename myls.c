#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <glob.h>
#include <pwd.h>
#include <grp.h>

#include "myls.h"

const char* default_dir_path = ".";

static int r_flag, i_flag, l_flag;

int main(int argc, char** argv) {
  int dir_path_index, dir_path_size;
  char** dir_paths = NULL;
  Directory* dir_list = NULL;
    
  r_flag = 0;
  i_flag = 0;
  l_flag = 0;

  dir_paths = (char**) calloc(10, sizeof(char*));
  dir_path_index = 0;
  dir_path_size = 10;

  /* Parse commands to check options */
  if (argc > 1) {
    for (int i = 1; i < argc; i++) {
      char* args = *(argv + i);

      /* Argument provided is option */
      if (*args == ARG_DELIM) {
	while(*(++args) != 0) {
	  if (*args == ARG_RECUR) {
	    r_flag = 1;
	  }

	  else if (*args == ARG_INDEX) {
	    i_flag = 1;
	  }

	  else if (*args == ARG_LFORM) {
	    l_flag = 1;
	  }

	  else {
	    fprintf(stderr, "Error: unrecognized argument: %c\n", *args);
	    exit(EXIT_FAILURE);
	  }
	}
      }

      /* Argument provided is (potentially) a directory */
      else {
	if (dir_path_index >= dir_path_size) {
	  dir_path_size++;
	  dir_paths = (char**) realloc(dir_paths, dir_path_size * sizeof(char*));
	}

	dir_paths[dir_path_index] = argv[i];
	dir_path_index++;
      }
    }
  }

  if (dir_path_index++ == 0) {
    dir_paths[0] = default_dir_path;
  }

  dir_list = (Directory*) malloc(dir_path_index * sizeof(Directory));

  for (int i = 0; i < dir_path_index && dir_paths[i] != NULL; ++i) {
    /* char* path = dir_paths[i]; */
    /* struct stat path_stat; */
    /* stat(path, &path_stat); */
    


    char* ppath = dir_paths[i];
    glob_t gl;
    int res = glob(ppath, 0, NULL, &gl);

    if (res == 0) {
      for (size_t j = 0; j < gl.gl_pathc; ++j) {
	char* path = gl.gl_pathv[j];
	struct stat path_stat;
	stat(path, &path_stat);
	if (S_ISDIR(path_stat.st_mode)) {
	  dir_list[i].dir_stream = NULL;
	  dir_list[i].dir_entry = NULL;
	  dir_list[i].files = NULL;
	  dir_list[i].child_dir = NULL;

	  dir_list[i].file_count = 0;
	  dir_list[i].child_dir_count = 0;
    
	  ls_dir(dir_list + i, dir_paths[i]);
	}

	else {
	  ls_fdir(path);
	}
      }
      
      globfree(&gl);
    }

    else if (res == GLOB_NOMATCH) {
      
    }

  }

 EXIT:
  mem_free(dir_paths);
  mem_free(dir_list);
  
  exit(EXIT_SUCCESS);
}




char* cat_path(const char* base, const char* end) {
  char* path;
  int base_len = (int) strlen(base);
  int end_len = (int) strlen(end);

  path = (char*) calloc(base_len + end_len + 2, sizeof(char));
  strncpy(path, base, base_len);
  strncpy(path + base_len + 1, end, end_len);
  path[base_len] = '/';

  return path;
}

char* cat_date(time_t t) {
  char* t_format;
  struct tm tm_format = *localtime(&t);

  t_format = (char*) calloc(30, sizeof(char));

  char* spc_d = "";
  char* spc_h = "";
  char* spc_m = "";

  if (tm_format.tm_mday < 10) spc_d = " ";
  if (tm_format.tm_hour < 10) spc_h = "0";
  if (tm_format.tm_min < 10) spc_m = "0";
  
  sprintf(t_format, "%s %s%d %s%d:%s%d %d",
	  month[tm_format.tm_mon],
	  spc_d, tm_format.tm_mday,
	  spc_h, tm_format.tm_hour,
	  spc_m, tm_format.tm_min,
	  1900 + tm_format.tm_year);
  
  return t_format;
}

char* cat_perm(mode_t m) {
  char* m_format = (char*) calloc(11, sizeof(char));
  
  m_format[0] = S_ISDIR(m) ? 'd' : '-';
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

int f_comp(const void* fa, const void* fb) {
  char* fa_s = (char*) ((File*) fa)->name;
  char* fb_s = (char*) ((File*) fb)->name;
  return strcmp(fa_s, fb_s);
}

int d_comp(const void* da, const void* db) {
  
}

void ls_dir(Directory* dir, const char* path) {
  printf("%s\n", path);
  dir->base_path = path;
  dir->dir_stream = opendir(path);

  if (dir->dir_stream == NULL) {
        
    perror("Error opening directory");
    /* TODO memory leak when failure */
    return;
  }
    
  dir->files = (File*) calloc(1, sizeof(File));
  dir->child_dir = (Directory*) calloc(1, sizeof(Directory));
 
  while ((dir->dir_entry = readdir(dir->dir_stream)) != NULL) {
    int idx = dir->file_count;
    File f;
    
    struct dirent* dir_ptr = dir->dir_entry;
    struct stat dir_meta;

    if ('.' == dir_ptr->d_name[0]) {
      continue; /* Skip hidden files */
    }

    f.name = dir_ptr->d_name;
    f.path = cat_path(path, f.name);
    f.ino = dir_ptr->d_ino;
    f.type = dir_ptr->d_type;

    stat(f.path, &dir_meta);

    f.size = dir_meta.st_size;
    f.hlinks = dir_meta.st_nlink;
    f.date = cat_date(dir_meta.st_mtime);
    f.perm = cat_perm(dir_meta.st_mode);
    f.group = getgrgid(dir_meta.st_gid)->gr_name;
    f.user = getpwuid(dir_meta.st_uid)->pw_name;

    if (f.type == DT_DIR) {
      int c_idx = dir->child_dir_count;
      /* TODO: setup dir */
      dir->child_dir = (Directory*) realloc(dir->child_dir,
					    (++dir->child_dir_count + 1) * sizeof(Directory));
    }
    
    dir->files[idx] = f;
    //printf("%s %s %s\n", dir->files[idx].perm, f.name, f.date);
    dir->files = (File*) realloc(dir->files, (++dir->file_count + 1) * sizeof(File));
  }

  qsort(dir->files, dir->file_count, sizeof(File), f_comp); /* Sort files in directories */

  for (int i = 0; i < dir->file_count; ++i) {
    print_dir(dir->files[i]);
  }

  if (r_flag && dir->child_dir_count != 0) {
    
  }

  /* Mem management */

  mem_free(dir->child_dir);

  for (int i = 0; i < dir->file_count; ++i) {
    mem_free(dir->files[i].date);
    mem_free(dir->files[i].path);
    mem_free(dir->files[i].perm);
  }

  closedir(dir->dir_stream);
  mem_free(dir->files);
}

void ls_fdir(const char* fpath) {
  File f;
  struct stat f_meta;

  stat(fpath, &f_meta);
  f.name = fpath;
  f.ino = f_meta.st_ino;
  f.size = f_meta.st_size;
  f.hlinks = f_meta.st_nlink;
  f.date = cat_date(f_meta.st_mtime);
  f.perm = cat_perm(f_meta.st_mode);
  f.group = getgrgid(f_meta.st_gid)->gr_name;
  f.user = getpwuid(f_meta.st_uid)->pw_name;
  
  print_dir(f);
}

void print_dir(File f) {
  if (i_flag) {
    printf("%d ", f.ino);
  }

  if (l_flag) {
    printf("%s %d %s %s %d %s ",
	   f.perm, f.hlinks, f.group, f.user, f.size, f.date);
  }

  printf("%s\n", f.name);    
}

void mem_free(void* ptr) {
  if (ptr != NULL) {
    free(ptr);
    ptr = NULL;
  }
}
