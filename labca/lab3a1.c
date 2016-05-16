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