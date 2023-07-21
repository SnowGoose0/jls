#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
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

      /* Argument provided is (potentially) directory */
      else {
	if (dir_path_index >= dir_path_size) {
	  dir_path_size++;
	  dir_paths = (char**) realloc(dir_paths, dir_path_size * sizeof(char*));
	}

	dir_paths[dir_path_index] = argv[dir_path_index];
	dir_path_index++;
      }
    }
  }

  if (dir_path_index++ == 0) {
    dir_paths[0] = default_dir_path;
  }

  dir_list = (Directory*) malloc(dir_path_index * sizeof(Directory));

  for (int i = 0; i < dir_path_index && dir_paths[i] != NULL; ++i) {
    dir_list[i].dir_stream = NULL;
    dir_list[i].dir_entry = NULL;
    dir_list[i].files = NULL;
    dir_list[i].child_dir = NULL;

    dir_list[i].file_count = 0;
    dir_list[i].child_dir_count = 0;
    
    ls_dir(dir_list + i, dir_paths[i]);
  }

 EXIT:
  for (int i = 0; i < dir_path_index; i++) closedir(dir_list[i].dir_stream);

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

void ls_dir(Directory* dir, const char* path) {
  dir->base_path = path;
  dir->dir_stream = opendir(path);
  dir->files = (File*) calloc(1, sizeof(File));
  dir->child_dir = (Directory*) calloc(1, sizeof(Directory));
  
  if (dir->dir_stream == NULL) {
    perror("Error opening directory");
    /* TODO memory leak when failure */
    return;
  }
  
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

    f.date = cat_date(dir_meta.st_mtime);
    f.group = getgrgid(dir_meta.st_gid)->gr_name;
    f.user = getpwuid(dir_meta.st_uid)->pw_name;

    if (f.type == DT_DIR) {
      int c_idx = dir->child_dir_count;
      /* TODO: setup dir */
      dir->child_dir = (Directory*) realloc(dir->child_dir,
					    (++dir->child_dir_count + 1) * sizeof(Directory));
    }
    
    dir->files[idx] = f;
    printf("%s %s\n", f.name, f.date);
    dir->files = (File*) realloc(dir->files, (++dir->file_count + 1) * sizeof(File));
  }

}

void mem_free(void* ptr) {
  if (ptr != NULL) {
    free(ptr);
    ptr = NULL;
  }
}
