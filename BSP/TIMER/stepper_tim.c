/**
 ****************************************************************************************************
 * @file        stepper_tim.c
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.0
 * @date        2021-10-19
 * @brief       ��ʱ�� ��������
 * @license     Copyright (c) 2020-2032, ������������ӿƼ����޹�˾
 ****************************************************************************************************
 * @attention
 *
 * ʵ��ƽ̨:����ԭ�� STM32F407������
 * ������Ƶ:www.yuanzige.com
 * ������̳:www.openedv.com
 * ��˾��ַ:www.alientek.com
 * �����ַ:openedv.taobao.com
 *
 * �޸�˵��
 * V1.0 20211019
 * ��һ�η���
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
/* ��ʱ�����þ�� ���� */

/* �߼���ʱ��PWM */
TIM_HandleTypeDef g_atimx_handle;           /* ��ʱ��x��� */
TIM_OC_InitTypeDef g_atimx_oc_chy_handle;   /* ��ʱ�������� */ 
uint8_t g_run_flag = 0;
/******************************************************************************************/

/**
 * @brief       �߼���ʱ��TIMX ͨ��1��2��3��4 PWM�����ʼ��
 * @note
 *              �߼���ʱ����ʱ������APB2, ��PCLK2 = 168Mhz, ��������PPRE2����Ƶ, ���
 *              �߼���ʱ��ʱ�� = 168Mhz
 *              ��ʱ�����ʱ����㷽��: Tout = ((arr + 1) * (psc + 1)) / Ft us.
 *              Ft=��ʱ������Ƶ��,��λ:Mhz
 *
 * @param       arr: �Զ���װֵ
 * @param       psc: ʱ��Ԥ��Ƶ��
 * @retval      ��
 */
uint8_t angle_all_1[12], angle_all_2[12];
int symbol = 1;
int pulse_count_1, pulse_count_2 = 0;
void atim_timx_oc_chy_init(uint16_t arr, uint16_t psc)
{
    ATIM_TIMX_PWM_CHY_CLK_ENABLE();                             /* TIMX ʱ��ʹ�� */

    
    g_atimx_handle.Instance = ATIM_TIMX_PWM;                    /* ��ʱ��x */
    g_atimx_handle.Init.Prescaler = psc;                        /* ��ʱ����Ƶ */
    g_atimx_handle.Init.CounterMode = TIM_COUNTERMODE_UP;       /* ���ϼ���ģʽ */
    g_atimx_handle.Init.Period = arr;                           /* �Զ���װ��ֵ */
    g_atimx_handle.Init.ClockDivision=TIM_CLOCKDIVISION_DIV1;   /* ��Ƶ���� */
    g_atimx_handle.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE; /*ʹ��TIMx_ARR���л���*/
    g_atimx_handle.Init.RepetitionCounter = 0;                  /* ��ʼʱ������*/
    HAL_TIM_PWM_Init(&g_atimx_handle);                          /* ��ʼ��PWM */
    
    g_atimx_oc_chy_handle.OCMode = TIM_OCMODE_PWM1;             /* ģʽѡ��PWM1 */
    g_atimx_oc_chy_handle.Pulse = arr/2;
    g_atimx_oc_chy_handle.OCPolarity = TIM_OCPOLARITY_HIGH;     /* ����Ƚϼ���Ϊ�� */
    g_atimx_oc_chy_handle.OCNPolarity = TIM_OCNPOLARITY_HIGH;
    g_atimx_oc_chy_handle.OCFastMode = TIM_OCFAST_DISABLE;
    g_atimx_oc_chy_handle.OCIdleState = TIM_OCIDLESTATE_RESET;
    g_atimx_oc_chy_handle.OCNIdleState = TIM_OCNIDLESTATE_RESET;
    HAL_TIM_PWM_ConfigChannel(&g_atimx_handle, &g_atimx_oc_chy_handle, ATIM_TIMX_PWM_CH1); /* ����TIMxͨ��y */
    HAL_TIM_PWM_ConfigChannel(&g_atimx_handle, &g_atimx_oc_chy_handle, ATIM_TIMX_PWM_CH2); /* ����TIMxͨ��y */   
    HAL_TIM_PWM_ConfigChannel(&g_atimx_handle, &g_atimx_oc_chy_handle, ATIM_TIMX_PWM_CH3); /* ����TIMxͨ��y */
    HAL_TIM_PWM_ConfigChannel(&g_atimx_handle, &g_atimx_oc_chy_handle, ATIM_TIMX_PWM_CH4); /* ����TIMxͨ��y */
  
    
}


