# CSC501 Project: Xinu Demand Paging
Demand paging is a method of mapping a large address space into a relatively small amount of physical memory. It allows a program to use an address space that is larger than the physical memory, and access non-contiguous sections of the physical memory in a contiguous way. Demand paging is accomplished by using a "backing store" (usually disk) to hold pages of memory that are not currently in use.
## Finished
* Backing store management
* Memory virtualization with page table and page fault
* Switch page directory base register and physical frame when context switch
* Memory allocate and free in virtual heap space
* Map virtual address to backing store
* Page replacing when physical memory full


##TODO
* can't operate backing store in concurrent environment
* can't issue error when invalid parameters are passed
* can't share memory when two processes mapped to one backing store 
* can't switch page replacing algorithm inside the running program