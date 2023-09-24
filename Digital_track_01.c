#include <stdio.h>
#include <string.h>
#include "Digital_track.h"
#include "Typ.h"
#include "delay.h"
#include "Motion.h"
#include "Marker.h"
#include "Integration_task.h"
#include "Mylib.h"
#include "A72.h"
#include "canp_hostcom.h"
#include "HS_Rec_Data_Pro.H"
#include "Mark_To_MasterCar_DataSlove.h"

volatile uint32_t AotoFirstTime;
volatile uint32_t WaitingTime;     //从车完成任务时间+20秒，需要开启自动启动路线
uint8_t StartAuto_Flag=0;

uint8_t Task_Table_Data[10][30] = {0};//全部路线

void Digital_Track_Init(void)
{
	static u8 Temp [] = {1,1,Code(2),TFTA(2),Gate(1),TrafficA(1),1,TFTB(1),TrafficB(4),4,
												Code(4),TrafficC(3),3,Alarm(4),TrafficD(3),3,TFTC(4),1,1,2,2,4,'\0'};
	static u8 Temp1[] = {1,Code(1),TFTA(1),TrafficA(1),TFTB(1),TrafficB(1),
												Code(1),TrafficC(1),TrafficD(1),TFTC(1),1,1,3,'\0'};
  static u8 Temp2[] = {3,TrafficB(4),3,'\0'};
	static u8 Temp3[] = {'\0'};
	static u8 Temp4[] = {1,'\0'};
	static u8 Temp5[] = {1,'\0'};
  static u8 Temp7[] = {1,1,2,Terrain_RFID(3),Liti(4),4,'\0'};		
	static u8 Temp8[] = {1,Terrain_RFID(1),1,'\0'};													//预设位4													
	
	Digital_Track_Table_Set(0, Temp);
	Digital_Track_Table_Set(1, Temp1);
	Digital_Track_Table_Set(2, Temp2);
	Digital_Track_Table_Set(3, Temp3);
	Digital_Track_Table_Set(4, Temp4);
	Digital_Track_Table_Set(5, Temp5);
	Digital_Track_Table_Set(7, Temp7);
}

uint8_t Current = 1;//车头方向
uint8_t next = 0;

void Change_Direction(void)	//该变车头方向
{
	if ((Current + 1 == next) || ((Current + 1 == 5) && (next == 1))) 
		turn_right_trackQ7(90);
	else if ((Current - 1) == next || ((Current - 1 == 0) && (next == 4))) 	//改变车头朝向
		turn_left_trackQ7(90);
	else if (Current == next) {}
	else if (Current == 5) turn_right_trackQ7(90);
	else {
		turn_right_trackQ7(90);
		turn_right_trackQ7(90);}
		Current = next;				//更新车头状态
}

/*
功能：载入主车路线任务
Table_Num：路线编号0-10 最多10条路线
Table_Data：主车路线任务数据
*/
void Digital_Track_Table_Set(uint8_t Table_Num, uint8_t* Table_Data) {
	memset(Task_Table_Data[Table_Num], 0, sizeof(Task_Table_Data[Table_Num]));//先清0
	memcpy(Task_Table_Data[Table_Num], Table_Data, strlen((const char *)Table_Data)+1);//加1是要将'/0'放入
}

