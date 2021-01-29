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

    getcontext(&(t->ctx));

    t->ctx.uc_stack.ss_sp = t->stack;
    t->ctx.uc_stack.ss_size = DEFAULT_STACK_SIZE;
    t->ctx.uc_stack.ss_flags = 0;
    t->ctx.uc_link = &(schedule.main);

    schedule.running_thread = id;

    makecontext(&(t->ctx),(void(*)(void))(yieldt_body),1,&schedule);
    swapcontext(&(schedule.main),&(t->ctx));

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