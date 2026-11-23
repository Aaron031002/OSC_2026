# Complete format
[ex31_explanation](https://hackmd.io/@V8zl_vkLSTWTJBBMtMS4ag/ryd7JJG1Qx)

# Concept graph
![image](https://hackmd.io/_uploads/r16NJyzyXl.png)

# Workflow
## `alloc_pages`
1. 從 order 0 開始，找`free_area[order]` 是否有東西 (若沒有，代表該 order 沒有 free pages 可以使用)

2. 找到之後 pop 出來 (並指定給 page)

3. 若找到的 free page 的 order 比需求的大，就切分 
    :::info
    找位於 page 指向的位址的 page 的 buddy，用 buddy 指標指向該位址，代表切分處，把 `buddy->order` 改成下面一個 order、`refcount` 改成 0。(因為要放回去，沒有要使用)
    
    再把 buddy push 到下面一個 order 的 free_area ( 即 `free_area[order - 1]` )
    :::
    :::success
    實際上 buddy 指向的是 mem_map 裡面 maintain 的 physical page
    :::

4. 把 `page->order` 指定成需求給的 order、`refcount` 改成 1 (已被分配)，最後 return 該 page 給 user(OS) 使用

## `free_pages`
1. 找到要 free 的 page 的 buddy 的位址

2. 確認 buddy 是否沒被配置，且 order 與要被 free 的 page 相同 
    :::warning
    只能跟自己的 buddy merge
    :::

3. 把 buddy 從他所屬的 free_area 移除 (因為要被 merge 了，不能留下切分點)

4. address 比較小的作為 page 的 head
(`if (buddy < page) page = buddy;`)

5. 把 merge 的 order 改成上一層、refcount 改成 0，最後 push 到上一層的 free_area

:::info
`free_area[i]` 裡面存的都是 address
:::