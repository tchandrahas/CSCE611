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
} file_metadata_entry;

typedef struct superblock
{
  unsigned long num_free_blocks;
  unsigned long directory_entry; // points to the metadata of very first file created in this directory
  unsigned long disk_block_allocation_bitmap; // points to disk allocation bitmap
} superblock;

/*--------------------------------------------------------------------------*/
/* FORWARD DECLARATIONS */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* F i l e S y s t e m  */
/*--------------------------------------------------------------------------*/

class FileSystem {

friend class File; /* -- not sure if we need this; feel free to delete */

private:
     /* -- DEFINE YOUR FILE SYSTEM DATA STRUCTURES HERE. */

     SimpleDisk * disk;
     unsigned int size;
     // file system should know the location of superblock of the file system
     unsigned long superblock_disk_block_no;
     // List contaning all the  meta data mappings of all files placed on stack
     file_metadata_entry* file_metadata;

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

    File * LookupFile(int _file_id);
    /* Find file with given id in file system. If found, return the initialized
     file object. Otherwise, return null. */

    bool CreateFile(int _file_id);
    /* Create file with given id in the file system. If file exists already,
     abort and return false. Otherwise, return true. */

    bool DeleteFile(int _file_id);
    /* Delete file with given id in the file system; free any disk block occupied by the file. */

};
#endif
