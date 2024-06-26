//Naama Kashani 312400476
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/wait.h>
#include <signal.h>

void close_all_fd(int fd1, int fd2) {

    int close1 = close(fd1);

    if (close1 == -1) {
        write(2, "Error in: close\n", strlen("Error in: close\n"));
        exit(-1);
    }
    int close2 = close(fd2);
    if (close2 == -1) {
        write(2, "Error in: close\n", strlen("Error in: close\n"));
        exit(-1);
    }

}

void extract_input(char *path, char lines[3][150], int *fds) {
    int fd;
    const int MAX_LINE_LENGTH = 150;
    char line[MAX_LINE_LENGTH];
    char ch;
    int i = 0, n = 0;

    fd = open(path, O_RDONLY);
    if (fd == -1) {
        close_all_fd(fds[0], fds[1]);
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
    if (close1 == -1) {
        write(2, "Error in: close\n", strlen("Error in: close\n"));
        close_all_fd(fds[0], fds[1]);
        exit(-1);
    }


}

int count_c_file(DIR *subdir_dir,char* file_name) {
    char *closedir_error = "Error in: closedir\n";
    int count = 0;
    char *opendir_error = "Error in: opendir\n";
    if (subdir_dir == NULL) {
        write(2, opendir_error, strlen(opendir_error));
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
                strcat(file_name, "/");
                strcat(file_name, filename);

            }
        }
    }
    return count;
}

int compile_c_file(char *file_to_compile, int *fds) {
    char *execlp_error = "Error in: execlp\n";
    if (dup2(fds[1], 2) == -1) {
        write(2, "Error in: dup2\n", strlen("Error in: dup2\n"));
        close_all_fd(fds[0], fds[1]);
        return -1;
    }

    execlp("gcc", "gcc", file_to_compile, "-o", "a.out", (char *) NULL);

    write(2, execlp_error, strlen(execlp_error));
    close_all_fd(fds[0], fds[1]);
    exit(-1);

}

int create_output_file(char *output_file, char *new_path) {
    char *open_error = "Error in: open\n";
    strcat(output_file, new_path);
    strcat(output_file, "/output_file.txt");
    int output_fd;
    if ((output_fd = open(output_file, O_CREAT | O_TRUNC | O_WRONLY, 0644)) < 0) {
        write(2 ,open_error, strlen(open_error));
    }
    return output_fd;
}

void run_c_file(int input_fd, int output_fd,int* fds) {
    char *execlp_error = "Error in: execlp\n";
    int temp;
    if (dup2(STDOUT_FILENO, temp) == -1) {

    }
    if (dup2(input_fd, STDIN_FILENO) == -1) {
        write(2, "Error in: dup2\n", strlen("Error in: dup2\n"));
        close_all_fd(fds[0], fds[1]);
        exit (-1);
    }
    if (dup2(output_fd, STDOUT_FILENO) == -1) {
        write(2, "Error in: dup2\n", strlen("Error in: dup2\n"));
        close_all_fd(fds[0], fds[1]);
        exit (-1);
    }
    if (dup2(fds[1], 2) == -1) {
        write(2, "Error in: dup2\n", strlen("Error in: dup2\n"));
        close_all_fd(fds[0], fds[1]);
        exit (-1);
    }

    execlp("./a.out", "a.out", NULL);
    write(temp, execlp_error, strlen(execlp_error));
}

