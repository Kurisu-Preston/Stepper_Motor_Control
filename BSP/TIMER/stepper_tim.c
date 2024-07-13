/**
 ****************************************************************************************************
 * @file        stepper_tim.c
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.0
 * @date        2021-10-19
 * @brief       定时器 驱动代码
 * @license     Copyright (c) 2020-2032, 广州市星翼电子科技有限公司
 ****************************************************************************************************
 * @attention
 *
 * 实验平台:正点原子 STM32F407开发板
 * 在线视频:www.yuanzige.com
 * 技术论坛:www.openedv.com
 * 公司网址:www.alientek.com
 * 购买地址:openedv.taobao.com
 *
 * 修改说明
 * V1.0 20211019
 * 第一次发布
 *
 ****************************************************************************************************
 */


#include "./BSP/TIMER/stepper_tim.h"
#include "./BSP/LED/led.h"
#include "./BSP/STEPPER_MOTOR/stepper_motor.h"
#include "./SYSTEM/usart/usart.h"
#include <stdio.h>
#include <string.h>

/******************************************************************************************/
/* 定时器配置句柄 定义 */

/* 高级定时器PWM */
TIM_HandleTypeDef g_atimx_handle;           /* 定时器x句柄 */
TIM_OC_InitTypeDef g_atimx_oc_chy_handle;   /* 定时器输出句柄 */ 
uint8_t g_run_flag = 0;
/******************************************************************************************/

/**
 * @brief       高级定时器TIMX 通道1、2、3、4 PWM输出初始化
 * @note
 *              高级定时器的时钟来自APB2, 而PCLK2 = 168Mhz, 我们设置PPRE2不分频, 因此
 *              高级定时器时钟 = 168Mhz
 *              定时器溢出时间计算方法: Tout = ((arr + 1) * (psc + 1)) / Ft us.
 *              Ft=定时器工作频率,单位:Mhz
 *
 * @param       arr: 自动重装值
 * @param       psc: 时钟预分频数
 * @retval      无
 */
uint8_t angle_all_1[12], angle_all_2[12];
int symbol = 1;
int pulse_count_1, pulse_count_2 = 0;
void atim_timx_oc_chy_init(uint16_t arr, uint16_t psc)
{
    ATIM_TIMX_PWM_CHY_CLK_ENABLE();                             /* TIMX 时钟使能 */

    
    g_atimx_handle.Instance = ATIM_TIMX_PWM;                    /* 定时器x */
    g_atimx_handle.Init.Prescaler = psc;                        /* 定时器分频 */
    g_atimx_handle.Init.CounterMode = TIM_COUNTERMODE_UP;       /* 向上计数模式 */
    g_atimx_handle.Init.Period = arr;                           /* 自动重装载值 */
    g_atimx_handle.Init.ClockDivision=TIM_CLOCKDIVISION_DIV1;   /* 分频因子 */
    g_atimx_handle.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE; /*使能TIMx_ARR进行缓冲*/
    g_atimx_handle.Init.RepetitionCounter = 0;                  /* 开始时不计数*/
    HAL_TIM_PWM_Init(&g_atimx_handle);                          /* 初始化PWM */
    
    g_atimx_oc_chy_handle.OCMode = TIM_OCMODE_PWM1;             /* 模式选择PWM1 */
    g_atimx_oc_chy_handle.Pulse = arr/2;
    g_atimx_oc_chy_handle.OCPolarity = TIM_OCPOLARITY_HIGH;     /* 输出比较极性为高 */
    g_atimx_oc_chy_handle.OCNPolarity = TIM_OCNPOLARITY_HIGH;
    g_atimx_oc_chy_handle.OCFastMode = TIM_OCFAST_DISABLE;
    g_atimx_oc_chy_handle.OCIdleState = TIM_OCIDLESTATE_RESET;
    g_atimx_oc_chy_handle.OCNIdleState = TIM_OCNIDLESTATE_RESET;
    HAL_TIM_PWM_ConfigChannel(&g_atimx_handle, &g_atimx_oc_chy_handle, ATIM_TIMX_PWM_CH1); /* 配置TIMx通道y */
    HAL_TIM_PWM_ConfigChannel(&g_atimx_handle, &g_atimx_oc_chy_handle, ATIM_TIMX_PWM_CH2); /* 配置TIMx通道y */   
    HAL_TIM_PWM_ConfigChannel(&g_atimx_handle, &g_atimx_oc_chy_handle, ATIM_TIMX_PWM_CH3); /* 配置TIMx通道y */
    HAL_TIM_PWM_ConfigChannel(&g_atimx_handle, &g_atimx_oc_chy_handle, ATIM_TIMX_PWM_CH4); /* 配置TIMx通道y */
  
    
}


