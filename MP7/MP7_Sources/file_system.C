/*
     File        : file_system.C

     Author      : Riccardo Bettati
     Modified    : 2017/05/01

     Description : Implementation of simple File System class.
                   Has support for numerical file identifiers.
 */

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

    /* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "assert.H"
#include "console.H"
#include "file_system.H"


/*--------------------------------------------------------------------------*/
/* CONSTRUCTOR */
/*--------------------------------------------------------------------------*/

FileSystem::FileSystem()
{
  file_metadata = NULL;
  Console::puts("Constructed the File System succesfully, but it is empty now..\n");
}

/*--------------------------------------------------------------------------*/
/* FILE SYSTEM FUNCTIONS */
/*--------------------------------------------------------------------------*/

bool FileSystem::Mount(SimpleDisk * _disk)
{
  Console::puts("mounting file system form disk...");
  // Do check if the disk pointer is null
  if(_disk == NULL)
  {
    Console::puts("\nDisk pointer is found to be NULL\n");
    return false;
  }
  if(disk != NULL)
  {
    // a disk is already mounted, so return  false
    return false;
  }
  disk = _disk;
  Conosle::puts("Now writing the disk with desired data structures..\n");
  // declare temporary data structures
  char temp_buffer[512];
  unsigned int i;
  for(i=0;i<512;i++)
  {
    temp_buffer[i] = 0xFF;
  }
  // fill the block 0 with all 1's
  // this is because we are going to use 0 as an invalid condition
  disk->write(0,temp_buffer);
  // prepare the initial superblock for writing
  superblock temp_superblock[0];
  // prepare the disk allocation bitmap for writing
  char temp_disk_block_allocation_bitmap[512];
  char temp_directory_entry[512];
  for(i=0;i<512;i++)
  {
    temp_disk_block_allocation_bitmap[i] = 0x00;
    temp_directory_entry[i]= 0x00;
  }
  // below number signifies that 0,1 and 2 are occupied
  temp_disk_block_allocation_bitmap[0] = 0x0F;
  // write the disk block at block number 2 of format disk
  disk->write(2,temp_disk_block_allocation_bitmap);
  // update this number in our superblock
  temp_superblock->num_free_blocks = num_512_writes - 4;
  temp_superblock->disk_block_allocation_bitmap = 2;
  temp_superblock->directory_entry = 3;
  // Write 0 in the directory entry
  disk->write(3,temp_directory_entry);
  // Finally write the superblock to disk and save the superblock disk number as a member of the file system
  disk->write(1,(char*)superblock);
  superblock_disk_block_no = 1;
  // Hopefully this sufficient for mounting
  Console::puts("done mounting our file system on the disk\n");
  return true;
}

bool FileSystem::Format(SimpleDisk * _disk, unsigned int _size)
{
  Console::puts("formatting disk..");Console::puts("for a file system of size ");Console::putui(_size);Console::puts("\n");
  // buffer the inputs
  SimpleDisk* format_disk = _disk;
  unsigned int formating_size = _size;
  // Write zero's to the disk until the given size, given by the number of 512 bytes(block size ?)
  // by default we will assume that block size is of 512 bytes
  unsigned int num_512_writes = (_size/512)+(((_size%512)>0)?1:0);
  unsigned int num_blocks_format_disk = num_512_writes;
  unsigned int i;
  // Just some print statement
  Console::puts("No.of disk blocks to be formatted for this file-system is ");Console::putui(num_512_bytes);Console::puts("\n");
  // declare the zero buffer and fill it with zero's
  char zero_buffer[512];
  for(i=0;i<512;i++)
  {
    zero_buffer[i] = 0x00;
  }
  // Write this 512 byte blocks to disk, desired number of times
  for(i=0;i<num_512_writes;i++)
  {
    format_disk->write(i,zero_buffer);
  }
  Console::puts("Done cleaning the disk..\n");
  return true;
}

