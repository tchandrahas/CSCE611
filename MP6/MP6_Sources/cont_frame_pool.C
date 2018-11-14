/*
 File: ContFramePool.C

 Author:
 Date  :

 */

/*--------------------------------------------------------------------------*/
/*
 POSSIBLE IMPLEMENTATION
 -----------------------

 The class SimpleFramePool in file "simple_frame_pool.H/C" describes an
 incomplete vanilla implementation of a frame pool that allocates
 *single* frames at a time. Because it does allocate one frame at a time,
 it does not guarantee that a sequence of frames is allocated contiguously.
 This can cause problems.

 The class ContFramePool has the ability to allocate either single frames,
 or sequences of contiguous frames. This affects how we manage the
 free frames. In SimpleFramePool it is sufficient to maintain the free
 frames.
 In ContFramePool we need to maintain free *sequences* of frames.

 This can be done in many ways, ranging from extensions to bitmaps to
 free-lists of frames etc.

 IMPLEMENTATION:

 One simple way to manage sequences of free frames is to add a minor
 extension to the bitmap idea of SimpleFramePool: Instead of maintaining
 whether a frame is FREE or ALLOCATED, which requires one bit per frame,
 we maintain whether the frame is FREE, or ALLOCATED, or HEAD-OF-SEQUENCE.
 The meaning of FREE is the same as in SimpleFramePool.
 If a frame is marked as HEAD-OF-SEQUENCE, this means that it is allocated
 and that it is the first such frame in a sequence of frames. Allocated
 frames that are not first in a sequence are marked as ALLOCATED.

 NOTE: If we use this scheme to allocate only single frames, then all
 frames are marked as either FREE or HEAD-OF-SEQUENCE.

 NOTE: In SimpleFramePool we needed only one bit to store the state of
 each frame. Now we need two bits. In a first implementation you can choose
 to use one char per frame. This will allow you to check for a given status
 without having to do bit manipulations. Once you get this to work,
 revisit the implementation and change it to using two bits. You will get
 an efficiency penalty if you use one char (i.e., 8 bits) per frame when
 two bits do the trick.

 DETAILED IMPLEMENTATION:

 How can we use the HEAD-OF-SEQUENCE state to implement a contiguous
 allocator? Let's look a the individual functions:

 Constructor: Initialize all frames to FREE, except for any frames that you
 need for the management of the frame pool, if any.

 get_frames(_n_frames): Traverse the "bitmap" of states and look for a
 sequence of at least _n_frames entries that are FREE. If you find one,
 mark the first one as HEAD-OF-SEQUENCE and the remaining _n_frames-1 as
 ALLOCATED.

 release_frames(_first_frame_no): Check whether the first frame is marked as
 HEAD-OF-SEQUENCE. If not, something went wrong. If it is, mark it as FREE.
 Traverse the subsequent frames until you reach one that is FREE or
 HEAD-OF-SEQUENCE. Until then, mark the frames that you traverse as FREE.

 mark_inaccessible(_base_frame_no, _n_frames): This is no different than
 get_frames, without having to search for the free sequence. You tell the
 allocator exactly which frame to mark as HEAD-OF-SEQUENCE and how many
 frames after that to mark as ALLOCATED.

 needed_info_frames(_n_frames): This depends on how many bits you need
 to store the state of each frame. If you use a char to represent the state
 of a frame, then you need one info frame for each FRAME_SIZE frames.

 A WORD ABOUT RELEASE_FRAMES():

 When we releae a frame, we only know its frame number. At the time
 of a frame's release, we don't know necessarily which pool it came
 from. Therefore, the function "release_frame" is static, i.e.,
 not associated with a particular frame pool.

 This problem is related to the lack of a so-called "placement delete" in
 C++. For a discussion of this see Stroustrup's FAQ:
 http://www.stroustrup.com/bs_faq2.html#placement-delete

 */