/**
 * @brief       定时器底层驱动，时钟使能，引脚配置
                此函数会被HAL_TIM_PWM_Init()调用
 * @param       htim:定时器句柄
 * @retval      无
 */
void HAL_TIM_PWM_MspInit(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == ATIM_TIMX_PWM)
    {
        GPIO_InitTypeDef gpio_init_struct;
        ATIM_TIMX_PWM_CHY_CLK_ENABLE();                             /* 开启通道y的GPIO时钟 */
        ATIM_TIMX_PWM_CH1_GPIO_CLK_ENABLE();                        /* IO时钟使能 */
        ATIM_TIMX_PWM_CH2_GPIO_CLK_ENABLE();                        /* IO时钟使能 */
        ATIM_TIMX_PWM_CH3_GPIO_CLK_ENABLE();                        /* IO时钟使能 */
        ATIM_TIMX_PWM_CH4_GPIO_CLK_ENABLE();                        /* IO时钟使能 */

        gpio_init_struct.Pin = ATIM_TIMX_PWM_CH1_GPIO_PIN;          /* 通道y的GPIO口 */
        gpio_init_struct.Mode = GPIO_MODE_AF_PP;                    /* 复用推挽输出 */
        gpio_init_struct.Pull = GPIO_PULLUP;                        /* 上拉 */
        gpio_init_struct.Speed = GPIO_SPEED_FREQ_HIGH;              /* 高速 */
        gpio_init_struct.Alternate = ATIM_TIMX_PWM_CHY_GPIO_AF;     /* 端口复用 */
        HAL_GPIO_Init(ATIM_TIMX_PWM_CH1_GPIO_PORT, &gpio_init_struct);
        
        gpio_init_struct.Pin = ATIM_TIMX_PWM_CH2_GPIO_PIN;          /* 通道y的GPIO口 */
        HAL_GPIO_Init(ATIM_TIMX_PWM_CH2_GPIO_PORT, &gpio_init_struct);
       
        gpio_init_struct.Pin = ATIM_TIMX_PWM_CH3_GPIO_PIN;          /* 通道y的GPIO口 */
        HAL_GPIO_Init(ATIM_TIMX_PWM_CH3_GPIO_PORT, &gpio_init_struct);
        
        gpio_init_struct.Pin = ATIM_TIMX_PWM_CH4_GPIO_PIN;          /* 通道y的GPIO口 */
        HAL_GPIO_Init(ATIM_TIMX_PWM_CH4_GPIO_PORT, &gpio_init_struct);
        
        HAL_NVIC_SetPriority(ATIM_TIMX_UP_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(ATIM_TIMX_UP_IRQn);
    }
}

/**
 * @brief       高级定时器共用中断服务函数
 * @param       无
 * @retval      无
 */
void ATIM_TIMX_UP_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&g_atimx_handle);            /* 定时器共用处理函数 */
}