File * FileSystem::LookupFile(int _file_id)
{
  Console::puts("looking up file..\n");
  // traverse through our data structure with a file pointer
  file_metadata_entry* traverse_file_metadata = NULL;
  traverse_file_metadata = file_metadata;
  while(traverse_file_metadata != NULL)
  {
    // if file pointer with the required file_id is found return it
    if(traverse_file_metadata->file_id == _file_id)
    {
      Console::puts("File Lookup succesful for file_id ");Console::putui(_file_id);Console::puts("\n");
      return traverse_file_metadata->file_ptr;
    }
    // if it is not found continue your search until you reach the end
    else
    {
      traverse_file_metadata = traverse_file_metadata->next_file_metadata_entry;
    }
  }
  // file is not found in our list of files stored by our file system, so return NULL
  return NULL;
}

bool FileSystem::CreateFile(int _file_id)
{
    Console::puts("creating file..\n");
    // get the superblock from the file system
    superblock temp_superblock[0];
    unsigned long temp_file_metadata_block_no;
    unsigned long temp_file_index_block_no;
    unsigned long temp_directory_entry;
    file_metadata temp_file_metadata[0];
    disk->read(superblock_disk_block_no,(char*)temp_superblock);
    Console::puts("Read Superblock from the file system..It has ");Console::putui(temp_superblock->num_free_blocks);Console::puts(" free blocks in it's file system");
    // Get the directory entry to create a new entry for file meta-data
    unsigned int temp_directory_entry = temp_superblock->directory_entry;
    if(temp_directory_entry == 0)
    {
      Console::puts("The directory entry is zero, This file creation would populate it\n");
      // get  a free block from the disk to write the newly created file data
      temp_file_metadata_block_no = FileSystem::get_free_block_no(disk,temp_superblock->disk_block_allocation_bitmap);
      temp_file_index_block_no = FileSystem::get_free_block_no(disk,temp_superblock->disk_block_allocation_bitmap);
      // populate the entries of temporary file metadata entry
      temp_file_metadata->file_id = _file_id;
      temp_file_metatdata->next_file_metadata_entry = 0;
      temp_file_metadata->index_block_no = temp_file_index_block_no;
      // Write the meta data block to the disk
      disk->write(temp_file_metadata_block_no,(char*)temp_file_metadata);
      return true;
    }
    else
    {
      // files are created previously in this directory
      temp_directory_entry = temp_superblock->directory_entry;
      Console::puts("The directory entry was found to be ");Console::putui();Console::puts("\n");
      disk->read(temp_directory_entry,(char*)temp_file_metadata);
      Console::puts("looking for a spot for empty file insertion...\n")
      while(temp_file_metadata->next_file_metadata_entry != 0)
      {
        Console::puts("Looking for last file created in our directory...\n");
        disk->read(temp_file_metadata->next_file_metadata_entry,(char*)temp_file_metadata);
      }
      Console::puts("We found the last file in our directory..\n");
      // get free blocks for block metadata and index blocks
      temp_file_metadata_block_no = get_free_block_no(disk,temp_superblock->disk_block_allocation_bitmap);
      temp_file_index_block_no = get_free_block_no(disk,temp_superblock->disk_block_allocation_bitmap);
      // populate the fields of temp_file_metadata
      temp_file_metadata->file_id = _file_id;
      temp_file_metadat->next_file_metadata_entry = 0;
      temp_file_metadata->index_block_no = temp_file_index_block_no;
      // write the metdata block to the disk
      disk->write(temp_file_metadata_block_no,(char*)temp_file_metadata);
      return true;
    }
    // traverse through the file meat data strcture to find an empty spot
    /*file_metadata_entry* traverse_file_metadata = NULL;
    file_metadata_entry* previous_file_metadata_entry = NULL;
    traverse_file_metadata = file_metadata;
    while(traverse_file_metadata != NULL)
    {
      previous_file_metadata_entry = traverser_file_metadata;
      traverse_file_metadata = traverse_file_metadata->next_file_metadata_entry;
    }
    // We have reached a NULL pointer so we can insert our new entry here
    if(previous_file_metadata_entry == NULL)
    {
      // first entry in the meta data linked list
      file_metadata = new file_metadata_entry;
      file_metadata->next_file_metadata_entry = NULL;
      file_metadata->file_id = _file_id;
      file_metadata->file_ptr = new file;
      Console::puts("File with file_id ");Console::putui(_file_id);Console::puts("created as first element of our metadata linked-list\n");
      return true;
    }
    else
    {
      traverse_file_metadata = new file_metadata_entry;
      previous_file_metadata_entry->next_file_metadata_entry = traverse_file_metadata;
      traverse_file_metadata->next_file_metadata_entry = NULL;
      traverse_file_metadata->file_id = _file_id;
      traverse_file_metadata->file_ptr = new file;
      Console::puts("File with file_id ");Console::putui(_file_id);Console::puts("attached to file with id ");Console::putui(previous_file_metadata_entry->file_id);Console::puts(" in our file metadata linked list\n");
      return true;
    }*/
    return false;
}

