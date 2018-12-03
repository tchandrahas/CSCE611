/*
     File        : file.C

     Author      : Riccardo Bettati
     Modified    : 2017/05/01

     Description : Implementation of simple File class, with support for
                   sequential read/write operations.
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
#include "file.H"

extern FileSystem* FILE_SYSTEM;
/*--------------------------------------------------------------------------*/
/* CONSTRUCTOR */
/*--------------------------------------------------------------------------*/

File::File(unsigned long _metadata_block_no)
 {
    /* We will need some arguments for the constructor, maybe pointer to disk
     block with file management and allocation data. */
    Console::puts("In file constructor.\n");
    // set the current position to zero
    current_position = 0;
    // buffer the file_id and index block number which are inputs to the constructor
    file_metadata_block_no = _metadata_block_no;
    // initialize the allocated size to zero
    allocated_size  = 0;
    // initialize the filled size to zero
    filled_size = 0;
}

/*--------------------------------------------------------------------------*/
/* FILE FUNCTIONS */
/*--------------------------------------------------------------------------*/

int File::Read(unsigned int _n, char * _buf)
{
    Console::puts("reading from file..\n");
    // buffer off the input variables
    /*unsigned int num_chars = _n;
    unsigned int i;
    char* output_buffer = _buf;
    // read the requested number of characters from file
    for(i=0;i< num_chars;i++)
    {
      output_buffer[i] = *(current_position+i);
    }
    // Adjust the current position pointer
    current_position = current_position + num_chars;*/
    assert(false);
    //Console::puts("read ");Console::putui(num_chars);Console::puts("\n");
}


void File::Write(unsigned int _n, const char * _buf)
{
    Console::puts("writing to file...");
    //buffer off the input variables
    unsigned int num_chars = _n;
    unsigned int i;
    char* input_buffer = (char*)_buf;
    bool block_request_result;
    unsigned char* write_buffer = new unsigned char[512];
    if((allocated_size == 0)|(allocated_size < (filled_size)+ _n))
    {
      block_request_result = FILE_SYSTEM->request_disk_block(file_metadata_block_no);
      if(block_request_result == true)
      {
        // block allocation successful
        allocated_size = allocated_size + 512;
        Console::puts("Allocation size is incremented by 512 bytes\n");
      }
    }
    // make the write buffer ready
    for(i=0;i<512;i++)
    {
      if(i < num_chars)
        write_buffer[i] = input_buffer[i];
      else if (i == num_chars)
        write_buffer[i] = '\0';
      else
          write_buffer[i] = 0;
    }
    // write to disk using file system helper function
    FILE_SYSTEM->data_block_write(write_buffer,file_metadata_block_no);
}

void File::Reset()
{
    Console::puts("reset current position in file...");
    current_position = 0;
    Console::puts("done\n");
}

void File::Rewrite()
{
  FILE_SYSTEM->erase_data_block(file_metadata_block_no);
    Console::puts("erase content of file\n");

}


bool File::EoF() {
    Console::puts("testing end-of-file condition\n");
    assert(false);
}
