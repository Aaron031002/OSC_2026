#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FDT_BEGIN_NODE 0x00000001
#define FDT_END_NODE   0x00000002
#define FDT_PROP       0x00000003
#define FDT_NOP        0x00000004
#define FDT_END        0x00000009

struct fdt_header {
    uint32_t magic;
    uint32_t totalsize;
    uint32_t off_dt_struct;     // offset that starts from header start point to structure block
    uint32_t off_dt_strings;
    uint32_t off_mem_rsvmap;
    uint32_t version;
    uint32_t last_comp_version;
    uint32_t boot_cpuid_phys;
    uint32_t size_dt_strings;
    uint32_t size_dt_struct;     
};

static inline uint32_t bswap32(uint32_t x) {
    return __builtin_bswap32(x);
}

static inline uint64_t bswap64(uint64_t x) {
    return __builtin_bswap64(x);
}

static inline const void* align_up(const void* ptr, size_t align) {
    return (const void*)(((uintptr_t)ptr + align - 1) & ~(align - 1));
}

int fdt_path_offset(const void* fdt, const char* path) {
    // TODO: Implement this function
    if (!fdt || !path || path[0] != '/')    // check input (the path needs to start with '/')
        return -1;

    /* header points to the start point of the device tree */
    const struct fdt_header* header = (const struct fdt_header*)fdt;    

    /* check MAGIC */
    if (bswap32(header->magic) != 0xd00dfeed)   
        return -1;

    /* move 1 byte a time, so use uint8_t*/
    const uint8_t* struct_base = (const uint8_t*)fdt + bswap32(header->off_dt_struct);  // struct_base points to the start point of structure block

    /* setting token parser to the first token */
    const uint8_t* p = struct_base;      

    if (strcmp(path, '/') == 0)     // the path of root node is 0, so return 0
        return 0;

    /*
        PART_1:

        split the path into components
        ex:
            /cpus/cpu@0/interrupt-controller
            to
            cpus, cpu@0, interrupt-controller 
    */

    char* copy = malloc(strlen(path) + 1);  // reserve for \0

    if (!copy)
        return -1;

    strcpy(copy, path + 1);     // skip the leading '/'

    /* count the number of components */
    size_t ncomp = 1;   // number of components
    for (const char* s; *s; s++){   
        if (*s == '/')
            ncomp++;
    }

    char** comp = malloc(ncomp * sizeof(*comp));    // allocate 24 byte for 3 char* component     
    int* matched = malloc((ncomp + 1) * sizeof(*matched));     // decide if the component in this level of depth has matched the path (include root)

    if (!comp || !matched){
        free(comp);
        free(matched);
        free(copy);
        return -1;
    }

    size_t k = 0;
    comp[k++] = copy;   // comp[0](char*) point to copy (the string of path)

    /* split out the components */
    for (char* s = copy; *s; s++){
        if (*s == '/'){
            *s = '\0';
            comp[k++] = s + 1;  // point the next component(comp[1]) to the next component in path (cpus\0"point to here"cpu@0/...)
        }
    }
    // now the component is in comp (comp[0] = cpus, comp[1] = cpu@0, comp[2] = interrupt-controller)

    /*
        PART_2:

        depth:

        root                    0
        cpus                    1
        cpu@0                   2
        interrupt-controller    3
    */

    int depth = -1;     // depth 0 for root
    matched[0] = 1;     // see root as matched

    for (;;){
        uint32_t token = bswap32(*(const uint32_t*)p);  // read the first token

        int token_offset = (int)(p - struct_base);  // offset to the current node

        p += p + sizeof(uint32_t);  // go to the next depth level

        switch (token){
            case FDT_BEGIN_NODE:
                depth++;

                const char* node_name = (const char*)p;

                p = align_up(p + strlen(node_name) + 1, 4);

                break;
            
            case FDT_END_NODE:
                break;
            
            case FDT_PROP:
                break;
        
            case FDT_NOP:
                break;

            case FDT_END:
            default:
        }
    }

}

const void* fdt_getprop(const void* fdt,
                        int nodeoffset,
                        const char* name,
                        int* lenp) {
    // TODO: Implement this function
}

int main() {
    /* Prepare the device tree blob */
    FILE* fp = fopen("qemu.dtb", "rb"); // open device tree
    if (!fp) {
        perror("fopen");
        return EXIT_FAILURE;
    }
    fseek(fp, 0, SEEK_END); // move to the END position
    long sz = ftell(fp);    // calculate the size of file
    void* fdt = malloc(sz); 
    fseek(fp, 0, SEEK_SET); // move to the START position
    if (fread(fdt, 1, sz, fp) != sz) {  // read 'size' count into the space just allocated and pointed by fdt
        fprintf(stderr, "Failed to read the device tree blob\n");   // size does not match
        free(fdt);
        fclose(fp);
        return EXIT_FAILURE;
    }
    fclose(fp);

    /* Find the node offset */
    int offset = fdt_path_offset(fdt, "/cpus/cpu@0/interrupt-controller");
    if (offset < 0) {
        fprintf(stderr, "fdt_path_offset\n");
        free(fdt);
        return EXIT_FAILURE;
    }

    /* Get the node property */
    int len;
    const void* prop = fdt_getprop(fdt, offset, "compatible", &len);
    if (!prop) {
        fprintf(stderr, "fdt_getprop\n");
        free(fdt);
        return EXIT_FAILURE;
    }
    printf("compatible: %.*s\n", len, (const char*)prop);

    offset = fdt_path_offset(fdt, "/memory");
    prop = fdt_getprop(fdt, offset, "reg", &len);
    const uint64_t* reg = (const uint64_t*)prop;
    printf("memory: base=0x%lx size=0x%lx\n", bswap64(reg[0]), bswap64(reg[1]));

    offset = fdt_path_offset(fdt, "/chosen");
    prop = fdt_getprop(fdt, offset, "linux,initrd-start", &len);
    const uint64_t* initrd_start = (const uint64_t*)prop;
    printf("initrd-start: 0x%lx\n", bswap64(initrd_start[0]));

    free(fdt);
    return 0;
}
