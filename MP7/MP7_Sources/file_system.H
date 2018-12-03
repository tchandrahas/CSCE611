/*
    File: file_system.H

    Author: R. Bettati
            Department of Computer Science
            Texas A&M University
    Date  : 10/04/05

    Description: Simple File System.


*/

#ifndef _FILE_SYSTEM_H_                   // include file only once
#define _FILE_SYSTEM_H_

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "file.H"
#include "simple_disk.H"

/*--------------------------------------------------------------------------*/
/* DATA STRUCTURES */
/*--------------------------------------------------------------------------*/

typedef struct file_metadata_entry
{
  int file_id;
  unsigned long index_block_no;
  unsigned long next_file_metadata_entry;
  unsigned long num_blocks_allocated;
} file_metadata_entry;

typedef struct superblock
{
  unsigned long num_free_blocks;
  unsigned long directory_entry; // points to the metadata of very first file created in this directory
  unsigned long disk_block_allocation_bitmap; // points to disk allocation bitmap
} superblock;

typedef struct created_file
{
  int file_id; // file id of created file
  struct File* file_ptr; // file pointer of created file
  struct created_file* next_created_file; // pointer to next created file, which will be this struct again
}created_file;
/*--------------------------------------------------------------------------*/
/* FORWARD DECLARATIONS */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* F i l e S y s t e m  */
/*--------------------------------------------------------------------------*/

class FileSystem {

//

private:
     /* -- DEFINE YOUR FILE SYSTEM DATA STRUCTURES HERE. */

     SimpleDisk * disk;
     unsigned int size;
     // file system should know the location of superblock of the file system
     unsigned long superblock_disk_block_no; // give the default value as zero
     // List contaning all the created file ids's and file pointers for those created files
     created_file* created_files; // allocate NULL in constructor

     unsigned long get_free_block_no(SimpleDisk* input_disk,unsigned long disk_allocation_bitmap_block_no);
     void set_block_free(SimpleDisk* disk, unsigned long _block_no);

public:

    FileSystem();
    /* Just initializes local data structures. Does not connect to disk yet. */

    bool Mount(SimpleDisk * _disk);
    /* Associates this file system with a disk. Limit to at most one file system per disk.
     Returns true if operation successful (i.e. there is indeed a file system on the disk.) */

    static bool Format(SimpleDisk * _disk, unsigned int _size);
    /* Wipes any file system from the disk and installs an empty file system of given size. */

    struct File* LookupFile(int _file_id);
    /* Find file with given id in file system. If found, return the initialized
     file object. Otherwise, return null. */

    bool CreateFile(int _file_id);
    /* Create file with given id in the file system. If file exists already,
     abort and return false. Otherwise, return true. */

    bool DeleteFile(int _file_id);
    /* Delete file with given id in the file system; free any disk block occupied by the file. */

    bool request_disk_block(unsigned long _metadata_block_no);

    bool data_block_read(unsigned char* _read_buffer,unsigned long _metadata_block_no);

    bool data_block_write(unsigned char* _write_buffer, unsigned long metadata_block_no);

    bool erase_data_block(unsigned long _metadata_block_no);
};
#endif
