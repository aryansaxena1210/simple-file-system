/*
    implementation of API
*/

#include "def.h"

pthread_mutex_t mutex_for_fs_stat;//mutex used by RSFS_stat()


//initialize file system - should be called as the first thing before accessing this file system 
int RSFS_init(){

    pthread_mutex_init(&mutex_for_fs_stat, NULL);
    pthread_mutex_init(&inode_bitmap_mutex, NULL);
    pthread_mutex_init(&inodes_mutex, NULL);
    pthread_mutex_init(&data_bitmap_mutex, NULL);
    pthread_mutex_init(&root_dir_mutex, NULL);
    pthread_mutex_init(&open_file_table_mutex, NULL);

    // initialize inode bitmap and inodes
    for(int i=0; i<NUM_INODES; i++){
        inode_bitmap[i] = 0;
    }

    for(int i=0; i<NUM_INODES; i++){
        for(int j=0; j<NUM_POINTERS; j++){
            inodes[i].block[j] = -1;
        }
        inodes[i].length = 0;
    }

    // initialize data bitmap and data blocks
    for(int i=0; i<NUM_DBLOCKS; i++){
        data_bitmap[i] = 0;
    }

    for(int i=0; i<NUM_DBLOCKS; i++){
        data_blocks[i] = malloc(BLOCK_SIZE);
        if(data_blocks[i] == NULL){
            printf("[RSFS_init] fail to allocate data block %d.\n", i);
            return -1;
        }
        memset(data_blocks[i], 0, BLOCK_SIZE);
    }

    // initialize open file table
    for(int i=0; i<NUM_OPEN_FILE; i++){
        open_file_table[i].used = 0;
        open_file_table[i].position = 0;
    }

    // allocate inode for root directory
    root_inode_number = allocate_inode(); //look inside bitmap, find first free location, initialize inode with that id, set bitmap of that lco to 1, return id.
    //to access the node again, do inodes[root_inode_number], and side note: bitmap[root_inode_number] if ofc ==1
    if(root_inode_number < 0){
        printf("[RSFS_init] fail to allocate root inode.\n");
        return -1;
    }

    return 0;
}


//create file
//if file does not exist, create the file and return 0;
//if file_name already exists, return -1; 
//otherwise (other errors), return -2.
int RSFS_create(char file_name){
    // check if file already exists
    struct dir_entry *entry = search_dir(file_name);
    if(entry != NULL) return -1;

    // allocate inode
    int inode_number = allocate_inode();
    if(inode_number < 0){
        printf("[create] fail to allocate an inode.\n");
        return -2;
    }

    // insert into directory
    entry = insert_dir(file_name, inode_number);
    if(entry == NULL){
        free_inode(inode_number);
        return -2;
    }

    return 0;
}


//open a file with RSFS_RDONLY or RSFS_RDWR flags
//return a file descriptor if succeed; 
//otherwise return a negative integer value
int RSFS_open(char file_name, int access_flag){
    // find the file in directory
    struct dir_entry *entry = search_dir(file_name);
    if(entry == NULL){
        printf("[RSFS_open] fail to find file with name: %c\n", file_name);
        return -1;
    }

    // allocate open file entry
    int fd = allocate_open_file_entry(access_flag, entry->inode_number);
    if(fd < 0) return -1;

    return fd;
}



//append the content in buf to the end of the file of descriptor fd
//return the number of bytes actually appended to the file
int RSFS_append(int fd, void *buf, int size){
    if(fd < 0 || fd >= NUM_OPEN_FILE) return -1;

    struct open_file_entry *entry = &open_file_table[fd];
    pthread_mutex_lock(&entry->entry_mutex);

    struct inode *inode = &inodes[entry->inode_number];
    int bytes_appended = 0;

    while(bytes_appended < size){
        // find which block and offset we are at
        int current_length = inode->length;
        int block_index = current_length / BLOCK_SIZE;
        int block_offset = current_length % BLOCK_SIZE;

        if(block_index >= NUM_POINTERS) break; // file is full

        // allocate a new block if needed
        if(inode->block[block_index] == -1){
            int block_number = allocate_data_block();
            if(block_number < 0) break;
            inode->block[block_index] = block_number;
        }

        // how many bytes can we write to this block
        int space_in_block = BLOCK_SIZE - block_offset;
        int bytes_to_write = size - bytes_appended;
        if(bytes_to_write > space_in_block) bytes_to_write = space_in_block;

        // copy data
        void *block_ptr = data_blocks[inode->block[block_index]];
        memcpy(block_ptr + block_offset, buf + bytes_appended, bytes_to_write);

        bytes_appended += bytes_to_write;
        inode->length += bytes_to_write;
    }

    pthread_mutex_unlock(&entry->entry_mutex);
    return bytes_appended;
}



//update current position of the file (which is in the open_file_entry) to offset
//return -1 if fd is invalid; otherwise return the current position after the update
int RSFS_fseek(int fd, int offset){
    if(fd < 0 || fd >= NUM_OPEN_FILE) return -1;

    struct open_file_entry *entry = &open_file_table[fd];
    pthread_mutex_lock(&entry->entry_mutex);

    struct inode *inode = &inodes[entry->inode_number];

    if(offset < 0 || offset >= inode->length){
        pthread_mutex_unlock(&entry->entry_mutex);
        return -1;
    }

    entry->position = offset;

    pthread_mutex_unlock(&entry->entry_mutex);
    return entry->position;
}



