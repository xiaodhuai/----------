#include "task.h"
#include "main.h"      
#include "stdio.h"
#include "wit_imu.h"   
#include "k230_track.h" 
#include "math.h" 

extern float target_angle;
extern int16_t base_speed;
extern float Yaw_Offset;
extern float current_angle;


// ---- Task 2 参数 ----
#define BASE_SPEED_STRAIGHT   60
#define BASE_SPEED_CURVE      30
#define RIGHT_HALF_CIRCLE  (-185.0f)  // 右半圆目标角（B→C，顺时针）
#define LEFT_HALF_CIRCLE   (+185.0f)  // 左半圆目标角（D→A，逆时针）
#define YAW_THRESHOLD         8.0f    // 退出阈值（度）


uint8_t lap_count = 0;  // 记录当前跑了第几圈（0代表第一圈，1代表第二圈...）
uint16_t selected_task = 1;
uint16_t task_running = 0;
uint8_t current_state = 0;
uint8_t last_line_status = 0x00;
uint8_t count = 0;           
static uint16_t wait_tick = 0;

void Task_Manager_Init(void) {
    selected_task = 1;  //  one = 1
    task_running = 0;  // startflag == 0 = stop  1= run
    current_state = 0;  // 
    last_line_status = 0x00; //
}

void Task_Key_Scan(void) 
{
	// === shift
    if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_0) == GPIO_PIN_RESET) 
    {
        HAL_Delay(20); 
        if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_0) == GPIO_PIN_RESET && !task_running) 
        {
            selected_task++;
            if (selected_task > 4) selected_task = 1;
            // printf("Task Selected: %d\r\n", selected_task);
            while(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_0) == GPIO_PIN_RESET); 
        }
    }
    // == start == 
    if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_1) == GPIO_PIN_RESET) 
    {
        HAL_Delay(20); 
        if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_1) == GPIO_PIN_RESET && !task_running) 
        {
            Yaw_Offset = IMU_Data.Yaw; 
            current_state = 0;
            last_line_status = 0x00;
            count = 0;
            task_running = 1; 
            while(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_1) == GPIO_PIN_RESET);
        }
    }
}
// (1)
static void Run_Task_4(void) 
{
	//
	if (count == 0) 
    {
        // 1. ??�J������??�ӡA�_�B�}�O������
        target_angle = 0.0f; // ��?����??0�]??���U��?��?�AYaw_Offset �w?�M�s�F�^
        base_speed = 60;     // 直线速度
    }
	else if (count == 1) 
	{
		task_running = 0;
	}
}
//

