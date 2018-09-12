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

/* -- (none) -- */

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

// Data structure to implement the pool list

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
    // Do Assertion if number of information frames supplied exceeds the number of information frames required

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
      ext_bitmap = (unsigned char*)(base_frame_no * n_info_frames * FRAME_SIZE);
    }
    else
    {
      // Management info should be stored in the given frame number in given number of information frames
      assert(_n_info_frames >= needed_info_frames(_n_frames));
      ext_bitmap = (unsigned char*)(info_frame_no * n_info_frames * FRAME_SIZE);
    }

    // Number of frames must be able to fill the bitmap
    //assert ((n_frames % 8) == 0);

    // Mark in bitmap that all frames are allocated
    for(int i = 0; i < n_frames; i=i+1)
    {
      ext_bitmap[i] = INFO_FREE_FRAME;
    }

    // if you are using frames with in the pool for information, mark them as used
    if(info_frame_no == 0)
    {
      n_info_frames = needed_info_frames(_n_frames);
      assert(n_info_frames < n_free_frames);
      for(int i=0;i<n_info_frames;i++)
      {
        ext_bitmap[i] = INFO_ALLOCATED_FRAME;
        n_free_frames--;
      }
    }

    // The very first frame in the pool becomes the head of the frames
    // verify this
    ext_bitmap[0] = INFO_HEAD_FRAME;

    Console::puts("Frame Pool initialized\n");

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
    while(i<n_frames)
    {
      inner_while_loop:
      while(ext_bitmap[i] == INFO_HEAD_FRAME || ext_bitmap[i] == INFO_ALLOCATED_FRAME)
      {
        // iterate until a free frame is encountered
        i++;
      }
      // check if _n_frames after the ith frame are free or not
      for(j=i;j<i+nget_frames;j++)
      {
        // If any allocated frame is found revert back to finding another one
        if(ext_bitmap[j] != INFO_FREE_FRAME)
        {
          i=i+1;
          goto inner_while_loop;
        }

        else if(j == i+nget_frames-1)
        {
          // This means that we have found _n_frames that are free in this frame pool
          goto outside_outer_while_loop;
        }
      }
    }
    outside_outer_while_loop:
    // An ith frame is encountered to be first in contigous free _n_frames
    frame_no = i+base_frame_no;
    // change the status of n_frames contigous frames found
    for(j=0;j<nget_frames;j++)
    {
      ext_bitmap[j+i] = INFO_ALLOCATED_FRAME;
    }
    // change the status of initial frame to be head of sequence
    ext_bitmap[i] = INFO_HEAD_FRAME;

    // change the number of available frames
    n_free_frames = n_free_frames - nget_frames;
    // return the starting frame number of contigous frames
    return(frame_no);
}

void ContFramePool::mark_inaccessible(unsigned long _base_frame_no,
                                      unsigned long _n_frames)
{
    // IMPLEMENTATION NEEEDED!
    int i;
    unsigned int bitmap_index;
    for(i=_base_frame_no; i < (_base_frame_no+_n_frames); i++)
    {
      assert((i > base_frame_no) && (i < base_frame_no+n_frames));
      ext_bitmap[bitmap_index] = INFO_ALLOCATED_FRAME;
      n_free_frames--;
    }
}

void ContFramePool::release_frames(unsigned long _first_frame_no)
{
    //  IMPLEMENTATION NEEEDED!
    // Caluculate the bitmap index based upon the number of bits used for frame information bits used
    unsigned int ext_bitmap_index = (_first_frame_no - base_frame_no);
    if(ext_bitmap[ext_bitmap_index]!=INFO_HEAD_FRAME)
    {
      Console::puts("Error, Frame being released is not the head of  a sequence of frame\n");
      assert(false);
    }
    // Else release the frames
    Console::puts("Head of the sequence found !!\n");
    unsigned int i = ext_bitmap_index;
    while((ext_bitmap[i] != INFO_FREE_FRAME)||(ext_bitmap[i] != INFO_HEAD_FRAME))
    {
      // change the information of the frames and increment the number of free frames
      ext_bitmap[i] = INFO_FREE_FRAME;
      n_free_frames++;
      i++;
    }
}

unsigned long ContFramePool::needed_info_frames(unsigned long _n_frames)
{
    // IMPLEMENTATION NEEEDED!
    // Caluculate how many frame information can be stored in a frame
    unsigned int frame_info_capacity =(8*(FRAME_SIZE))/INFO_BITS_PER_FRAME;
    return (_n_frames/frame_info_capacity + (_n_frames % frame_info_capacity > 0 ? 1 : 0));

}