/*--------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

#define INFO_FREE_FRAME_CHECK(i) (((ext_bitmap0[i/8])&(0x01<<(i%8))) == (0x00<<(i%8))) && (((ext_bitmap1[i/8])&(0x01<<(i%8))) == (0x00<<(i%8)))
#define INFO_HEAD_FRAME_CHECK(i) (((ext_bitmap0[i/8])&(0x01<<(i%8))) == (0x01<<(i%8))) && (((ext_bitmap1[i/8])&(0x01<<(i%8))) == (0x01<<(i%8)))
#define INFO_ALLOCATED_FRAME_CHECK(i) (((ext_bitmap0[i/8])&(0x01<<(i%8))) == (0x01<<(i%8))) && (((ext_bitmap1[i/8])&(0x01<<(i%8))) == (0x00<<(i%8)))

#define CHANGE_TO_FREE_FRAME(i) ext_bitmap0[(i/8)] &= (~(0x01<<(i%8))); ext_bitmap1[(i/8)] &= (~(0x01<<(i%8)))
#define CHANGE_TO_ALLOCATED_FRAME(i) ext_bitmap0[(i/8)] |=(0x01<<(i%8)); ext_bitmap1[(i/8)] &= (~(0x01<<(i%8)))
#define CHANGE_TO_HEAD_FRAME(i) ext_bitmap0[(i/8)] |= (0x01<<(i%8));ext_bitmap1[(i/8)] |= (0x01<<(i%8))
/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "cont_frame_pool.H"
#include "console.H"
#include "utils.H"
#include "assert.H"

/*--------------------------------------------------------------------------*/
/* DATA STRUCTURES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* CONSTANTS */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* FORWARDS */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* METHODS FOR CLASS   C o n t F r a m e P o o l */
/*--------------------------------------------------------------------------*/

