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
  // initialize the varible fields to default values
  created_files = NULL;
  superblock_disk_block_no = 0;
  disk = NULL;
  size = 0;
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
  disk = _disk;
  Console::puts("Now writing the disk with desired data structures..\n");
  // declare temporary data structures
  char temp_buffer[512];
  unsigned int i;
  for(i=0;i<512;i++)
  {
    temp_buffer[i] = 0xFF;
  }
  // fill the block 0 with all 1's
  // this is because we are going to use 0 as an invalid condition
  disk->write(0,(unsigned char*)temp_buffer);
  // prepare the initial superblock for writing
  superblock temp_superblock[0];
  // prepare the disk allocation bitmap for writing
  unsigned char temp_disk_block_allocation_bitmap[512];
  unsigned char temp_directory_entry[512];
  for(i=0;i<512;i++)
  {
    temp_disk_block_allocation_bitmap[i] = 0x00;
    temp_directory_entry[i]= 0x00;
  }
  // below number signifies that 0,1 and 2 are occupied
  temp_disk_block_allocation_bitmap[0] = 0x07;
  // write the disk block at block number 2 of format disk
  disk->write(2,temp_disk_block_allocation_bitmap);
  // update this number in our superblock
  temp_superblock->num_free_blocks = 2048  - 3;
  temp_superblock->disk_block_allocation_bitmap = 2;
  temp_superblock->directory_entry = 0;
  // Finally write the superblock to disk and save the superblock disk number as a member of the file system
  disk->write(1,(unsigned char*)temp_superblock);
  superblock_disk_block_no = 1;
  created_files = NULL;
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
  //size = _size;
  // Write zero's to the disk until the given size, given by the number of 512 bytes(block size ?)
  // by default we will assume that block size is of 512 bytes
  unsigned int num_512_writes = (_size/512)+(((_size%512)>0)?1:0);
  unsigned int num_blocks_format_disk = num_512_writes;
  unsigned int i;
  // Just some print statement
  Console::puts("No.of disk blocks to be formatted for this file-system is ");Console::putui(num_512_writes);Console::puts("\n");
  // declare the zero buffer and fill it with zero's
  unsigned char* zero_buffer = new unsigned char[512];
  for(i=0;i<512;i++)
  {
    zero_buffer[i] = 0x00;
  }
  // Write this 512 byte blocks to disk, desired number of times
  for(i=0;i<num_512_writes;i++)
  {
    format_disk->write((unsigned long)i,zero_buffer);
  }
  delete zero_buffer;
  Console::puts("Done cleaning the disk..\n");
  return true;
}

File * FileSystem::LookupFile(int _file_id)
{
  Console::puts("looking up file..\n");
  // traverse through our data structure with a file pointer
  created_file* traverse_file = NULL;
  traverse_file = created_files;
  while(traverse_file!= NULL)
  {
    // if file pointer with the required file_id is found return it
    if(traverse_file->file_id == _file_id)
    {
      Console::puts("File Lookup succesful for file_id ");Console::putui(_file_id);Console::puts("\n");
      return traverse_file->file_ptr;
    }
    // if it is not found continue your search until you reach the end
    else
    {
      traverse_file = traverse_file->next_created_file;
    }
  }
  // file is not found in our list of files stored by our file system, so return NULL
  return NULL;
}