//
static void Run_Task_2(void)//一圈八字
{
    
	switch(current_state)	
    {
        // ------------------------------------------------------------
        // ?嗆?0: A?孵??啁頧砍笆朣?A? 撖寡?蝥?(-38.0摨?
        // ------------------------------------------------------------
        case 0:
            base_speed = 0;         // ?銝嚗?悟閫漲?臬??頧祈膠憭?
            target_angle = -38.0f;  // 雿?蝚砌?銝芸笆閫瑪閫漲

            wait_tick++;
            if (wait_tick >= 10)    // 10ms * 70 = 700ms 撘箄?頧砍摩撟嗆??迅摰?
            {
                count = 0;         
                current_state = 1;  // ?園?堆????C ?寧蝥踹??
                wait_tick = 0;
            }
            break;
            
        // ------------------------------------------------------------
        // ?嗆?1: ?渡瑪?券??蝑?? C ?寥?蝥?
        // ------------------------------------------------------------
        case 1:
            target_angle = -38.0f;
            base_speed = 60;         
            
            // 敶?頧行???C ?寥?蝥踹紡??count ?? 1 ??
            if (count >= 1) 
            {  
							  base_speed = 0;
                current_state = 2;   // ?瑪嚗?游??啁??2嚗??啗??湛?
             
            }
            break;

        // ------------------------------------------------------------
        // ?嗆?2: C?孵??啣?甇?頧穿???喳??絲?寞??(0.0摨?
        // ------------------------------------------------------------
        case 2:
					 base_speed = 0;
            target_angle = 0.0f;    // ???迤 0 摨?
            count = 1;              // ?香霈⊥嚗甇Ｗ??唳?摨憯啣僕??

            wait_tick++;
            if (wait_tick >= 10)    // 撘箄??迫 700ms ?Ｗ?憪踵?
            {
                count = 1;       
                current_state = 3;  // 餈?嗆?3嚗?憪??敺芾蕨
                wait_tick = 0;
            }
            break;

        // ------------------------------------------------------------
        // ?嗆?3: ?喳??儐餈對?蝑?? B ?寥?蝥?
        // ------------------------------------------------------------
        case 3:
            base_speed = 30;        // ??憭游儐餈寥漲
            // 摨??寞 K230 ?唳?芸頝瑪韏啣憫蝥?..

            if (count >= 2) 
            {   
							base_speed = 0;
                current_state = 4;   // ? B ?對??餌??4 ?頧祈澈
            }

            break;

        // ------------------------------------------------------------
        // ?嗆?4: B?孵??啁頧砍笆朣?B? 撖寡?蝥?(-142.0摨?
        // ------------------------------------------------------------
        case 4:
            base_speed = 0;
            target_angle = -144.0f; // 頧游笆蝘啁?蝚砌?銝芾?摨?

            wait_tick++;
            if (wait_tick >= 10)    // ?脫?恣??700ms
            {
                count = 2;
                current_state = 5;  // ?園?堆???D ?寧蝥踹??
                wait_tick = 0;
            }
            break;

        // ------------------------------------------------------------
        // ?嗆?5: ?渡瑪?券??蝑?? D ?寥?蝥?
        // ------------------------------------------------------------
        case 5:
            base_speed = 60;

            if (count >= 3) 
            {
							  base_speed = 0;
                current_state = 6;   // ? D ?寥?蝥選???嗆?6
               
            }
            break;

        // ------------------------------------------------------------
        // ?嗆?6: D?孵??啣?甇?頧穿??撌血??絲?寞??(180.0摨?
        // ------------------------------------------------------------
        case 6:
					base_speed = 0;
            target_angle = 180.0f;  // ???迤 180 摨
            wait_tick++;
            if (wait_tick >= 10)    // ?脫?恣??700ms
            {
                count = 3;
                current_state = 7;  // 餈?嗆?7嚗?憪椰??敺芾蕨??A
                wait_tick = 0;
            }
            break;

        // ------------------------------------------------------------
        // ?嗆?7: 撌血??儐餈對??游? A ??
        // ------------------------------------------------------------
        case 7:
            base_speed = 30;

            if (count >= 4) 
            {
                current_state = 8;   // ?啗噢蝏嚗?膠?嗆?
            }
            break;

        // ------------------------------------------------------------
        // ?嗆?8 / 暺恕?嗆? 摰?銝?游?嚗撩?嗅?頧血?雿?
        // ------------------------------------------------------------
        case 8:
        default:
            base_speed = 0;
            task_running = 0;       // 敶餃???
            current_state = 0;      // ?嗆憭?
            count = 0;
            wait_tick = 0;
            break;
    }
    

}





#define SPEED_STRAIGHT       60    // 直道狂飙速度 (case 0 和 case 2)
#define SPEED_CURVE_RUN      30    // 弯道基础循迹速度 (缓冲结束后的过弯速)

// 2. 🛑 进弯后重刹缓冲 (因为count变1时，车已经完全越过横线进弯了)
#define TICK_IN_BRAKE        10    // 进弯重刹持续时间 (可以设短一点，比如8~12次tick)
#define SPEED_IN_BRAKE       0    // 进弯刹车速度 (由于已经进弯，建议给15或20的点刹，给0可能会猛点头直冲出去)

// 3. ⏳ 出弯后恢复缓冲 (因为count变2时，车已经完全越过出弯横线进入直道了)
#define TICK_OUT_BUFFER      8     // 出弯中速保护时间 (单位: 10ms/tick)
#define SPEED_OUT_BUFFER     40    // 出弯过渡速度 (不立刻给60，先给40让陀螺仪摆正车头)

