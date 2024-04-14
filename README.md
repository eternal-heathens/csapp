[CSAPP-Lab.md](https://github.com/eternal-heathens/csapp/files/14971010/CSAPP-Lab.md)
1. datalab 为使用优先运算符进行一些位运算，与实现二进制浮点数的计算方法，参杂一些数学证明结果的使用
2. bomb 使用gdb读懂汇编代码中，放入寄存器或栈的值，解除炸弹
3. attack lab 用机器码代表的汇编命令、汇编转机器码工具，通过栈溢出修改栈内容完成攻击，难点在基于栈的模型的指令执行顺序
4. arcLab  part 1主要是熟悉y86指令集的一些指令，写一些小功能， part2是根据书中SEQ阶段的实现，将IIADDQ添加到各个需要的hcl代码，脚步会帮我们转换成varilog，part3 主要是提高指令并行的性能，将流水线架构的性能极致发挥出来，如将IIADDQ从SEQ架构转到PIPE架构中，循环展开减少不必要的判断与跳转，调换指令等减少气泡的产生，难点在于验证所有优化策略的可行性
5. cache lab part1实现基于LRU替换策略的缓存命中模型，part2 基于空间局限性完成矩阵转置的性能优化，难点在于part2矩阵对称时，缓存行数少，对称方向的暂存数据的思考点上
6. shlab，实现一个简易的shell，难点在于信号阻塞、信号发送和信号处理上，特别是信号处理，经常导致信号量对不上，切换shell进程没被唤醒，shell控制进程切换失败
7. mallclab 实现手动的malloc和free 方法，用隐仕链表、显示空闲链表、分离空闲链表等实现，难点在于指针使用上，链表转换和你怎么遍历有关，注意初始化和相关链表操作的处理
8. proxylab 实现一个proxy 服务器，实现基于生产者消费者的线程池模型和读者优先的读写者缓存模型，难点在于part3的信号量的并发死锁上