ContFramePool::ContFramePool(unsigned long _base_frame_no,
                             unsigned long _n_frames,
                             unsigned long _info_frame_no,
                             unsigned long _n_info_frames)
{
    // TODO: IMPLEMENTATION NEEEDED!

    // Assert that requested frames are multiple of 8
    assert ((_n_frames % 8) == 0);
    // Copy the inputs to local variables
    base_frame_no = _base_frame_no;
    n_frames = _n_frames;
    n_free_frames = _n_frames;
    info_frame_no = _info_frame_no;
    n_info_frames = _n_info_frames;

    // Check the info_frame_no to decide where to place the management information
    if(info_frame_no == 0)
    {
      // Management info should be stored internally in the first frame
      n_info_frames = needed_info_frames(_n_frames);
      // Each frame need two bits, so we use two different frames to store first and second bits
      ext_bitmap0 = (unsigned char*)(base_frame_no * FRAME_SIZE);
      ext_bitmap1 = (unsigned char*)((base_frame_no+n_info_frames/2) * FRAME_SIZE);
    }
    else
    {
      // Management info should be stored in the given frame number in given number of information frames
      assert(_n_info_frames >= needed_info_frames(_n_frames));
      n_info_frames = _n_info_frames;
      ext_bitmap0 = (unsigned char*)(info_frame_no * FRAME_SIZE);
      ext_bitmap1 = (unsigned char*)((info_frame_no+_n_info_frames/2) * FRAME_SIZE);
    }

    // Number of frames must be able to fill the bitmap

    // Mark in bitmap that all frames are free
    // 00 indicates the frame is FREE
    // 01 indicates that frame is Allocated
    // 11 indicates that frame is head of sequence
    for(int i = 0; i < (n_frames); i=i+1)
    {
      ext_bitmap0[i/8] = 0x00;
      ext_bitmap1[i/8] = 0x00;
      //Console::puti(INFO_FREE_FRAME_CHECK(i));Console::puti(INFO_ALLOCATED_FRAME_CHECK(i));Console::puti(INFO_HEAD_FRAME_CHECK(i));Console::puts("\n");
    }

    // if you are using frames with in the pool for information, mark them as used
    if(info_frame_no == 0)
    {
      assert(n_info_frames < n_free_frames);
      for(unsigned int i=0;i<(n_info_frames);i++)
      {
        //Console::puti(i/8);Console::puts(",");Console::puti(ext_bitmap0[i/8]);Console::puts(",");Console::puti(ext_bitmap1[i/8]);Console::puts("\n");
        CHANGE_TO_ALLOCATED_FRAME(i);
        //Console::putui(ext_bitmap0[i/8]);Console::puts(",");Console::putui(ext_bitmap1[i/8]);Console::puts("\n");
        //Console::puti(INFO_FREE_FRAME_CHECK(i));Console::puti(INFO_ALLOCATED_FRAME_CHECK(i));Console::puti(INFO_HEAD_FRAME_CHECK(i));Console::puts("\n");
        n_free_frames--;
      }
    }

    // The very first frame in the pool becomes the head of the frames
    CHANGE_TO_HEAD_FRAME(0);

    // Malloc for an entry in frame_pool linked list
    frame_pool_entry.base_frame_no = base_frame_no;
    frame_pool_entry.n_frames = n_frames;
    frame_pool_entry.ext_bitmap0 = ext_bitmap0+0;
    frame_pool_entry.ext_bitmap1 = ext_bitmap1+0;

    // put this frame pool into frame pool linked list
    // first check if this is the first in the frame pool linked list
    if(frame_pool_ll_head == NULL)
    {
      frame_pool_ll_head = &frame_pool_entry;
      frame_pool_ll_current = frame_pool_ll_head;
    }
    else
    {
      frame_pool_ll_current->next_frame_pool = &frame_pool_entry;
      frame_pool_ll_current = &frame_pool_entry;
      frame_pool_ll_current->next_frame_pool = NULL;
    }

    //Console::puts("Frame Pool initialized");Console::puts(" with base_frame_no as ");Console::puti(base_frame_no);Console::puts(" with number of frames as ");Console::puti(n_frames);Console::puts("\n");
    //Console::puts("Info frames are counted to be ");Console::puti(n_info_frames);Console::puts("Free frames are counted to be ");Console::puti(n_free_frames);Console::puts("\n");
    for(int i=0;i<3;i++)
    {
      //Console::puti(INFO_FREE_FRAME_CHECK(i));Console::puti(INFO_ALLOCATED_FRAME_CHECK(i));Console::puti(INFO_HEAD_FRAME_CHECK(i));Console::puts("\n");
    }

}

