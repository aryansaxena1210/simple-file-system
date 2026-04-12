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

//------ your implementation -------
    return -1;
}


//open a file with RSFS_RDONLY or RSFS_RDWR flags
//return a file descriptor if succeed; 
//otherwise return a negative integer value
int RSFS_open(char file_name, int access_flag){
    
//------ your implementation -------
    return -1;
}



//append the content in buf to the end of the file of descriptor fd
//return the number of bytes actually appended to the file
int RSFS_append(int fd, void *buf, int size){
    
//------ your implementation -------
    return -1;

}



//update current position of the file (which is in the open_file_entry) to offset
//return -1 if fd is invalid; otherwise return the current position after the update
int RSFS_fseek(int fd, int offset){

//------ your implementation -------
    return -1;

}



//read up to size bytes to buf from file's current position towards the end
//return -1 if fd is invalid; otherwise return the number of bytes actually read
int RSFS_read(int fd, void *buf, int size){

//------ your implementation -------
    return -1;

}


//write the content of size (bytes) in buf to the file (of descripter fd) 
int RSFS_write(int fd, void *buf, int size){

//------ your implementation -------
    return -1;

}



//close file: return 0 if succeed; otherwise return -1
int RSFS_close(int fd){
    
//------ your implementation -------
    return -1;

}



//delete file
int RSFS_delete(char file_name){

//------ your implementation -------
    return -1;

}



//print status of the file system
// - current status of the file system: File Name, Length, iNode #
// - usage of data blocks: Total Data Blocks, Used, Unused
// - usage of inodes: Total iNode Blocks, Used, Unused
// - # of total opened files
void RSFS_stat(){

//------ your implementation -------

}
