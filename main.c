

#include "./SYSTEM/sys/sys.h"
#include "./SYSTEM/usart/usart.h"
#include "./SYSTEM/delay/delay.h"
#include "./BSP/LED/led.h"
#include "./BSP/TIMER/stepper_tim.h"
#include "./BSP/KEY/key.h"
#include "./BSP/LCD/lcd.h"
#include "./BSP/STEPPER_MOTOR/stepper_motor.h"
#include <stdio.h>
#include <string.h>
#include "./BSP/TOUCH/touch.h"



extern uint8_t g_run_flag;

int main(void)
{     
    uint8_t key,t;
    int angle = 0;
    uint8_t angle_print[12], reset_log_1[12], reset_log_2[12];

    
    HAL_Init();
    sys_stm32_clock_init(336, 8, 2, 7);
    delay_init(168);
    usart_init(115200);
    led_init();
    key_init();
    lcd_init();
    stepper_init(16800 - 1, 168 - 1);
    tp_dev.init();
    

    g_point_color = WHITE;
    g_back_color  = BLACK;
    lcd_show_string(10,10,200,16,16,"Stepper Motor Control",g_point_color);
    lcd_show_string(10,30,200,16,16,"KEY0:angle ++",g_point_color);
    lcd_show_string(10,50,200,16,16,"KEY1:angle --",g_point_color);
    lcd_show_string(10,70,200,16,16,"KEY2:start",g_point_color);
    lcd_show_string(10,90,200,16,16,"MODE: MOTOR1",g_point_color);
    lcd_show_string(10,110,200,16,16,"MODE: MOTOR2",g_point_color);
    lcd_show_string(10,130,200,16,16,"MODE: MOTOR1&2",g_point_color);
    lcd_show_string(10,150,200,16,16,"MODE: RESET1",g_point_color);
    lcd_show_string(10,170,200,16,16,"MODE: RESET2",g_point_color);
    
    
    while (1)
    {   
        memset(angle_print, 0, sizeof(angle_print));
        sprintf((char *)angle_print,"ANGLE:%d",angle);
        int len = strlen((char *)angle_print);
        for (int j = len; j < sizeof(angle_print) - 1; j++) 
        {
            angle_print[j] = ' '; 
        }
        sprintf((char *)reset_log_1,"           ");
        sprintf((char *)reset_log_2,"           ");

        key = key_scan(0);
        
        tp_dev.scan(0);
        if (tp_dev.sta & TP_PRES_DOWN)
        {
            if (tp_dev.x[0] < lcddev.width && tp_dev.y[0] < lcddev.height)
            {
                if (tp_dev.x[0] > 10 && tp_dev.x[0] < 210 && tp_dev.y[0] > 90 && tp_dev.y[0] < 106)
                {
                    lcd_show_string(10,90,200,16,16,"MODE: MOTOR1",BLUE);
                    lcd_show_string(10,110,200,16,16,"MODE: MOTOR2",g_point_color);
                    lcd_show_string(10,130,200,16,16,"MODE: MOTOR1&2",g_point_color);
                    lcd_show_string(10,150,200,16,16,"MODE: RESET1",g_point_color);
                    lcd_show_string(10,170,200,16,16,"MODE: RESET2",g_point_color);
                    symbol = 1;
                }
                if (tp_dev.x[0] > 10 && tp_dev.x[0] < 210 && tp_dev.y[0] > 110 && tp_dev.y[0] < 126)
                {
                    lcd_show_string(10,90,200,16,16,"MODE: MOTOR1",g_point_color);
                    lcd_show_string(10,110,200,16,16,"MODE: MOTOR2",BLUE);
                    lcd_show_string(10,130,200,16,16,"MODE: MOTOR1&2",g_point_color);
                    lcd_show_string(10,150,200,16,16,"MODE: RESET1",g_point_color);
                    lcd_show_string(10,170,200,16,16,"MODE: RESET2",g_point_color);
                    symbol = 2;
                }
                if (tp_dev.x[0] > 10 && tp_dev.x[0] < 210 && tp_dev.y[0] > 130 && tp_dev.y[0] < 146)
                {
                    lcd_show_string(10,90,200,16,16,"MODE: MOTOR1",g_point_color);
                    lcd_show_string(10,110,200,16,16,"MODE: MOTOR2",g_point_color);
                    lcd_show_string(10,130,200,16,16,"MODE: MOTOR1&2",BLUE);
                    lcd_show_string(10,150,200,16,16,"MODE: RESET1",g_point_color);
                    lcd_show_string(10,170,200,16,16,"MODE: RESET2",g_point_color);
                    symbol = 3;
                }
                if (tp_dev.x[0] > 10 && tp_dev.x[0] < 210 && tp_dev.y[0] > 150 && tp_dev.y[0] < 166)
                {
                    lcd_show_string(10,90,200,16,16,"MODE: MOTOR1",g_point_color);
                    lcd_show_string(10,110,200,16,16,"MODE: MOTOR2",g_point_color);
                    lcd_show_string(10,130,200,16,16,"MODE: MOTOR1&2",g_point_color);
                    lcd_show_string(10,150,200,16,16,"MODE: RESET1",BLUE);
                    lcd_show_string(10,170,200,16,16,"MODE: RESET2",g_point_color);
                    symbol = 4;
                }
                if (tp_dev.x[0] > 10 && tp_dev.x[0] < 210 && tp_dev.y[0] > 170 && tp_dev.y[0] < 186)
                {
                    lcd_show_string(10,90,200,16,16,"MODE: MOTOR1",g_point_color);
                    lcd_show_string(10,110,200,16,16,"MODE: MOTOR2",g_point_color);
                    lcd_show_string(10,130,200,16,16,"MODE: MOTOR1&2",g_point_color);
                    lcd_show_string(10,150,200,16,16,"MODE: RESET1",g_point_color);
                    lcd_show_string(10,170,200,16,16,"MODE: RESET2",BLUE);
                    symbol = 5;
                }
            }
        }
        else 
        {
            delay_ms(10);
        }
        
        if(key == KEY0_PRES)
        {
            if(g_run_flag == 0)
            {
                angle += 9;
                if(angle >= 0)
                {
                    g_stepper.angle = angle;
                    g_stepper.dir = CW;
                }else 
                {
                    g_stepper.angle = -angle;
                    g_stepper.dir = CCW;
                }
                memset(angle_print, 0, sizeof(angle_print));
                sprintf((char *)angle_print,"ANGLE:%d",angle);
                int len = strlen((char *)angle_print);
                for (int j = len; j < sizeof(angle_print) - 1; j++) 
                {
                    angle_print[j] = ' '; 
                }
            }
        }
        else if(key == KEY1_PRES)
        {
            if(g_run_flag == 0)
            {
               angle -= 9;
                if(angle >= 0)
                {
                    g_stepper.angle = angle;
                    g_stepper.dir = CW;
                }else 
                {
                    g_stepper.angle = -angle;
                    g_stepper.dir = CCW;
                }
                memset(angle_print, 0, sizeof(angle_print));
                sprintf((char *)angle_print,"ANGLE:%d",angle);
                int len = strlen((char *)angle_print);
                for (int j = len; j < sizeof(angle_print) - 1; j++) 
                {
                    angle_print[j] = ' '; 
                }
            }
        }
        else if(key == KEY2_PRES)
        {         
            if(g_run_flag == 0 && symbol == 1)
            {
                angle = 0;
                stepper_set_angle(g_stepper.angle, g_stepper.dir, STEPPER_MOTOR_1);
                g_stepper.angle = angle;
            }
            if(g_run_flag == 0 && symbol == 2)
            {
                angle = 0;
                stepper_set_angle(g_stepper.angle, g_stepper.dir, STEPPER_MOTOR_2);
                g_stepper.angle = angle;
            }
            if(g_run_flag == 0 && symbol == 3)
            {
                angle = 0;
                stepper_set_angle(g_stepper.angle, g_stepper.dir, STEPPER_MOTOR_1);
                stepper_set_angle(g_stepper.angle, g_stepper.dir, STEPPER_MOTOR_2);
                g_stepper.angle = angle;
            }
            if(g_run_flag == 0 && symbol == 4)
            {
                if((int)(-pulse_count_1*MAX_STEP_ANGLE) >= 0)
                {
                    g_stepper.angle = (int)(-pulse_count_1*MAX_STEP_ANGLE);
                    g_stepper.dir = CW;
                }else 
                {
                    g_stepper.angle = -(int)(-pulse_count_1*MAX_STEP_ANGLE);
                    g_stepper.dir = CCW;
                }
                stepper_set_angle(g_stepper.angle, g_stepper.dir, STEPPER_MOTOR_1);
                sprintf((char *)reset_log_1,"MOTOR1RESET");
                pulse_count_1 = 0;
                memset(angle_all_1, 0, sizeof(angle_all_1));
                sprintf((char *)angle_all_1,"TOTAL1:%d",(int)(pulse_count_1*MAX_STEP_ANGLE));
                int len2 = strlen((char *)angle_all_1);
                for (int j = len2; j < sizeof(angle_all_1) - 1; j++) 
                {
                    angle_all_1[j] = ' ';
                }
                lcd_show_string(10,250,200,16,16,(char *)reset_log_1, RED);
                delay_ms(1000);
            }
            if(g_run_flag == 0 && symbol == 5)
            {    
                if((int)(-pulse_count_2*MAX_STEP_ANGLE) >= 0)
                {
                    g_stepper.angle = (int)(-pulse_count_2*MAX_STEP_ANGLE);
                    g_stepper.dir = CW;
                }else 
                {
                    g_stepper.angle = -(int)(-pulse_count_2*MAX_STEP_ANGLE);
                    g_stepper.dir = CCW;
                }
                stepper_set_angle(g_stepper.angle, g_stepper.dir, STEPPER_MOTOR_2);
                sprintf((char *)reset_log_2,"MOTOR2RESET");
                pulse_count_2 = 0;
                memset(angle_all_2, 0, sizeof(angle_all_2));
                sprintf((char *)angle_all_2,"TOTAL2:%d",(int)(pulse_count_2*MAX_STEP_ANGLE));
                int len3 = strlen((char *)angle_all_2);
                for (int j = len3; j < sizeof(angle_all_2) - 1; j++) 
                {
                    angle_all_2[j] = ' ';
                }
                lcd_show_string(10,270,200,16,16,(char *)reset_log_2, RED);
                delay_ms(1000);
            }            
        }
        t++;
        if(t % 20 == 0)
        {
            LED0_TOGGLE();
        }
        delay_ms(10);
        lcd_show_string(10,190,200,16,16,(char *)angle_print, RED);
        lcd_show_string(10,210,200,16,16,(char *)angle_all_1, RED);
        lcd_show_string(10,230,200,16,16,(char *)angle_all_2, RED);
        lcd_show_string(10,250,200,16,16,(char *)reset_log_1, RED);
        lcd_show_string(10,270,200,16,16,(char *)reset_log_2, RED);
    }
}
