#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct cpio_t {
    char magic[6];
    char ino[8];
    char mode[8];
    char uid[8];
    char gid[8];
    char nlink[8];
    char mtime[8];
    char filesize[8];
    char devmajor[8];
    char devminor[8];
    char rdevmajor[8];
    char rdevminor[8];
    char namesize[8];
    char check[8];
};

/**
 * @brief Convert a hexadecimal string to integer
 *
 * @param s hexadecimal string
 * @param n length of the string
 * @return integer value
 */
static int hextoi(const char* s, int n) {
    int r = 0;
    while (n-- > 0) {
        r = r << 4;
        if (*s >= 'A')
            r += *s++ - 'A' + 10;
        else if (*s >= 0)
            r += *s++ - '0';
    }
    return r;
}

/**
 * @brief Align a number to the nearest multiple of a given number
 *
 * @param n number
 * @param byte alignment
 * @return aligned number
 */
static int align(int n, int byte) {
    return (n + byte - 1) & ~(byte - 1);
}

void initrd_list(const void* rd) {
    // TODO: Implement this function
    const char *base = (const char*)rd;
    int offset = 0;

    /* Loop each headers */
    while (1){
        /* 'header' points to the start address of header */
        const struct cpio_t *header = (const struct cpio_t*)(base + offset);
        
        /* 1. Check the magic number */
        if (memcmp(header->magic, "070701", 6) != 0){
            printf("Invalid CPIO magic");
            return;
        }

        /* 2. Decode and get the file size & name size */
        int filesize = hextoi(header->filesize, 8);
        int namesize = hextoi(header->namesize, 8);

        if (namesize <= 0 || filesize < 0){
            printf("Invalid CPIO entry\n");
            return;
        }

        /* 3. Add the file name pointer points to the filename behind the header of the file */
        const char *name = base + offset + sizeof(struct cpio_t);
        
        /* 4. Check if this file is the final file */
        if (strcmp(name, "TRAILER!!!") == 0){
            break;
        }

        /* 5. Print out the file size & file name */
        printf("%i %s\n", filesize, name);

        /* 6. Calculate the offset from the very beginning to the file content start point */
        int data_offset = align(offset + sizeof(struct cpio_t) + namesize, 4);
    
        /* 7. Calculate the address of the next header */
        offset = align(data_offset + filesize, 4);
    }
}

void initrd_cat(const void* rd, const char* filename) {
    // TODO: Implement this function
    const char *base = (const char*)rd;
    int offset = 0;

    while (1){
        const struct cpio_t *header = (const struct cpio_t*)(base + offset);
        
        if (memcmp(header->magic, "070701", 6) != 0){
            printf("Invalid CPIO magic");
            return;
        }

        int namesize = hextoi(header->namesize, 8);
        int filesize = hextoi(header->filesize, 8);

        if (namesize <= 0 || filesize < 0){
            printf("Invalid CPIO entry\n");
            return;
        }

        const char* name = base + offset + sizeof(struct cpio_t);

        if (strcmp(name, "TRAILER!!!") == 0)
            break;

        int data_offset = align(offset + sizeof(struct cpio_t) + namesize, 4);

        if (strcmp(name, filename) == 0){
            fwrite(base + data_offset, 1, filesize, stdout);    // (start ptr, size per write, total write size, stream)
            return;
        }

        offset = align(data_offset + filesize, 4);
    }
    
    printf("initrd_cat: %s: No such file\n", filename);
}

int main() {
    /* Prepare the initial RAM disk */
    FILE* fp = fopen("initramfs.cpio", "rb");
    if (!fp) {
        perror("fopen");
        return EXIT_FAILURE;
    }
    fseek(fp, 0, SEEK_END);     // move to the end of file
    long sz = ftell(fp);    // ftell returns totals bytes from file start point to the current point
    void* rd = malloc(sz); 
    fseek(fp, 0, SEEK_SET);
    if (fread(rd, 1, sz, fp) != sz) {
        fprintf(stderr, "Failed to read the device tree blob\n");
        free(rd);
        fclose(fp);
        return EXIT_FAILURE;
    }
    fclose(fp);

    initrd_list(rd);
    initrd_cat(rd, "osc.txt");
    initrd_cat(rd, "test.txt");
    initrd_cat(rd, "penguin.txt");

    free(rd);
    return 0;
}
