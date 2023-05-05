//Naama Kashani 312400476
#include <stdio.h>
#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

int compress_file(char *input_file, char *output_file) {

    int input_fd = open(input_file, O_RDONLY);
    int output_fd = open(output_file, O_WRONLY | O_TRUNC | O_CREAT, 0644);
    if (input_fd < 0 || output_fd < 0) {
        write(STDOUT_FILENO, "Error in: open\n", strlen("Error in: open\n"));
        return -1;
    }
    char c;
    int bytes_read;
    while (bytes_read = read(input_fd, &c, 1) > 0) {
        if (c == ' ' || c == '\n' || c == '\t' || c == '\0' || c == '\r' || c == '\v' || c == '\f') {
            continue;
        } else {
            c = tolower(c);
            write(output_fd, &c, 1);

        }
    }
    int close1 = close(input_fd);
    int close2 = close(output_fd);
    if (close1 == -1 || close2 == -1) {
        write(STDOUT_FILENO, "Error in: close", strlen("Error in: close"));
        return -1;
    }

}


int check_identical(char *path_file1, char *path_file2) {
    int file1 = open(path_file1, O_RDONLY);
    int file2 = open(path_file2, O_RDONLY);
    if (file1 < 0 || file2 < 0) {
        write(STDOUT_FILENO, "Error in: open\n", strlen("Error in: open\n"));
        return -1;
    }
    // Read the files and compare their contents
    char buffer1, buffer2;
    while (1) {
        int x1 = read(file1, &buffer1, 1);
        if (x1 < 0) {
            write(STDOUT_FILENO, "Error in: read\n", strlen("Error in: read\n"));
            return -1;
        }
        int x2 = read(file2, &buffer2, 1);
        if (x2 < 0) {
            write(STDOUT_FILENO, "Error in: read\n", strlen("Error in: read\n"));
            return -1;
        }
        if (x2 == 0 && x1 == 0) {
            int close1 = close(file1);
            int close2 = close(file2);
            if (close1 == -1 || close2 == -1) {
                write(STDOUT_FILENO, "Error in: close\n", strlen("Error in: close\n"));
                return -1;
            }
            return 1;

        }
        if (x1 == 0 || x2 == 0) {
            return 0;

        }
        if (buffer1 != buffer2) {
            return 0;
        }
    }
}

int check_similar(char *argv[]) {
    char *compressed_file1 = "output1";
    char *compressed_file2 = "output2";

    compress_file(argv[1], compressed_file1);
    compress_file(argv[2], compressed_file2);

    int result = check_identical(compressed_file1, compressed_file2);
    int x = remove("output1");
    int y = remove("output2");
    if (x == -1 || y == -1) {
        write(STDOUT_FILENO, "Error in: remove\n", strlen("Error in: remove\n"));
        return -1;
    }
    if (result == 1) {
        return 3;
    }
    return 2;

}
void add_path(char *new_path, char *path, char *name) {
    strcat(new_path, path);
    strcat(new_path, "/");
    strcat(new_path, name);
}

int main(int argc, char *argv[]) {

    if (argc != 3) {
        return -1;
    }
    char home_path[1024];
    char *cwd = getcwd(home_path, sizeof(home_path));
    if (cwd == NULL) {
        write(STDOUT_FILENO, "Error in: getcwd\n", strlen("Error in: getcwd\n"));
        return(-1);
    }
    char file_path1[1024]="", file_path2[1024]="";
    if(argv[1][0] != '/'){
        add_path(file_path1, home_path, argv[1]);
    }
    else{
        strcpy(file_path1, argv[1]);
    }
    if(argv[2][0] != '/'){
        add_path(file_path2, home_path, argv[2]);

    }
    else{
        strcpy(file_path2, argv[2]);
    }

    if ((access(file_path1, F_OK) != 0) || (access(file_path1, R_OK) != 0) || (access(file_path2, F_OK) != 0) ||
        (access(file_path2, R_OK) != 0)) {
        write(STDOUT_FILENO, "Error in: access\n", strlen("Error in: access\n"));
        return -1;
    }

    int identical = check_identical(file_path1, file_path2);
    if (identical == 1) {
        return 1;
    }
    if (identical == -1) {
        return -1;
    }
    int similar = check_similar(argv);
    return similar;


}