unsigned long ContFramePool::get_frames(unsigned int _n_frames)
{
    // TODO: IMPLEMENTATION NEEEDED!
    assert(n_free_frames >= _n_frames);

    // Find a frame that is not being used
    unsigned int frame_no = base_frame_no;
    unsigned int nget_frames = _n_frames;
    unsigned int i = 0;
    unsigned int j;
    while_loop:
    while(i<n_frames)
    {
      //Console::puts("Here in loop3\n");
      if(INFO_FREE_FRAME_CHECK(i))
      {
        // check if _n_frames after the ith frame are free or not
        for(j=i;j<i+nget_frames;j++)
        {
          // If any allocated frame is found revert back to finding another one
          if(INFO_FREE_FRAME_CHECK(j))
          {
            if(j == (i+nget_frames-1))
            {
              goto outside;
            }
          }

          else
          {
            //Console::puts("Here in loop1\n");
            i=j+1;
            goto while_loop;
          }


        }
      }
      else
      {
        // iterate until a free frame is encountered
        //Console::puts("Here in loop2\n");
        //Console::puts("searched for  a free frame ");Console::puti(i);Console::puti(INFO_FREE_FRAME_CHECK(i));Console::puti(INFO_HEAD_FRAME_CHECK(i));Console::puti(INFO_ALLOCATED_FRAME_CHECK(i));Console::puts("\n");
        i++;
        goto while_loop;
      }
    }
    outside:
    // An ith frame is encountered to be first in contigous free _n_frames
    frame_no = i+base_frame_no;
    //Console::puts("The i value was found out to be ");Console::puti(i);Console::puts("\n");
    // change the status of n_frames contigous frames found
    for(j=i;j<i+nget_frames;j++)
    {
      //ext_bitmap[j+i] = INFO_ALLOCATED_FRAME;
      CHANGE_TO_ALLOCATED_FRAME(j);
      //Console::puti(j+i);Console::puti(INFO_FREE_FRAME_CHECK(j+i));Console::puti(INFO_HEAD_FRAME_CHECK(j+i));Console::puti(INFO_ALLOCATED_FRAME_CHECK(j+i));Console::puts("\n");
    }
    // change the status of initial frame to be head of sequence
    //ext_bitmap[i] = INFO_HEAD_FRAME;
    /*for(j=i;j<i+nget_frames;j++)
    {
      Console::puti(j);Console::puti(INFO_FREE_FRAME_CHECK(j));Console::puti(INFO_HEAD_FRAME_CHECK(j));Console::puti(INFO_ALLOCATED_FRAME_CHECK(j));Console::puts("\n");
    }*/
    CHANGE_TO_HEAD_FRAME(i);
    /*for(j=i;j<i+nget_frames;j++)
    {
      Console::puti(j);Console::puti(INFO_FREE_FRAME_CHECK(j));Console::puti(INFO_HEAD_FRAME_CHECK(j));Console::puti(INFO_ALLOCATED_FRAME_CHECK(j));Console::puts("\n");
    }*/
    // change the number of available frames
    n_free_frames = n_free_frames - nget_frames;
    frame_pool_entry.n_free_frames = n_free_frames;
    // return the starting frame number of contigous frames
    //Console::puts("Got "); Console::puti(_n_frames); Console::puts(" frames from "); Console::puti(frame_no); Console::puts(" ,remaining free frames = "); Console::puti(n_free_frames); Console::puts("\n");
    return(frame_no);
}

void ContFramePool::mark_inaccessible(unsigned long _base_frame_no,
                                      unsigned long _n_frames)
{
    // IMPLEMENTATION NEEEDED!
    int i;
    unsigned int bitmap_position;
    // Get the head position, change that position to HEAD of the sequence, decrement the number of free frames
    bitmap_position = _base_frame_no - base_frame_no;
    CHANGE_TO_HEAD_FRAME(bitmap_position);
    n_free_frames--;
    frame_pool_entry.n_free_frames = n_free_frames;
    // Mark rest of the frames as Allocated, decrement the number of free frames
    for(i=_base_frame_no+1; i < (_base_frame_no+_n_frames); i++)
    {
      bitmap_position = i-base_frame_no;
      CHANGE_TO_ALLOCATED_FRAME(bitmap_position);
      n_free_frames--;
      frame_pool_entry.n_free_frames = n_free_frames;
    }
    //Console::puts("The ");Console::puti(_n_frames);Console::puts("  frames starting from ");Console::puti(_base_frame_no);Console::puts(" are marked inaccessible\n");
}

