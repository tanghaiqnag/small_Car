#ifndef __DIGITAL_TRACKING_h
#define __DIGITAL_TRACKING_h
#include "stm32f4xx.h"
#include <stdbool.h>
#include "sys.h"

volatile extern uint32_t AotoFirstTime;
volatile extern uint32_t WaitingTime;
extern uint8_t StartAuto_Flag;
extern uint8_t n;

void Scheduler_run(void);
void KEY_Check(void);
void Digital_Tracking(void);

extern uint32_t Power_check_times;      // 电量检测周期
extern uint32_t WIFI_Upload_data_times; // 通过Wifi上传数据周期
extern uint32_t RFID_Init_Check_times;
extern uint32_t function_init_times; // 任务检测周期
/*
功能：载入路线任务
Table_Num：路线编号0-10 最多10条路线
Table_Data：从车路线任务数据
*/
void Digital_Track_Table_Set(uint8_t Table_Num, uint8_t* Table_Data);

/*
功能：启动任务
Table_Num：路线编号0-10 最多10条路线
Is_Init_Current：启动前是否从设车头方向，重设的方向为路线的第0个变量
Is_RuKu_Flag：是否入库
*/
void Start_Digital_Track_Table(u8 Table_Num, bool Is_Init_Current, bool Is_RuKu_Flag);

void Digital_Track_Init(void);

extern uint8_t Current;//车头方向

enum {
	Terrain_ENUM = 5,
	RFID_ENUM,
	TFTA_ENUM,
	TFTB_ENUM,
	TFTC_ENUM,
	Liti_ENUM,
	SYN_ENUM,
	Light_ENUM,
	TrafficA_ENUM,
	TrafficB_ENUM,
	TrafficC_ENUM,
	TrafficD_ENUM,
	Alarm_ENUM,
	Code_ENUM,
	NoRFID_ENUM,
	Range_ENUM,
	Task_ENUM,
	Static_ENUM,
	Park_Correct_ENUM,
	Gate_ENUM,
	ETC_ENUM,
	WaitSTrack_ENUM,
	Terrain_RFID_ENUM,
};//add?

#define Terrain(x) 			(((x - 1) << 6) | Terrain_ENUM)			//地形标志物	
#define RFID(x)		 			(((x - 1) << 6) | RFID_ENUM)			  //RFID标志物
#define TFTA(x)		 			(((x - 1) << 6) | TFTA_ENUM)			  //A TFT标志物
#define TFTB(x)		 			(((x - 1) << 6) | TFTB_ENUM)			  //B TFT标志物
#define TFTC(x)		 			(((x - 1) << 6) | TFTC_ENUM)			  //C TFT标志物
#define Liti(x)		 			(((x - 1) << 6) | Liti_ENUM)		 		//立体显示标志物
#define SYN(x)		 			(((x - 1) << 6) | SYN_ENUM)					//语音播报标志物
#define Light(x)	 			(((x - 1) << 6) | Light_ENUM)				//智能路灯标志物
#define TrafficA(x)			(((x - 1) << 6) | TrafficA_ENUM)		//A 交通灯标志物
#define TrafficB(x)			(((x - 1) << 6) | TrafficB_ENUM)		//B 交通灯标志物
#define TrafficC(x)			(((x - 1) << 6) | TrafficC_ENUM)		//C 交通灯标志物
#define TrafficD(x)			(((x - 1) << 6) | TrafficD_ENUM)		//D 交通灯标志物
#define Alarm(x)	 			(((x - 1) << 6) | Alarm_ENUM)				//报警台标志物
#define Code(x)		 			(((x - 1) << 6) | Code_ENUM)				//二维码标志物
#define NoRFID(x)	 			(((x - 1) << 6) | NoRFID_ENUM)			//不识别RFID
#define Range(x)	 			(((x - 1) << 6) | Range_ENUM)				//测距
#define Task(x)		 			(((x - 1) << 6) | Task_ENUM)				//执行相关任务
#define Static(x)				(((x - 1) << 6) | Static_ENUM)			//识别静态标志物图形
#define Park_Correct(x) (((x - 1) << 6) | Park_Correct_ENUM)	//停车矫正
#define Gate(x) 				(((x - 1) << 6) | Gate_ENUM)				//道闸标志物
#define ETC(x)		 			(((x - 1) << 6) | ETC_ENUM)					//ETC标志物
#define WaitSTrack(x)		 	(((x - 1) << 6) | WaitSTrack_ENUM)				//控制小车行走
#define Terrain_RFID(x)		 	(((x - 1) << 6) | Terrain_RFID_ENUM)			//地形和RFID结合

#define Task_First_Dat					Terrain_ENUM								//特殊任务首地址

enum {
	L_Mapan_ENUM = 101,
	R_Mapan_ENUM,
	Avoid_ENUM,
	LED_Tba_ENUM,
	Go_ENUM,
	Back_ENUM,
	Host_Con_Slave_Run_ENUM,
	Slave_Con_Host_Run_ENUM,
	Stop_HostCar_ENUM,
};
#define L_Mapan				L_Mapan_ENUM													//码盘向左转弯
#define R_Mapan				R_Mapan_ENUM													//码盘向右转弯
#define Avoid					Avoid_ENUM														//规避后路线
#define LED_Tba				LED_Tba_ENUM													//任务板声光任务
#define Go_ENUM				Go_ENUM															//前进
#define Back_ENUM 		Back_ENUM														//后退

#define Special_Task_First_Dat			L_Mapan					//普通任务首地址
#endif
