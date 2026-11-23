#include <iostream>
#include <list>
#include <vector>
#include <algorithm>

#define NUM_PAGES 0x280000
#define MAX_ORDER 10

struct page {
    int order = 0;
    int refcount = 0;
};

std::vector<page> mem_map;
std::vector<std::list<page*>> free_area;

struct page* get_buddy(struct page* page, unsigned int order) {
    return &mem_map[(page - mem_map.data()) ^ (1 << order)];
}

struct page* alloc_pages(unsigned int order) {
    // TODO: Implement this function
    if (order > MAX_ORDER)
        return nullptr;

    /* 1. find a free page */
    int cur = order;
    while (cur < MAX_ORDER && free_area[cur].empty()){
        cur++;
    }
    if (cur > MAX_ORDER){
        printf("[alloc] order %u: out of memory\n", order);
        return nullptr;
    }

    /* 2. Take a free page out and pop out from free area */
    struct page* page = free_area[cur].front();
    free_area[cur].pop_front();
    
    /* 3. Split */
    while (cur > order){
        cur--;
        struct page* buddy = get_buddy(page, cur);  // create a buddy points to the real physical page(mem_map)
        buddy->order = cur;
        buddy->refcount = 0;
        free_area[cur].push_front(buddy);
    }

    /* 4. update the info of the take out page */
    page->order = order;
    page->refcount = 1;
    return page;
}

void free_pages(struct page* page) {
    // TODO: Implement this function
    
    /* Reject NULL and double free */
    if (page == nullptr || page->refcount == 0)
    return;

    int order = page->order;
    page->refcount = 0;

    /* 1. iterative merge */
    while (order < MAX_ORDER){
        struct page* buddy = get_buddy(page, order);

        /* 2. Buddy is not being used & order is the same */
        if (buddy->refcount != 0 || buddy->order != order)  // !(refcount = 0 && order = order)
            break;
    
        /* 3. Remove the buddy from the free list */
        auto& list = free_area[order];
        auto it = std::find(list.begin(), list.end(), buddy);   // find the buddy in the free_area
        if (it == list.end())   // cannot find the buddy
            break;
        list.erase(it);

        /* 4. The lower address becomes the head of the merged block */
        if (buddy < page)
            page = buddy;
        order++;
    }

    /* 5. put the final merged block into free_area[order]*/
    page->order = order;
    page->refcount = 0;
    free_area[order].push_front(page);
}

void dump() {
    for (int i = MAX_ORDER; i >= 0; i--)
        std::cout << "free_area[" << i << "] " << free_area[i].size()
                  << std::endl;
}

int main() {
    mem_map.resize(NUM_PAGES);
    free_area.resize(MAX_ORDER + 1);
    for (size_t i = 0; i < NUM_PAGES; i += (1 << MAX_ORDER)) {
        mem_map[i].order = MAX_ORDER;   /// initialize each pages as order 10
        free_area[MAX_ORDER].push_back(&mem_map[i]);
    }

    std::cout << "\np1:\n";
    struct page* p1 = alloc_pages(1);
    dump();

    std::cout << "\np2:\n";
    struct page* p2 = alloc_pages(1);
    dump();

    std::cout << "\np3:\n";
    struct page* p3 = alloc_pages(1);
    dump();

    free_pages(p1);
    free_pages(p2);
    free_pages(p3);

    std::cout << "\nfree:\n";
    dump();
    return 0;
}
