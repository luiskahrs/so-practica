#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <errno.h>

int main(int argc, char *argv[])
{
    int fd, offset;
    char *data;
    struct stat sbuf;

    if ((fd = open("/home/utnso/workspace/so-practica/inputejemplo", O_RDONLY)) == -1) {
    	printf("%s", get_current_dir_name());
        perror("open");
        exit(1);
    }

    if (stat("/home/utnso/workspace/so-practica/inputejemplo", &sbuf) == -1) {
        perror("stat");
        exit(1);
    }

    offset = 0;
    if (offset < 0 || offset > sbuf.st_size-1) {
        fprintf(stderr, "mmapdemo: offset must be in the range 0-%d\n", sbuf.st_size-1);
        exit(1);
    }

    data = mmap((caddr_t)0, sbuf.st_size, PROT_READ, MAP_SHARED, fd, 0);

    if (data == (caddr_t)(-1)) {
        perror("mmap");
        exit(1);
    }

    printf("Imprimo lo que se encuentra dentro del archivo: %s", data);
    return 0;
}