bool FileSystem::DeleteFile(int _file_id)
{
    Console::puts("deleting file..\n");
    // First look up for required id in our file metadata linked list
    // traverse through the file meat data strcture to find an empty spot
    file_metadata_entry* traverse_file_metadata = NULL;
    file_metadata_entry* previous_file_metadata_entry = NULL;
    traverse_file_metadata = file_metadata;
    while(traverse_file_metadata != NULL)
    {
      if(traverse_file_metadata->file_id == _file_id)
      {
        if(previous_file_metadata_entry == NULL)
        {
          // the file to be removed is the head of our metadata linked-list
          file_metadata = file_metadata->next_file_metadata_entry;
          Console::puts("File with file_id ");Console::putui(_file_id);Console::puts("removed from top of our metadata linked-list\n");
          delete traverse_file_metadata;
          return true;
        }
        else
        {
          previous_file_metadata_entry->next_file_metadata_entry = traverse_file_metadata->next_file_metadata_entry;
          Console::puts("File with file_id ");Console::putui(_file_id);Console::puts("removed from our metadata linked-list at ");Console::putui(previous_file_metadata_entry-.file_id);Console::puts("\n");
          delete traverse_file_metadata;
          return true;
        }
      }
      else
      {
        // else continue the search
        previous_file_metadata_entry = traverser_file_metadata;
        traverse_file_metadata = traverse_file_metadata->next_file_metadata_entry;
      }
    }
    return false;
}

unsigned long get_free_block_no(SimpleDisk* input_disk,unsigned long disk_allocation_bitmap_block_no)
{
  // declare the intermediate variables for our program
  char temp_disk_allocation_bitmap[512];
  unsigned int temp_block_number = 0;
  // read the disk allocation bitmap from the disk
  input_disk->read(disk_allocation_bitmap_block_no,temp_disk_allocation_bitmap);
  // traverse till we find a free block in the disk
  while((temp_disk_allocation_bitmap[temp_block_number/8] & ((0x01)<< temp_block_number%8)))
  {
    Console::puts("Block ");Console::putui(temp_block_number);Console::putui("is not empty\n");
    if(temp_block_number == 1024)
    {
      // just to prevent it from going into an infinite loop
      break;
    }
  }
  Console::puts("Block ");Console::putui(temp_block_number);Console::putui("is empty\n");
  // mark the free block as occupied
  temp_disk_allocation_bitmap[temp_block_number/8] = (temp_disk_allocation_bitmap[temp_block_number/8]|(0x01 << (temp_block_number%8)));
  // write back the updated block map information to disk
  input_disk->write(temp_block_number, temp_disk_allocation_bitmap);
  return temp_block_number;
}
