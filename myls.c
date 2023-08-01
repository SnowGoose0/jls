#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <time.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <glob.h>
#include <pwd.h>
#include <grp.h>

#include "myls.h"
#include "utils.h"

char* default_dir_path = ".";

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
	    fprintf(stderr, ERR_ODNE, *args);
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

  if (dir_path_index == 0) {
    dir_path_index = 1;
    dir_paths[0] = default_dir_path;
  }

  ssort(dir_paths, dir_path_index, sizeof(char*), s_comp);
  dir_list = (Directory*) malloc(dir_path_index * sizeof(Directory));

  /* Print files */
  int f_count = 0;
  
  for (int i = 0; i < dir_path_index && dir_paths[i] != NULL; ++i) {
    glob_t gl;

    if (glob(dir_paths[i], 0, NULL, &gl) == 0) {
      for (size_t j = 0; j < gl.gl_pathc; ++j) {
	struct stat path_stat;
	stat(gl.gl_pathv[j], &path_stat);
	
	if (S_ISREG(path_stat.st_mode)) {
	  ls_fdir(gl.gl_pathv[j]);
	  f_count++;
	}
      }
      
      globfree(&gl);
    }

    else {
      fprintf(stderr, ERR_FDNE, dir_paths[i]);
    }
  }

  /* Print directories */
  for (int i = 0; i < dir_path_index && dir_paths[i] != NULL; ++i) {
    char* ppath = dir_paths[i];
    glob_t gl;
    int res = glob(ppath, 0, NULL, &gl);

    if (res == 0) {
      for (size_t j = 0; j < gl.gl_pathc; ++j) {
	char* path = gl.gl_pathv[j];
	struct stat path_stat;
	stat(path, &path_stat);
	
	if (S_ISDIR(path_stat.st_mode)) {
	  if (dir_path_index != 1 || r_flag) {
	    if (i != 0 || f_count) putchar('\n');
	    printf("%s:\n", path);
	  }
	  
	  init_dir(dir_list + i);
	  ls_dir(dir_list + i, dir_paths[i]);
	}
      }
      
      globfree(&gl);
    }

    /* else if (res == GLOB_NOMATCH) { */
    /*   fprintf(stderr, ERR_FDNE, dir_paths[i]); */
    /* } */
  }

  mem_free(dir_paths);
  mem_free(dir_list);
  
  exit(EXIT_SUCCESS);
}

void ls_dir(Directory* dir, char* path) {
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
    Directory d;
    
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
      init_dir(&d);
      d.base_path = f.path;
      dir->child_dir[c_idx] = d;
      dir->child_dir = (Directory*) realloc(dir->child_dir,
					    (++dir->child_dir_count + 1) * sizeof(Directory));
    }
    
    dir->files[idx] = f;
    //printf("%s %s %s\n", dir->files[idx].perm, f.name, f.date);
    dir->files = (File*) realloc(dir->files, (++dir->file_count + 1) * sizeof(File));
  }

  ssort(dir->files, dir->file_count, sizeof(File), f_comp); /* Sort files in directories */
  ssort(dir->child_dir, dir->child_dir_count, sizeof(Directory), d_comp);

  print_dir(dir, i_flag, l_flag);

  if (r_flag && dir->child_dir_count != 0) {
    for (int i = 0; i < dir->child_dir_count; ++i) {
      printf("\n%s:\n", dir->child_dir[i].base_path);
      ls_dir(dir->child_dir + i, dir->child_dir[i].base_path);
    }
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
  size_t align_max[10] = {0,0,0,0,0,0,0,0,0,0};

  stat(fpath, &f_meta);
  f.name = fpath;
  f.ino = f_meta.st_ino;
  f.size = f_meta.st_size;
  f.hlinks = f_meta.st_nlink;
  f.date = cat_date(f_meta.st_mtime);
  f.perm = cat_perm(f_meta.st_mode);
  f.group = getgrgid(f_meta.st_gid)->gr_name;
  f.user = getpwuid(f_meta.st_uid)->pw_name;
  
  print_fdir(f, align_max, i_flag, l_flag);

  mem_free(f.date);
  mem_free(f.perm);
}
