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
    current_position = new unsigned int;
    *current_position = 0;
    // buffer the file_id and index block number which are inputs to the constructor
    file_metadata_block_no = new unsigned long;
    *file_metadata_block_no = _metadata_block_no;
    file_metadata_block_no_buffer = new unsigned long;
    *file_metadata_block_no_buffer = _metadata_block_no;
    Console::puts("Initialized the file_meta_data_block_no");Console::putui(*file_metadata_block_no_buffer);
    // initialize the allocated size to zero
    allocated_size  = new unsigned long;
    *allocated_size = 0;
    // initialize the filled size to zero
    filled_size = new unsigned long;
    *filled_size = 0;
}

/*--------------------------------------------------------------------------*/
/* FILE FUNCTIONS */
/*--------------------------------------------------------------------------*/

int File::Read(unsigned int _n, char * _buf)
{
    Console::puts("reading from file..");Console::putui(_n);Console::puts("\n");
    // buffer off the input variables
    unsigned int num_chars = _n;
    unsigned int i;
    unsigned char* output_buffer = (unsigned char*)_buf;
    unsigned int return_value;
    unsigned int print_value;
    // read the requested number of characters from file
    Console::puts("Getting allocated blocks...");
    Console::puts("Value of file_metadata_block is ");Console::putui(*file_metadata_block_no_buffer);Console::puts("\n");
    Console::putui(*file_metadata_block_no_buffer);Console::puts("\n");

    *allocated_size = FILE_SYSTEM->get_allocated_size(*file_metadata_block_no_buffer);Console::puts("..done\n");
    Console::puts("Allocated size in number of blocks is ");Console::putui(*allocated_size);
    if(*allocated_size == 1)
    {
      Console::puts("Only one size allocated to the file..we will get that\n");
      assert(FILE_SYSTEM->data_block_read(output_buffer,*file_metadata_block_no_buffer));
    }
    for(i=0;i< num_chars;i++)
    {
      if(output_buffer[i] == '\0')
      {
        return_value = i;
        Console::puts("The length determined is ");Console::putui(return_value);Console::puts("\n");
        break;
      }
    }
    // Adjust the current position pointer
    current_position = current_position + num_chars;
    //Console::puts("read ");Console::putui(num_chars);Console::puts("\n");
    return return_value;
}


void File::Write(unsigned int _n, const char * _buf)
{
    Console::puts("writing to file...");Console::putui(_n);Console::puts("characters to file\n");
    unsigned long allocated_size_bytes;
    //buffer off the input variables
    unsigned int num_chars = _n;
    unsigned int i;
    char* input_buffer = new char[_n];
    input_buffer = (char*)_buf;
    bool block_request_result;
    unsigned char* write_buffer = new unsigned char[512];
    *allocated_size = FILE_SYSTEM->get_allocated_size(*file_metadata_block_no_buffer);
    Console::puts("Allocated size in number of blocks is ");Console::putui(*allocated_size);
    if((*allocated_size == 0)|((*allocated_size*512) < (*filled_size)+ _n))
    {
      Console::puts("Allocated Size not sufficient\n");
      block_request_result = FILE_SYSTEM->request_disk_block(*file_metadata_block_no_buffer);
      if(block_request_result == true)
      {
        // block allocation successful
        allocated_size = allocated_size + 512;
        Console::puts("Allocation size is incremented by 512 bytes\n");
      }
    }
    else
    {
      allocated_size_bytes = *allocated_size*512;
      Console::puts("Allocated size to the file is ");Console::putui(allocated_size_bytes);
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
    assert(FILE_SYSTEM->data_block_write(write_buffer,*file_metadata_block_no_buffer));
    delete write_buffer;
    delete input_buffer;
}

void File::Reset()
{
    Console::puts("reset current position in file...");
    *current_position = 0;
    Console::puts("done\n");
}

void File::Rewrite()
{
    Console::puts("Called erase_data_block helper function of the file-system\n");
    assert(FILE_SYSTEM->erase_data_block(*file_metadata_block_no_buffer));
    Console::puts("erased content of file\n");
}


bool File::EoF() {
    Console::puts("testing end-of-file condition\n");

    assert(false);
}
