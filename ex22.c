//Naama Kashani 312400476
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/wait.h>

void extract_input(char *path, char lines[3][150]) {
    int fd;
    const int MAX_LINE_LENGTH = 150;
    char line[MAX_LINE_LENGTH];
    char ch;
    int i = 0, n = 0;

    fd = open(path, O_RDONLY);
    if (fd == -1) {
        exit(-1);
    }

    while (read(fd, &ch, 1) > 0 && n < 3) {
        if (ch == '\n') {
            line[i] = '\0';
            strcpy(lines[n], line);
            i = 0;
            n++;
        } else if (i < MAX_LINE_LENGTH - 1) {
            line[i] = ch;
            i++;
        }
    }

    if (n < 3 && i > 0) {
        line[i] = '\0';
        strcpy(lines[n], line);
        n++;
    }

    int close1 = close(fd);


}

int count_c_file(int error_fd, DIR *subdir_dir, char *file_to_compile) {
    char *closedir_error = "Error in: closedir\n";
    int count = 0;
    char *opendir_error = "Error in: opendir\n";
    if (subdir_dir == NULL) {
        write(error_fd, opendir_error, strlen(opendir_error));
        return -1;
    }
    struct dirent *subdir_entry;

    while ((subdir_entry = readdir(subdir_dir)) != NULL) {
        if (subdir_entry->d_type == DT_REG) {
            char *filename = subdir_entry->d_name;
            const char *suffix = ".c";
            size_t suffix_len = strlen(suffix);
            if (strlen(filename) >= suffix_len && !strcmp(filename + strlen(filename) - suffix_len, suffix)) {
                count++;
                strcat(file_to_compile, "/");
                strcat(file_to_compile, filename);
            }
        }
    }
    return count;
}

int compile_c_file(int error_fd, char *file_to_compile) {
    char *execlp_error = "Error in: execlp\n";
    dup2(error_fd, 2);
    execlp("gcc", "gcc", file_to_compile, "-o", "a.out", (char *) NULL);
    write(error_fd, execlp_error, strlen(execlp_error));
    exit(-1);

}

int create_output_file(int error_fd, char *output_file, char *new_path) {
    char *open_error = "Error in: open\n";
    strcat(output_file, new_path);
    strcat(output_file, "/output_file.txt");
    int output_fd;
    if ((output_fd = open(output_file, O_CREAT | O_TRUNC | O_WRONLY, 0644)) < 0) {
        write(error_fd, open_error, strlen(open_error));
    }
    return output_fd;
}

void run_c_file(int error_fd, int input_fd, int output_fd) {
    char *execlp_error = "Error in: execlp\n";
    dup2(input_fd, STDIN_FILENO);
    dup2(output_fd, STDOUT_FILENO);
    execlp("./a.out", "a.out", NULL);
    write(error_fd, execlp_error, strlen(execlp_error));
}

void run_compare(int error_fd, char *home_path, char *output_file, char *expected_output) {
    char *execvp_error = "Error in: execvp\n";
    char *chdir_error = "Error in: chdir\n";
    char *args[] = {"./comp.out", output_file, expected_output,
                    NULL};
    if (chdir(home_path) == -1) {
        write(error_fd, chdir_error, strlen(chdir_error));
        exit(-1);
    }
    execvp(args[0], args);
    write(error_fd, execvp_error, strlen(execvp_error));

}

void write_result(int result, int num, char *name) {
    switch (num) {
        case 1:
            strcat(name, ",100,EXCELLENT\n");
            write(result, name, strlen(name));
            break;
        case 2:
            strcat(name, ",50,WRONG\n");
            write(result, name, strlen(name));
            break;
        case 3:
            strcat(name, ",75,SIMILAR\n");
            write(result, name, strlen(name));
            break;
        default:
            break;
    }
}

