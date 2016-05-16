
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>


#define UNIT_BLOCK_SIZE 1024

/*
	super_block
*/

struct super_block
{
	int magicNumber;
	int nInodes;
	int nBlocks;
	int blockSize;
	int fragSize;
	int blocksPerGroup;
	int inodesPerGroup;
	int fragsPerGroup;
	int firstBlock;
	//--------------------
	int nGroups;
	//group_des_t groupDesPtr;
};

typedef struct super_block* super_block_t;



int load_super_block(super_block_t superBlock, int fd){
	void* blockBuffer = malloc(UNIT_BLOCK_SIZE);
	pread(fd,blockBuffer,UNIT_BLOCK_SIZE,1024);
	uint32_t* ptr4=blockBuffer;
	uint16_t* ptr2=blockBuffer;
	superBlock->magicNumber=*(ptr2+28);
	superBlock->nInodes=*(ptr4);
	superBlock->nBlocks=*(ptr4+1);
	superBlock->blockSize=UNIT_BLOCK_SIZE<<*(ptr4+6);
	superBlock->fragSize=UNIT_BLOCK_SIZE<<*(ptr4+7);
	superBlock->blocksPerGroup=*(ptr4+8);
	superBlock->inodesPerGroup=*(ptr4+10);
	superBlock->fragsPerGroup=*(ptr4+9);
	superBlock->firstBlock=*(ptr4+5);
	// superBlock->nInodes=*(ptr4++);
	// superBlock->nBlocks=*(ptr4);

	// ptr4=blockBuffer+20;
	// superBlock->firstBlock=*(ptr4++);
	// superBlock->blockSize=UNIT_BLOCK_SIZE<<*(ptr4++);

	free(blockBuffer);
	return 0;
}

void print_super_block(super_block_t superBlock,FILE* stream){
	fprintf(stream,"%x,%d,%d,%d,%d,%d,%d,%d,%d\n",
		superBlock->magicNumber,superBlock->nInodes,
		superBlock->nBlocks,superBlock->blockSize,
		superBlock->fragSize,superBlock->blocksPerGroup,
		superBlock->inodesPerGroup,superBlock->fragsPerGroup,
		superBlock->firstBlock);
}


void print_super_block_wrapper(super_block_t superBlock,int fd){
	FILE* stream =fdopen(fd,"w");
	print_super_block(superBlock,stream);
}


/*
*	free block/inode bitmaps
*/

struct group_des
{
	int nBlocks;
	int nFreeBlocks;
	int nFreeInodes;
	int nDirs;
	int inodeMapBlock;
	int blockMapBlock;
	int inodeTableBlock;
	//--------bellow here are not part of EXT2 structs

	int nInodes;
	int inodeStart; //first inode for this block group
	int allocated_space; //for inodePtr
	int nAllocated;
	inode_t inodePtr;
};

typedef struct group_des* group_des_t;

int is_free(void* mapBuffer, uint32_t pos)
{
	//returns 1 if block/inode at bit position pos is free, or 0 if not
	if (mapBuffer[pos]==0)
		return 1;
	else
		return 0;

}

//where uint32_t start is the first block in this group

void print_block_map(void* mapBuffer, uint32_t start, uint32_t size, FILE* stream, int block)
{
	uint32_t i;
	for (i=0; i<size; i++)
	{
		if (is_free(mapBuffer, i))
			fprintf(stream, "%x, %d\n", block, start+i+1 );
	}
}

void print_inode_map (void* mapBuffer, FILE* stream, group_des_t groupDes)
{
	//also registers the allocated inodes
	uint32_t size = groupDes->nInodes;
	int start = groupDes->inodeStart;
	int block = groupDes->inodeMapBlock;
	uint32_t i;
	for (i=0; i<size; i++)
	{
		uint32_t nth = start + i;
		if (is_free(mapBuffer, i))
			fprintf(stream, "%x, %d\n", block, nth );
	}
}


/*
*  inode
*/
struct inode
{
	int ninode;
	int file_type;
	int mode;
	int owner;
	int group;
	int link_count;
	int creation_time;
	int modification_time;
	int access_time;
	int file_size;
	int nblocks;
	uint32_t* block_ptr;
};

typedef struct inode* inode_t;

