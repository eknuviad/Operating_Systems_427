/*
 * =====================================================================================
 *
 *	Filename:  		sma.c
 *
 *  Description:	Base code for Assignment 3 for ECSE-427 / COMP-310
 *
 *  Version:  		1.0
 *  Created:  		6/11/2020 9:30:00 AM
 *  Revised:  		-
 *  Compiler:  		gcc
 *
 *  Author:  		Mohammad Mushfiqur Rahman
 *      
 *  Instructions:   Please address all the "TODO"s in the code below and modify 
 * 					them accordingly. Feel free to modify the "PRIVATE" functions.
 * 					Don't modify the "PUBLIC" functions (except the TODO part), unless
 * 					you find a bug! Refer to the Assignment Handout for further info.
 * =====================================================================================
 */

/* Includes */
#include "sma.h" // Please add any libraries you plan to use inside this file
#include <stdbool.h>
/* Definitions*/
#define MAX_TOP_FREE (128 * 1024) // Max top free block size = 128 Kbytes
//	TODO: Change the Header size if required
#define FREE_BLOCK_HEADER_SIZE 2 * sizeof(char *) + 2*sizeof(int) // Size of the Header in a free memory block
//	TODO: Add constants here
#define BLOCK_TAIL_SIZE 2*sizeof(int) // Size of the Tail in a block of memory
//	TODO: Add constants here
#define EXTRA_SPACE (10*1024)
#define FREE 0
#define NOT_FREE 1
#define HEAD_TAIL_COST 2 * sizeof(char *) + 2*sizeof(int) + 2*sizeof(int)
typedef enum //	Policy type definition
{
	WORST,
	NEXT
} Policy;

char *sma_malloc_error;
static void *freeListHead = NULL;			  //	The pointer to the HEAD of the doubly linked free memory list
static void *freeListTail = NULL;			  //	The pointer to the TAIL of the doubly linked free memory list
unsigned long totalAllocatedSize = 0; //	Total Allocated memory in Bytes
unsigned long totalFreeSize = 0;	  //	Total Free memory in Bytes in the free memory list
Policy currentPolicy = WORST;		  //	Current Policy
//	TODO: Add any global variables here
void *last_allocated_ptr;
void *INIT_POINT_BREAK;
void *POINT_BREAK;
int free_block_size;
static bool wasSet = false;

char str[60];


/*
 * =====================================================================================
 *	Public Functions for SMA
 * =====================================================================================
 */


//add a TL header to block
void add_TAG_LEN_header(void *ptr, int size, int tag){
	//attach TL to head
	int *head_ptr = (int *)ptr;
	*(int *)head_ptr = tag;
	head_ptr ++; 
	*(int *) head_ptr = size; 	
}

//change the size of a block
void change_size (void *ptr, int new_size, int tag){
	//assuming ptr is right after L
	int *mv_down_ptr = (int *) ptr;
	mv_down_ptr--;
	*mv_down_ptr = new_size;
	mv_down_ptr --;//T head
	add_TAG_LEN_header((void *)mv_down_ptr, new_size, tag);
	char *mv_up_ptr =(char *) ptr;
	mv_up_ptr+= 2*sizeof(char *) + new_size;//T tail	
	add_TAG_LEN_header((void *)mv_up_ptr, new_size, tag);
}

//set breakpoint once at the start of program
void set_INITIAL_PB(void *ptr){
	if(wasSet){
		return;
	}else{
		wasSet = true;
		INIT_POINT_BREAK = ptr;
	}
}

/*
 *	Funcation Name: sma_malloc
 *	Input type:		int
 * 	Output type:	void*
 * 	Description:	Allocates a memory block of input size from the heap, and returns a 
 * 					pointer pointing to it. Returns NULL if failed and sets a global error.
 */
void *sma_malloc(int size)
{
	void *pMemory = NULL;
	// Checks if the free list is empty
	if (freeListHead == NULL)
	{		
		// Allocate memory by increasing the Program Break
		pMemory = allocate_pBrk(size);
	}
	// If free list is not empty
	else
	{
		// Allocate memory from the free memory list
		pMemory = allocate_freeList(size);
		// If a valid memory could NOT be allocated from the free memory list
		if (pMemory == (void *)-2)
		{	
			// Allocate memory by increasing the Program Break
			pMemory = allocate_pBrk(size);
		}
	}
	// Validates memory allocation
	if (pMemory < 0 || pMemory == NULL)
	{
		sma_malloc_error = "Error: Memory allocation failed!";
		return NULL;
	}
	// Updates SMA Info
	totalAllocatedSize += size;
	brk(sbrk(0));
	return pMemory;
}

