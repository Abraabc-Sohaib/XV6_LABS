#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"
#include "kernel/param.h"

// Recursive exec-find function
void exec_files(char *curr_path, char *target_name, char **exec_cmd) {
    int dir_fd;
    struct stat st_info;
    char path_buf[512], *buf_ptr;
    struct dirent entry;

    if((dir_fd = open(curr_path, O_RDONLY)) < 0){
        fprintf(2, "exec: cannot open %s\n", curr_path);
        return;
    }
    if(fstat(dir_fd, &st_info) < 0){
        fprintf(2, "exec: cannot stat %s\n", curr_path);
        close(dir_fd);
        return;
    }
    if(st_info.type != T_DIR){
        fprintf(2, "exec: %s is not a directory\n", curr_path);
        close(dir_fd);
        return;
    }

    strcpy(path_buf, curr_path);
    buf_ptr = path_buf + strlen(path_buf);
    *buf_ptr++ = '/';

    while(read(dir_fd, &entry, sizeof(entry)) == sizeof(entry)){
        if(entry.inum == 0)
            continue;

        memmove(buf_ptr, entry.name, DIRSIZ);
        buf_ptr[DIRSIZ] = 0;

        if(stat(path_buf, &st_info) < 0){
            fprintf(2, "exec: cannot stat %s\n", path_buf);
            continue;
        }

        if(st_info.type == T_FILE){
            if(strcmp(entry.name, target_name) == 0){
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
                    fprintf(2, "exec: exec %s failed\n", args[0]);
                    exit(1);
                } else {
                    wait(0);
                }
            }
        } else if(st_info.type == T_DIR &&
                  strcmp(entry.name, ".") != 0 &&
                  strcmp(entry.name, "..") != 0){
            char new_path[512];
            int len = strlen(curr_path);
            memmove(new_path, curr_path, len);
            new_path[len] = '/';
            memmove(new_path + len + 1, entry.name, DIRSIZ);
            new_path[len + 1 + DIRSIZ] = 0;

            exec_files(new_path, target_name, exec_cmd);
        }
    }
    close(dir_fd);
}

int main(int argc, char *argv[]) {
    if(argc < 4){
        fprintf(2, "Usage: exec <path> <filename> <command> [args ...]\n");
        exit(1);
    }

    char *exec_cmd[MAXARG];
    int n = 0;
    for(int i = 3; i < argc; i++){
        exec_cmd[n++] = argv[i];
    }
    exec_cmd[n] = 0;

    exec_files(argv[1], argv[2], exec_cmd);

    exit(0);
}

