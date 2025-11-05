

/*
*********************************************************************************************************
*                                            EXAMPLE CODE
*
*               This file is provided as an example on how to use Micrium products.
*
*               Please feel free to use any application code labeled as 'EXAMPLE CODE' in
*               your application products.  Example code may be used as is, in whole or in
*               part, or may be used as a reference only. This file can be modified as
*               required to meet the end-product requirements.
*
*               Please help us continue to provide the Embedded community with the finest
*               software available.  Your honesty is greatly appreciated.
*
*               You can find our product's user manual, API reference, release notes and
*               more information at https://doc.micrium.com.
*               You can contact us at www.micrium.com.
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*
*                                              uC/OS-II
*                                            EXAMPLE CODE
*
* Filename : main.c
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                            INCLUDE FILES
*********************************************************************************************************
*/
#define _CRT_SECURE_NO_WARNINGS
#include  <cpu.h>
#include  <lib_mem.h>
#include  <os.h>

#include  "app_cfg.h"
#define TASK_STACKSIZE 2048





/*
*********************************************************************************************************
*                                            LOCAL DEFINES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                       LOCAL GLOBAL VARIABLES
*********************************************************************************************************
*/

static  OS_STK  StartupTaskStk[APP_CFG_STARTUP_TASK_STK_SIZE];


/*
*********************************************************************************************************
*                                         FUNCTION PROTOTYPES
*********************************************************************************************************
*/

static  void  StartupTask(void* p_arg);


/*
*********************************************************************************************************
*                                                main()
*
* Description : This is the standard entry point for C code.  It is assumed that your code will call
*               main() once you have performed all necessary initialization.
*
* Arguments   : none
*
* Returns     : none
*
* Notes       : none
*********************************************************************************************************
*/

static void task(void* p_arg);

int missDeadline = 0;
int missTask;
void task(void* p_arg) {
    task_para_set* task_data = (task_para_set*)p_arg;

    int timeTag;
    timeTag = OSTimeGet();

    if (task_data->TaskArriveTime > timeTag) OSTimeDly(task_data->TaskArriveTime - timeTag);

    while (missDeadline == 0) {
        while (task_data->TaskRemainTime > 0 && missDeadline == 0) {
            if (missDeadline == 1) break;

            LOG_print(3, "./Output.txt", "%2d  task(%2d) is running\t\n", OSTime, task_data->TaskID);
            timeTag = OSTimeGet();

            while (OSTimeGet() == timeTag) {
                if (missTask != 0 && missDeadline == 0)
                {
                    missDeadline = 1;
                    LOG_print(3, "./Output.txt", "%2d  Missdeadline\ttask(%2d)(%2d)\t-------------------\n", OSTime, missTask, TaskParameter[missTask].TaskCount);
                    break;
                }
            }
        }
        if (missDeadline==1) break;

        OS_ENTER_CRITICAL();
        timeTag = OSTimeGet();

        task_data->TaskDeadLine += task_data->TaskPeriodic;
        task_data->TaskResponseTime = timeTag - task_data->TaskArriveTime;
        task_data->TaskArriveTime += task_data->TaskPeriodic;

        if (timeTag >= task_data->TaskArriveTime) task_data->TaskDelay = 0;
        else task_data->TaskDelay = task_data->TaskArriveTime - timeTag;

        task_data->TaskRemainTime = task_data->TaskExecutionTime;
        OS_EXIT_CRITICAL();

        OSTimeDly(task_data->TaskDelay);

        if (OSTCBCur->CompletedFlag == 1 && ((task_para_set*)(OSTCBCur->OSTCBExtPtr))->TaskDelay == 0) {
            if (OSTCBHighRdy == OSTCBCur) {
                LOG_print(3, "./Output.txt","%2d  Completion\ttask(%2d)(%2d)\ttask(%2d)(%2d)\t%d\t%d\t%d\t\n", 
                    OSTimeGet(),
                    ((task_para_set*)(OSTCBCur->OSTCBExtPtr))->TaskID,
                    ((task_para_set*)(OSTCBCur->OSTCBExtPtr))->TaskCount++,
                    ((task_para_set*)(OSTCBHighRdy->OSTCBExtPtr))->TaskID,
                    ((task_para_set*)(OSTCBHighRdy->OSTCBExtPtr))->TaskCount,
                    ((task_para_set*)(OSTCBCur->OSTCBExtPtr))->TaskResponseTime,
                    ((task_para_set*)(OSTCBCur->OSTCBExtPtr))->TaskResponseTime - ((task_para_set*)(OSTCBCur->OSTCBExtPtr))->TaskExecutionTime, 
                    ((task_para_set*)(OSTCBCur->OSTCBExtPtr))->TaskDelay
                );
                OSTCBCur->CompletedFlag = 0;
            }
        }
    }
    while (1) {}
}