bool FileSystem::CreateFile(int _file_id)
{
    int* input_file_id = new int;
    *input_file_id = _file_id;
    Console::puts("creating file..");Console::puts("with file_id ");Console::putui(*input_file_id);Console::puts("\n");
    // get the superblock from the file system
    superblock* temp_superblock = new superblock;
    unsigned long new_file_metadata_block_no;
    unsigned long temp_file_metadata_block_no;
    unsigned long new_file_index_block_no;
    unsigned long temp_directory_entry;
    file_metadata_entry* new_file_metadata = new file_metadata_entry;
    file_metadata_entry* temp_file_metadata = new file_metadata_entry;
    File* new_file;
    created_file* new_created_file;
    disk->read((unsigned long)superblock_disk_block_no,(unsigned char*)temp_superblock);
    Console::puts("Superblock of this file system is expected to be at ");Console::putui(superblock_disk_block_no);Console::puts("\n");
    Console::puts("Superblock of this file system is expected to be at ");Console::putui(superblock_disk_block_no);Console::puts("\n");
    Console::puts("Read Superblock from the file system..It has ");Console::putui(temp_superblock->num_free_blocks);Console::puts(" free blocks in it's file system");Console::puts("Disk Block allocation map located at ");Console::putui(temp_superblock->disk_block_allocation_bitmap);Console::puts("Directory Entry at ");Console::putui(temp_superblock->directory_entry);Console::puts("\n");
    // Get the directory entry to create a new entry for file meta-data
    temp_directory_entry = temp_superblock->directory_entry;
    Console::puts("The directory entry is found to be ");Console::putui(temp_directory_entry);Console::puts("\n");
    if(temp_directory_entry  == 0)
    {
      Console::puts("The directory entry is zero, This file creation would populate it\n");
      // get  a free block from the disk to write the newly created file data
      new_file_metadata_block_no = FileSystem::get_free_block_no(disk,temp_superblock->disk_block_allocation_bitmap);
      new_file_index_block_no = FileSystem::get_free_block_no(disk,temp_superblock->disk_block_allocation_bitmap);
      Console::puts("The metadata block number of new file is");Console::putui(new_file_metadata_block_no);Console::puts("\n");
      Console::puts("The index block number of new file is");Console::putui(new_file_index_block_no);Console::puts("\n");
      // populate the entries of temporary file metadata entry
      new_file_metadata->file_id = _file_id;
      new_file_metadata->next_file_metadata_entry = 0;
      new_file_metadata->index_block_no = new_file_index_block_no;
      // Write the meta data block to the disk
      disk->write(new_file_metadata_block_no,(unsigned char*)new_file_metadata);
      // update the directory entry and number of free blocks information in the super block
      temp_superblock->directory_entry = new_file_metadata_block_no;
      temp_superblock->num_free_blocks--;
      temp_superblock->num_free_blocks--;
      Console::puts("Superblock of this file system is expected to be at ");Console::putui(superblock_disk_block_no);Console::puts("\n");
      disk->write(superblock_disk_block_no,(unsigned char*)temp_superblock);
      Console::puts("Wrote the updated superblock information to the disk..\n");
    }
    else
    {
      Console::puts("The directory entry was found to be ");Console::putui(temp_directory_entry);Console::puts("\n");
      disk->read((unsigned long)temp_directory_entry,(unsigned char*)temp_file_metadata);
      Console::puts("looking for a spot for empty file insertion...\n");
      temp_file_metadata_block_no = temp_directory_entry;
      while(temp_file_metadata->next_file_metadata_entry != 0)
      {
        Console::puts("Looking for last file created in our directory...\n");
        temp_file_metadata_block_no = temp_file_metadata->next_file_metadata_entry;
        disk->read(temp_file_metadata->next_file_metadata_entry,(unsigned char*)temp_file_metadata);
      }
      Console::puts("We found the last file in our directory..\n");
      // get free blocks for block metadata and index blocks
      new_file_metadata_block_no = get_free_block_no(disk,temp_superblock->disk_block_allocation_bitmap);
      new_file_index_block_no = get_free_block_no(disk,temp_superblock->disk_block_allocation_bitmap);
      Console::puts("The metadata block number of new file is");Console::putui(new_file_metadata_block_no);Console::puts("\n");
      Console::puts("The index block number of new file is");Console::putui(new_file_index_block_no);Console::puts("\n");
      // populate the fields of temp_file_metadata
      new_file_metadata->file_id = _file_id;
      new_file_metadata->next_file_metadata_entry = 0;
      new_file_metadata->index_block_no = new_file_index_block_no;
      new_file_metadata->num_blocks_allocated = 0;
      // link the new file metadata to old file metadata
      temp_file_metadata->next_file_metadata_entry = new_file_metadata_block_no;
      // write the new metdata block to the disk
      disk->write(new_file_metadata_block_no,(unsigned char*)new_file_metadata);
      // write the previous metadata block to disk
      disk->write(temp_file_metadata_block_no,(unsigned char*)temp_file_metadata);
      Console::puts("Written the newly created file metadata to the disk\n");
      Console::puts("Written a empty index block for newly created file to the disk\n");
      temp_superblock->num_free_blocks--;
      temp_superblock->num_free_blocks--;
      Console::puts("Superblock of this file system is expected to be at ");Console::putui(superblock_disk_block_no);Console::puts("\n");
      disk->write(superblock_disk_block_no,(unsigned char*)temp_superblock);
      Console::puts("Wrote the updated superblock information to the disk..\n");
    }
    // traverse through the created files strcture to find an empty spot
    Console::puts("Checking already created files\n");
    created_file* traverse_file = NULL;
    created_file* previous_file = NULL;
    traverse_file = (created_file*) created_files;
    while(traverse_file!=NULL)
    {
      previous_file = traverse_file;
      traverse_file = traverse_file->next_created_file;
      Console::puts("Traversing through the created files..\n");
    }
    // We have reached a NULL pointer so we can insert our new file here
    if(previous_file == NULL)
    {
      // first entry in the created files linked list
      Console::puts("Creating a new file..\n");
      new_file = new File(new_file_metadata_block_no);
      new_created_file = new created_file;
      new_created_file->next_created_file = NULL;
      new_created_file->file_id = *input_file_id;
      new_created_file->file_ptr = new_file;
      created_files = new_created_file;
      Console::puts("File with file_id ");Console::putui(new_created_file->file_id);Console::puts("created as first element of our created-files linked-list\n");
      delete temp_superblock;
      return true;
    }
    else
    {
      new_file = new File(new_file_metadata_block_no);
      new_created_file = new created_file;
      previous_file->next_created_file = new_created_file;
      new_created_file->next_created_file= NULL;
      new_created_file->file_id = *input_file_id;
      Console::puts("New file creation is for file with file_id ");Console::putui(_file_id);Console::puts("\n");
      new_created_file->file_ptr = new_file;
      Console::puts("File with file_id ");Console::putui(new_created_file->file_id);Console::puts("attached to file with id ");Console::putui(previous_file->file_id);Console::puts(" in our file created-files linked list\n");
      delete temp_superblock;
      return true;
    }
    return false;
}

