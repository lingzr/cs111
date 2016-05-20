#!/usr/bin/python

# Open csv files output from the labca.c
inode_file = open("inode.csv", "r+")
bitmap_file = open("bitmap.csv", "r+")
group_file = open("group.csv", "r+")
super_file = open("super.csv", "r+")
directory_file = open("directory.csv", "r+")
indirect_file = open("indirect.csv", "r+")

#convert files to lists
#each index contains a line of the file
inode_list = inode_file.readlines()
bitmap_list = bitmap_file.readlines()
group_list = group_file.readlines()
super_list = super_file.readlines()
directory_list = directory_file.readlines()
indirect_list = indirect_file.readlines()

#7 functions to print out the report
def unallocated_block():
	
	#print used_block_list
	block_bit_map_num = []
	for line in group_list:
		block_bit_map_num.append(line.split(',')[5])
	
	free_blocks=[]
	for line in bitmap_list:
		if line.split(',')[0] in block_bit_map_num:
			free_blocks.append(int(line.split(',')[1]))


	entry_num=-1		
	for line in inode_list:
		used_block_list = line.split(',')[11:22]
		entry_num=-1
		for item in used_block_list:
			entry_num=entry_num+1
			#print int(item,16)
			if int(item,16) in free_blocks:
				print 'UNALLOCATED BLOCK < %d > REFERENCED BY INODE < %d > ENTRY < %d >' % (int(item,16), int(line.split(',')[0]), int(entry_num))


	return


def duplicately_allocated_block():
	return

def unallocated_inode():
	return

def missing_inode():
	return

def incorrect_link_count():
	return

def incorrect_directory_entry():
	return

def invalid_block_pointer():
	return


#call the seven functions

unallocated_block()
duplicately_allocated_block()
unallocated_inode()
missing_inode()
incorrect_link_count()
incorrect_directory_entry()
invalid_block_pointer()




# Close opend file
inode_file.close()
bitmap_file.close() 
group_file.close() 
super_file.close() 
directory_file.close() 
indirect_file.close() 



