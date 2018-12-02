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

/*--------------------------------------------------------------------------*/
/* CONSTRUCTOR */
/*--------------------------------------------------------------------------*/

File::File() {
    /* We will need some arguments for the constructor, maybe pointer to disk
     block with file management and allocation data. */
    Console::puts("In file constructor.\n");
    assert(false);
}

/*--------------------------------------------------------------------------*/
/* FILE FUNCTIONS */
/*--------------------------------------------------------------------------*/

int File::Read(unsigned int _n, char * _buf) {
    Console::puts("reading from file..\n");
    // buffer off the input variables
    unsigned int num_chars = _n;
    unsigned int i;
    char* output_buffer = _buf;
    // read the requested number of characters from file
    for(i=0;i< num_chars;i++)
    {
      output_buffer[i] = *(current_position+i);
    }
    // Adjust the current position pointer
    current_position = current_position + num_chars;
    Console::puts("read ");Console::putui(num_chars);Console::puts("\n");
}


void File::Write(unsigned int _n, const char * _buf) {
    Console::puts("writing to file...");
    //buffer off the input variables
    unsigned int num_chars = _n;
    unsigned int i;
    char* input_buffer = _buf;
    // read the requested number of characters from file
    for(i=0;i< num_chars;i++)
    {
      *(current_position+i) = input_buffer[i];
    }
    // Adjust the current position pointer
    current_position = current_position + num_chars;
    Console::puts("wrote ");Console::putui(num_chars);Console::puts("\n");

}

void File::Reset() {
    Console::puts("reset current position in file...\n");

    assert(false);

}

void File::Rewrite() {
    Console::puts("erase content of file\n");
    assert(false);
}


bool File::EoF() {
    Console::puts("testing end-of-file condition\n");
    assert(false);
}
