#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>

int main(void) {
    printf("Hello\n");
    char *filename = "input.txt";
    int fd;
    if ((fd = open(filename, O_RDWR | O_TRUNC, 0644)) < 0) {
        strerror(errno);
    }
    ftruncate(fd, 1024);
    char buf[1024];
    int nread = 0;
    if ((nread = read(fd, buf, sizeof(buf)) < 0)) {
        strerror(errno);
    }
    // buf[nread] = '\0';
    write(STDOUT_FILENO, buf, sizeof(buf));
    close(fd);
}
