
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>


#define UNIT_BLOCK_SIZE 1024


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
	int s_log_block_size_shift;
	//group_des_t groupDesPtr;
};
struct super_block s;
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
	superBlock->s_log_block_size_shift=*(ptr4+6);
	superBlock->fragSize=UNIT_BLOCK_SIZE<<*(ptr4+7);
	superBlock->blocksPerGroup=*(ptr4+8);
	superBlock->inodesPerGroup=*(ptr4+10);
	superBlock->fragsPerGroup=*(ptr4+9);
	superBlock->firstBlock=*(ptr4+5);
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

//--------------------------------------------------------------------------------

struct inode
{
	int ninode;
	int file_type;
	uint16_t mode;
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
struct inode in;
//take in disk inage(fd), block number
//load the inode contents to the Inode structure
int load_inode (inode_t Inode, int fd, int block_num, int inode_number,int groupDesNumber)
{
	void* buffer = (void*) malloc (128);
	pread(fd,buffer,128,s.blockSize*block_num+128*(inode_number-1));
	uint32_t* ptr4=buffer;
	uint16_t* ptr2=buffer;
	Inode->ninode = inode_number+groupDesNumber*(s.inodesPerGroup);
	Inode->mode = *(ptr2);
	Inode->owner = *(ptr2+1);
	Inode->group = *(ptr2+12);
	Inode->link_count = *(ptr2+13);
	Inode->creation_time = *(ptr4+3);
	Inode->modification_time = *(ptr4+4);
	Inode->access_time = *(ptr4+2);
	Inode->file_size = *(ptr4+1);
	Inode->nblocks = *(ptr4+7)/(2<<s.s_log_block_size_shift);

	Inode->block_ptr = (uint32_t*)malloc (sizeof(uint32_t)*15);
	Inode->block_ptr[0]=*(ptr4+10);
	Inode->block_ptr[1]=*(ptr4+11);
	Inode->block_ptr[2]=*(ptr4+12);
	Inode->block_ptr[3]=*(ptr4+13);
	Inode->block_ptr[4]=*(ptr4+14);
	Inode->block_ptr[5]=*(ptr4+15);
	Inode->block_ptr[6]=*(ptr4+16);
	Inode->block_ptr[7]=*(ptr4+17);
	Inode->block_ptr[8]=*(ptr4+18);
	Inode->block_ptr[9]=*(ptr4+19);
	Inode->block_ptr[10]=*(ptr4+20);
	Inode->block_ptr[11]=*(ptr4+21);
	Inode->block_ptr[12]=*(ptr4+22);
	Inode->block_ptr[13]=*(ptr4+23);
	Inode->block_ptr[14]=*(ptr4+24);

	//printf("Inode Mode:%x\n",Inode->mode);
	Inode->file_type = '?';
	if ((Inode->mode&0x8000)==0x8000)
		Inode->file_type='f';
	if ((Inode->mode&0x4000)==0x4000)
		Inode->file_type='d';
	if ((Inode->mode&0xA000)==0xA000)
		Inode->file_type='s';

	free(buffer);
	return 0;
}

//take in an inode, print out its content.
void print_inode (inode_t Inode, FILE* stream)
{
	fprintf(stream, "%d,%c,%o,%d,%d,%d,%x,%x,%x,%d,%d,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x\n",
			 //inode number,
				Inode->ninode,
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

//----------------------------------------------------------------------------------
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
	//inode_t inodePtr;
};
struct group_des  gd;
typedef struct group_des* group_des_t;


int load_groupDes_block(group_des_t groupDes,int fd,int groupDesNumber,FILE* bitMapStream,FILE* inodeStream, FILE* directoryStream){
	void* blockBuffer = malloc(s.blockSize);
	pread(fd,blockBuffer,s.blockSize,(s.firstBlock+1)*s.blockSize+(32*groupDesNumber));
	uint32_t* ptr4=blockBuffer;
	uint16_t* ptr2=blockBuffer;
	if((s.nBlocks)-groupDesNumber*(s.blocksPerGroup)>s.blocksPerGroup){
		groupDes->nBlocks=s.blocksPerGroup;	
	}
	else{
		groupDes->nBlocks=(s.nBlocks)-groupDesNumber*(s.blocksPerGroup);
	}

	if((s.nInodes)-groupDesNumber*(s.inodesPerGroup)>s.inodesPerGroup){
		groupDes->nInodes=s.inodesPerGroup;	
	}
	else{
		groupDes->nInodes=(s.nInodes)-groupDesNumber*(s.inodesPerGroup);
	}

	groupDes->nFreeBlocks=*(ptr2+6);
	groupDes->nFreeInodes=*(ptr2+7);
	groupDes->nDirs=*(ptr2+8);
	groupDes->inodeMapBlock=*(ptr4+1);
	groupDes->blockMapBlock=*(ptr4);
	groupDes->inodeTableBlock=*(ptr4+2);
	//go through free block and inode
	pread(fd,blockBuffer,s.blockSize,groupDes->blockMapBlock*s.blockSize);
	int i=0;
	uint8_t* ptr1=blockBuffer;
	for(i=0;i<groupDes->nBlocks;i=i+8){
		int j=0;
		int t=ptr1[i/8];
		//printf("every 8 bit of the block bit map: %d\n",t);
		for(j=0;j<8;j++){
			if((t&(1<<j))>0){

			}
			else{
				fprintf(bitMapStream, "%x,%d\n", groupDes->blockMapBlock,i+j+1+groupDesNumber*(s.blocksPerGroup));
			}
		}
	}
	pread(fd,blockBuffer,s.blockSize,groupDes->inodeMapBlock*s.blockSize);
	ptr1=blockBuffer;
	for(i=0;i<groupDes->nInodes;i=i+8){
		int j=0;
		int t=ptr1[i/8];
		//printf("every 8 bit of the block bit map: %d\n",t);
		for(j=0;j<8;j++){
			if((t&(1<<j))>0){
				//Loading question 4!!!
				//printf("number od inodes in bg:%d,inode number:%d",groupDes->nInodes,i+j+1);
				load_inode (&in,fd, groupDes->inodeTableBlock, i+j+1,groupDesNumber);
				print_inode (&in, inodeStream);
				/*
				*	if the inode is of a directory type traverse the block registered under that inode
				*/
				if (in.file_type=='d')
				{
					int k;
					//iteraate all the associated blocks belong to the inode
					for (k=0; k<in.nblocks; k++)
					{
						//containt the whole block 
						void* directory_entry_Buffer = malloc(s.blockSize);
						pread(fd,directory_entry_Buffer,s.blockSize,(in.block_ptr[k])*s.blockSize);
						//call a function to take in a block and print out the result
						print_directory_entry(directory_entry_Buffer, directoryStream);
						//printf("%d\n", k);
					}
				}
				
			}
			else{
				fprintf(bitMapStream, "%x,%d\n", groupDes->inodeMapBlock,i+j+1+groupDesNumber*(s.inodesPerGroup));
			}
		}
	}
	free(blockBuffer);
	return 0;
}

void print_groupDes_block(group_des_t groupDes,FILE* stream){
	fprintf(stream,"%d,%d,%d,%d,%x,%x,%x\n",
		groupDes->nBlocks,groupDes->nFreeBlocks,
		groupDes->nFreeInodes,groupDes->nDirs,
		groupDes->inodeMapBlock,groupDes->blockMapBlock,
		groupDes->inodeTableBlock);
}

void print_groupDes_wrapper(group_des_t groupDes,int fd){
	FILE* stream =fdopen(fd,"w");
	print_groupDes_block(groupDes,stream);
}


//-----------------------------------------------------------------------------------
// struct directory_entry
// {
// 	int paretn_inode_number;
// 	int entry_number;
// 	int entry_length;
// 	int name_length;
// 	int inode_number_of_file_entry;
// 	char* name;	
// };

// typedef struct directory_entry* directory_entry_t;



int print_directory_entry(void* directory_entry_Buffer, FILE* directoryStream)
{
	uint8_t* ptr_1 = (uint8_t*) directory_entry_Buffer;
	int i=0;
	int entryNumber=0;
	for (i=0; i<s.blockSize; )
	{
		//the size of the directory entry
		int size = *(uint16_t*)(ptr_1+4);
		int name_size = *(ptr_1+6);
		char* name = (char*) malloc (sizeof(char)* (name_size+1));
		int k=0;
		for (k; k<name_size; k++)
		{
			name[k]=*(ptr_1+8+k);
			//printf("%c\n",name[0] );
		}
		//printf("%d\n",name_size );
		name[name_size]='\0';

		//if (*(uint32_t*)(ptr_1)!=0)
		{

			fprintf(directoryStream, "%d %d %d %d %d \"%s\"\n",
					1,
					entryNumber,
					*(uint16_t*)(ptr_1+4),
					*(ptr_1+6),
					*(uint32_t*)(ptr_1),
					name);
			
		}
		ptr_1 = ptr_1+size;
		i=i+size;
		printf("i:%d size:%d\n",i, size );
		entryNumber++;

	}
}


//-----------------------------------------------------------------------------------
int main(){
	int fd=open("disk-image",O_RDONLY);
	printf("-----super.csv-----\n");
	int fd_super=creat("super.csv",S_IRUSR|S_IWUSR);
	load_super_block(&s,fd);
	print_super_block_wrapper(&s,fd_super);

	printf("-----group.csv-----\n");
	printf("-----bitmap.csv-----\n");
	printf("-----inode.csv-----\n");
	printf("-----directory.csv-----\n");
	int fd_groupDes=creat("group.csv",S_IRUSR|S_IWUSR);
	int fd_bitMap=creat("bitmap.csv",S_IRUSR|S_IWUSR);
	int fd_inode=creat("inode.csv",S_IRUSR|S_IWUSR);
	int fd_directory=creat("directory.csv",S_IRUSR|S_IWUSR);
	FILE* inodeStream =fdopen(fd_inode,"w");
	FILE* bitMapStream =fdopen(fd_bitMap,"w");
	FILE* directoryStream =fdopen(fd_directory,"w");
	int nBlockDes=s.nBlocks/s.blocksPerGroup;
	if(s.nBlocks%s.blocksPerGroup>0){
		nBlockDes++;
	}
	int i=0;
	for(i=0;i<nBlockDes;i++){
		load_groupDes_block(&gd,fd,i,bitMapStream,inodeStream,directoryStream );
		print_groupDes_wrapper(&gd,fd_groupDes);		
	}
	
	// printf("-----inode.csv-----\n");
	// int fd_inode=creat("inode.csv",S_IRUSR|S_IWUSR);
	// FILE* fd_inodeStream =fdopen(fd_inode,"w");
	// wraper_print_inodes(fd,fd_inodeStream);

}