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

static void *extern_heap(size_t words);
static void *coalesce(void *bp);
static void * find_fit(size_t size);
static void  place(void * bp,size_t size);
static void * best_fit(size_t size);
static void insert_new_free(void * pos);
static void del_free(void *p);
static void insert_before(void* p,void * pos);

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

//显示空闲列表,如果PRED在前，SUCC在后，mem_sbrk会分配失败
#define FREE_PRED(bp) ((char *)(bp) + WSIZE)
#define FREE_SUCC(bp) ((char *)(bp))
#define GETP(bp) (*(char **)bp)
#define GET_PRED(bp) (GETP(FREE_PRED(bp)))
#define GET_SUCC(bp) (GETP(FREE_SUCC(bp)))
#define PUTP(bp,val) (*(char **)(bp) = (char *) (val))

// static unsigned int list;
// static unsigned long  t;
static char * heap_listp;
// static char * first_freep = &list;
static char * first_freep ;

// static char * final_freep = &t;

/* 给定序号，找到链表头节点位置 */
#define GET_HEAD(num) ((unsigned int *)(long)(GET(heap_listp + WSIZE * num)))
/* 给定bp,找到前驱和后继 */
#define GET_PRE(bp) ((unsigned int *)(long)(GET(bp)))
#define GET_SUC(bp) ((unsigned int *)(long)(GET((unsigned int *)bp + 1)))





//FILO
static void insert_new_free(void * pos){
    char * p;
    
    // printf("insert_new_free %x\n",(char *)GET_SUCC(first_freep));
    if( ((p = GET_SUCC(first_freep)) != NULL)){
        // printf("a\n");
        // printf("insert_before %x\n",(char*)p);
        insert_before(p,pos);
    }else{
        // printf("b\n");
        GET_SUCC(first_freep) = (char*)pos;
        //出现在这个else里代表没有新的空闲块了，新加入的快的succ会出现非空的情况，需要重置
        GET_SUCC(pos) = 0;
        // printf("insert_new_free pos %x\n",(char *)GET_SUCC(pos));
        // printf("insert_new_free %x\n",(char *)GET_SUCC(first_freep));
        GET_PRED(pos) = first_freep;
        // printf("insert_new_free %x\n",(char *)GET_PRED(pos));
    };
   
    // printf("insert_new_free %x\n",(char *)p);
    // printf("insert_new_free %x\n",(char *)pos);
    
}

//通用插入
static void insert_before(void* p,void * pos){
    // printf("insert_before %x\n",(char*)GET_SUCC( GET_PRED(p)));
    // printf("insert_before %x\n",(char*)pos);
    // printf("insert_before %x\n",(char*)GET_PRED(p));
    GET_SUCC( GET_PRED(p) ) = (char*) pos;
    
    // printf("insert_before %x\n",(char *)GET_PRED(p));
    GET_PRED(pos) = GET_PRED(p);
    
    // printf("insert_before %x\n",(char*)GET_PRED(pos));
    GET_PRED(p) = (char *) pos;
    // mem_sbrk(0); 
    // printf("insert_before %x\n",(char*)GET_PRED(p));
    GET_SUCC(pos) = (char *)p;
    // printf("insert_before %x\n",GET_SUCC(pos));
    // printf("insert_before %x\n",(char *)GET_SUCC(first_freep));
}

//分配时从空闲链表删除
static void del_free(void *p){
    // printf("del_free %x\n",p);
    // printf("del_free %x\n",(char *)GET_SUCC(GET_PRED(p)));
    GET_SUCC(GET_PRED(p)) = GET_SUCC(p);
    // printf("del_free %x\n",(char *)GET_PRED(p));
    // printf("del_free %x\n",(char *)GET_SUCC(p));
    if(GET_SUCC(p) != NULL){
        GET_PRED(GET_SUCC(p)) = GET_PRED(p);
    }
    
}

/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    // printf("here first_freep  %x\n",(char *)first_freep);
    // printf("here %x\n",(char *)GET_SUCC(first_freep));
    // printf("here first_freep  %x\n",(char *)final_freep);
    // printf("here %x\n",final_freep);
    
    // printf("here %x\n",(char *)FREE_SUCC(first_freep));
    // printf("here %x\n",(char *)GET_SUCC(first_freep));
    // printf("here %x\n",(char *)GET_PRED(final_freep));
    // GET_SUCC(final_freep) = first_freep;
    // printf("here %x\n",(char *)first_freep);
    // printf("here %x\n",(char *)final_freep);
    // printf("here %x\n",(char *)GET_SUCC(first_freep));
    // printf("here %x\n",(char *)GET_PRED(final_freep));
    //初始化堆
    if((heap_listp = mem_sbrk(4*WSIZE) )== (void * )-1){
        return -1;
    }
    

    // printf("here %x\n",(char *)GET_SUCC(first_freep));

    //初始化
    PUT(heap_listp,0);
    first_freep = heap_listp;
    // printf("here %x\n",(char *)GET_SUCC(first_freep));

    // PUT(heap_listp+WSIZE,0);
    PUT(heap_listp+(1*WSIZE),PACK(DSIZE,1));
    PUT(heap_listp+(2*WSIZE),PACK(DSIZE,1));
    PUT(heap_listp+(3*WSIZE),PACK(0,1));
    heap_listp += (DSIZE);

    // printf("heap_listp: %x \n",heap_listp);
    // printf("GET_SIZE : %d \n" ,GET_SIZE(FTRP(heap_listp)));
    void * newp =  extern_heap((1 << 6)/WSIZE);
    // printf("here %x\n",(char * )newp);
    if(newp == NULL){
        return -1;
    }
    return 0;
}

