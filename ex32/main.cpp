#include <iostream>
#include <list>
#include <vector>

#define PAGE_SIZE (1UL << 12)
#define NUM_PAGES 0x280000
#define MAX_ORDER 10

typedef unsigned long phys_addr_t;

struct page {
    int order = 0;
    int refcount = 0;
};

std::vector<page> mem_map;
std::vector<std::list<page*>> free_area;

struct page* get_buddy(struct page* page, unsigned int order) {
    return &mem_map[(page - mem_map.data()) ^ (1 << order)];
}

void memory_reserve(phys_addr_t base, size_t size) {
    // TODO: Implement this function
    phys_addr_t last_reserv_byte = base + (size - 1);   // address "size" no need to be reserved
    
    /* 1. Calculate the reserved region with pfn representation */
    unsigned long start_pfn = base / PAGE_SIZE;     // the start pfn that has to be reserved
    unsigned long end_pfn = last_reserv_byte / PAGE_SIZE + 1;   // the last + 1 pfn that has to be reserved (so exclude pfn 'end_pfn')

    /* 2. Desending order loop (because it will need spliting) */
    for (int order = MAX_ORDER; order >= 0; order--){
        auto& list = free_area[order];
        auto it = list.begin();     // 'it' points to the start address of free_area

        /* Loop in a specific free list */
        while (it != list.end()){
            struct page* page = *it;    // the start address of the free page in mem_map 
            /* calculate the free page range */
            unsigned long blk_start = page - mem_map.data();    // the pfn this free area belongs to (mem_map.data() is the address of the first element)
            unsigned long blk_end = blk_start + (1 << order);   // blk_end is the start address of the last + 1 pfn loacted at (1 << order) is the number of pages this order have
            
            /* 3.1 completely outside the reserved range: keep it */
            if (blk_end <= start_pfn || blk_start >= end_pfn){
                ++it;
                continue;
            }

            /* Any overlap: take it out of this order's free list */
            it = list.erase(it);    // erase and move the iterator to the next element
        
            /* 3.2 Completely inside: mark occupied, it will never return */
            if (start_pfn <= blk_start && blk_end <= end_pfn){
                page->order = order;
                page->refcount = 1;
                continue;
            }

            /* 3.3 Partially overlap: split and push both into free_area[order-1]
             *     then continue looping to re-evaluate it
             *     finally, it will be completely inside state
             */
            struct page* buddy = get_buddy(page, order-1);
            buddy->order = order - 1;
            buddy->refcount = 0;
            page->order = order - 1;
            page->refcount = 0;
            
            free_area[order - 1].push_front(page);
            free_area[order - 1].push_front(buddy);
        }
    }
}

void dump() {
    for (int i = MAX_ORDER; i >= 0; i--)
        std::cout << "free_area[" << i << "] " << free_area[i].size()
                  << std::endl;
}

void mm_init() {
    mem_map.resize(NUM_PAGES);  // setup pages in memory
    free_area.resize(MAX_ORDER + 1);    // create free_area for pages
    for (size_t i = 0; i < NUM_PAGES; i += (1 << MAX_ORDER)) {
        mem_map[i].order = MAX_ORDER;
        free_area[MAX_ORDER].push_back(&mem_map[i]);
    }
    memory_reserve(0, 0x82a69510);
}

int main() {
    mm_init();
    dump();
    return 0;
}
