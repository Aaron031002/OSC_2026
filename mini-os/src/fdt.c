#include "fdt.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define FDT_BEGIN_NODE 0x00000001
#define FDT_END_NODE   0x00000002
#define FDT_PROP       0x00000003
#define FDT_NOP        0x00000004
#define FDT_END        0x00000009
#define FDT_MAGIC      0xd00dfeed

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

static inline __attribute__((unused)) uint64_t bswap64(uint64_t x) {
    return __builtin_bswap64(x);
}

static inline const void* align_up(const void* ptr, size_t align) {
    return (const void*)(((uintptr_t)ptr + align - 1) & ~(align - 1));
}

static int node_name_match(const char* node_name, const char* want)
{
    const char *want_at = strchr(want, '@');    // strchr: return a pointer point to the address '@' at want 

    if (want_at)
        return strcmp(node_name, want) == 0;    // node_name also has '@'

    size_t n = strlen(want);

    /* find if the first n char are identical */
    return strncmp(node_name, want, n) == 0 && (node_name[n] == '\0' || node_name[n] == '@');   // strncmp: compare if it is match for the first n char 
}

int fdt_path_offset(const void* fdt, const char* path) {
    // TODO: Implement this function
    if (!fdt || !path || path[0] != '/')    // check input (the path needs to start with '/')
        return -1;

    /* header points to the start point of the device tree */
    const struct fdt_header* header = (const struct fdt_header*)fdt;    

    /* check MAGIC */
    if (bswap32(header->magic) != FDT_MAGIC)   
        return -1;

    /* move 1 byte a time, so use uint8_t*/
    const uint8_t* struct_base = (const uint8_t*)fdt + bswap32(header->off_dt_struct);  // struct_base points to the start point of structure block

    /* setting token parser to the first token */
    const uint8_t* p = struct_base;      

    if (strcmp(path, "/") == 0)     // the path of root node is 0, so return 0
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
    for (const char* s = copy; *s; s++){   
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

        p += sizeof(uint32_t);  // go to the next depth level

        switch (token){
            case FDT_BEGIN_NODE:
                depth++;

                const char* node_name = (const char*)p;

                p = align_up(p + strlen(node_name) + 1, 4);

                if (depth == 0){
                    break;
                }

                if ((size_t)depth <= ncomp){
                    matched[depth] = matched[depth-1] && node_name_match(node_name, comp[depth-1]);
                }

                if ((size_t)depth == ncomp && matched[depth]){
                    free(matched);
                    free(comp);
                    free(copy);
                
                    return token_offset;
                }

                break;
            
            case FDT_END_NODE:
                depth--;

                break;
            
            case FDT_PROP:
                uint32_t len = bswap32(*(const uint32_t*)p);

                p += 8;     // len + nameoff

                p = align_up(p + len, 4);   // skip property value

                break;
        
            case FDT_NOP:
                break;

            case FDT_END:
            default:
                free(matched);
                free(comp);
                free(copy);
                
                return -1;
        }
    }

}

const void* fdt_getprop(const void* fdt,
                        int nodeoffset,
                        const char* name,
                        int* lenp) {
    // TODO: Implement this function
    if (!fdt || nodeoffset < 0 || !name){
        return NULL;
    }

    const struct fdt_header* header = (const struct fdt_header*)fdt;

    if (bswap32(header->magic) != FDT_MAGIC)
        return NULL;

    const uint8_t* struct_base = (const uint8_t*)fdt + bswap32(header->off_dt_struct);
    const char* string_base = (const char*)fdt + bswap32(header->off_dt_strings);

    // nodeoffset is the return of fdt_path_offset (base to node)
    const uint8_t* p = struct_base + nodeoffset;

    if (bswap32(*(const uint32_t*)p) != FDT_BEGIN_NODE)
        return NULL;

    // skip FDT_BEGIN_NODE
    p += sizeof(uint32_t);  

    const char* node_name = (const char*)p;
    
    p = align_up(p + strlen(node_name) + 1, 4);     // padding

    for (;;){
        uint32_t token = bswap32(*(const uint32_t*)p);

        p += sizeof(uint32_t);  // move on

        switch (token){
            case FDT_PROP:
                const uint32_t len = bswap32(*(const uint32_t*)p);
                const uint32_t nameoff = bswap32(*(const uint32_t*)(p + 4));

                const void* value = p + 8;  // len & nameoff followed by value

                const char* prop_name = string_base + nameoff;

                if (strcmp(prop_name, name) == 0){  // find the right property
                    if (lenp)
                        *lenp = (int)len;

                    return value;
                }

                /* if it is not the right prop, find the next one */
                p = align_up((const uint8_t*)value + len, 4);   

                break;

            case FDT_NOP:
                break;

            /* the property area has ended if encounter the following token */
            case FDT_BEGIN_NODE:
            case FDT_END_NODE:
            case FDT_END:
            default:
                return NULL;
        }


    }
}

