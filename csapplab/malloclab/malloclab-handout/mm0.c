/*
 * mm-naive.c - The fastest, least memory-efficient malloc package.
 * 
 * In this naive approach, a block is allocated by simply incrementing
 * the brk pointer.  A block is pure payload. There are no headers or
 * footers.  Blocks are never coalesced or reused. Realloc is
 * implemented directly using mm_malloc and mm_free.
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
    /* Team name */
    "ateam",
    /* First member's full name */
    "Harry Bovik",
    /* First member's email address */
    "bovik@cs.cmu.edu",
    /* Second member's full name (leave blank if none) */
    "",
    /* Second member's email address (leave blank if none) */
    ""
};

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)


#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

#define WSIZE 4
#define DSIZE 8
#define CHUNKSIZE (1<<12)

#define MAX(x,y) ((x)>(y)?(x):(y))
#define PACK(size,alloc) ((size)|(alloc))
//void * 类型要先强转
#define GET(p)  (*(unsigned int *)(p))
#define PUT(p,val) (*(unsigned int *)(p) = (val))

#define GET_SIZE(p) (GET(p) & ~0x7)
#define GET_ALLOC(p) (GET(p) & 0x1)

#define HDRP(bp) ((char *)(bp) - WSIZE)
#define FTRP(bp) ((char *)(bp) + GET_SIZE(HDRP(bp))- DSIZE)

#define NEXT_BLKP(bp) ((char *)(bp) + GET_SIZE(((char *)(bp) - WSIZE)))
#define PREV_BLKP(bp) ((char *)(bp) - GET_SIZE(((char *)(bp) - DSIZE)))
static void *extern_heap(size_t words);
static void *coalesce(void *bp);
static void * find_fit(size_t size);
static void  place(void * bp,size_t size);
static void * best_fit(size_t size);
static void insert_new_free(void * pos);
static void del_free(void *p);
static void insert_before(void* p,void * pos);

static char * heap_listp;
/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    //初始化堆
    if((heap_listp = mem_sbrk(4*WSIZE) )== (void * )-1){
        return -1;
    }
    PUT(heap_listp,0);
    PUT(heap_listp+(1*WSIZE),PACK(DSIZE,1));
    PUT(heap_listp+(2*WSIZE),PACK(DSIZE,1));
    PUT(heap_listp+(3*WSIZE),PACK(0,1));
    heap_listp += DSIZE;

    if(extern_heap(CHUNKSIZE/WSIZE) == NULL){
        return -1;
    }
    return 0;
}

static void *extern_heap(size_t words){
    char *bp;
    size_t size;

    //双字对其
    size = (words % 2 ) ? (words + 1) * WSIZE : words * WSIZE;
    if((long) (bp = mem_sbrk(size)) == -1){
        return NULL;
    }
    
    //将结尾块更新成新的空闲块头部
    PUT(HDRP(bp),PACK(size,0));
    PUT(FTRP(bp),PACK(size,0));
    //新的结尾块
    PUT(HDRP(NEXT_BLKP(bp)),PACK(0,1));


    //找不到匹配块时合并
    return coalesce(bp);
}

static void *coalesce(void *bp){

    size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));

    //case 1
    if(prev_alloc && next_alloc){
        return bp;
    }

    //case 2
    else if(prev_alloc && !next_alloc){
        size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
        PUT(HDRP(bp),PACK(size,0));
        PUT(FTRP(bp),PACK(size,0));
    }


    //case 3
    else if(!prev_alloc && next_alloc){
        size += GET_SIZE(HDRP(PREV_BLKP(bp)));
        PUT(HDRP(PREV_BLKP(bp)),PACK(size,0));
        PUT(FTRP(bp),PACK(size,0));
        bp = PREV_BLKP(bp);
    }
    //case 4
    else{
        size += GET_SIZE(HDRP(PREV_BLKP(bp)));
        size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
        PUT(HDRP(PREV_BLKP(bp)),PACK(size,0));
        PUT(FTRP(NEXT_BLKP(bp)),PACK(size,0));
        bp = PREV_BLKP(bp);
    }
    return bp;
}

static void * find_fit(size_t size){
    // 首次适配
    // 按链表查找第一个未分配且size够大的
    char * bp =  heap_listp;
    while (GET_SIZE(HDRP (bp)) > 0 )
    {
        //这里不用减去头部和尾部？
        if((GET_SIZE(HDRP(bp))) >= size && !GET_ALLOC(HDRP(bp))){
            return bp;
        }
        bp = NEXT_BLKP(bp);
    }
    return NULL;
}

static void place(void * bp,size_t size){
    //超出部分>=最小快分割
    if((GET_SIZE(HDRP(bp))-size) >= (2*DSIZE)){
        size_t remain_size = GET_SIZE(HDRP(bp))-size;
        PUT(HDRP(bp),PACK(size,1));
        PUT(FTRP(bp),PACK(size,1));
        PUT(HDRP(NEXT_BLKP(bp)),PACK(remain_size,0));
        PUT(FTRP(NEXT_BLKP(bp)),PACK(remain_size,0));
    }else{
        PUT(HDRP(bp),PACK(GET_SIZE(HDRP(bp)),1));
        PUT(FTRP(bp),PACK(GET_SIZE(HDRP(bp)),1));
    }
}

/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
    //原版，隐式链表,没有特殊数据结构，直接sbrk开辟新空间
    // int newsize = ALIGN(size + SIZE_T_SIZE);
    // void *p = mem_sbrk(newsize);
    // if (p == (void *)-1)
	// return NULL;
    // else {
    //     *(size_t *)p = size;
    //     return (void *)((char *)p + SIZE_T_SIZE);
    // }

    size_t extendsize;
    char *bp;
    if(size == 0){
        return NULL;
    }
    int newsize = ALIGN(size + SIZE_T_SIZE);
    if((bp = find_fit(newsize)) != NULL){
        place(bp,newsize);
        return bp;
    }
    extendsize = MAX(newsize,CHUNKSIZE);
    if((bp = extern_heap(extendsize/WSIZE)) == NULL){
        return NULL;
    }
    place(bp,newsize);
    return bp;
}



/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr)
{
    size_t size = GET_SIZE(HDRP(ptr));
    PUT(HDRP(ptr),PACK(size,0));
    PUT(FTRP(ptr),PACK(size,0));
    coalesce(ptr);
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{
    void *oldptr = ptr;
    void *newptr;
    size_t copySize;
    
    newptr = mm_malloc(size);
    if (newptr == NULL)
    return NULL;
    // copySize = *(size_t *)((char *)oldptr - SIZE_T_SIZE);
    copySize = GET_SIZE(HDRP(oldptr));
    if (size < copySize)
        copySize = size;
    memcpy(newptr, oldptr, copySize);
    mm_free(oldptr);
    return newptr;
}














