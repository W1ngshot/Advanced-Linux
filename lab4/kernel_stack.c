#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>
#include <errno.h>

#define DEVICE_PATH "/dev/int_stack"
#define STACK_IOCTL_SET_SIZE _IOW('k', 1, int)

void handle_error(int code) {
    if (code == ERANGE || code == -ERANGE) {
        fprintf(stderr, "ERROR: stack is full\n");
        exit(-34);
    }
    perror("ERROR");
    exit(code);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <command> [arg]\n", argv[0]);
        return 1;
    }

    int fd = open(DEVICE_PATH, O_RDWR);
    if (fd < 0) {
        perror("Could not open device");
        return 1;
    }

    if (strcmp(argv[1], "set-size") == 0) {
        if (argc < 3) return 1;
        int size = atoi(argv[2]);
        
        if (size <= 0) {
            fprintf(stderr, "ERROR: size should be > 0\n");
            close(fd);
            return 1;
        }

        if (ioctl(fd, STACK_IOCTL_SET_SIZE, &size) < 0) {
            handle_error(errno);
        }
    } 
    else if (strcmp(argv[1], "push") == 0) {
        if (argc < 3) return 1;
        int val = atoi(argv[2]);
        
        if (write(fd, &val, sizeof(int)) < 0) {
            handle_error(errno);
        }
    }
    else if (strcmp(argv[1], "pop") == 0) {
        int val;
        ssize_t res = read(fd, &val, sizeof(int));
        
        if (res == 0) {
            printf("NULL\n");
        } else if (res < 0) {
            handle_error(errno);
        } else {
            printf("%d\n", val);
        }
    }
    else if (strcmp(argv[1], "unwind") == 0) {
        int val;
        while (read(fd, &val, sizeof(int)) > 0) {
            printf("%d\n", val);
        }
    }

    close(fd);
    return 0;
}
