#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"
#include "kernel/param.h"
// curr_path   : current directory path being searched
// target_name : name of the file to look for
// exec_cmd    : array of strings representing the command to execute with -exec option

// dir_fd      : file descriptor for the currently opened directory
// st_info     : struct stat holding information about a file or directory
// path_buf    : buffer holding the full path of the current entry being processed
// buf_ptr     : pointer to the position in path_buf where the next entry name is appended
// entry       : struct dirent representing a directory entry read from the directory
// new_path    : buffer holding the path for recursive calls when entering subdirectories
// exec_pos    : index of "-exec" argument in argv, or -1 if not present
// exec_args   : array of strings containing the command and its arguments for -exec
// pid         : process ID returned by fork()
// i, n       : counters used for iterating through arguments or arrays
// len         : length of the current path, used when constructing new_path
// curr_path   : current directory path being searched
// target_name : name of the file to look for
// exec_cmd    : array of strings representing the command to execute with -exec option

// dir_fd      : file descriptor for the currently opened directory
// st_info     : struct stat holding information about a file or directory
// path_buf    : buffer holding the full path of the current entry being processed
// buf_ptr     : pointer to the position in path_buf where the next entry name is appended
// entry       : struct dirent representing a directory entry read from the directory
// new_path    : buffer holding the path for recursive calls when entering subdirectories
// exec_pos    : index of "-exec" argument in argv, or -1 if not present
// exec_args   : array of strings containing the command and its arguments for -exec
// pid         : process ID returned by fork()
// i, n       : counters used for iterating through arguments or arrays
// len         : length of the current path, used when constructing new_path

int matchhere(char*, char*);
int matchstar(int, char*, char*);

int
match(char *re, char *text)
{
  if(re[0] == '^')
    return matchhere(re+1, text);
  do{  // must look at empty string
    if(matchhere(re, text))
      return 1;
  }while(*text++ != '\0');
  return 0;
}

// matchhere: search for re at beginning of text
int matchhere(char *re, char *text)
{
  if(re[0] == '\0')
    return 1;
  if(re[1] == '*')
    return matchstar(re[0], re+2, text);
  if(re[0] == '$' && re[1] == '\0')
    return *text == '\0';
  if(*text!='\0' && (re[0]=='.' || re[0]==*text))
    return matchhere(re+1, text+1);
  return 0;
}

// matchstar: search for c*re at beginning of text
int matchstar(int c, char *re, char *text)
{
  do{  // a * matches zero or more instances
    if(matchhere(re, text))
      return 1;
  }while(*text!='\0' && (*text++==c || c=='.'));
  return 0;
}

void find_files(char *curr_path, char *target_name, char **exec_cmd) {
    int dir_fd;
    struct stat st_info;

    if((dir_fd = open(curr_path, O_RDONLY)) < 0){
        fprintf(2, "find: cannot open %s\n", curr_path);
        return;
    }
    if(fstat(dir_fd, &st_info) < 0){
        fprintf(2, "find: cannot stat %s\n", curr_path);
        close(dir_fd);
        return;
    }
    if(st_info.type != T_DIR){
        fprintf(2, "find: %s is not a directory\n", curr_path);
        close(dir_fd);
        return;
    }

    char path_buf[512], *buf_ptr;
    struct dirent entry;

    strcpy(path_buf, curr_path);
    buf_ptr = path_buf + strlen(path_buf);
    *buf_ptr++ = '/';

    while(read(dir_fd, &entry, sizeof(entry)) == sizeof(entry)){
        if(entry.inum == 0)
            continue;
        memmove(buf_ptr, entry.name, DIRSIZ);
        buf_ptr[DIRSIZ] = 0;

        if(stat(path_buf, &st_info) < 0){
            fprintf(2, "\nError! find: cannot stat %s\n", path_buf);
            continue;
        }

        if(st_info.type == T_FILE){
            if(match(target_name, entry.name)) {
                if(exec_cmd == 0){
                    printf("%s\n", path_buf);
                } else {
                    int pid = fork();
                    if(pid == 0){
                       char *args[MAXARG];
                        int i = 0;
                        while(exec_cmd[i] != 0){
                            args[i] = exec_cmd[i];
                            i++;
                        }
                        args[i] = path_buf;
                        args[i+1] = 0;
                        exec(args[0], args);
                        fprintf(2, "\nError! find: exec %s failed\n", args[0]);
                        exit(1);
                    } else {
                        wait(0);
                    }
                }
            }
        } else if(st_info.type == T_DIR && strcmp(entry.name, ".") != 0 && strcmp(entry.name, "..") != 0){
            char new_path[512];
            int len = strlen(curr_path);
            memmove(new_path, curr_path, len);
            new_path[len] = '/';
            memmove(new_path + len + 1, entry.name, DIRSIZ);
            new_path[len + 1 + DIRSIZ] = 0;

            find_files(new_path, target_name, exec_cmd);
        }
    }
    close(dir_fd);
}

int main(int argc, char *argv[]) {
    if(argc < 3){
        fprintf(2, "\nError! Usage: find <path> <filename> <optional -exec command ...>\n");
        exit(1);
    }
    int exec_pos = -1;
    for(int i = 3; i < argc; i++){
        if(strcmp(argv[i], "-exec") == 0){
            exec_pos = i;
            break;
        }
    }
    if(exec_pos == -1){
        find_files(argv[1], argv[2], 0);
    } else {
        char *exec_args[MAXARG];
        int n = 0;
        for(int i = exec_pos + 1; i < argc; i++){
            exec_args[n++] = argv[i];
        }
        exec_args[n] = 0;
        find_files(argv[1], argv[2], exec_args);
    }
    exit(0);
}