/* 中断回调函数 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if(htim->Instance==TIM8 && symbol == 1)
    {
        g_run_flag = 1;                             /* 标志位置一 */
        g_stepper.pulse_count--;                    /* 每一个完整的脉冲就-- */
        if(g_stepper.dir == CW)
        {
           g_stepper.add_pulse_count++;             /* 绝对位置++ */
           pulse_count_1++;
        }else
        {
           g_stepper.add_pulse_count--;             /* 绝对位置-- */
           pulse_count_1--;
        }

        if(g_stepper.pulse_count<=0)                /* 当脉冲数等于0的时候 代表需要发送的脉冲个数已完成，停止定时器输出 */
        {
            printf("电机1累计旋转的角度:%d\r\n",(int)(pulse_count_1*MAX_STEP_ANGLE));  /* 打印累计转动了多少角度 */
            printf("%d\r\n", symbol);
            memset(angle_all_1, 0, sizeof(angle_all_1)); /* 打印总旋转角度 */
            sprintf((char *)angle_all_1,"TOTAL1:%d",(int)(pulse_count_1*MAX_STEP_ANGLE));
            int len2 = strlen((char *)angle_all_1);
            for (int j = len2; j < sizeof(angle_all_1) - 1; j++) 
            {
                angle_all_1[j] = ' ';
            }
            stepper_stop(STEPPER_MOTOR_1);          /* 停止接口一输出 */
            g_run_flag = 0;
        }
    }
    if(htim->Instance==TIM8 && symbol == 2)
    {
        g_run_flag = 1;                             /* 标志位置一 */
        g_stepper.pulse_count--;                    /* 每一个完整的脉冲就-- */
        if(g_stepper.dir == CW)
        {
           g_stepper.add_pulse_count++;             /* 绝对位置++ */
           pulse_count_2++;
        }else
        {
           g_stepper.add_pulse_count--;             /* 绝对位置-- */
           pulse_count_2--;
        }

        if(g_stepper.pulse_count<=0)                /* 当脉冲数等于0的时候 代表需要发送的脉冲个数已完成，停止定时器输出 */
        {
            printf("电机2累计旋转的角度:%d\r\n",(int)(pulse_count_2*MAX_STEP_ANGLE));  /* 打印累计转动了多少角度 */
            printf("%d\r\n", symbol);
            memset(angle_all_2, 0, sizeof(angle_all_2)); /* 打印总旋转角度 */
            sprintf((char *)angle_all_2,"TOTAL2:%d",(int)(pulse_count_2*MAX_STEP_ANGLE));
            int len3 = strlen((char *)angle_all_2);
            for (int j = len3; j < sizeof(angle_all_2) - 1; j++) 
            {
                angle_all_2[j] = ' ';
            }
            stepper_stop(STEPPER_MOTOR_2);          /* 停止接口二输出 */
            g_run_flag = 0;
        }
    }
    if(htim->Instance==TIM8 && symbol == 3)
    {
        g_run_flag = 1;                             /* 标志位置一 */
        g_stepper.pulse_count--;                    /* 每一个完整的脉冲就-- */
        if(g_stepper.dir == CW)
        {
           g_stepper.add_pulse_count++;             /* 绝对位置++ */
           pulse_count_2++;
        }else
        {
           g_stepper.add_pulse_count--;             /* 绝对位置-- */
           pulse_count_2--;
        }

        if(g_stepper.pulse_count<=0)                /* 当脉冲数等于0的时候 代表需要发送的脉冲个数已完成，停止定时器输出 */
        {
            memset(angle_all_1, 0, sizeof(angle_all_1)); /* 打印总旋转角度 */
            sprintf((char *)angle_all_1,"TOTAL1:%d",(int)(pulse_count_1*MAX_STEP_ANGLE));
            int len2 = strlen((char *)angle_all_1);
            for (int j = len2; j < sizeof(angle_all_1) - 1; j++) 
            {
                angle_all_1[j] = ' ';
            }
            memset(angle_all_2, 0, sizeof(angle_all_2)); /* 打印总旋转角度 */
            sprintf((char *)angle_all_2,"TOTAL2:%d",(int)(pulse_count_2*MAX_STEP_ANGLE));
            int len3 = strlen((char *)angle_all_2);
            for (int j = len3; j < sizeof(angle_all_2) - 1; j++) 
            {
                angle_all_2[j] = ' ';
            }
            stepper_stop(STEPPER_MOTOR_1);          /* 停止接口一输出 */
            stepper_stop(STEPPER_MOTOR_2);          /* 停止接口二输出 */
            g_run_flag = 0;
        }
    }
    if(htim->Instance==TIM8 && symbol == 4)
    {
        g_run_flag = 1;                             /* 标志位置一 */
        g_stepper.pulse_count--;                    /* 每一个完整的脉冲就-- */
        if(g_stepper.dir == CW)
        {
           g_stepper.add_pulse_count++;             /* 绝对位置++ */
        }else
        {
           g_stepper.add_pulse_count--;             /* 绝对位置-- */
        }

        if(g_stepper.pulse_count<=0)                /* 当脉冲数等于0的时候 代表需要发送的脉冲个数已完成，停止定时器输出 */
        {
            printf("重置电机\r\n");
            printf("%d\r\n", symbol);
            stepper_stop(STEPPER_MOTOR_1);
            g_run_flag = 0;
        }
    }
    if(htim->Instance==TIM8 && symbol == 5)
    {
        g_run_flag = 1;                             /* 标志位置一 */
        g_stepper.pulse_count--;                    /* 每一个完整的脉冲就-- */
        if(g_stepper.dir == CW)
        {
           g_stepper.add_pulse_count++;             /* 绝对位置++ */
        }else
        {
           g_stepper.add_pulse_count--;             /* 绝对位置-- */
        }

        if(g_stepper.pulse_count<=0)                /* 当脉冲数等于0的时候 代表需要发送的脉冲个数已完成，停止定时器输出 */
        {
            printf("重置电机\r\n");
            printf("%d\r\n", symbol);
            stepper_stop(STEPPER_MOTOR_2);
            g_run_flag = 0;
        }
    }
}

