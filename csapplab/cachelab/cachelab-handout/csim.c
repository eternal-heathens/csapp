#include <fcntl.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cachelab.h"


extern char *optarg;
typedef __uint64_t uint64_t;
typedef struct lineNode{
    int tag; //根据不同缓存同步机制，不一样，我们是Store的时候直接更新，LRU存了T即可认为是Valid，如果是mesi，需要特殊处理
    uint64_t t_tag; //标志t
} *groupNode; //定义为指针则为数组

groupNode* cache; //多个组的数组

// s位组索引
int  s;
// 组数量
int  S;
//关联度
int E; 
//块偏移
int b;
//块大小
int B;
// 时间
int T = 0;
//结果
int result[3];

void hit(int groupIndex,int t_tag);

int main(int argc, char **argv)
{
    FILE *traceFile;

    //读取参数与文件   
    const char *optstring = "hvsEbt"; 
    int o;
    while ((o = getopt(argc, argv, optstring)) != -1) {
        switch (o) {
            case 'h':
                printf("Optional help flag that prints usage info \n");
                break;
            case 'v':
                printf("Optional verbose flag that displays trace info\n");
                break;
            case 's':
                //用 optarg会segmentfault 
                s = atoi(argv[optind]);
                if(s <= 0) exit(1);
                S = 1 << s;
                break;
            case 'E':
                E = atoi(argv[optind]);
                break;
            case 'b':
                b = atoi(argv[optind]);
                if(b <= 0) exit(1);
                B = 1 << s;
                break;
            case 't':
                traceFile = fopen(argv[optind],"r");
                if (traceFile == NULL) {
                    printf("file null \n");
                    exit(1);
                }
                break;
        }
    }
    printf("111\n");
    //开辟一个数组模拟缓存映射
    cache = (groupNode *)malloc(sizeof(groupNode) * S);
    for(int i = 0; i < S ;i++){
        cache[i] = (struct lineNode *)malloc(sizeof(struct lineNode) * E);
        for(int j = 0; j < E; j++){
            cache[i][j].tag = 0;
        }
    }

    //组选择，行匹配，成功则命中
    //文件按行读取
    char oper;
    uint64_t address;
    int size;
    while(fscanf(traceFile," %c %lx,%d\n",&oper,&address,&size) == 3){
        //I 忽略
        if(oper == 'I') continue;

        //组号、标记
        int groupIndex = (address >> b) & ~(~0u<<s);
        int t_tag = address >> (b+s);
        T++;
        hit(groupIndex,t_tag);


        //如果是M，先Store，再load，再执行一次匹配
        if (oper == 'M') hit(groupIndex,t_tag);


    }
    
    printSummary(result[0], result[1], result[2]);
    return 0;
}

void hit(int groupIndex,int t_tag){
    //查看是否匹配
    groupNode group = cache[groupIndex];
    int min_t_pos = 0; 
    int null_pos = -1;
    for (int i = 0; i < E; i++){
        if(group[i].tag == 0){
            null_pos =i;
        }else{
            if(group[i].t_tag == t_tag){
                //输出hit
                group[i].tag = T;
                result[0]++;
                return;
            }
            if(group[i].tag<group[min_t_pos].tag){
                min_t_pos = i;
            } 
        }
    }
    //失败则从内存读取，更新数组    
    //替换策略，如果组内没满则拆入，满了则LRU替换
    if(null_pos != -1){
        group[null_pos].tag = T;
        group[null_pos].t_tag = t_tag; 
        result[1]++;
        //输出miss 
    }else{
        group[min_t_pos].tag = T;
        group[min_t_pos].t_tag = t_tag; 
        result[1]++;
        result[2]++;
        //输出miss eviction
    }
    return;
}
