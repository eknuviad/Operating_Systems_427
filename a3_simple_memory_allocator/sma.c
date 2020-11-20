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
char *freeListHead = NULL;			  //	The pointer to the HEAD of the doubly linked free memory list
char *freeListTail = NULL;			  //	The pointer to the TAIL of the doubly linked free memory list
unsigned long totalAllocatedSize = 0; //	Total Allocated memory in Bytes
unsigned long totalFreeSize = 0;	  //	Total Free memory in Bytes in the free memory list
Policy currentPolicy = WORST;		  //	Current Policy
//	TODO: Add any global variables here
void *last_allocated_ptr;
int INIT_POINT_BREAK;
int POINT_BREAK;
int free_block_size;
bool wasSet = false;




/*
 * =====================================================================================
 *	Public Functions for SMA
 * =====================================================================================
 */



void add_TAG_LEN_header(void *ptr, int size, int tag){
	
	//attach ->TL to head
	void *cur_ptr = ptr; //ptr should be right below T
	int *head_ptr = (int *) cur_ptr;
	*head_ptr = tag;
	head_ptr ++; //move to size column
	*head_ptr = size;
	
}

void change_size (void *ptr, int new_size){
	char *mv_down_ptr = (char *) ptr;
	int *head_L = (int *) mv_down_ptr;
	head_L--;
	*head_L = new_size;

	char *mv_up_ptr =(char *) ptr;
	mv_up_ptr+= 2*sizeof(char) + new_size;
	mv_up_ptr+=sizeof(int);//L of tail of cur block
	*(int *)mv_up_ptr = new_size;

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
void *sma_realloc(void *ptr, int size)
{
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

	if(wasSet == false){
		INIT_POINT_BREAK = sbrk(0);
		wasSet == true;
	}
	sbrk(2*sizeof(int));
	brk(sbrk(0));
	add_TAG_LEN_header(sbrk(0), size, 1);
	newBlock = sbrk(0);

	sbrk(size + excessSize);
	POINT_BREAK = sbrk(0);
	//	Allocates the Memory Block
	allocate_block(newBlock, size, excessSize, 0);

	int alloc_size = get_blockSize(newBlock);
	char *set_last_alloc = (char *) newBlock;
	set_last_alloc += (2*sizeof(char) + alloc_size + BLOCK_TAIL_SIZE); //to account for blockTL
	last_allocated_ptr = (void *) set_last_alloc;

	POINT_BREAK = sbrk(0);
	
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
void *allocate_worst_fit(int size)
{
	void *worstBlock = NULL;
	int excessSize;
	int blockFound = 0;

	//	TODO: 	Allocate memory by using Worst Fit Policy
	//	Hint:	Start off with the freeListHead and iterate through the entire list to 
	//			get the largest block

	//	Checks if appropriate block is found.
	if (blockFound)
	{
		//	Allocates the Memory Block
		allocate_block(worstBlock, size, excessSize, 1);
	}
	else
	{
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
void *allocate_next_fit(int size)
{
	void *nextBlock = NULL;
	int excessSize;
	int blockFound = 0;

	//	TODO: 	Allocate memory by using Next Fit Policy
	//	Hint:	You should use a global pointer to keep track of your last allocated memory address, and 
	//			allocate free blocks that come after that address (i.e. on top of it). Once you reach 
	//			Program Break, you start from the beginning of your heap, as in with the free block with
	//			the smallest address)

	//Im thinking of looping through freelist from last allocated pointer to tail
	//set blockfound to 1 if we find a block size >= size
	//excesssize will be blocksize -size
	//set next block to block we found

	//	Checks if appropriate found is found.
	if (blockFound)
	{
		//	Allocates the Memory Block
		allocate_block(nextBlock, size, excessSize, 1);
	}
	else
	{
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
void allocate_block(void *newBlock, int size, int excessSize, int fromFreeList)
{
	void *excessFreeBlock; //	pointer for any excess free block
	int addFreeBlock;

	// 	Checks if excess free size is big enough to be added to the free memory list
	//	Helps to reduce external fragmentation

	//	TODO: Adjust the condition based on your Head and Tail size (depends on your TAG system)
	//	Hint: Might want to have a minimum size greater than the Head/Tail sizes
	addFreeBlock = excessSize > FREE_BLOCK_HEADER_SIZE + BLOCK_TAIL_SIZE;

	//	If excess free size is big enough
	if (addFreeBlock)
	{
		//	TODO: Create a free block using the excess memory size, then assign it to the Excess Free Block
	
		//create TLPN--TL of excess block after moving past PN and size of prev block
		char *excess = (char *) newBlock;
		excess += (2*sizeof(char)+ (size-excessSize)); //to account for PNblock. We should be at the head of new free block
		
		add_TAG_LEN_header((void*) excess, excessSize,0); //Add TL head of xtrafree
		int *excess_TL = (int *) excess;
		excess_TL ++;
		excess_TL ++; //moves to new free head L
		excessFreeBlock = (void *) excess_TL;
		char *add_tail = (char *) excess_TL;
		add_tail +=(2*sizeof(char)+excessSize);
		add_TAG_LEN_header((void *) add_tail, excessSize,0); //Add TL tail of xtrafree

		//	Checks if the new block was allocated from the free memory list
		if (fromFreeList)
		{
			//Adjust/Modify PN of xtra free
			//	Removes new block and adds the excess free block to the free list
			replace_block_freeList(newBlock, excessFreeBlock); //value just after L passed here
		}
		else
		{
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
			add_tail +=(2*sizeof(char)+ size + excessSize);
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
	
	int oldlength = get_blockSize(oldBlock);
	int newLength = get_blockSize(newBlock);

	int adjusted_length = oldlength - newLength;

	char *old_ptr = (char *) oldBlock;
	old_ptr -= 2*sizeof(int);
	add_TAG_LEN_header((void *) old_ptr, adjusted_length, 1); // add TL to head of old block
		
	old_ptr += FREE_BLOCK_HEADER_SIZE + get_blockSize(oldBlock);

	add_TAG_LEN_header((void *) old_ptr, adjusted_length, 1); // add TL to tail of old block

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

	
	int *left_check = (int *) block;
	left_check -= (2*sizeof(int));//move to T of current block

	if((void*) left_check > INIT_POINT_BREAK){
		left_check -= (2*sizeof(int));//move to T of prev block
		if(*(int *)left_check == FREE){ //
			char *left_block = (char *) left_check;
			left_check++;
			//get length to offset by
			int left_block_length = *(int *) left_check;//at Tail L of left

			//let N of left block point to cur block
			left_block -= sizeof(int) + left_block_length + sizeof(char); //at N of left
			char *block_ptr = (char *)block;
			block_ptr ++; //cur block N
			*left_block = block_ptr;
			
			//let P point cur block point to left block
			block_ptr--;//cur block P
			left_block--;//left block P
			*block_ptr = left_block;
			change_size((void *) left_block, left_block_length + HEAD_TAIL_COST + get_blockSize(block));//change both head and tail length

		
			remove_block_freeList(block);
			block = (void *) left_block;
		}
	}

	int block_size = get_blockSize(block);
	if(block + 2*sizeof(char) + block_size + BLOCK_TAIL_SIZE < POINT_BREAK){ 
		char *right_check = (char *) block;
		right_check += 2*sizeof(char) + block_size + BLOCK_TAIL_SIZE; //T of right block
		if( *(int *)right_check == FREE){

			int *cur_block = (int *) block; 
			right_check += sizeof(int); //after T of right block
			int rblock_size = *(int *) right_check;
			right_check += sizeof(int) + sizeof(char); //N of block
			change_size(block, block_size + HEAD_TAIL_COST + rblock_size);//change both head and tail length

			//let cur N point to right block
			char *cur_N = (char *) block;
			cur_N++;
			*cur_N = right_check;
			
			//let right block point to cur block
			right_check--; //at P
			*right_check =(char *)block;

			remove_block_freeList((void *)right_check);
		}
	}
	

	char *block_ptr = (char *) block;
	int new_size = get_blockSize(block);
	if(block + new_size + BLOCK_TAIL_SIZE == POINT_BREAK && new_size > MAX_TOP_FREE){
		new_size = new_size - (MAX_TOP_FREE/2); 
		int *ptr =(int *) block;
		ptr --;
		*ptr = new_size; //replace old int size;
		sbrk(-(MAX_TOP_FREE/2));
		brk(sbrk(0));
		POINT_BREAK = sbrk(0);
	}
	
	//insert into free list

	if(freeListHead == NULL){
		freeListHead = block;
		freeListTail = block;
		char *free_P_ptr = (char *) freeListHead;
		*free_P_ptr = NULL;
		free_P_ptr ++;
		free_P_ptr = NULL; //representing the N pointer
		//move the pointer an int before the size of block
		//add free tag to it
	}else{
		if (block < freeListHead){
			char *ptr = (char *) block;
			ptr ++;// N of block
			*ptr = (char *)freeListHead;
			*(char *)freeListHead = (char *)block; 
		}else{
			char *tmp_ptr = (char *) freeListHead;
			tmp_ptr++; //at N
			while (*(char *) tmp_ptr != NULL){
				if((void *) tmp_ptr > block)break;
				tmp_ptr = *tmp_ptr;
			}//once we exit we should be at block before null
			
			char *set_N = tmp_ptr;
			*set_N = (char *) block; //let previous block point to current block

			//let cur point to previous
			char *set_cur = (char *) block;
			*set_cur = tmp_ptr;
			set_cur++;
			//let cur next point to tail
			*set_cur = freeListTail;
			
		}
	}

	//	Updates SMA info
	totalAllocatedSize -= get_blockSize(block);
	totalFreeSize += get_blockSize(block);
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


//does similar to replace except pointers of block before and after current 
//block are updated

	char *old_B = (char *) block; //pointer at P
	char *old_P = old_B; //want to store the value of P
	old_B++; //pointer at N
	char *old_N = old_B;
	old_B--; //pointer back to P

	char *tmp_ptr = (char *) freeListHead;
	tmp_ptr++;//N
	while (*(char *) tmp_ptr != *(char*) old_B){
		tmp_ptr = *tmp_ptr;	
	}//once we exit we should be at block before old block

	char *prevBlock = tmp_ptr;

	tmp_ptr = (char *) freeListTail;//P
	while (*(char *) tmp_ptr != *(char*) old_B){
		tmp_ptr = *tmp_ptr;
	}//once we exit we should be at block after old block

	char *nextBlock = tmp_ptr;
	*nextBlock = prevBlock; //change the P of next block
	prevBlock++;
	*prevBlock = nextBlock; //change the N of previous block
	

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
int get_largest_freeBlock()
{
	int largestBlockSize = 0;

	//	TODO: Iterate through the Free Block List to find the largest free block and return its size

	char *char_tmp = freeListHead;
	int prevBlockSize = largestBlockSize;

	while (*(char *)char_tmp != NULL){
		void *void_tmp = (void *) char_tmp;
		prevBlockSize = get_blockSize;
		if(prevBlockSize > largestBlockSize){
			largestBlockSize = prevBlockSize;
		}
		char *N_char_tmp = char_tmp++;
		char_tmp = N_char_tmp;
	}

	return largestBlockSize;
}