//take in disk inage(fd), block number
//load the inode contents to the Inode structure
int load_inode (inode_t Inode, FILE* fd, int block_num, int inode_number)
{
	void* buffer = (void*) malloc (1024);
	pread(fd,buffer,UNIT_BLOCK_SIZE,1024*block_num);
	uint32_t* ptr4=buffer;
	uint16_t* ptr2=buffer;
	Inode->inode_number = inode_number;
	Inode->mode = *(ptr2);
	Inode->owner = *(ptr2+2);
	Inode->group = *(ptr2+24);
	Inode->link_count = *(ptr2+26);
	Inode->creation_time = *(ptr4+12);
	Inode->modification_time = *(ptr4+16);
	Inode->access_time = *(ptr4+8);
	Inode->file_size = *(ptr4+4);
	Inode->nblocks = *(ptr4+28);

	Inode->block_ptr = (uint32_t*)malloc (sizeof(uint32_t)*15);
	Inode->block_ptr[0]=*(ptr4+40);
	Inode->block_ptr[1]=*(ptr4+44);
	Inode->block_ptr[2]=*(ptr4+48);
	Inode->block_ptr[3]=*(ptr4+52);
	Inode->block_ptr[4]=*(ptr4+56);
	Inode->block_ptr[5]=*(ptr4+60);
	Inode->block_ptr[6]=*(ptr4+64);
	Inode->block_ptr[7]=*(ptr4+68);
	Inode->block_ptr[8]=*(ptr4+72);
	Inode->block_ptr[9]=*(ptr4+76);
	Inode->block_ptr[10]=*(ptr4+80);
	Inode->block_ptr[11]=*(ptr4+84);
	Inode->block_ptr[12]=*(ptr4+88);
	Inode->block_ptr[13]=*(ptr4+92);
	Inode->block_ptr[14]=*(ptr4+96);

	Inode->file_type = '?';
	if (*(ptr2)==0x8000)
		Inode->file_type='f';
	if (*(ptr2)==0x4000)
		Inode->file_type='d';
	if (*(ptr2)==0xA000)
		Inode->file_type='s';

}

//take in an inode, print out its content.
void print_inode (inode_t Inode, FILE* stream)
{

	fprintf(stream, "%d, %c, %o, %d, %d, %d, %x, %x, %x, %d, %d, %x, %x, %x, %x, %x, %x, %x, %x, %x, %x, %x, %x, %x, %x, %x",
			 //inode number,
				Inode->file_type,
				Inode->mode,
				Inode->owner,
				Inode->group,
				Inode->link_count,
				Inode->creation_time,
				Inode->modification_time,
				Inode->access_time,
				Inode->file_size,
				Inode->nblocks,
				Inode->block_ptr[0],
				Inode->block_ptr[1],
				Inode->block_ptr[2],
				Inode->block_ptr[3],
				Inode->block_ptr[4],
				Inode->block_ptr[5],
				Inode->block_ptr[6],
				Inode->block_ptr[7],
				Inode->block_ptr[8],
				Inode->block_ptr[9],
				Inode->block_ptr[10],
				Inode->block_ptr[11],
				Inode->block_ptr[12],
				Inode->block_ptr[13],
				Inode->block_ptr[14]
				);
}

//print all the inode that is not free
//inode_start_block_num: an array of the starting block number of inode in each groups
//num_groups: number of groups in the disk image
//inode_bit_map: an array of inode_bit_map, where inode_bit_map has the block id to the actual block 
//num_inode_per_group: number of inode per group
void print_inodes (uint32_t * inode_start_block_num, uint32_t num_groups, uint32_t* inode_bit_map, uint32_t num_inode_per_group, FILE* fd, FILE* stream)
{
	int i = 0;
	//go over from the start of inode from each groups
	for (i =0; i<num_groups; i++)
	{
		void* bit_map_buffer = (void*) malloc (1024);
		//get the bitmap buffer
		pread(fd,bit_map_buffer ,UNIT_BLOCK_SIZE,1024*inode_bit_map[i]);
		//checking the inode in each group
		int j=0;
		for (j=0; j< num_inode_per_group; j++)
		{
			int current_inode_block_num = j+inode_start_block_num[i];
			if (! is_free(bit_map_buffer, j))
			{
				inode_t inode_to_be_print;
				load_inode(inode_to_be_print, fd, current_inode_block_num, j+1+i*num_inode_per_group);
				print_inode (inode_to_be_print, stream);
			}
		}
		
	}
}


// struct super_block
// {
// 	int magicNumber;
// 	int nInodes;
// 	int nBlocks;
// 	int blockSize;
// 	int fragSize;
// 	int blocksPerGroup;
// 	int inodesPerGroup;
// 	int fragsPerGroup;
// 	int firstBlock;
// 	//--------------------
// 	int nGroups;
// // 	//group_des_t groupDesPtr;
// // };
// void wraper_print_inodes(FILE* fd, FILE* stream )
// {
// 	super_block_t superBlock;
// 	load_super_block(superBlock, fd);
// 	uint32_t num_groups = superBlock->nInodes/superBlock->inodesPerGroup;
// 	uint32_t * inode_start_block_num = (uint32_t*) malloc (sizeof(uint32_t)*num_groups);
// 	int i;
// 	for (i=0; i< num_groups; i++)
// 	{
// 		inode_start_block_num[i]=
// 	}
// 	print_inodes ()

// }




int main(){
	int fd=open("disk-image",O_RDONLY);
	int fd_super=creat("super.csv",S_IRUSR|S_IWUSR);

	struct super_block s;
	load_super_block(&s,fd);
	printf("-----super block entry: super.csv-----\n");
	print_super_block_wrapper(&s,fd_super);
}