//read up to size bytes to buf from file's current position towards the end
//return -1 if fd is invalid; otherwise return the number of bytes actually read
int RSFS_read(int fd, void *buf, int size){
    if(fd < 0 || fd >= NUM_OPEN_FILE) return -1;

    struct open_file_entry *entry = &open_file_table[fd];
    pthread_mutex_lock(&entry->entry_mutex);

    struct inode *inode = &inodes[entry->inode_number];
    int bytes_read = 0;

    while(bytes_read < size && entry->position < inode->length){
        int block_index = entry->position / BLOCK_SIZE;
        int block_offset = entry->position % BLOCK_SIZE;

        int bytes_remaining_in_block = BLOCK_SIZE - block_offset;
        int bytes_remaining_in_file = inode->length - entry->position;
        int bytes_to_read = size - bytes_read;

        if(bytes_to_read > bytes_remaining_in_block) bytes_to_read = bytes_remaining_in_block;
        if(bytes_to_read > bytes_remaining_in_file) bytes_to_read = bytes_remaining_in_file;

        void *block_ptr = data_blocks[inode->block[block_index]];
        memcpy(buf + bytes_read, block_ptr + block_offset, bytes_to_read);

        bytes_read += bytes_to_read;
        entry->position += bytes_to_read;
    }

    pthread_mutex_unlock(&entry->entry_mutex);
    return bytes_read;
}


//write the content of size (bytes) in buf to the file (of descripter fd) 
int RSFS_write(int fd, void *buf, int size){
    if(fd < 0 || fd >= NUM_OPEN_FILE) return -1;

    struct open_file_entry *entry = &open_file_table[fd];
    pthread_mutex_lock(&entry->entry_mutex);

    struct inode *inode = &inodes[entry->inode_number];
    int bytes_written = 0;

    while(bytes_written < size){
        int block_index = entry->position / BLOCK_SIZE;
        int block_offset = entry->position % BLOCK_SIZE;

        if(block_index >= NUM_POINTERS) break;

        // allocate block if needed
        if(inode->block[block_index] == -1){
            int block_number = allocate_data_block();
            if(block_number < 0) break;
            inode->block[block_index] = block_number;
        }

        int space_in_block = BLOCK_SIZE - block_offset;
        int bytes_to_write = size - bytes_written;
        if(bytes_to_write > space_in_block) bytes_to_write = space_in_block;

        void *block_ptr = data_blocks[inode->block[block_index]];
        memcpy(block_ptr + block_offset, buf + bytes_written, bytes_to_write);

        bytes_written += bytes_to_write;
        entry->position += bytes_to_write;
    }

    // truncate file to current position if we wrote past the end
    if(entry->position > inode->length){
        inode->length = entry->position;
    } else {
        inode->length = entry->position;
    }

    pthread_mutex_unlock(&entry->entry_mutex);
    return bytes_written;
}

//delete file
int RSFS_close(int fd){
    if(fd < 0 || fd >= NUM_OPEN_FILE) return -1;

    free_open_file_entry(fd);
    return 0;
}

int RSFS_delete(char file_name){
    struct dir_entry *entry = search_dir(file_name);
    if(entry == NULL) return -1;

    int inode_number = entry->inode_number;
    struct inode *inode = &inodes[inode_number];

    // free all data blocks
    for(int i=0; i<NUM_POINTERS; i++){
        if(inode->block[i] != -1){
            free_data_block(inode->block[i]);
            inode->block[i] = -1;
        }
    }

    // free inode
    free_inode(inode_number);

    // remove directory entry
    delete_dir(file_name);

    return 0;
}

//print status of the file system
// - current status of the file system: File Name, Length, iNode #
// - usage of data blocks: Total Data Blocks, Used, Unused
// - usage of inodes: Total iNode Blocks, Used, Unused
// - # of total opened files
void RSFS_stat(){
    pthread_mutex_lock(&mutex_for_fs_stat);

    printf("\nCurrent status of the file system:\n\n");
    printf("\t%10s %8s %9s\n", "File Name", "Length", "iNode #");

    // print all files in directory
    for(int i=0; i<BLOCK_SIZE/sizeof(struct dir_entry); i++){
        struct dir_entry *entry = (struct dir_entry *)root_data_block + i;
        if(entry->name != 0){
            int inum = (unsigned char)entry->inode_number;
            printf("\t%10c %8d %9d\n", entry->name, inodes[inum].length, inum);
        }
    }

    // count data blocks
    int used_dblocks = 0;
    for(int i=0; i<NUM_DBLOCKS; i++){
        if(data_bitmap[i] == 1) used_dblocks++;
    }

    // count inodes
    int used_inodes = 0;
    for(int i=0; i<NUM_INODES; i++){
        if(inode_bitmap[i] == 1) used_inodes++;
    }

    // count open files
    int open_files = 0;
    for(int i=0; i<NUM_OPEN_FILE; i++){
        if(open_file_table[i].used == 1) open_files++;
    }

    printf("\nTotal Data Blocks: %4d,  Used: %d,  Unused: %d\n", NUM_DBLOCKS, used_dblocks, NUM_DBLOCKS-used_dblocks);
    printf("Total iNode Blocks: %3d,  Used: %d,  Unused: %d\n", NUM_INODES, used_inodes, NUM_INODES-used_inodes);
    printf("Total Opened Files: %3d\n", open_files);

    pthread_mutex_unlock(&mutex_for_fs_stat);
}