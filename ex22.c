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
        printf("Not a valid configuration file\n");
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

    close(fd);


}

void compile_and_run_files(char *path, char *input_file, char *expected_output) {
    pid_t wpid;
    int result = open("result.csv", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (result <0){
        printf("Error in: open");
        exit(-1);
    }
    int error_fd = open("error_file.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (error_fd <0){
        printf("Error in: open");
        exit(-1);
    }
    int input_fd = open(input_file, O_RDONLY);
    if (input_fd <0){
        printf("Error in: open");
        exit(-1);
    }
    struct dirent *entry;
    int status;
    if(chdir(path)==-1){
        printf("Error in: chdir");
        exit(-1);
    }
    DIR *dir;
    dir = opendir(path);
    if (dir == NULL) {
        printf("Error in: opendir");
        exit(-1);
    }

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_DIR && strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            char *name = entry->d_name;
            char new_path[1024] = "";
            strcat(new_path, path);
            strcat(new_path, "/");
            strcat(new_path, name);
            if(chdir(new_path)==-1){
                printf("Error in: chdir");
                continue;
            }

            int count = 0;
            DIR *subdir_dir = opendir(".");
            if (subdir_dir == NULL) {
                perror("Error in: opendir");
                continue;
            }
            char file_to_compile[1024] ="";
            strcpy(file_to_compile,new_path);
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
            if (count == 1) {
                pid_t first_pid = fork();
                if (first_pid == -1) {
                    printf("Error in: fork");
                    continue;
                } else if (first_pid == 0) {
                    dup2(error_fd, 2);
                    execlp("gcc", "gcc", file_to_compile, "-o", "a.out", (char *) NULL);

                } else { // parent process

                    waitpid(first_pid, &status, 0);
                    if (WEXITSTATUS(status) != 0) {
                        strcat(name, ",10,COMPILATION_ERROR\n");
                        write(result, name, strlen(name));
                        name = "";
                    } else {
                        char output_file[1024]="";
                        strcat(output_file,new_path);
                        strcat(output_file,"/output_file.txt");
                        int output_fd;
                        if ((output_fd = open(output_file, O_CREAT|O_TRUNC|O_WRONLY, 0644)) < 0){
                            perror("Error in: open");
                            continue;
                        }
                        pid_t second_pid = fork();
                        if (second_pid == -1) {
                            printf("Error in: fork");
                            continue;
                        } else if (second_pid == 0) {
                            dup2(input_fd, STDIN_FILENO);
                            dup2(output_fd, STDOUT_FILENO);
                            execlp("./a.out", "a.out", NULL);
                        } else {
                            sleep(5);
                            if (waitpid(second_pid, &status, WNOHANG) <= 0) {
                                strcat(name, ",20,TIMEOUT\n");
                                write(result, name, strlen(name));
                                name = "";

                            } else {
                                close(output_fd);
                                pid_t third_pid = fork();
                                if (third_pid == -1) {
                                    printf("Error in: fork");
                                    continue;
                                } else if (third_pid == 0) {
                                    char *args[] = {"/home/naama/ex2-os/ex22/comp.out", output_file, expected_output,
                                                    NULL};
                                    execvp(args[0], args);
                                    perror("exe failed");
                                } else {
                                    waitpid(third_pid, &status, 0);
                                    switch (WEXITSTATUS(status)) {
                                        case 1:
                                            strcat(name, ",100,EXCELLENT\n");
                                            write(result, name, strlen(name));
                                            name = "";
                                            break;
                                        case 2:
                                            strcat(name, ",50,WRONG\n");
                                            write(result, name, strlen(name));
                                            name = "";
                                            break;
                                        case 3:

                                            strcat(name, ",75,SIMILAR\n");
                                            write(result, name, strlen(name));
                                            name = "";
                                            break;
                                        default:


                                            //remove files of user
                                    }
                                }


                            }
                        }
                    }
                }

            } else {
                strcat(name, ",0,NO_C_FILE\n");
                write(result, name, strlen(name));
                name = "";
            }
        }

        chdir(path);
    }
    int close_dir=closedir(dir);
    if (close_dir==-1){
        printf("Error in: closedir");
        exit(-1);

    }
    close(result);
    close(input_fd);
    close(error_fd);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        return 0;
    }
    char lines[3][150];
    extract_input(argv[1], lines);
    if (access(lines[0], F_OK) != 0) {
        printf("Not a valid directory\n");
        return -1;
    }
    if (access(lines[1], F_OK) != 0) {
        printf("Input file not exist\n");
        return -1;
    }
    if (access(lines[2], F_OK) != 0) {
        printf("Output file not exist\n");
        return -1;
    }
    compile_and_run_files(lines[0], lines[1], lines[2]);


}