bool FileSystem::DeleteFile(int _file_id)
{
    Console::puts("deleting file..\n");
    // First look up for required id in our file created-files linked list
    // traverse through the file meat data strcture to find an empty spot
    created_file* traverse_file = NULL;
    created_file* previous_file = NULL;
    traverse_file = created_files;
    while(traverse_file != NULL)
    {
      if(traverse_file->file_id == _file_id)
      {
        if(previous_file == NULL)
        {
          // the file to be removed is the head of our metadata linked-list
          created_files = created_files->next_created_file;
          Console::puts("File with file_id ");Console::putui(_file_id);Console::puts("removed from top of our created-files linked-list\n");
          delete traverse_file;
          // sufficient for now, but disk cleanup needed
          return true;
        }
        else
        {
          previous_file->next_created_file = traverse_file->next_created_file;
          Console::puts("File with file_id ");Console::putui(_file_id);Console::puts("removed from our created-files linked-list at ");Console::putui(previous_file->file_id);Console::puts("\n");
          delete traverse_file;
          // sufficient for now, but disk clean up needed
          return true;
        }
      }
      else
      {
        // else continue the search
        previous_file = traverse_file;
        traverse_file = traverse_file->next_created_file;
      }
    }
    return false;
}

unsigned long FileSystem::get_free_block_no(SimpleDisk* input_disk,unsigned long disk_allocation_bitmap_block_no)
{
  // declare the intermediate variables for our program
  unsigned char temp_disk_allocation_bitmap[512];
  unsigned int temp_block_number = 0;
  // read the disk allocation bitmap from the disk
  Console::puts("Superblock of this file system is expected to be at ");Console::putui(superblock_disk_block_no);Console::puts("\n");
  input_disk->read(disk_allocation_bitmap_block_no,temp_disk_allocation_bitmap);
  // traverse till we find a free block in the disk
  while((temp_disk_allocation_bitmap[temp_block_number/8] & ((0x01)<< temp_block_number%8)))
  {
    Console::puts("Block ");Console::putui(temp_block_number);Console::puts("is not empty\n");
    temp_block_number++;
  }
  Console::puts("Block ");Console::putui(temp_block_number);Console::puts("is empty\n");
  // mark the free block as occupied
  temp_disk_allocation_bitmap[temp_block_number/8] = (temp_disk_allocation_bitmap[temp_block_number/8]|(0x01 << (temp_block_number%8)));
  // write back the updated block map information to disk
  input_disk->write(disk_allocation_bitmap_block_no, temp_disk_allocation_bitmap);
  return temp_block_number;
}
void FileSystem::set_block_free(SimpleDisk* disk, unsigned long _block_no)
{

}