/**
 * @brief       ��ʱ���ײ�������ʱ��ʹ�ܣ���������
                �˺����ᱻHAL_TIM_PWM_Init()����
 * @param       htim:��ʱ�����
 * @retval      ��
 */
void HAL_TIM_PWM_MspInit(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == ATIM_TIMX_PWM)
    {
        GPIO_InitTypeDef gpio_init_struct;
        ATIM_TIMX_PWM_CHY_CLK_ENABLE();                             /* ����ͨ��y��GPIOʱ�� */
        ATIM_TIMX_PWM_CH1_GPIO_CLK_ENABLE();                        /* IOʱ��ʹ�� */
        ATIM_TIMX_PWM_CH2_GPIO_CLK_ENABLE();                        /* IOʱ��ʹ�� */
        ATIM_TIMX_PWM_CH3_GPIO_CLK_ENABLE();                        /* IOʱ��ʹ�� */
        ATIM_TIMX_PWM_CH4_GPIO_CLK_ENABLE();                        /* IOʱ��ʹ�� */

        gpio_init_struct.Pin = ATIM_TIMX_PWM_CH1_GPIO_PIN;          /* ͨ��y��GPIO�� */
        gpio_init_struct.Mode = GPIO_MODE_AF_PP;                    /* ����������� */
        gpio_init_struct.Pull = GPIO_PULLUP;                        /* ���� */
        gpio_init_struct.Speed = GPIO_SPEED_FREQ_HIGH;              /* ���� */
        gpio_init_struct.Alternate = ATIM_TIMX_PWM_CHY_GPIO_AF;     /* �˿ڸ��� */
        HAL_GPIO_Init(ATIM_TIMX_PWM_CH1_GPIO_PORT, &gpio_init_struct);
        
        gpio_init_struct.Pin = ATIM_TIMX_PWM_CH2_GPIO_PIN;          /* ͨ��y��GPIO�� */
        HAL_GPIO_Init(ATIM_TIMX_PWM_CH2_GPIO_PORT, &gpio_init_struct);
       
        gpio_init_struct.Pin = ATIM_TIMX_PWM_CH3_GPIO_PIN;          /* ͨ��y��GPIO�� */
        HAL_GPIO_Init(ATIM_TIMX_PWM_CH3_GPIO_PORT, &gpio_init_struct);
        
        gpio_init_struct.Pin = ATIM_TIMX_PWM_CH4_GPIO_PIN;          /* ͨ��y��GPIO�� */
        HAL_GPIO_Init(ATIM_TIMX_PWM_CH4_GPIO_PORT, &gpio_init_struct);
        
        HAL_NVIC_SetPriority(ATIM_TIMX_UP_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(ATIM_TIMX_UP_IRQn);
    }
}

/**
 * @brief       �߼���ʱ�������жϷ�����
 * @param       ��
 * @retval      ��
 */
void ATIM_TIMX_UP_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&g_atimx_handle);            /* ��ʱ�����ô����� */
}