uint8_t* Task_Table;//路线
uint8_t RuKu_Flag=false;
uint8_t Auto_Run_Flag=false;
/*
功能：启动主车任务
Table_Num：路线编号0-10 最多10条路线
Is_Init_Current：启动主车前是否重新设置车头方向，重设的方向为路线的第0个变量
Is_RuKu_Flag：是否入库
*/
void Start_Digital_Track_Table(u8 Table_Num,bool Is_Init_Current,bool Is_RuKu_Flag) {
	//led_timerOpen();//2022年11月14日
	Task_Table = Task_Table_Data[Table_Num];
	if (Is_Init_Current == true) {
		Current = *Task_Table;//获取当前方向
	}	
	RuKu_Flag=Is_RuKu_Flag;
	Auto_Run_Flag = true;
}
/*
	函数指针数组替代'switch case'
	使得程序更模块化更易于维护
*/
static uint8_t Task_Index = 0;//当前第几个任务
static uint8_t Task_Number = 0;
static uint8_t Init_Flag = true;
static uint8_t Marker_Value = 0;
uint8_t next_task()
{
	next = Task_Table[Task_Index+1];//读取下一个任务
	Marker_Value = Task_Table[Task_Index+1] & 0x3F;//获取标志物数值
	return  1;	
}
uint8_t change_direction()
{
	if (((next & 0x3F) >= Task_First_Dat) && ((next & 0x3F) < Special_Task_First_Dat)) 
		{
			next = ((next >> 6) & 0x03) + 1;		//获取方向
		}

		Change_Direction();			//改变方向

	//更改模式(判断属于什么模式:是否前进、处理任务、前进、是否入库)

	if (Task_Index == Task_Number) {//检测此任务是否为最后一个任务
		
		if (RuKu_Flag == true) {//任务完成是入库吗		
			return  5;
		}
		
		else {
			return  4;//不用停车入库
		}
	}
	else {
		if (Marker_Value < 5) {
			return  3; //是行驶
		}
		else {
			return  2;
		}
	}
}
uint8_t process_task()
{
	Process_Task(Marker_Value);
	return  6;
}
uint8_t go_track()
{
	track_PID(60);
	go_forward(65, Mylib.go_value);
	return 6;
}
uint8_t no_enter_garage()
{
	Auto_Run_Flag = false;
	Init_Flag = true;
}
uint8_t enter_garage()
{
	uint8_t state=0;//车库相关数据初始化
	printf_LCD((char *)"%d",Marker_InitStruct.Light.FirstGear);
	Floor=(MO6%Marker_InitStruct.Light.FirstGear)+1;
	printf_LCD((char *)"%d",Floor);
	Garage_Track();					//倒车入库
	delay_ms(200);
	Finish_Task();					//关闭LED，开启无线充电
	Auto_Run_Flag = false;
	Init_Flag = true;
}
uint8_t task_ok()
{
	Task_Index += 1;	//完成子任务，子任务加1
	printf_LCD("FinishTaskNum:%d\r\n",Task_Index);
	return  0;
}
void Digital_Tracking(void)
{
	static uint8_t Mode = 0;
	uint8_t (*p[7])() = {next_task,change_direction,process_task,
	go_track,no_enter_garage,enter_garage,task_ok};
	if(Auto_Run_Flag == true)
	{
		if (Init_Flag == true) {    //判断初始化flag
			Init_Flag = false;                   //保证初始化一次
			Task_Number = strlen((char*)Task_Table)-2;//任务数量
			Task_Index = 0;
			Mode = 0;
		}
		if(Mode >= 0 && Mode <= 6)
		{
			p[Mode]();
		}
	}
}
//void Digital_Tracking(void){//欲使用函数指针数组替代
//	
//	static uint8_t Init_Flag = true;
//	static uint8_t Task_Number = 0;
//	static uint8_t Task_Index = 0;//当前第几个任务
//	static uint8_t Mode = 0;
//	static uint8_t Marker_Value = 0;
//	if(Auto_Run_Flag == true)
//	{
//		if (Init_Flag == true) {    //判断初始化flag
//			Init_Flag = false;                   //保证初始化一次
//			Task_Number = strlen((char*)Task_Table)-2;//任务数量
//			Task_Index = 0;
//			Mode = 0;
//		}
//		switch (Mode)
//		{
//			case 0:
//			{
//				next = Task_Table[Task_Index+1];//读取下一个任务
//				Marker_Value = Task_Table[Task_Index+1] & 0x3F;//获取标志物数值
//				Mode = 1;	
//			}
//			break;
//			case 1: 									//改变方向
//			{
//				if (((next & 0x3F) >= Task_First_Dat) && ((next & 0x3F) < Special_Task_First_Dat)) 
//					{
//						next = ((next >> 6) & 0x03) + 1;		//获取方向
//					}

//					Change_Direction();			//改变方向

//				//更改模式(判断属于什么模式:是否前进、处理任务、前进、是否入库)

//				if (Task_Index == Task_Number) {//检测此任务是否为最后一个任务
//					
//					if (RuKu_Flag == true) {//任务完成是入库吗		
//						Mode = 5;
//						break;
//					}
//					
//					else {
//						Mode = 4;//不用停车入库
//						break;
//					}
//				}
//				else {
//					if (Marker_Value < 5) {
//						Mode = 3; //是行驶
//					}
//					else {
//						Mode = 2;
//					}
//				}
//			
//			}		
//			break;
//			case 2://处理任务
//			{
//				Process_Task(Marker_Value);
//				Mode = 6;
//			}		
//			break;
//			case 3://行进
//			{
//				track_PID(60);
//				go_forward(65, Mylib.go_value);
//				Mode = 6;
//			}			
//			break;
//			case 4://非入库
//			{
//				Auto_Run_Flag = false;
//				Init_Flag = true;
//				
//				//StartAuto_Flag=1;
// 			  //AotoFirstTime=gt_get();    /****打开自动开启路线****/

//			}			
//			break;
//			case 5://入库     
//			{
//				uint8_t state=0;//车库相关数据初始化
//				printf_LCD((char *)"%d",Marker_InitStruct.Light.FirstGear);
//				Floor=(MO6%Marker_InitStruct.Light.FirstGear)+1;
//				printf_LCD((char *)"%d",Floor);
//				
//				//Init_Garage();
//				//state = 1;
//				//state=Garage_Reduction();			//车库复位
//				//if(state)
//				//{
//					Garage_Track();							//倒车入库
//					delay_ms(200);
//					//Garage_Rise();							//车库上升
//				//}
//				Finish_Task();								//关闭LED，开启无线充电
//				Auto_Run_Flag = false;
//				Init_Flag = true;
//			}	
//			break;
//			case 6://完成了一个任务
//			{
//				Task_Index += 1;	//完成子任务，子任务加1
//				printf_LCD("FinishTaskNum:%d\r\n",Task_Index);
//				Mode = 0;			
//			}			
//			break;
//			default://非行进任务为最后一个任务	
//			{

//			}
//			break;
//			}
//	}
//}