/*
 *	Funcation Name: sma_free
 *	Input type:		void*
 * 	Output type:	void
 * 	Description:	Deallocates the memory block pointed by the input pointer
 */
void sma_free(void *ptr)
{
	//	Checks if the ptr is NULL
	if (ptr == NULL)
	{
		puts("Error: Attempting to free NULL!");
	}
	//	Checks if the ptr is beyond Program Break
	else if (ptr > sbrk(0))
	{
		puts("Error: Attempting to free unallocated space!");
	}
	else
	{
		//	Adds the block to the free memory list
		add_block_freeList(ptr);
	}
}

/*
 *	Funcation Name: sma_mallopt
 *	Input type:		int
 * 	Output type:	void
 * 	Description:	Specifies the memory allocation policy
 */
void sma_mallopt(int policy)
{
	// Assigns the appropriate Policy
	if (policy == 1)
	{
		currentPolicy = WORST;
	}
	else if (policy == 2)
	{
		currentPolicy = NEXT;
	}
}

/*
 *	Funcation Name: sma_mallinfo
 *	Input type:		void
 * 	Output type:	void
 * 	Description:	Prints statistics about current memory allocation by SMA.
 */
void sma_mallinfo()
{
	//	Finds the largest Contiguous Free Space (should be the largest free block)
	int largestFreeBlock = get_largest_freeBlock();
	char str[60];
	//	Prints the SMA Stats
	sprintf(str, "Total number of bytes allocated: %lu", totalAllocatedSize);
	puts(str);
	sprintf(str, "Total free space: %lu", totalFreeSize);
	puts(str);
	sprintf(str, "Size of largest contigious free space (in bytes): %d", largestFreeBlock);
	puts(str);
}

/*
 *	Funcation Name: sma_realloc
 *	Input type:		void*, int
 * 	Output type:	void*
 * 	Description:	Reallocates memory pointed to by the input pointer by resizing the
 * 					memory block according to the input size.
 */
void *sma_realloc(void *ptr, int size){
	// TODO: 	Should be similar to sma_malloc, except you need to check if the pointer address
	//			had been previously allocated.
	// Hint:	Check if you need to expand or contract the memory. If new size is smaller, then
	//			chop off the current allocated memory and add to the free list. If new size is bigger
	//			then check if there is sufficient adjacent free space to expand, otherwise find a new block
	//			like sma_malloc.
	//			Should not accept a NULL pointer, and the size should be greater than 0.
}

/*
 * =====================================================================================
 *	Private Functions for SMA
 * =====================================================================================
 */

/*
 *	Funcation Name: allocate_pBrk
 *	Input type:		int
 * 	Output type:	void*
 * 	Description:	Allocates memory by increasing the Program Break
 */
void *allocate_pBrk(int size)
{
	void *newBlock = NULL;
	int excessSize = EXTRA_SPACE;

	//	TODO: 	Allocate memory by incrementing the Program Break by calling sbrk() or brk()
	//	Hint:	Getting an exact "size" of memory might not be the best idea. Why?
	//			Also, if you are getting a larger memory, you need to put the excess in the free list

	set_INITIAL_PB(sbrk(0));
	
	POINT_BREAK = sbrk(0);
	sbrk(2*sizeof(int));
	brk(sbrk(0));
	newBlock = sbrk(0);
	sbrk(FREE_BLOCK_HEADER_SIZE + size + BLOCK_TAIL_SIZE+ excessSize);
	POINT_BREAK = sbrk(0);
	brk(sbrk(0));
	//	Allocates the Memory Block
	allocate_block(newBlock, size, excessSize, 0);
	POINT_BREAK = sbrk(0);
	brk(sbrk(0));
	return newBlock;
}



/*
 *	Funcation Name: allocate_freeList
 *	Input type:		int
 * 	Output type:	void*
 * 	Description:	Allocates memory from the free memory list
 */
void *allocate_freeList(int size)
{
	void *pMemory = NULL;
	if (currentPolicy == WORST)
	{	
		// Allocates memory using Worst Fit Policy
		pMemory = allocate_worst_fit(size);
	}
	else if (currentPolicy == NEXT)
	{
		// Allocates memory using Next Fit Policy
		pMemory = allocate_next_fit(size);
	}
	else
	{
		pMemory = NULL;
	}
	return pMemory;
}