void run_compare(char *home_path, char *output_file, char *expected_output, int *fds) {
    char *execvp_error = "Error in: execvp\n";
    char *chdir_error = "Error in: chdir\n";
    char *args[] = {"./comp.out", output_file, expected_output,
                    NULL};
    if (chdir(home_path) == -1) {
        write(2, chdir_error, strlen(chdir_error));
        close_all_fd(fds[0], fds[1]);
        exit(-1);
    }
    if (dup2(fds[1], 2) == -1) {
        write(2, "Error in: dup2\n", strlen("Error in: dup2\n"));
        close_all_fd(fds[0], fds[1]);
        exit (-1);
    }
    execvp(args[0], args);
    write(2, execvp_error, strlen(execvp_error));

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
void add_path(char *new_path, char *path, char *name) {
    strcat(new_path, path);
    strcat(new_path, "/");
    strcat(new_path, name);
}
void open_files(int *fd,char* home_path) {
    char *open_error = "Error in: open\n";
    int len_open_error = strlen(open_error);
    char error_file[1024], result_file[1024];
    add_path(error_file, home_path, "errors.txt");
    add_path(result_file, home_path, "results.csv");
    int result = open(result_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    int error_fd = open(error_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (error_fd < 0) {
        write(2, open_error, len_open_error);
        exit(-1);
    }
    if (result < 0) {
        write(2, open_error, len_open_error);
        exit(-1);
    }

    fd[0] = result;
    fd[1] = error_fd;


}

void remove_files() {
    remove("a.out");
    remove("output_file.txt");

}

void close_dir(DIR *dir) {
    char *closedir_error = "Error in: closedir\n";
    int close_dir = closedir(dir);
    if (close_dir == -1) {
        write(2, closedir_error, strlen(closedir_error));
        exit(-1);

    }
}

void new_iteration(char *path, int *fds, DIR *dir) {
    char *chdir_error = "Error in: chdir\n";
    remove_files();
    if (chdir(path) == -1) {
        write(2, chdir_error, strlen(chdir_error));
        close_all_fd(fds[0], fds[1]);
        close_dir(dir);
        exit(-1);
    }
}




void handle_alarm(int signum,pid_t pid,int* flag) {
    kill(pid, SIGTERM);
    flag[0]=1;

}
void handler(int sig) {
    if(sig == SIGALRM) {
        kill(getpid(), SIGTERM);
    }
}

void compile_and_run_files(char *path, char *input_file, char *expected_output, int *fds, char *home_path ,char * relative ) {
    struct dirent *entry;
    int status;
    char *open_error = "Error in: open\n";
    char *fork_error = "Error in: fork\n";
    char *chdir_error = "Error in: chdir\n";
    char *opendir_error = "Error in: opendir\n";
    char *closedir_error = "Error in: closedir\n";
    int len_open_error = strlen(open_error);
    if (chdir(path) == -1) {
        write(2, chdir_error, strlen(chdir_error));
        close_all_fd(fds[0], fds[1]);
        exit(-1);
    }
    DIR *dir;
    dir = opendir(path);
    if (dir == NULL) {
        write(2, opendir_error, strlen(opendir_error));
        close_all_fd(fds[0], fds[1]);
        exit(-1);
    }


    while ((entry = readdir(dir)) != NULL) {

        if (entry->d_type == DT_DIR && strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            char *name = entry->d_name;
            char new_path[1024] = "";
            add_path(new_path, path, name);
            if (chdir(new_path) == -1) {
                write(2, chdir_error, strlen(chdir_error));
                new_iteration(path, fds, dir);
                continue;
            }


            DIR *subdir_dir = opendir(".");
            if (subdir_dir == NULL) {
                write(2, opendir_error, strlen(opendir_error));
                close_dir(subdir_dir);
                new_iteration(path, fds, dir);
                continue;
            }
            char file_to_compile[1024]="";
            add_path(file_to_compile,relative,name);
            int count = count_c_file(subdir_dir,file_to_compile);
            if (chdir(home_path) == -1) {
                write(2, chdir_error, strlen(chdir_error));
                close_all_fd(fds[0], fds[1]);
                exit(-1);
            }
            if (count < 0) {
                close_dir(subdir_dir);
                new_iteration(path, fds, dir);
                continue;
            }
            if (count == 0) {
                strcat(name, ",0,NO_C_FILE\n");
                write(fds[0], name, strlen(name));
            } else {
                pid_t first_pid = fork();
                if (first_pid == -1) {
                    write(2, fork_error, len_open_error);
                    close_dir(subdir_dir);
                    new_iteration(path, fds, dir);
                    continue;
                } else if (first_pid == 0) {
                    compile_c_file(file_to_compile, fds);
                    close_dir(subdir_dir);
                    new_iteration(path, fds, dir);
                    continue;
                } else { // parent process
                    waitpid(first_pid, &status, 0);
                    if (WEXITSTATUS(status) != 0) {
                        char newname[1024];
                        strcpy(newname, name);
                        strcat(name, ",10,COMPILATION_ERROR\n");
                        write(fds[0], name, strlen(name));

                        DIR *fix_dir = opendir(path);
                        if (fix_dir == NULL) {
                            write(2, opendir_error, strlen(opendir_error));
                            close_all_fd(fds[0], fds[1]);
                            close_dir(dir);
                            exit(-1);
                        }
                        struct dirent *fix_entry = readdir(fix_dir);
                        while (strcmp(fix_entry->d_name, newname) != 0) {
                            fix_entry = readdir(fix_dir);
                        }
                        dir = fix_dir;
                        close_dir(subdir_dir);
                        continue;

                    } else {
                        if (chdir(home_path) == -1) {
                            write(2, chdir_error, strlen(chdir_error));
                            close_all_fd(fds[0], fds[1]);
                            exit(-1);
                        }
                        char output_file[1024] = "";
                        int output_fd = create_output_file(output_file, new_path);
                        if (output_fd < 0) {
                            close_dir(subdir_dir);
                            new_iteration(path, fds, dir);
                            continue;

                        }
                        int input_fd = open(input_file, O_RDONLY);
                        if (input_fd < 0) {
                            write(2, open_error, len_open_error);
                            close_dir(subdir_dir);
                            new_iteration(path, fds, dir);
                            continue;
                        }

                        signal(SIGALRM, (void (*)(int))handle_alarm);
                        pid_t second_pid = fork();
                        if (second_pid == -1) {
                            write(2, fork_error, len_open_error);
                            close_dir(subdir_dir);
                            new_iteration(path, fds, dir);
                            continue;
                        } else if (second_pid == 0) {
                            alarm(5);
                            run_c_file(input_fd, output_fd,fds);
                            close_dir(subdir_dir);
                            new_iteration(path, fds, dir);
                            continue;
                        } else {
                            int close1 = close(input_fd);
                            int close2 = close(output_fd);
                            if (close1 == -1 || close2 == -1) {
                                write(2, "Error in: close\n", strlen("Error in: close\n"));
                                close_dir(subdir_dir);
                                new_iteration(path, fds, dir);
                                continue;
                            }

                            waitpid(second_pid, &status, 0);
                            alarm(0);

                            if (!(WIFEXITED(status))) {
                                strcat(name, ",20,TIMEOUT\n");
                                write(fds[0], name, strlen(name));
                                close_dir(subdir_dir);
                                new_iteration(path, fds, dir);
                                continue;

                            } else {
                                pid_t third_pid = fork();
                                if (third_pid == -1) {
                                    write(2, fork_error, len_open_error);
                                    close_dir(subdir_dir);
                                    new_iteration(path, fds, dir);
                                    continue;
                                } else if (third_pid == 0) {
                                    run_compare(home_path, output_file, expected_output, fds);
                                    close_dir(subdir_dir);
                                    new_iteration(path, fds, dir);
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
            close_dir(subdir_dir);
            new_iteration(path, fds, dir);
        }

    }
    close_all_fd(fds[0], fds[1]);
    close_dir(dir);

}

int main(int argc, char *argv[]) {
    int fds[2];

    if (argc != 2) {
        exit(-1);
    }
    char lines[3][150];
    char home_path[1024];
    char *cwd = getcwd(home_path, sizeof(home_path));
    if (cwd == NULL) {
        write(2, "Error in: getcwd\n", strlen("Error in: getcwd\n"));
        close_all_fd(fds[0], fds[1]);
        exit(-1);
    }
    open_files(fds,home_path);
    char students[1024] = "", input[1024] = "", output[1024] = "";
    extract_input(argv[1], lines, fds);
    if (lines[0][0] != '/') {
        add_path(students, home_path, lines[0]);
    } else {
        strcpy(students, lines[0]);
    }
    if (lines[1][0] != '/') {
        add_path(input, home_path, lines[1]);
    } else {
        strcpy(input, lines[1]);
    }
    if (lines[2][0] != '/') {
        add_path(output, home_path, lines[2]);
    } else {
        strcpy(output, lines[2]);
    }


    if (access(students, F_OK) != 0) {
        write(STDOUT_FILENO, "Not a valid directory\n", strlen("Not a valid directory\n"));
        close_all_fd(fds[0], fds[1]);
        return -1;
    }
    if (access(input, F_OK) != 0) {
        write(STDOUT_FILENO, "Input file not exist\n", strlen("Input file not exist\n"));
        close_all_fd(fds[0], fds[1]);
        return -1;
    }
    if (access(output, F_OK) != 0) {
        write(STDOUT_FILENO, "Output file not exist\n", strlen("Output file not exist\n"));
        close_all_fd(fds[0], fds[1]);
        return -1;
    }
    compile_and_run_files(students, input, output, fds, home_path,lines[0]);
}
