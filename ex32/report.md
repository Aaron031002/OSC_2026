# Complete format
[ex32_explanation](https://hackmd.io/@V8zl_vkLSTWTJBBMtMS4ag/SkvvZWX1Qe)

# Concept graph
![image](https://hackmd.io/_uploads/r16NJyzyXl.png)
![image](https://hackmd.io/_uploads/rycbzWX17x.png)

# Workflow
## `memory_reserve(phys_addr_t base, size_t size)`

1. 計算 reserved region 且用 page frame number(pfn) 表示

    ```
    phys_addr_t last_reserv_byte = base + (size - 1);
    unsigned long start_pfn = base / PAGE_SIZE;
    unsigned long end_pfn = last_reserv_byte / PAGE_SIZE + 1;
    ```
    :::info
    `start_pfn` 要無條件捨去
    `end_pfn` 要無條件進入
    :::

2. Loop 從高 order 到低 order 的 `free_area[order]` (因為後面可能要 split)，且 trace `free_area[order]` 裡面的每一個 element
 
    **2-1.** 
    計算 trace 到的 `free_area[order][i]` 的 region range
    ```
    // page 是 free_area[order][i] 的位址
    
    unsigned long blk_start = page - mem_map.data();
    unsigned long blk_end = blk_start + (1 << order); // blk_end 指到最後一個的下一個 page 的起始位址
    ```
    :::spoiler 示意圖
    ![image](https://hackmd.io/_uploads/r16zQbXkQg.png)
    :::
    
    **2-2-1. Free area completely outside**
    如果 free area 完全在需要保存的 region 外面，就留著並繼續 trace 下一個，以後可以 allocate
    ```
    if (blk_end <= start_pfn || blk_start >= end_pfn){
        ++it;
        continue;
    }
    ```
    
    **2-2-2. Free area completely inside**
    紀錄成已經被 allocated，之後不可以配置出去
    ```
    if (start_pfn <= blk_start && blk_end <= end_pfn){
        page->order = order;
        page->refcount = 1;
        continue;
    }
    ```
    
    **2-2-3 Partially overlap**
    split 成兩個 buddy，並 push 到下一層 order，讓下一次 loop 處理，最後都會變成 completely inside (因為是用 pfn 表示)
    Ref: [Concept graph-2](#Concept-graph)
    
    ```
    struct page* buddy = get_buddy(page, order-1);
    buddy->order = order - 1;
    buddy->refcount = 0;
    page->order = order - 1;
    page->refcount = 0;

    free_area[order - 1].push_front(page);
    free_area[order - 1].push_front(buddy);
    ```
    
    
    
    :::warning
    如果 free area 與 reserved region 有任何 overlap，都要先把該 free area 從 `free_area` 移除
    :::