/*
 *	Funcation Name: allocate_worst_fit
 *	Input type:		int
 * 	Output type:	void*
 * 	Description:	Allocates memory using Worst Fit from the free memory list
 */
void *allocate_worst_fit(int size){
	void *worstBlock = freeListHead;
	int excessSize;
	int blockFound = 0;
	//	TODO: 	Allocate memory by using Worst Fit Policy
	//	Hint:	Start off with the freeListHead and iterate through the entire list to 
	//			get the largest block
	if(freeListHead != NULL){
		int worstsize = get_largest_freeBlock();
		void **tmp_ptr = freeListHead;
		while(*((int*)tmp_ptr - 1) > worstsize){			
			tmp_ptr++;
			tmp_ptr = *tmp_ptr;	
		}	
		blockFound = 1;
		worstBlock = tmp_ptr;
		excessSize = worstsize -  size;
	}
	//	Checks if appropriate block is found.
	if (blockFound){
		//	Allocates the Memory Block
		allocate_block(worstBlock, size, excessSize, 1);
	}
	else{
		//	Assigns invalid address if appropriate block not found in free list
		worstBlock = (void *)-2;
	}
	return worstBlock;
}

/*
 *	Funcation Name: allocate_next_fit
 *	Input type:		int
 * 	Output type:	void*
 * 	Description:	Allocates memory using Next Fit from the free memory list
 */
void *allocate_next_fit(int size){
	void *nextBlock = NULL;
	int excessSize;
	int blockFound = 0;
	//	TODO: 	Allocate memory by using Next Fit Policy
	//	Hint:	You should use a global pointer to keep track of your last allocated memory address, and 
	//			allocate free blocks that come after that address (i.e. on top of it). Once you reach 
	//			Program Break, you start from the beginning of your heap, as in with the free block with
	//			the smallest address)
	if(freeListHead != NULL){
		void **tmp_ptr = freeListHead;
		while(*tmp_ptr  > POINT_BREAK){		
			if(*((int*)tmp_ptr - 1) > size){
				break;
			}	
			tmp_ptr++;
			tmp_ptr = *tmp_ptr;	
		}	
		blockFound = 1;
		int block_size = get_blockSize(tmp_ptr);
		excessSize = block_size -  size;
	}
	//	Checks if appropriate found is found.
	if (blockFound){
		//	Allocates the Memory Block
		allocate_block(nextBlock, size, excessSize, 1);
	}
	else{
		//	Assigns invalid address if appropriate block not found in free list
		nextBlock = (void *)-2;
	}
	return nextBlock;
}

/*
 *	Funcation Name: allocate_block
 *	Input type:		void*, int, int, int
 * 	Output type:	void
 * 	Description:	Performs routine operations for allocating a memory block
 */
void allocate_block(void *newBlock, int size, int excessSize, int fromFreeList){
	void *excessFreeBlock; //	pointer for any excess free block
	int addFreeBlock;
	// 	Checks if excess free size is big enough to be added to the free memory list
	//	Helps to reduce external fragmentation

	//	TODO: Adjust the condition based on your Head and Tail size (depends on your TAG system)
	//	Hint: Might want to have a minimum size greater than the Head/Tail sizes
	addFreeBlock = excessSize > FREE_BLOCK_HEADER_SIZE + BLOCK_TAIL_SIZE;
	//	If excess free size is big enough
	if (addFreeBlock){
		//	TODO: Create a free block using the excess memory size, then assign it to the Excess Free Block
		//allocated block TL...TL
		int *setTL_head = (int *) newBlock;
		setTL_head--;
		setTL_head--;
		add_TAG_LEN_header((void *)setTL_head, size, 1);
		char *setTL_tail = (char *) newBlock;
		setTL_tail += 2*sizeof(char *) + size;
		add_TAG_LEN_header((void *)setTL_tail, size, 1);//add tail to alloc block in use
		
		//create TLPN--TL of excess block after moving past PN and size of prev block
		char *excess = (char *) newBlock;
		excess += (2*sizeof(char *)+ size + 2*sizeof(int)); //to account for PNblock. We should be at the T of new free block
		add_TAG_LEN_header((void*) excess, excessSize,0); //Add TL head of xtrafree
		int *excess_TL = (int *) excess;
		excess_TL ++;
		excess_TL ++; //moves to new free head L
		excessFreeBlock = (void *) excess_TL;
		char *add_tail = (char *) excess_TL;
		add_tail +=(2*sizeof(char *)+excessSize);
		add_TAG_LEN_header((void *) add_tail, excessSize,0); //Add TL tail of xtrafree

		//	Checks if the new block was allocated from the free memory list
		if (fromFreeList){
			//Adjust/Modify PN of xtra free
			//	Removes new block and adds the excess free block to the free list
			replace_block_freeList(newBlock, excessFreeBlock); //value just after L passed here
		}
		else{
			//Creates PN of xtra free
			//	Adds excess free block to the free list
			add_block_freeList(excessFreeBlock);
		}
	}
	//	Otherwise add the excess memory to the new block
	else
	{
		//	TODO: Add excessSize to size and assign it to the new Block
			char *new_size = (char *) newBlock;
			new_size -= 4*sizeof(char); //go to begining of block T
			add_TAG_LEN_header((void *) new_size, size + excessSize, 1); //add tail TL to allocated block
			char *add_tail = (char *) newBlock;
			add_tail +=(2*sizeof(char *)+ size + excessSize);
			add_TAG_LEN_header(add_tail, size + excessSize, 1); //add tail TL to allocated block
		//	Checks if the new block was allocated from the free memory list
		if (fromFreeList)
		{
			//	Removes the new block from the free list
			remove_block_freeList(newBlock);
		}
	}
}

