start.S -> start_kernel() -> shell() -> load command
-> load_kernel() -> wait for python to transmit magic & size -> make a pointer to point to the new kernel address 
-> write the new kernel content, start from the new address -> jump to that new address to execute new kernel -> start.S -> start_kernel() -> new kernel started 