/* �жϻص����� */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if(htim->Instance==TIM8 && symbol == 1)
    {
        g_run_flag = 1;                             /* ��־λ��һ */
        g_stepper.pulse_count--;                    /* ÿһ�������������-- */
        if(g_stepper.dir == CW)
        {
           g_stepper.add_pulse_count++;             /* ����λ��++ */
           pulse_count_1++;
        }else
        {
           g_stepper.add_pulse_count--;             /* ����λ��-- */
           pulse_count_1--;
        }

        if(g_stepper.pulse_count<=0)                /* ������������0��ʱ�� ������Ҫ���͵������������ɣ�ֹͣ��ʱ����� */
        {
            printf("���1�ۼ���ת�ĽǶ�:%d\r\n",(int)(pulse_count_1*MAX_STEP_ANGLE));  /* ��ӡ�ۼ�ת���˶��ٽǶ� */
            printf("%d\r\n", symbol);
            memset(angle_all_1, 0, sizeof(angle_all_1)); /* ��ӡ����ת�Ƕ� */
            sprintf((char *)angle_all_1,"TOTAL1:%d",(int)(pulse_count_1*MAX_STEP_ANGLE));
            int len2 = strlen((char *)angle_all_1);
            for (int j = len2; j < sizeof(angle_all_1) - 1; j++) 
            {
                angle_all_1[j] = ' ';
            }
            stepper_stop(STEPPER_MOTOR_1);          /* ֹͣ�ӿ�һ��� */
            g_run_flag = 0;
        }
    }
    if(htim->Instance==TIM8 && symbol == 2)
    {
        g_run_flag = 1;                             /* ��־λ��һ */
        g_stepper.pulse_count--;                    /* ÿһ�������������-- */
        if(g_stepper.dir == CW)
        {
           g_stepper.add_pulse_count++;             /* ����λ��++ */
           pulse_count_2++;
        }else
        {
           g_stepper.add_pulse_count--;             /* ����λ��-- */
           pulse_count_2--;
        }

        if(g_stepper.pulse_count<=0)                /* ������������0��ʱ�� ������Ҫ���͵������������ɣ�ֹͣ��ʱ����� */
        {
            printf("���2�ۼ���ת�ĽǶ�:%d\r\n",(int)(pulse_count_2*MAX_STEP_ANGLE));  /* ��ӡ�ۼ�ת���˶��ٽǶ� */
            printf("%d\r\n", symbol);
            memset(angle_all_2, 0, sizeof(angle_all_2)); /* ��ӡ����ת�Ƕ� */
            sprintf((char *)angle_all_2,"TOTAL2:%d",(int)(pulse_count_2*MAX_STEP_ANGLE));
            int len3 = strlen((char *)angle_all_2);
            for (int j = len3; j < sizeof(angle_all_2) - 1; j++) 
            {
                angle_all_2[j] = ' ';
            }
            stepper_stop(STEPPER_MOTOR_2);          /* ֹͣ�ӿڶ���� */
            g_run_flag = 0;
        }
    }
    if(htim->Instance==TIM8 && symbol == 3)
    {
        g_run_flag = 1;                             /* ��־λ��һ */
        g_stepper.pulse_count--;                    /* ÿһ�������������-- */
        if(g_stepper.dir == CW)
        {
           g_stepper.add_pulse_count++;             /* ����λ��++ */
           pulse_count_2++;
        }else
        {
           g_stepper.add_pulse_count--;             /* ����λ��-- */
           pulse_count_2--;
        }

        if(g_stepper.pulse_count<=0)                /* ������������0��ʱ�� ������Ҫ���͵������������ɣ�ֹͣ��ʱ����� */
        {
            memset(angle_all_1, 0, sizeof(angle_all_1)); /* ��ӡ����ת�Ƕ� */
            sprintf((char *)angle_all_1,"TOTAL1:%d",(int)(pulse_count_1*MAX_STEP_ANGLE));
            int len2 = strlen((char *)angle_all_1);
            for (int j = len2; j < sizeof(angle_all_1) - 1; j++) 
            {
                angle_all_1[j] = ' ';
            }
            memset(angle_all_2, 0, sizeof(angle_all_2)); /* ��ӡ����ת�Ƕ� */
            sprintf((char *)angle_all_2,"TOTAL2:%d",(int)(pulse_count_2*MAX_STEP_ANGLE));
            int len3 = strlen((char *)angle_all_2);
            for (int j = len3; j < sizeof(angle_all_2) - 1; j++) 
            {
                angle_all_2[j] = ' ';
            }
            stepper_stop(STEPPER_MOTOR_1);          /* ֹͣ�ӿ�һ��� */
            stepper_stop(STEPPER_MOTOR_2);          /* ֹͣ�ӿڶ���� */
            g_run_flag = 0;
        }
    }
    if(htim->Instance==TIM8 && symbol == 4)
    {
        g_run_flag = 1;                             /* ��־λ��һ */
        g_stepper.pulse_count--;                    /* ÿһ�������������-- */
        if(g_stepper.dir == CW)
        {
           g_stepper.add_pulse_count++;             /* ����λ��++ */
        }else
        {
           g_stepper.add_pulse_count--;             /* ����λ��-- */
        }

        if(g_stepper.pulse_count<=0)                /* ������������0��ʱ�� ������Ҫ���͵������������ɣ�ֹͣ��ʱ����� */
        {
            printf("���õ��\r\n");
            printf("%d\r\n", symbol);
            stepper_stop(STEPPER_MOTOR_1);
            g_run_flag = 0;
        }
    }
    if(htim->Instance==TIM8 && symbol == 5)
    {
        g_run_flag = 1;                             /* ��־λ��һ */
        g_stepper.pulse_count--;                    /* ÿһ�������������-- */
        if(g_stepper.dir == CW)
        {
           g_stepper.add_pulse_count++;             /* ����λ��++ */
        }else
        {
           g_stepper.add_pulse_count--;             /* ����λ��-- */
        }

        if(g_stepper.pulse_count<=0)                /* ������������0��ʱ�� ������Ҫ���͵������������ɣ�ֹͣ��ʱ����� */
        {
            printf("���õ��\r\n");
            printf("%d\r\n", symbol);
            stepper_stop(STEPPER_MOTOR_2);
            g_run_flag = 0;
        }
    }
}