// ====================================================================
static void Run_Task_3(void) 
{
    static int wait_tick = 0;   
    static int last_count = -1; 

    // 【核心机制】只要 count 发生改变（出了黑线瞬间），立刻重置计时器
    if (count != last_count)
    {
        wait_tick = 0;
        last_count = count;
    }

    switch (count)
    {
        case 0:
            // ---------------- A→B 直线段 ----------------
            // 此时车子在直道狂飙，直到车头完全“吃满并离开”弯道口的横线，count才会变成1
            target_angle = 0.0f;
            base_speed = SPEED_STRAIGHT; // 60
            break;

        case 1:
            // ---------------- B→C 右半圆弧 (进弯重刹磨合期) ----------------
            wait_tick++;
            if (wait_tick <= TICK_IN_BRAKE) 
            {
                // 【黄金点刹】车子刚越过横线，用 SPEED_IN_BRAKE (比如15) 强行把60的极速按下来！
                base_speed = SPEED_IN_BRAKE;   
            }
            else 
            {
                // 磨合期结束，速度安全降到30左右，开启正常的30巡迹过弯
                base_speed = SPEED_CURVE_RUN; 
            }
            break;

        case 2:
            // ---------------- C→D 直线段 (出弯防画龙缓冲) ----------------
            // 当小车在弯道末尾，完全越过出弯横线的瞬间，count变成2。此时车头可能有点歪。
            target_angle = -180.0f; // 强制锁死直道目标角度
            
            wait_tick++;
            if (wait_tick <= TICK_OUT_BUFFER)
            {
                // 【出弯保护】不要立刻开60！用40的速度一边往前走，一边给陀螺仪时间把车头对准-180度
                base_speed = SPEED_OUT_BUFFER;   
            }
            else
            {
                // 车头彻底对正，大脚油门，直道爆发出 60 极速！
                base_speed = SPEED_STRAIGHT; 
            }
            break;

        case 3:
            // ---------------- D→A 左半圆弧 (第二个进弯重刹) ----------------
            wait_tick++;
            if (wait_tick <= TICK_IN_BRAKE) 
            {
                base_speed = SPEED_IN_BRAKE;   
            }
            else 
            {
                base_speed = SPEED_CURVE_RUN; 
            }
            break;

        case 4:
        default:
            // 回到 A 点，全场结束停车
            task_running = 0; 
            base_speed = 0;
            count = 0;
            wait_tick = 0;
            break;
    }
}
static void Run_Task_5(void)
{
switch(current_state)	
    {
        // ------------------------------------------------------------
        // ?嗆?0: A?孵??啁頧砍笆朣?A? 撖寡?蝥?(-38.0摨?
        // ------------------------------------------------------------
        case 0:
            base_speed = 0;         // ?銝嚗?悟閫漲?臬??頧祈膠憭?
            target_angle = -38.0f;  // 雿?蝚砌?銝芸笆閫瑪閫漲

            wait_tick++;
            if (wait_tick >= 10)    // 10ms * 70 = 700ms 撘箄?頧砍摩撟嗆??迅摰?
            {
                count = 0;         
                current_state = 1;  // ?園?堆????C ?寧蝥踹??
                wait_tick = 0;
            }
            break;
            
        // ------------------------------------------------------------
        // ?嗆?1: ?渡瑪?券??蝑?? C ?寥?蝥?
        // ------------------------------------------------------------
        case 1:
            target_angle = -38.0f;
            base_speed = 60;         
            
            // 敶?頧行???C ?寥?蝥踹紡??count ?? 1 ??
            if (count >= 1) 
            {  
							  base_speed = 0;
                current_state = 2;   // ?瑪嚗?游??啁??2嚗??啗??湛?
             
            }
            break;

        // ------------------------------------------------------------
        // ?嗆?2: C?孵??啣?甇?頧穿???喳??絲?寞??(0.0摨?
        // ------------------------------------------------------------
        case 2:
					 base_speed = 0;
            target_angle = 0.0f;    // ???迤 0 摨?
            count = 1;              // ?香霈⊥嚗甇Ｗ??唳?摨憯啣僕??

            wait_tick++;
            if (wait_tick >= 10)    // 撘箄??迫 700ms ?Ｗ?憪踵?
            {
                count = 1;       
                current_state = 3;  // 餈?嗆?3嚗?憪??敺芾蕨
                wait_tick = 0;
            }
            break;

        // ------------------------------------------------------------
        // ?嗆?3: ?喳??儐餈對?蝑?? B ?寥?蝥?
        // ------------------------------------------------------------
        case 3:
            base_speed = 30;        // ??憭游儐餈寥漲
            // 摨??寞 K230 ?唳?芸頝瑪韏啣憫蝥?..

            if (count >= 2) 
            {   
							base_speed = 0;
                current_state = 4;   // ? B ?對??餌??4 ?頧祈澈
            }

            break;

        // ------------------------------------------------------------
        // ?嗆?4: B?孵??啁頧砍笆朣?B? 撖寡?蝥?(-142.0摨?
        // ------------------------------------------------------------
        case 4:
            base_speed = 0;
            target_angle = -144.0f; // 頧游笆蝘啁?蝚砌?銝芾?摨?

            wait_tick++;
            if (wait_tick >= 10)    // ?脫?恣??700ms
            {
                count = 2;
                current_state = 5;  // ?園?堆???D ?寧蝥踹??
                wait_tick = 0;
            }
            break;

        // ------------------------------------------------------------
        // ?嗆?5: ?渡瑪?券??蝑?? D ?寥?蝥?
        // ------------------------------------------------------------
        case 5:
            base_speed = 60;

            if (count >= 3) 
            {
							  base_speed = 0;
                current_state = 6;   // ? D ?寥?蝥選???嗆?6
               
            }
            break;

        // ------------------------------------------------------------
        // ?嗆?6: D?孵??啣?甇?頧穿??撌血??絲?寞??(180.0摨?
        // ------------------------------------------------------------
        case 6:
					base_speed = 0;
            target_angle = 180.0f;  // ???迤 180 摨
            wait_tick++;
            if (wait_tick >= 10)    // ?脫?恣??700ms
            {
                count = 3;
                current_state = 7;  // 餈?嗆?7嚗?憪椰??敺芾蕨??A
                wait_tick = 0;
            }
            break;

        // ------------------------------------------------------------
        // ?嗆?7: 撌血??儐餈對??游? A ??
        // ------------------------------------------------------------
        case 7:
            base_speed = 30;

            if (count >= 4) 
            {
                current_state = 8;   // ?啗噢蝏嚗?膠?嗆?
            }
            break;

      
       case 8:
        lap_count++;            // 当前圈数加 1
        
        if (lap_count >= 3) 
        {
            // 【情况 A：跑满 3 圈】执行真正的停车、任务结束
            base_speed = 0;
            task_running = 0;   // 关闭任务运行标志
            current_state = 0;  // 状态机整体复位
            count = 0;
            wait_tick = 0;
            lap_count = 0;      // 圈数清零，为下一次启动做准备
        }
        else 
        {
            // 【情况 B：未满 3 圈】无缝进入下一圈
            count = 0;          // 必须清空单圈的特征点计数！
            wait_tick = 0;      // 清空时间锁
            current_state = 0;  // 强行把状态机拉回 case 0，小车在 A 点直接开始第二圈
        }
        break;

    default:
        base_speed = 0;
        task_running = 0;
        current_state = 0;
        count = 0;
        wait_tick = 0;
        lap_count = 0;
        break;
            }
         

}


 static void Run_Task_1(void)
 {
 switch(current_state)	
{
    // ============================================================
    // 【第一圈】 状态 0 ~ 7
    // ============================================================
    
    // 状态 0: 第一圈 - A点原地旋转 -> 目标 -38度
    case 0:
        base_speed = 0;         
        target_angle = -38.0f;  

        wait_tick++;
        if (wait_tick >= 10)    // 700ms 
        {
            count = 0;         
            current_state = 1;  
            wait_tick = 0;
        }
        break;
        
    // 状态 1: 第一圈 - 保持对角推进 -> 等待触发 C 点
    case 1:
        base_speed = 60;     
        
        if (count >= 1) 
        {  
            base_speed = 0;
            current_state = 2;   
        }
        break;

    // 状态 2: 第一圈 - C点原地扭头 -> 目标 0度
    case 2:
        base_speed = 0;
        target_angle = 0.0f;            

        wait_tick++;
        if (wait_tick >= 10)    
        {
            count = 1;       
            current_state = 3;  
            wait_tick = 0;
        }
        break;

    // 状态 3: 第一圈 - 保持 0 度推进 -> 等待 B 点
    case 3:
        base_speed = 30;        

        if (count >= 2) 
        {   
            base_speed = 0;
            current_state = 4;   
        }
        break;

    // 状态 4: 第一圈 - B点原地大调头 -> 目标 -144度
    case 4:
        base_speed = 0;
        target_angle = -146.0f; 

        wait_tick++;
        if (wait_tick >= 10)    
        {
            count = 2;
            current_state = 5;  
            wait_tick = 0;
        }
        break;

    // 状态 5: 第一圈 - 保持对角返回 -> 等待 D 点 
    case 5:
        base_speed = 60;

        if (count >= 3) 
        {
            base_speed = 0;
            current_state = 6;   
        }
        break;

    // 状态 6: 第一圈 - D点原地回正 -> 目标 180度
    case 6:
        base_speed = 0;
        target_angle = 180.0f;  
        
        wait_tick++;
        if (wait_tick >= 10)    
        {
            count = 3;
            current_state = 7;  
            wait_tick = 0;
        }
        break;

    // 状态 7: 第一圈 - 直行冲向 A 点 -> 触发后切入第二圈
    case 7:
        base_speed = 30;

        if (count >= 4) 
        {
            base_speed = 0;
            count = 0;          // 【核心】清空计数，让下一圈重新从1开始算
            current_state = 10; // 跃迁到第二圈的起点
            wait_tick = 0;
        }
        break;


    // ============================================================
    // 【第二圈】 状态 10 ~ 17 （动作完全独立，可单独微调本圈参数）
    // ============================================================
    
    // 状态 10: 第二圈 - A点原地旋转 -> 目标 -38度
    case 10:
        base_speed = 0;         
        target_angle = -34.0f;  

        wait_tick++;
        if (wait_tick >= 10)    
        {
            count = 0;         
            current_state = 11;  
            wait_tick = 0;
        }
        break;
        
    // 状态 11: 第二圈 - 保持对角推进 -> 等待触发 C 点
    case 11:
        base_speed = 60;         
        
        if (count >= 1) 
        {  
            base_speed = 0;
            current_state = 12;   
        }
        break;

    // 状态 12: 第二圈 - C点原地扭头 -> 目标 0度
    case 12:
        base_speed = 0;
        target_angle = 0.0f;           

        wait_tick++;
        if (wait_tick >= 10)    
        {
            count = 1;       
            current_state = 13;  
            wait_tick = 0;
        }
        break;

    // 状态 13: 第二圈 - 保持 0 度推进 -> 等待 B 点
    case 13:
        base_speed = 30;        

        if (count >= 2) 
        {   
            base_speed = 0;
            current_state = 14;   
        }
        break;

    // 状态 14: 第二圈 - B点原地大调头 -> 目标 -144度
    case 14:
        base_speed = 0;
        target_angle = -148.0f; 

        wait_tick++;
        if (wait_tick >= 10)    
        {
            count = 2;
            current_state = 15;  
            wait_tick = 0;
        }
        break;

    // 状态 15: 第二圈 - 保持对角返回 -> 等待 D 点
    case 15:
        base_speed = 60;
        if (count >= 3) 
        {
            base_speed = 0;
            current_state = 16;   	
        }
        break;

    // 状态 16: 第二圈 - D点原地回正 -> 目标 180度
    case 16:
        base_speed = 0;
        target_angle = 180.0f;  
        
        wait_tick++;
        if (wait_tick >= 10)    
        {
            count = 3;
            current_state = 17;  
            wait_tick = 0;
        }
        break;

    // 状态 17: 第二圈 - 直行冲向 A 点 -> 触发后切入最后一圈
    case 17:
        base_speed = 30;

        if (count >= 4) 
        {
            base_speed = 0;
            count = 0;          // 【核心】再次清空计数
            current_state = 20; // 跃迁到最后一圈的起点
            wait_tick = 0;
        }
        break;


    // ============================================================
    // 【第三圈 - 最后一圈】 状态 20 ~ 28
    // ============================================================
    
    // 状态 20: 最后一圈 - A点原地旋转 -> 目标 -38度
    case 20:
        base_speed = 0;         
        target_angle = -37.0f;  

        wait_tick++;
        if (wait_tick >= 10)    
        {
            count = 0;         
            current_state = 21;  
            wait_tick = 0;
        }
        break;
        
    // 状态 21: 最后一圈 - 保持对角推进 -> 等待触发 C 点
    case 21:
        base_speed = 60;         
        
        if (count >= 1) 
        {  
            base_speed = 0;
            current_state = 22;   
        }
        break;

    // 状态 22: 最后一圈 - C点原地扭头 -> 目标 0度
    case 22:
        base_speed = 0;
        target_angle = 0.0f;    
        count = 1;              

        wait_tick++;
        if (wait_tick >= 10)    
        {
            count = 1;       
            current_state = 23;  
            wait_tick = 0;
        }
        break;

    // 状态 23: 最后一圈 - 保持 0 度推进 -> 等待 B 点
    case 23:
        base_speed = 30;        

        if (count >= 2) 
        {   
            base_speed = 0;
            current_state = 24;   
        }
        break;

    // 状态 24: 最后一圈 - B点原地大调头 -> 目标 -144度
    case 24:
        base_speed = 0;
        target_angle = -146.0f; 

        wait_tick++;
        if (wait_tick >= 10)    
        {
            count = 2;
            current_state = 25;  
            wait_tick = 0;
        }
        break;

    // 状态 25: 最后一圈 - 保持对角返回 -> 等待 D 点
    case 25:
        base_speed = 60;

        if (count >= 3) 
        {
            base_speed = 0;
            current_state = 26;   
        }
        break;

    // 状态 26: 最后一圈 - D点原地回正 -> 目标 180度
    case 26:
        base_speed = 0;
        target_angle = 180.0f;  
        
        wait_tick++;
        if (wait_tick >= 10)    
        {
            count = 3;
            current_state = 27;  
            wait_tick = 0;
        }
        break;

    // 状态 27: 最后一圈 - 直行冲向终点 A 点
    case 27:
        base_speed = 30;
        if (count >= 4) 
        {
            base_speed = 0;
            current_state = 28; // 冲线成功，进入终点停车保护状态
        }
        break;

 // 状态 28: 第四圈 - A点原地旋转 -> 目标 -38度
        case 28:
            base_speed = 0;         
            target_angle = -38.0f;  

            wait_tick++;
            if (wait_tick >= 10)    
            {
                count = 0;         
                current_state = 29;  
                wait_tick = 0;
            }
            break;
            
        // 状态 29: 第四圈 - 保持对角推进 -> 等待触发 C 点
        case 29:
            base_speed = 60;         
            
            if (count >= 1) 
            {  
                base_speed = 0;
                current_state = 30;   
            }
            break;

        // 状态 30: 第四圈 - C点原地扭头 -> 目标 0度
        case 30:
            base_speed = 0;
            target_angle = 0.0f;    
            count = 1;              

            wait_tick++;
            if (wait_tick >= 10)    
            {
                count = 1;       
                current_state = 31;  
                wait_tick = 0;
            }
            break;

        // 状态 31: 第四圈 - 保持 0 度推进 -> 等待 B 点
        case 31:
            base_speed = 30;        
            target_angle = 0.0f;

            if (count >= 2) 
            {   
                base_speed = 0;
                current_state = 32;   
            }
            break;

        // 状态 32: 第四圈 - B点原地大调头 -> 目标 -144度
        case 32:
            base_speed = 0;
            target_angle = -147.0f; 

            wait_tick++;
            if (wait_tick >= 10)    
            {
                count = 2;
                current_state = 33;  
                wait_tick = 0;
            }
            break;

        // 状态 33: 第四圈 - 保持对角返回 -> 等待 D 点
        case 33:
            base_speed = 60;

            if (count >= 3) 
            {
                base_speed = 0;
                current_state = 34;   
            }
            break;

        // 状态 34: 第四圈 - D点原地回正 -> 目标 180度
        case 34:
            base_speed = 0;
            target_angle = 180.0f;  
            
            wait_tick++;
            if (wait_tick >= 10)    
            {
                count = 3;
                current_state = 35;  
                wait_tick = 0;
            }
            break;

        // 状态 35: 第四圈 - 直行冲向终点 A 点 -> 触发后完全停车，去状态 40
        case 35:
            base_speed = 30;

            if (count >= 4) 
            {
                base_speed = 0;
                current_state = 40; // 🎬 四圈全部完美跑完，切入终点停车保护状态
            }
            break;


        // ============================================================
        // 🏁【终点安全停车】 状态 40
        // ============================================================
        case 40:
            base_speed = 0;
            task_running = 0;   // 关闭任务运行标志，车子完全静止
            current_state = 0;  
            count = 0;
            wait_tick = 0;
            break;

        default:
            base_speed = 0;
            task_running = 0;
            current_state = 0;  
            count = 0;
            wait_tick = 0;
            break;
    }
}

void Task_Dispatcher(void)
{
    if (task_running == 1) 
    {
        switch(selected_task) 
        {
            case 1: Run_Task_1(); break; 
            case 2: Run_Task_2(); break; 
            case 3: Run_Task_3(); break;
//            case 4: Run_Task_4();  break;
        }
    }
    else 
    {
        base_speed = 0; // �p�G?�b�]��?�A��?��?�t��
    }
}