void open_files(int *fd) {
    char *open_error = "Error in: open\n";
    int len_open_error = strlen(open_error);
    int result = open("results.csv", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    int error_fd = open("errors.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (error_fd < 0) {
        exit(-1);
    }
    if (result < 0) {
        write(error_fd, open_error, len_open_error);
        exit(-1);
    }

    fd[0] = result;
    fd[1] = error_fd;


}

void remove_files() {
    remove("a.out");
    remove("output_file.txt");

}

void new_iteration(int error_fd, char *path) {
    char *chdir_error = "Error in: chdir\n";

    remove_files();
    if (chdir(path) == -1) {
        write(error_fd, chdir_error, strlen(chdir_error));
        exit(-1);
    }
}

void close_all( DIR *dir, int fd1, int fd2) {
    char *closedir_error = "Error in: closedir\n";
    int close_dir = closedir(dir);
    if (close_dir == -1) {
        write(fd2, closedir_error, strlen(closedir_error));
        exit(-1);

    }
    int close1 = close(fd1);

    if (close1 == -1) {
        write(fd2, "Error in: close", strlen("Error in: close"));
        exit(-1);
    }
    int close2 = close(fd2);

}
void add_path(char* new_path, char * path, char*name){
    strcat(new_path, path);
    strcat(new_path, "/");
    strcat(new_path, name);
}

void compile_and_run_files(char *path, char *input_file, char *expected_output,int* fds,char* home_path) {
    struct dirent *entry;
    int status;
    char *open_error = "Error in: open\n";
    char *fork_error = "Error in: fork\n";
    char *chdir_error = "Error in: chdir\n";
    char *opendir_error = "Error in: opendir\n";
    char *closedir_error = "Error in: closedir\n";
    int len_open_error = strlen(open_error);
    if (chdir(path) == -1) {
        write(fds[1], chdir_error, strlen(chdir_error));
        exit(-1);
    }
    DIR *dir;
    dir = opendir(path);
    if (dir == NULL) {
        write(fds[1], opendir_error, strlen(opendir_error));
        exit(-1);
    }
    DIR *fix_dir = opendir(path);
    if (dir == NULL) {
        write(fds[1], opendir_error, strlen(opendir_error));
        exit(-1);
    }

    while ((entry = readdir(dir)) != NULL) {

        if (entry->d_type == DT_DIR && strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            char *name = entry->d_name;
            char new_path[1024] = "";
            add_path(new_path,path,name);
            if (chdir(new_path) == -1) {
                write(fds[1], chdir_error, strlen(chdir_error));
                new_iteration(fds[1],path);
                continue;
            }
            char file_to_compile[1024] = "";
            strcpy(file_to_compile, new_path);
            DIR *subdir_dir = opendir(".");
            if (subdir_dir == NULL) {
                write(fds[1], opendir_error, strlen(opendir_error));
                new_iteration(fds[1],path);
                continue;
            }
            int count = count_c_file(fds[1], subdir_dir, file_to_compile);
            if (count < 0) {
                new_iteration(fds[1],path);
                continue;
            }
            if (count == 0) {
                strcat(name, ",0,NO_C_FILE\n");
                write(fds[0], name, strlen(name));
            } else {
                pid_t first_pid = fork();
                if (first_pid == -1) {
                    write(fds[1], fork_error, len_open_error);
                    new_iteration(fds[1],path);
                    continue;
                } else if (first_pid == 0) {
                    int compile = compile_c_file(fds[1], file_to_compile);
                    new_iteration(fds[1],path);
                    continue;
                } else { // parent process
                    waitpid(first_pid, &status, 0);
                    if (WEXITSTATUS(status) != 0) {
                        char newname[1024];
                        strcpy(newname, name);
                        strcat(name, ",10,COMPILATION_ERROR\n");
                        write(fds[0], name, strlen(name));
                        struct dirent *fix_entry = readdir(fix_dir);
                        while (strcmp(fix_entry->d_name, newname) != 0) {
                            fix_entry = readdir(fix_dir);
                        }
                        dir = fix_dir;
                        continue;

                    } else {
                        char output_file[1024] = "";
                        int output_fd = create_output_file(fds[1],output_file, new_path);
                        if (output_fd < 0) {
                            new_iteration(fds[1],path);
                            continue;

                        }
                        int input_fd = open(input_file, O_RDONLY);
                        if (input_fd < 0) {
                            write(fds[1], open_error, len_open_error);
                            new_iteration(fds[1],path);
                            continue;
                        }
                        pid_t second_pid = fork();
                        if (second_pid == -1) {
                            write(fds[1], fork_error, len_open_error);
                            new_iteration(fds[1],path);
                            continue;
                        } else if (second_pid == 0) {
                            run_c_file(fds[1],input_fd, output_fd);
                            new_iteration(fds[1],path);
                            continue;
                        } else {
                            int close1 = close(input_fd);
                            int close2 = close(output_fd);
                            if (close1 == -1 || close2 == -1) {
                                write(fds[1], "Error in: close", strlen("Error in: close"));
                                new_iteration(fds[1],path);
                                continue;
                            }
                            sleep(5);
                            if (waitpid(second_pid, &status, WNOHANG) <= 0) {
                                strcat(name, ",20,TIMEOUT\n");
                                write(fds[0], name, strlen(name));

                            } else {
                                pid_t third_pid = fork();
                                if (third_pid == -1) {
                                    write(fds[1], fork_error, len_open_error);
                                    new_iteration(fds[1],path);
                                    continue;
                                } else if (third_pid == 0) {
                                    run_compare(fds[1],home_path, output_file, expected_output);
                                    new_iteration(fds[1],path);
                                    continue;
                                } else {
                                    waitpid(third_pid, &status, 0);
                                    write_result(fds[0], WEXITSTATUS(status), name);
                                }
                            }
                        }
                    }
                }
            }
            int close_subdir = closedir(subdir_dir);
            if (close_subdir == -1) {
                write(fds[1], closedir_error, strlen(closedir_error));

            }
            new_iteration(fds[1],path);
        }

    }
    close_all(dir, fds[0], fds[1]);
}

int main(int argc, char *argv[]) {
    int fds[2];
    open_files(fds);
    if (argc != 2) {
        exit -1;
    }
    char lines[3][150];
    char home_path[1024];
    char * cwd= getcwd(home_path, sizeof(home_path));
    if (cwd == NULL) {
        write(fds[1], "Error in: getcwd", strlen("Error in: getcwd"));
        exit(-1);
    }
    char students[1024]="",input[1024]="",output[1024]="";
    extract_input(argv[1], lines);
    add_path(students,home_path,lines[0]);
    add_path(input,home_path,lines[1]);
    add_path(output,home_path,lines[2]);

    if (access(students, F_OK) != 0) {
        printf("Not a valid directory\n");
        return -1;
    }
    if (access(input, F_OK) != 0) {
        printf("Input file not exist\n");
        return -1;
    }
    if (access(output, F_OK) != 0) {
        printf("Output file not exist\n");
        return -1;
    }
    compile_and_run_files(students,input,output,fds,home_path);
}
