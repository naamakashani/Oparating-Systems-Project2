#include <stdio.h>
#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>


int compress_file(const char *input_file, const char *output_file) {
    int input_fd = open(input_file, O_RDONLY);
    int output_fd = open(output_file, O_WRONLY| O_TRUNC | O_CREAT, 0644);
    if (input_fd < 0 || output_fd < 0) {
        return -1;
    }
    char c;
    int bytes_read;
    while (bytes_read = read(input_fd, &c, 1) > 0) {
            if (!isspace(c) && c != '\n') {
                c = tolower(c);
                write(output_fd, &c, 1);

            }
        }
    close(input_fd);
    close(output_fd);
}


int check_identical(char *path_file1, char *path_file2) {
    int file1 = open(path_file1, O_RDONLY);
    int file2 = open(path_file2, O_RDONLY);
    // Read the files and compare their contents
    char buffer1, buffer2;
    while (1) {
        int x1 = read(file1, &buffer1, 1);
        if (x1 < 0) {
            perror("can't read from file 1");
            return -1;
        }
        int x2 = read(file2, &buffer2, 1);
        if (x2 < 0) {
            perror("can't read from file 2");
            return -1;
        }
        if (x1 == 0 && x1 == 0) {
            close(file1);
            close(file2);
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
    const char *compressed_file1 = "output1";
    const char *compressed_file2 = "output2";

    compress_file(argv[1], compressed_file1);
    compress_file(argv[2], compressed_file2);

    int result = check_identical(compressed_file1, compressed_file2);
    if (result == 1) {
        return 3;
    }
    return 2;

}

int main(int argc, char *argv[]) {
    // Check if exactly two arguments are passed
    if (argc != 3) {
        return -1;
    }
//checks if files exist and have read premission.
    if (access(argv[1], F_OK) != 0) {
        perror("file 1 does not exist");
        return -1;
    }
    if (access(argv[1], R_OK) != 0) {
        perror("file 1 does not have permission");
        return -1;
    }
    if (access(argv[2], F_OK) != 0) {
        perror("file 2 does not exist");
        return -1;
    }
    if (access(argv[2], R_OK) != 0) {
        perror("file 2 does not have permission");
        return -1;
    }


    int identical = check_identical(argv[1], argv[2]);
    if (identical == 1) {
        return 1;
    }
    if (identical == -1) {
        return -1;
    }
    int similar = check_similar(argv);
    return similar;


}

