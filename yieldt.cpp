#include"yieldt.h"



static void yieldt_body(schedule_t *ps)
{
    int id = ps->running_thread;

    if(id != -1){
        yieldt_t *t = &(ps->threads[id]);

        t->func(t->arg);

        t->state = FREE;
        ps->running_thread = -1;
    }
}


//创建用户线程  返回计划中已创建线程的索引
int yieldt_create(schedule_t &schedule,Fun func,void *arg)
{
    int id = 0;
    for(id =0;id < schedule.max_index;++id){
        if(schedule.threads[id].state == FREE){
            break;
        }
    }

    if(id == schedule.max_index){
        schedule.max_index++;
    }

    yieldt_t *t = &(schedule.threads[id]);
    
    t->state = RUNNABLE;
    t->func = func;
    t->arg = arg;

    getcontext(&(t->ctx));          //初始化ctx结构体，将当前的上下文保存到ctx中

    t->ctx.uc_stack.ss_sp = t->stack;
    t->ctx.uc_stack.ss_size = DEFAULT_STACK_SIZE;
    t->ctx.uc_stack.ss_flags = 0;
    t->ctx.uc_link = &(schedule.main);

    schedule.running_thread = id;

    /*
    makecontext修改通过getcontext取得的上下文ucp(这意味着调用makecontext前必须先调用getcontext)。
     然后给该上下文指定一个栈空间ucp->stack，设置后继的上下文ucp->uc_link.

    当上下文通过setcontext或者swapcontext激活后，执行func函数，argc为func的参数个数，后面是func的参数序列。
    当func执行返回后，继承的上下文被激活，如果继承上下文为NULL时，线程退出。
        
    */
    makecontext(&(t->ctx),(void(*)(void))(yieldt_body),1,&schedule);
    swapcontext(&(schedule.main),&(t->ctx));   //保存当前上下文到oucp结构体中，然后激活upc上下文。 

    return id;
}

/*挂起当前正在运行的线程，切换到主线程*/
void yieldt_yield(schedule_t &schedule)
{
    if(schedule.running_thread != -1){
        yieldt_t *t = &(schedule.threads[schedule.running_thread]);
        t->state = SUSPEND;
        schedule.running_thread = -1;

        swapcontext(&(t->ctx),&(schedule.main));
    }
}



/*恢复索引等于id的线程*/
void yieldt_resume(schedule_t &schedule,int id)
{
    if(id < 0 || id >= schedule.max_index){
        return;
    }

    yieldt_t *t = &(schedule.threads[id]);

    if(t->state == SUSPEND){
        swapcontext(&(schedule.main),&(t->ctx));
    }
}
/*测试调度中的所有线程是否都结束*/
int schedule_finished(const schedule_t &schedule)
{
    if(schedule.running_thread != -1){
        return 0;
    }else{
        for(int i =0;i<schedule.max_index;++i){
            if(schedule.threads[i].state != FREE){
                return 0;
            }
        }
    }
    return 1;
}