/*void task(void* p_arg) { // with RM
    task_para_set* task_data = p_arg;

    while (1) {
        task_data->TaskRemainTime = task_data->TaskExecutionTIme;
        task_data->TaskStartTime = OSTimeGet();
        task_data->TaskDuration = 0;
        task_data->TaskPrermpTime = 0;

        while (task_data->TaskRemainTime > 0) {
            printf("%2d  task(%2d) is running\n", OSTimeGet(), task_data->TaskID);
            task_data->TaskRemainTime -= 1;
            OSTimeDly(1);
        }

        task_data->TaskCount += 1;
        task_data->TaskDuration = OSTimeGet() - task_data->TaskStartTime;
        task_data->TaskDly = task_data->TaskPeriodic - task_data->TaskDuration;

        if (task_data->TaskDly > 0) OSTimeDly(task_data->TaskDly);
    }
}*/


int  main(void)
{
#if OS_TASK_NAME_EN > 0u
    CPU_INT08U  os_err;
#endif

    OS_TCB* ptcb;
    CPU_IntInit();

    Mem_Init();                                                 /* Initialize Memory Managment Module                   */
    CPU_IntDis();                                               /* Disable all Interrupts                               */
    CPU_Init();                                                 /* Initialize the uC/CPU services                       */

    OSInit();                                                   /* Initialize uC/OS-II                                  */


    OutFileInit();
    InputFile();

    Task_STK = malloc(TASK_NUMBER * sizeof(int*));


    for (int a = 0; a < TASK_NUMBER - 1; a++) {
        for (int b = a + 1; b < TASK_NUMBER; b++) {
            if (TaskParameter[a].TaskPeriodic > TaskParameter[b].TaskPeriodic) {
                task_para_set temp = TaskParameter[a];
                TaskParameter[a] = TaskParameter[b];
                TaskParameter[b] = temp;
            }
        }
    }

    for (int i = 0; i < TASK_NUMBER; i++) TaskParameter[i].TaskPriority = i + 1;

    for (int n = 0; n < TASK_NUMBER; n++)
    {
        Task_STK[n] = malloc(TASK_STACKSIZE * sizeof(int));
        OSTaskCreateExt(task,
            &TaskParameter[n],
            &Task_STK[n][TASK_STACKSIZE - 1],
            TaskParameter[n].TaskPriority,
            TaskParameter[n].TaskID,
            &Task_STK[n][0],
            TASK_STACKSIZE,
            &TaskParameter[n],
            (OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR));

    }

#if OS_TASK_NAME_EN > 0u
    OSTaskNameSet(APP_CFG_STARTUP_TASK_PRIO,
        (INT8U*)"Startup Task",
        &os_err);
#endif
    ptcb = OSTCBList;
    OSTimeSet(0);

    printf("==========TCB linked list==========\n");
    printf("Task \t Prev_TCB_addr   TCB_addr   Next_TCB_addr\n");
    for (int i = TASK_NUMBER;i >= 0;i--) if (OSTCBTbl[i].OSTCBPrio != 0) printf("%2d \t %6x \t %6x \t %6x\n", OSTCBTbl[i].OSTCBPrio, OSTCBTbl[i].OSTCBPrev, &OSTCBTbl[i], OSTCBTbl[i].OSTCBNext);
    printf("\n");

    OSStart();                                                  /* Start multitasking (i.e. give control to uC/OS-II)   */


    while (DEF_ON) {                                            /* Should Never Get Here.                               */
        ;
    }
}


/*
*********************************************************************************************************
*                                            STARTUP TASK
*
* Description : This is an example of a startup task.  As mentioned in the book's text, you MUST
*               initialize the ticker only once multitasking has started.
*
* Arguments   : p_arg   is the argument passed to 'StartupTask()' by 'OSTaskCreate()'.
*
* Returns     : none
*
* Notes       : 1) The first line of code is used to prevent a compiler warning because 'p_arg' is not
*                  used.  The compiler should not generate any code for this statement.
*********************************************************************************************************
*/

static  void  StartupTask(void* p_arg)
{
    (void)p_arg;

    OS_TRACE_INIT();                                            /* Initialize the uC/OS-II Trace recorder               */

#if OS_CFG_STAT_TASK_EN > 0u
    OSStatTaskCPUUsageInit(&err);                               /* Compute CPU capacity with no task running            */
#endif

#ifdef CPU_CFG_INT_DIS_MEAS_EN
    CPU_IntDisMeasMaxCurReset();
#endif

    APP_TRACE_DBG(("uCOS-III is Running...\n\r"));

    while (DEF_TRUE) {                                          /* Task body, always written as an infinite loop.       */
        OSTimeDlyHMSM(0u, 0u, 1u, 0u);
        APP_TRACE_DBG(("Time: %d\n\r", OSTimeGet()));
    }
}