bool FileSystem::request_disk_block(unsigned long _metadata_block_no)
{
  // get the superblock first to make the changes you made spread across the system
  superblock* temp_superblock = new superblock;
  Console::puts("Superblock of this file system is expected to be at ");Console::putui(superblock_disk_block_no);Console::puts("\n");
  disk->read(superblock_disk_block_no,(unsigned char*)temp_superblock);
  // get the metadata of the file from the input
  file_metadata_entry  temp_file_metadata[0];
  disk->read(_metadata_block_no,(unsigned char*)temp_file_metadata);
  // now get the free block from bitmap we acquired
  unsigned long new_block_request = FileSystem::get_free_block_no(disk,temp_superblock->disk_block_allocation_bitmap);
  if(new_block_request == 0)
  {
    Console::puts("Got no new blocks from the get_free_block procedure, Something must be wrong\n");
    return 0;
  }
  // update the free blocks field in superblock of the file system
  temp_superblock->num_free_blocks--;
  // update the num_blocks allocated in file metadata field
  temp_file_metadata->num_blocks_allocated++;
  unsigned char temp_file_index_block[512];
  unsigned long* temp_file_index_block_unsigned_long_ptr = (unsigned long*) temp_file_index_block;
  disk->read(temp_file_metadata->index_block_no,temp_file_index_block);
  unsigned int i;
  for(i=0;i<temp_file_metadata->num_blocks_allocated;i++)
  {
    if(i < (temp_file_metadata->num_blocks_allocated-1))
    {
      temp_file_index_block_unsigned_long_ptr[i] = temp_file_index_block_unsigned_long_ptr[i];
    }
    else if(i == temp_file_metadata->num_blocks_allocated-1)
    {
      temp_file_index_block_unsigned_long_ptr[i] = new_block_request;
    }
  }
  // write back index block, superblock and metadata
  disk->write(superblock_disk_block_no,(unsigned char*)temp_superblock);
  disk->write(_metadata_block_no,(unsigned char*)temp_file_metadata);
  disk->write(temp_file_metadata->index_block_no,temp_file_index_block);
  delete temp_superblock;
  return true;
}

bool FileSystem::data_block_read(unsigned char* _read_buffer,unsigned long _metadata_block_no)
{
  // get the metadata block using the input
  file_metadata_entry temp_file_metadata[0];
  disk->read(_metadata_block_no,(unsigned char*)temp_file_metadata);
  // get the index block content using the obtained metadata block
  unsigned char temp_file_index_block[512];
  unsigned long* temp_file_index_block_unsigned_long_ptr = (unsigned long*)temp_file_index_block;
  disk->read(temp_file_metadata->index_block_no,temp_file_index_block);
  if(temp_file_metadata->num_blocks_allocated == 1)
  {
    // read from that block
    disk->read(temp_file_index_block_unsigned_long_ptr[0],_read_buffer);
    Console::puts("Read from the block number in the index node\n");
  }
}

bool FileSystem::data_block_write(unsigned char* _write_buffer, unsigned long _metadata_block_no)
{
  file_metadata_entry temp_file_metadata[0];
  disk->read(_metadata_block_no,(unsigned char*)temp_file_metadata);
  // get the index block content using the obtained metadata block
  unsigned char temp_file_index_block[512];
  unsigned long* temp_file_index_block_unsigned_long_ptr = (unsigned long*)temp_file_index_block;
  disk->read(temp_file_metadata->index_block_no,temp_file_index_block);
  if(temp_file_metadata->num_blocks_allocated == 1)
  {
    // read from that block
    disk->write(temp_file_index_block_unsigned_long_ptr[0],_write_buffer);
    Console::puts("Write to the block number in the index node\n");
  }
  return true;
}

bool FileSystem::erase_data_block(unsigned long _metadata_block_no)
{
  file_metadata_entry temp_file_metadata[0];
  disk->read(_metadata_block_no,(unsigned char*)temp_file_metadata);
  // get the index block content using the obtained metadata block
  unsigned char temp_file_index_block[512];
  unsigned char null_array[512];
  unsigned int i;
  unsigned long* temp_file_index_block_unsigned_long_ptr = (unsigned long*)temp_file_index_block;
  disk->read(temp_file_metadata->index_block_no,temp_file_index_block);
  // populate the null array
  for(i=0;i<512;i++)
  {
    null_array[i] = '\0';
  }
  if(temp_file_metadata->num_blocks_allocated == 1)
  {
    disk->write(temp_file_index_block_unsigned_long_ptr[0],null_array);
    Console::puts("Written null array to the block number in the index\n");
  }
  return true;
}