/*
 *	Funcation Name: replace_block_freeList
 *	Input type:		void*, void*
 * 	Output type:	void
 * 	Description:	Replaces old block with the new block in the free list
 */
void replace_block_freeList(void *oldBlock, void *newBlock)
{
	//	TODO: Replace the old block with the new block
	void **cp_P_old = (void *)oldBlock;
	void **cp_N_old = (void *)oldBlock;
	cp_N_old ++;
	void **rp_PN_new = (void *)newBlock;
	*rp_PN_new = cp_P_old;
	rp_PN_new++;
	*rp_PN_new = cp_N_old;
	remove_block_freeList(oldBlock);
	add_block_freeList(newBlock);
	//	Updates SMA info
	totalAllocatedSize += (get_blockSize(oldBlock) - get_blockSize(newBlock));
	totalFreeSize += (get_blockSize(newBlock) - get_blockSize(oldBlock));
}

/*
 *	Funcation Name: add_block_freeList
 *	Input type:		void*
 * 	Output type:	void
 * 	Description:	Adds a memory block to the the free memory list
 */
void add_block_freeList(void *block)
{
	//	TODO: 	Add the block to the free list
	//	Hint: 	You could add the free block at the end of the list, but need to check if there
	//			exits a list. You need to add the TAG to the list.
	//			Also, you would need to check if merging with the "adjacent" blocks is possible or not.
	//			Merging would be tideous. Check adjacent blocks, then also check if the merged
	//			block is at the top and is bigger than the largest free block allowed (128kB).

	void *newblock = block;
	int *left_check = (int *) block;
	left_check -= 2;//move to beginT of current block

	//merge with left block if free and not at inital Point break PB
	if((void*) left_check > INIT_POINT_BREAK){
		left_check -=2; //move to T of left
		if(*(int *)left_check == FREE){ 
			left_check += 2;//beginT of current block
			int left_block_length = get_blockSize((void *)left_check); //size of left block
			char *left_block = (char *) left_check;
			//let N of left block point to cur block
			left_block -= 2*sizeof(int) + left_block_length + 2*sizeof(char *); //at P of left
			//change the tag of current to free
			int *ch_TAG = (int *) block;
			ch_TAG -= 2;
			*ch_TAG = 0;
			int blocksize = get_blockSize(block);	
			change_size((void *) left_block, left_block_length + HEAD_TAIL_COST + blocksize, 0);//change both head and tail length
			newblock = (void *) left_block;	
		}
	}

	//merge with right block if free and not at PB
	int block_size = get_blockSize(newblock);
	if(newblock + 2*sizeof(char *) + block_size + BLOCK_TAIL_SIZE < POINT_BREAK){ 
		char *right_check = (char *) newblock;
		right_check += 2*sizeof(char *) + block_size + BLOCK_TAIL_SIZE; //T of right block
		puts(str);
		if( *(int *)right_check == FREE){
			right_check += sizeof(int); //after T of right block
			int rblock_size = *(int *) right_check;
			int *ch_TAG = (int *) block;
			ch_TAG -= 2;
			*ch_TAG = 0;
			change_size(newblock, block_size + HEAD_TAIL_COST + rblock_size, 0);//change both head and tail length
		}
	}
	
	int new_size = get_blockSize(newblock);
	if(newblock + new_size + BLOCK_TAIL_SIZE == POINT_BREAK && new_size > MAX_TOP_FREE){
		new_size = new_size - MAX_TOP_FREE/2; 
		int *ptr =(int *) newblock;
		ptr --;
		*ptr = new_size; //replace old int size at head;
		char *TL = (char *)newblock;
		TL += 2*sizeof(char *) + new_size;
		add_TAG_LEN_header((void *)TL, new_size, 0); //replace tail tag
		sbrk(-(MAX_TOP_FREE/2));
		brk(sbrk(0));
		POINT_BREAK = sbrk(0);
	}
	
	//insert into free list
	if(freeListHead == NULL){
		//This commented code should allocate the next pointer
		//but issues around referening the next node were noted.
		// void **ptr = newblock;
		// *ptr = NULL;
		// ptr++;
		// *ptr = NULL;		
		// freeListHead = newblock;
		// freeListTail = newblock;
		char *head = *((char **)(block));
		char *tail = *((char **)(block) + 1);
		freeListHead = head;
		freeListTail = tail;
	}else{
		if (newblock < freeListHead){
			void **tmp = newblock;
			*tmp = freeListHead;
		}else{
			void **tmp_ptr = freeListHead;
			while(tmp_ptr != NULL){
				tmp_ptr++;
				tmp_ptr = *tmp_ptr;
			}
			void **ptr = newblock;
			*ptr = tmp_ptr;
			ptr++;
			*ptr = tmp_ptr++;
			tmp_ptr = newblock;
			void **n_ptr = *((void **)newblock + 1);
			if(n_ptr != NULL){
				*n_ptr = *(void **)newblock;
			}else{
				freeListTail = newblock;
			}
			sprintf(str, "inserted");
			puts(str);
		}
	}

	//	Updates SMA info
	totalAllocatedSize -= get_blockSize(newblock);
	totalFreeSize += get_blockSize(newblock);
}

