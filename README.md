# CSC501 Project: Xinu Demand Paging
Demand paging is a method of mapping a large address space into a relatively small amount of physical memory. It allows a program to use an address space that is larger than the physical memory, and access non-contiguous sections of the physical memory in a contiguous way. Demand paging is accomplished by using a "backing store" (usually disk) to hold pages of memory that are not currently in use.
## Finished
* In-memory backing stores emulating disk storage
* Memory virtualization with page table and page fault handling
* Switch page directory base register and physical frame when context switch
* Memory allocate and free within virtual heap space
* Map virtual address to backing store, share virtual memory
* Two page replacing algorithms run when physical memory full
    1. Second-Chance (SC)
    2. Aging (AGING)
* Pages loaded from backing stores when referenced again

## TODO
* the third page replacing algorithm -- LRU
* can't operate backing store in concurrent environment
* can't issue error when invalid parameters are passed
* can't share memory when two processes mapped to one backing store 
* can't switch page replacing algorithm inside the running program
* there will be an infinite page fault in test case 3 when page num is small

## Note
### What Xinu do?
* original memory addressing method of Xinu is straightforward, all staff are stored in a 16MB large memory
* all address is directly the physical address, no intermediate change or convert

### Memory layout  

|location|data|
| ---------- | --- |
|16M-4G|Virtual Heap (process- specific)|
|2048- 4095 pages|backing store 1-8|
|1024-2047 pages|physical frame|
|X-1023 pages|kernel heap|
|X-X pages|Hole|
|24-X pages|kernel stack|
|0-24 pages|Xinu text, data, bss|  
  
* when processes access their virtual memory (address above 16MB), the actual read and write will happen in 1024-2047 pages
* when frames are full, the frames will be swapped to the 2048-4095 pages

### What hardware do?
* to implement the paging system, we highly depend on the hardware behavior since the actual access action of physical memory is done by hardware
* original hardware directly interpret the address as physical address
* by change the bits of control register 0, the hardware will work in a paging mode
* in this mode, every time accessing memory, hardware will read the control register 3, and use the content of cr3 as a pointer point to the page directory base
* then just as the textbook, hardware use the page directory and page table to assemble the finally physical address (our code should set the page table with correct value, so the hardware can find the correct physical address)
* when hardware find a page table entry is not present, it issues an interrupt of page fault, our code should handle the page fault

### Different processes access same virtual address
* different processes can access the same virtual address, but they will have different mapping, so the physical frames they eventually read are different

### Why need the general page table?
* after we set the control register cr0, the hardware will not use the original memory accessing method
* but original Xinu source code still use the original address to access important kernel space of OS (the lower 16MB), like kernel stack, kernel heap, OS data structures and even the physical frame area and backing store area we defined
* so in lower 16MB (actual space the machine has), the demand paging method should act just like original method
* so the lower 16MB of all page tables of our processes should have the same mapping which map to the lower 16MB in OS kernel

### How to get a page in demand?
* when initialized, the page directory and page table are almost empty
* when processes access their virtual memory, the hardware will found these page entries are not present and issue a page fault interrupt
* in page fault handler, we should allocate the real frame and initialize corresponding page table entry with correct information
* next time the process accesses the same virtual address, hardware will found the present page table entry and automatically use the information in page table found the real, physical frame  

### How to read a swapped frame to memory again?
* processes which have a virtual heap space must map all its virtual heap to the backing store, these maps will be recorded by the bs mapping table
* when requiring a new frame, our code will found the physical frames are full
* swap a frame to backing store according to the replacing algorithm
* our code should also set the corresponding page table entry to not present state
* when process access the same virtual address again, the hardware will check the present bit of page table entry but found it's false (we set it)
* hardware will issue an interrupt of page fault, in page fault handler, we should read the frame from backing store according to the record of backing store mapping table


