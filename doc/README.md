# 代码风格以及命名
欢迎来到BookOS起源
开始吧！
我们C代码和汇编代码，我们分开说明
## 汇编代码:  
    1.标签都是大写字母开头，单词间没有间隔  
    举例: LableStart, Entry, LoopForFoo123    
    2.宏定义都是大写字母，单词间用下划线隔开  
    举例: LOADER_START_SECTOR, FOO_MAX_NR, TIMES_2019_FAST  
    3.其它的所有命名都是小写  
    举例:  mov byte [0], al  dw 0xaa55  
    汇编的规则就这些，其它的可以自我发挥！
## C代码:  
    1.函数命名: 第一个单词小写，第二个单词也小写，第一个单词和第二个单词用下划线隔开，后面的单词都大写，  
        没有下划线。  
    举例：init, init_cpu, create_windowButton, alloc_kernelMemoryPage，window_setTitle  
    2.结构体命名：结构体名称小写，每个单词之间用下划线隔开，在最后加上_s，并且要写typedef，  
        typedef后的新名字为和前面一样只是最后改成了_t  
    举例：  
    typedef struct foo_program_s  
    {  
        int foo;  
    }foo_program_t;  
    3.注意多实用空格，其它的可以自我发挥。  