/*
 *	Funcation Name: remove_block_freeList
 *	Input type:		void*
 * 	Output type:	void
 * 	Description:	Removes a memory block from the the free memory list
 */
void remove_block_freeList(void *block)
{
	//	TODO: 	Remove the block from the free list
	//	Hint: 	You need to update the pointers in the free blocks before and after this block.
	//			You also need to remove any TAG in the free block.
	if (freeListHead == NULL || block == NULL){
		return;
	}
	//If the block to be removed is head of freelist
	if(freeListHead == block){
		//set the next node to be the head
		void **next = *((void **)(block) + 1); 
		freeListHead = next;
	}
	if(freeListHead != block && freeListTail !=block){
		void **prev = *((void **)(block));
		void **next = *((void **)(block) + 1);
		*prev++;
		prev = next;
		*prev--;
		next = prev;
	}
	int size = get_blockSize(block);
	//update the tag of allocated block
	int *h_TAG = (int *)block;
	h_TAG--;
	h_TAG--; //at T
	*h_TAG = 1;
	char *t_TAG = (char *)block;
	t_TAG += 2*sizeof(char *) + size;
	*(int *) t_TAG = 1;

	//	Updates SMA info
	totalAllocatedSize += get_blockSize(block);
	totalFreeSize -= get_blockSize(block);
}

/*
 *	Funcation Name: get_blockSize
 *	Input type:		void*
 * 	Output type:	int
 * 	Description:	Extracts the Block Size
 */
int get_blockSize(void *ptr)
{
	int *pSize;
	//	Points to the address where the Length of the block is stored
	pSize = (int *)ptr;
	pSize--;
	//	Returns the deferenced size
	return *(int *)pSize;
}

/*
 *	Funcation Name: get_largest_freeBlock
 *	Input type:		void
 * 	Output type:	int
 * 	Description:	Extracts the largest Block Size
 */
int get_largest_freeBlock(){
	int largestBlockSize = 0;
	//	TODO: Iterate through the Free Block List to find the largest free block and return its size
	int prevBlockSize = largestBlockSize;
	void **tmp = freeListHead;
	while (tmp != NULL){
		prevBlockSize = get_blockSize (tmp);
		if(prevBlockSize > largestBlockSize){
			largestBlockSize = prevBlockSize;
		}
		tmp++;
		tmp = *tmp;		
	}
	return largestBlockSize;
}