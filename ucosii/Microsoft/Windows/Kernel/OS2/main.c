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

static  void  StartupTask (void  *p_arg);


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

int lcm=1;
void task(void* p_arg) {
    task_para_set* task_data;
    task_data = p_arg;

    while (1) {
        printf("%2d  task(%2d) is running\n", OSTimeGet(), task_data->TaskID);
        if(TaskParameter[OSPrioHighRdy].TaskID == 0) printf("%2d  task(%2d)(%2d)\ttask(%2d)\t%2d\n", OSTimeGet(), task_data->TaskID, task_data->TaskCount, 63, OSCtxSwCtr);
        else if (OSTimeGet()>0 && OSTimeGet()%lcm!=0) printf("%2d  task(%2d)(%2d)\ttask(%2d)\t%2d\n", OSTimeGet(), task_data->TaskID, task_data->TaskCount, 63, OSCtxSwCtr);
        else printf("%2d  task(%2d)(%2d)\ttask(%2d)(%2d)\t%2d\n", OSTimeGet(), task_data->TaskID, task_data->TaskCount, TaskParameter[OSPrioHighRdy].TaskID, TaskParameter[OSPrioHighRdy].TaskCount, OSCtxSwCtr);
        //printf("%d\n", TaskParameter[OSPrioHighRdy].TaskID);
        
        if ((Output_err = fopen_s(&Output_fp, "./Output.txt", "a")) == 0)
        {
            fprintf(Output_fp, "%2d  task(%2d) is running\n", OSTimeGet(), task_data->TaskID);
            if (TaskParameter[OSPrioHighRdy].TaskID == 0) fprintf(Output_fp, "%2d  task(%2d)(%2d)\ttask(%2d)\t%2d\n", OSTimeGet(), task_data->TaskID, task_data->TaskCount, 63, OSCtxSwCtr);
            else if (OSTimeGet() > 0 && OSTimeGet() % lcm != 0) fprintf(Output_fp, "%2d  task(%2d)(%2d)\ttask(%2d)\t%2d\n", OSTimeGet(), task_data->TaskID, task_data->TaskCount, 63, OSCtxSwCtr);
            else fprintf(Output_fp, "%2d  task(%2d)(%2d)\ttask(%2d)(%2d)\t%2d\n", OSTimeGet(), task_data->TaskID, task_data->TaskCount, TaskParameter[OSPrioHighRdy].TaskID, TaskParameter[OSPrioHighRdy].TaskCount, OSCtxSwCtr);
            fclose(Output_fp);
        }
        task_data->TaskCount += 1;
        OSTimeDly(task_data->TaskPeriodic);
    }
}

int  main (void)
{
#if OS_TASK_NAME_EN > 0u
    CPU_INT08U  os_err;
#endif


    CPU_IntInit();

    Mem_Init();                                                 /* Initialize Memory Managment Module                   */
    CPU_IntDis();                                               /* Disable all Interrupts                               */
    CPU_Init();                                                 /* Initialize the uC/CPU services                       */

    OSInit();                                                   /* Initialize uC/OS-II                                  */


    OutFileInit();
    InputFile();

    Task_STK = malloc(TASK_NUMBER * sizeof(int*));

    for (int n = 0; n < TASK_NUMBER; n++) {
        Task_STK[n] = malloc(TASK_STACKSIZE * sizeof(int));
        OSTaskCreateExt(task,
            &TaskParameter[n],
            &Task_STK[n][TASK_STACKSIZE - 1],
            TaskParameter[n].TaskID, //modefied
            TaskParameter[n].TaskID,
            &Task_STK[n][0],
            TASK_STACKSIZE,
            &TaskParameter[n],
            (OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR));
        lcm *= TaskParameter[n].TaskPeriodic;
    }

    printf("%2d\t**********\ttask(%2d)(%2d)\t%2d\n", OSTimeGet(), TaskParameter[0].TaskID, TaskParameter[0].TaskCount, OSCtxSwCtr);
    if ((Output_err = fopen_s(&Output_fp, "./Output.txt", "a")) == 0)
    {
        fprintf(Output_fp, "%2d\t**********\ttask(%2d)(%2d)\t%2d\n", OSTimeGet(), TaskParameter[0].TaskID, TaskParameter[0].TaskCount, OSCtxSwCtr);
        fclose(Output_fp);
    }

#if OS_TASK_NAME_EN > 0u
    OSTaskNameSet(         APP_CFG_STARTUP_TASK_PRIO,
                  (INT8U *)"Startup Task",
                           &os_err);
#endif
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

static  void  StartupTask (void *p_arg)
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