void ContFramePool::release_frames(unsigned long _first_frame_no)
{
    //  IMPLEMENTATION NEEEDED!
    // First we have to figure out which frame pool this frame belongs to
    // We do this by traversing through the linked list
    assert((frame_pool_ll_head!=NULL)||(frame_pool_ll_current!=NULL));
    frame_pool* traverse_frame_pool = frame_pool_ll_head;
    while(traverse_frame_pool != NULL)
    {
      if((_first_frame_no >= traverse_frame_pool->base_frame_no)&&(_first_frame_no < (traverse_frame_pool->base_frame_no + traverse_frame_pool->n_frames)))
      {
        // we found the right frame pool
        break;
      }
      else
      {
        traverse_frame_pool = traverse_frame_pool->next_frame_pool;
      }
    }

    // Caluculate the bitmap index based upon the number of bits used for frame information bits used
    //Console::puts("The base_frame_no is found to be "); Console::puti(traverse_frame_pool->base_frame_no);Console::puts("\n");
    unsigned int local_ext_bitmap_index = (_first_frame_no - traverse_frame_pool->base_frame_no);
    //Console::puts("The index is found to be "); Console::puti(local_ext_bitmap_index);Console::puts("\n");
    unsigned char* local_ext_bitmap0 = traverse_frame_pool->ext_bitmap0;
    unsigned char* local_ext_bitmap1 = traverse_frame_pool->ext_bitmap1;
    // Assertion to verify if bitmap pointers are not NULL
    assert((local_ext_bitmap0!=NULL)&&(local_ext_bitmap1!=NULL));
    //Console::puts("The information of frame is ");Console::puti((local_ext_bitmap0[local_ext_bitmap_index/8])&(0x01<<(local_ext_bitmap_index%8)));Console::puts(", ");Console::puti((local_ext_bitmap1[local_ext_bitmap_index/8])&(0x01<<(local_ext_bitmap_index%8)));Console::puts("\n");
    //Console::puts("The expectation is ");Console::puti((0x01<<(local_ext_bitmap_index%8)));Console::puts(", ");Console::puti((0x01<<(local_ext_bitmap_index%8)));Console::puts("\n");
    if((((local_ext_bitmap0[local_ext_bitmap_index/8])&(0x01<<(local_ext_bitmap_index%8))) == (0x01<<(local_ext_bitmap_index%8))) && (((local_ext_bitmap1[local_ext_bitmap_index/8])&(0x01<<(local_ext_bitmap_index%8))) == (0x01<<(local_ext_bitmap_index%8))))
    {
      //Console::puts("Head of the sequence found at");Console::puti(local_ext_bitmap_index);Console::puts("\n");
      unsigned int i = local_ext_bitmap_index+1;
      //Console::puts("The information present is ");Console::puti((local_ext_bitmap0[i/8])&(0x01<<(i%8)));Console::puts(", ");Console::puti((local_ext_bitmap1[i/8])&(0x01<<(i%8)));Console::puts("\n");
      while((((local_ext_bitmap0[i/8])&(0x01<<(i%8))) == (0x01<<(i%8))) && (((local_ext_bitmap1[i/8])&(0x01<<(i%8))) == (0x00<<(i%8))))
      {
        // change the information of the frames and increment the number of free frames
        //local_ext_bitmap[i] = INFO_FREE_FRAME;
        local_ext_bitmap0[(i/8)] &=(~(0x01 << (i%8))); local_ext_bitmap1[(i/8)] &=(~(0x01 << (i%8)));
        traverse_frame_pool->n_free_frames++;
        i++;
      }
      //Console::puts("Number of frames freed = ");Console::puti(i-(local_ext_bitmap_index));Console::puts(" from frame_no ");Console::puti(_first_frame_no);Console::puts(" \n");
    }
    else
    {
      Console::puts("Error, Frame being released is not the head of  a sequence of frame\n");
      assert(false);
    }
}

unsigned long ContFramePool::needed_info_frames(unsigned long _n_frames)
{
    // IMPLEMENTATION NEEEDED!
    // Caluculate how many frame information can be stored in a frame
    unsigned int frame_info_capacity =(8*(FRAME_SIZE))/INFO_BITS_PER_FRAME;
    unsigned long return_value = _n_frames/frame_info_capacity + ((_n_frames % frame_info_capacity) > 0 ? 1 : 0);
    //Minimum 2 frames are needed to store ext_bitmap0 and ext_bitmap1
    if(return_value > 2)
    {
      return return_value;
    }
    else
    {
      return 2;
    }

}