static void *extern_heap(size_t words){
    char *bp;
    size_t size;

    //双字对其
    size = (words % 2 ) ? (words + 1) * WSIZE : words * WSIZE;
    // printf("here %x\n",(char *)GET_SUCC(first_freep));
    if((long) (bp = mem_sbrk(size)) == -1){
        return NULL;
    }
    // printf("GET_SIZE : %x " ,GET_SIZE(((char *)(bp) - DSIZE)));
    // printf("here %x\n",(char *)GET_SUCC(first_freep));
    //将结尾块更新成新的空闲块头部
    PUT(HDRP(bp),PACK(size,0));
    PUT(FTRP(bp),PACK(size,0));
    //新的结尾块
    PUT(HDRP(NEXT_BLKP(bp)),PACK(0,1));
    // printf("extern_heap size: %d \n",size);
    // printf("bp: %x \n",bp);
    //找不到匹配块时合并
    return coalesce(bp);
}

static void *coalesce(void *bp){
    // printf("GET_SIZE : %d " ,GET_SIZE(((char *)(bp) - DSIZE)));
    // printf("coalesce here %x\n",(char *)PREV_BLKP(bp));
    // printf("coalesce here %x\n",(char *)heap_listp);
    // printf("coalesce here %x\n",(char *)FTRP(PREV_BLKP(bp)));
    size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));

    //case 1
    

    if(prev_alloc && next_alloc){
        // mem_sbrk(0);
        // printf("coalesce1 \n");
        insert_new_free(bp);
        // mem_sbrk(0);
    }

    //case 2
    else if(prev_alloc && !next_alloc){
        // printf("coalesce2 \n");
        //剔除后面的空闲链表项
        del_free(NEXT_BLKP(bp));

        size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
        PUT(HDRP(bp),PACK(size,0));
        PUT(FTRP(bp),PACK(size,0));
        
        insert_new_free(bp);
    }


    //case 3
    else if(!prev_alloc && next_alloc){
        size += GET_SIZE(HDRP(PREV_BLKP(bp)));
        PUT(HDRP(PREV_BLKP(bp)),PACK(size,0));
        PUT(FTRP(bp),PACK(size,0));
        bp = PREV_BLKP(bp);
        // printf("coalesce 3\n");
    }
    //case 4
    else{
        // printf("coalesce4 \n");
        //剔除后面的空闲链表项
        del_free(NEXT_BLKP(bp));

        size += GET_SIZE(HDRP(PREV_BLKP(bp)));
        size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
        PUT(HDRP(PREV_BLKP(bp)),PACK(size,0));
        PUT(FTRP(NEXT_BLKP(bp)),PACK(size,0));
        bp = PREV_BLKP(bp);
    }
    return bp;
}

static void * find_fit(size_t size){
    return best_fit(size);
}

static void * best_fit(size_t size){
    // 最佳适配

    size_t min_gap = 1<<30;
    void *best_addr = NULL;
    for (void *bp = GET_SUCC(first_freep); bp != NULL; bp = GET_SUCC(bp)){
        // printf("place succ first_freep %x\n",(char *)GET_SUCC(first_freep));
        // printf("best_fit bp %x\n",(char *)bp);
        // printf("bp %x\n",(char *)final_freep);
        size_t bp_size = GET_SIZE(HDRP(bp));
        // printf("bp_size %d\n",bp_size);
        // printf("size %d\n",size);
        if(bp_size == 0){
            return NULL;
        }
        // printf("need size  %d\n",size);
        // printf("free bp_size  %d\n",bp_size);
        if((bp_size - size) > 0 && (bp_size - size) < min_gap){
            // printf("here \n");
            min_gap = bp_size - size;
            best_addr = bp;
            if(bp_size == size) return best_addr;
        }
        // printf("next best_fit %x\n",(char *)GET_SUCC(bp));
    }
    // printf("best_addr bp %x\n",(char *)best_addr);
    return best_addr;
    
}

static void * first_fit(size_t size){
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
    del_free(bp);
    // printf("place delete :%x\n",bp);
    if((GET_SIZE(HDRP(bp))-size) >= (2*DSIZE)){
        size_t remain_size = GET_SIZE(HDRP(bp))-size;
        PUT(HDRP(bp),PACK(size,1));
        PUT(FTRP(bp),PACK(size,1));
        PUT(HDRP(NEXT_BLKP(bp)),PACK(remain_size,0));
        PUT(FTRP(NEXT_BLKP(bp)),PACK(remain_size,0));

        //分割后加入空闲链表
        
        insert_new_free(NEXT_BLKP(bp));
        // printf("place insert_new_free :%x\n",NEXT_BLKP(bp));
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

    // printf("mm_malloc size :%d \n",size);
    // printf("mm_malloc newsize :%d \n",newsize);
    if((bp = find_fit(newsize)) != NULL){
        
        place(bp,newsize);
        return bp;
    }
    // printf("mm_malloc extern \n");
    extendsize = MAX(newsize,CHUNKSIZE);
    if((bp = extern_heap(extendsize/WSIZE)) == NULL){
        return NULL;
    }
    // printf("mm_malloc bp :%x \n",bp);
    place(bp,newsize);
    return bp;
}



/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr)
{
    // printf("free\n");
    size_t size = GET_SIZE(HDRP(ptr));
    PUT(HDRP(ptr),PACK(size,0));
    PUT(FTRP(ptr),PACK(size,0));
    coalesce(ptr);
    // printf("mm_free first_freep %x\n",(char *)GET_SUCC(first_freep));
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














