#include "Bsp_init.h"
#include "misc.h"
#include "delay.h"
#include "gpio.h"
#include "cba.h"
#include "ultrasonic.h"
#include "canp_hostcom.h"
#include "hard_can.h"
#include "bh1750.h"
#include "xiaochuang.h"
#include "power_check.h"
#include "can_user.h"
#include "roadway_check.h"
#include "uart_a72.h"
#include "Can_check.h"
#include "Timer.h"
#include "Rc522.h"
#include "PID_Track.h"
#include "Digital_track.h"
#include "Mark_Fun.h"
#include "Mylib.h"
void Hardware_Init()
{
		NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0); // 中断分组
		delay_init(168);
		//Infrared_Init();												//红外初始化
		MX_GPIO_Init();     // 任务版初始化
		Cba_Init();         // 核心板上按键、LED、蜂鸣器初始化
		Ultrasonic_Init();  // 超声波初始化
		Hard_Can_Init();    // CAN总线初始化
		BH1750_Configure(); // BH1750初始化配置
		BKRC_Voice_Init();  // 语音识别初始化
		Electricity_Init(); // 电量检测初始化
		UartA72_Init();
		Can_check_Init(83, 7);            // CAN总线定时器初始化
		roadway_check_TimInit(167, 1999); // 路况检测
		Timer_Init(167, 999);             // 串行数据通讯时间帧
		Readcard_daivce_Init();           // RFID初始化
		PID_Track_Init();
		Digital_Track_Init(); // 任务路径初始化
		Init_Code();          // 二维码初始化
		Init_GoValue();
		Init_TRAFFIC(); // 交通灯初始化
	
		//init_light();
}

