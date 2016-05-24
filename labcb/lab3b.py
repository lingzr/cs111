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



	# for free_block in free_blocks:
	# 	print free_block
	# 	for inode in inode_list:
	# 		used_block_list = inode.split(',')[11:22]
	# 		indirectly_used_blocks = get_indirectly_used_blocks_of_one_node(inode)
	# 		if free_block in used_block_list or free_block in indirectly_used_blocks:
	# 			print 'UNALLOCATED BLOCK < %d >  REFERENCED BY ' % (free_block)
	# 			if free_block in used_block_list:
	# 				print 'INODE < %d > ENTRY < %d >' %(int(inode.split(',')[0]), inode.split(',')[11:22].index(free_block))
	# 			else:
	# 				index=indirectly_used_blocks.index(free_block)
	# 				print 'INDIRECT BLOCK < %d > ENTRY < %d >' %(indirectly_used_blocks[index].split(',')[0], indirectly_used_blocks[index].split(',')[1])




	entry_num=-1		
	for line in inode_list:
		used_block_list = line.split(',')[11:22]
		indirectly_used_blocks = get_indirectly_used_blocks_of_one_node(line)
		entry_num=-1
		for item in used_block_list:
			entry_num=entry_num+1
			#print int(item,16)
			if int(item,16) in free_blocks:
				print 'UNALLOCATED BLOCK < %d > REFERENCED BY INODE < %d > ENTRY < %d >' % (int(item,16), int(line.split(',')[0]), int(entry_num))

	return


def get_indirectly_used_blocks_of_one_node(inode_line):
	
	singly_block_ptr = []
	doubly_block_ptr = []
	triply_block_ptr = []
	singly_block_ptr.append(inode_line.split(',')[23])
	doubly_block_ptr .append( inode_line.split(',')[24])
	triply_block_ptr .append( inode_line.split(',')[25])

	indirectly_used_blocks = []

	for line in indirect_list:
		if line.split(',')[0] in  triply_block_ptr:
			doubly_block_ptr.append(line.split(',')[2])
			indirectly_used_blocks.append(line)

	for line in indirect_list:
		if line.split(',')[0] in doubly_block_ptr:
			singly_block_ptr.append(line.split(',')[2])
			indirectly_used_blocks.append(line)

	for line in indirect_list:
		if line.split(',')[0] in singly_block_ptr:
			indirectly_used_blocks.append(line)

	return indirectly_used_blocks




#find out block used by multiple inode
def duplicately_allocated_block():
	dic = {}
	#to save all used blocks with their inode number, entry numbrt to a dictionary
	for index in range(len(inode_list)):
		#inode_list[index]get the current inode 11-22 is the block number
		entry_num=0
		for block_num in inode_list[index].split(',')[11:23]:
			#the block num is unique so far
			if int(block_num, 16)==0:
				break
			elif int(block_num, 16) not in dic:
				dic[int(block_num, 16)]=[[int(inode_list[index].split(',')[0]), entry_num]]
			else:
				dic[int(block_num, 16)].append([int(inode_list[index].split(',')[0]), entry_num])

			entry_num=entry_num+1
	#go through the dictionary, if the associated array has more than one entry, print them out.

	for key in dic:
		output = ''
		if len(dic[key])>1:
			#print 'MULTIPLY REFERENCED BLOCK < %d > BY'%(key) 
			output=output+'MULTIPLY REFERENCED BLOCK < %d > BY '%(key) 
			for item in dic[key]:
				#print 'INODE < %d > ENTRY < %d > '%(item[0], item[1])
				output = output+'INODE < %d > ENTRY < %d > '%(item[0], item[1])
			print output
		
	return

def unallocated_inode():
	#store all the unallocated inode {block_num:[[parent_num, entry_num],[parent_num, entry_num]]}
	dick={}
	#get all the inode number from the list
	inode_num = []
	for inode in inode_list:
		inode_num.append(int(inode.split(',')[0]))
	#print inode_num
	#go through all the directory
	for directory in directory_list:
		#if a inode used by directory is not in inode table
		#print directory.split(',')[4]
		if len(directory.split(',')) == 6 and (int(directory.split(',')[4]) not in inode_num):
			#if not put in the dick
			if int (directory.split(',')[4]) not in dick:
				dick[int(directory.split(',')[4])]=[[int(directory.split(',')[0]), int(directory.split(',')[1])]]
			else:
				dick[int(directory.split(',')[4])].append([int(directory.split(',')[0]), int(directory.split(',')[1])])

	#print out
	for key in dick:
		output=''
		if len(dick[key])>1:
			output=output+'UNALLOCATED INODE < %d > REFERENCED BY '%(key)
			for item in dick[key]:
				output=output+'DIRECTORY < %d > ENTRY < %d >'%(item[0], item[1])
			print output
		else:
			print 'UNALLOCATED INODE < %d > REFERENCED BY DIRECTORY < %d > ENTRY < %d >'%(key, dick[key][0][0], dick[key][0][1])

	return

def missing_inode():
	#free inode list
	inode_bit_map_num = []
	for line in group_list:
		inode_bit_map_num.append(int (line.split(',')[4]))

	
	free_inodes=[]
	for line in bitmap_list:
		if int(line.split(',')[0]) in inode_bit_map_num:
			free_inodes.append(int(line.split(',')[1]))
	

	used_inodes=[]
	for inode in directory_list:
		if len(inode.split(','))== 6:
			used_inodes.append(int (inode.split(',')[4]))

	total_num_of_inode = super_list[0].split(',')[1]

	#print used_inodes
	#print free_inodes


	for inode_num in range(int (total_num_of_inode)):
		if inode_num not in used_inodes and inode_num not in free_inodes:
			free_list_num =0
			for item in bitmap_list:
				if inode_num>=int(item.split(',')[1]):
					free_list_num = int (item.split(',')[0])
			if (inode_num>10):
				print 'MISSING INODE < %d > SHOULD BE IN FREE LIST < %d >'%(inode_num, free_list_num )

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



