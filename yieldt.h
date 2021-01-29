#ifndef MY_YIELDT_H
#define MY_YIELDT_H

#include<ucontext.h>
#include<vector>

#define DEFAULT_STACK_SIZE (1024*128)
#define MAX_YEILD_SIZE  1024

enum ThreadState{FREE,RUNNABLE,RUNNING,SUSPEND};

/*
typedef struct ucontext {  
    struct ucontext *uc_link;  //当当前上下文(如使用makecontext创建的上下文）运行终止时系统会恢复uc_link指向的上下文
    sigset_t         uc_sigmask;  uc_sigmask为该上下文中的阻塞信号集合
    stack_t          uc_stack;  uc_stack为该上下文中使用的栈
    mcontext_t       uc_mcontext;  uc_mcontext保存的上下文的特定机器表示，包括调用线程的特定寄存器等
    ...  
} ucontext_t; 
*/



struct schedule_t;

typedef void (*Fun)(void *arg);

typedef struct yieldt_t
{
    ucontext_t ctx;
    Fun func;
    void *arg;
    enum ThreadState state;     //该线程状态
    char stack[DEFAULT_STACK_SIZE]; 
}yeildt_t;

typedef struct schedule_t
{
    ucontext_t main;
    int running_thread;
    yieldt_t *threads;
    int max_index;  
    
    schedule_t():running_thread(-1),max_index(0){
        threads = new yieldt_t[MAX_YEILD_SIZE];
        for(int i =0;i<MAX_YEILD_SIZE;i++){
            threads[i].state = FREE;
        }
    }

    ~schedule_t(){
        delete [] threads;
    }
}schedule_t;


static void yieldt_body(schedule_t *ps);


//创建用户线程  返回计划中已创建线程的索引
int yieldt_create(schedule_t &schedule,Fun func,void *arg);

/*挂起当前正在运行的线程，切换到主线程*/
void yieldt_yield(schedule_t &schedule);
/*恢复索引等于id的线程*/
void yieldt_resume(schedule_t &schedule,int id);
/*测试调度中的所有线程是否都结束*/
int schedule_finished(const schedule_t &schedule);

#endif