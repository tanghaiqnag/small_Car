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
#include "stm32f4xx.h"
#include <stdbool.h>
#include "infrared.h"
#include "cba.h"
#include "ultrasonic.h"
#include "hard_can.h"
#include "bh1750.h"
#include "xiaochuang.h"
#include "power_check.h"
#include "can_user.h"
#include "data_base.h"
#include "roadway_check.h"
#include "tba.h"
#include "data_base.h"
#include "swopt_drv.h"
#include "uart_a72.h"
#include "Can_check.h"
#include "delay.h"
#include "can_user.h"
#include "Timer.h"
#include "Rc522.h"
#include "Mark_Fun.h"
#include "Mylib.h"
#include "Digital_track.h"
#include "gpio.h"
#include "Init_marker.h"
#include "PID_Track.h"
#include "Marker.h"
#include "Typ.h"
#include "string.h"
#include "HS_Rec.h"
#include "HS_Send.h"
#include "crc24.h"
#include <math.h>
#include "Mark_To_MasterCar_DataSlove.h"
#include "Bsp_init.h"
#include "quene.h"

RCC_ClocksTypeDef RCC_Clocks;
uint8_t  pNum_index = 0;				 //����ָ������ִ���������
uint32_t Power_check_times;      // �����������
uint32_t WIFI_Upload_data_times; // ͨ��Wifi�ϴ���������
uint32_t RFID_Init_Check_times;
uint32_t function_init_times; // ����������

volatile uint32_t AotoFirstTime;
volatile uint32_t WaitingTime;     //�ӳ��������ʱ��+20�룬��Ҫ�����Զ�����·��
uint8_t StartAuto_Flag=0;

uint8_t Task_Table_Data[10][30] = {0};//ȫ��·��


void Digital_Track_Init(void)
{
	static u8 Temp [] = {1,1,Code(2),TFTA(2),Gate(1),TrafficA(1),1,TFTB(1),TrafficB(4),4,
												Code(4),TrafficC(3),3,Alarm(4),TrafficD(3),3,TFTC(4),1,1,2,2,4,'\0'};
	static u8 Temp1[] = {1,1,1,3,'\0'};
  static u8 Temp2[] = {3,TrafficB(4),3,'\0'};
	static u8 Temp3[] = {'\0'};
	static u8 Temp4[] = {1,'\0'};
	static u8 Temp5[] = {1,'\0'};
  static u8 Temp7[] = {1,1,2,Terrain_RFID(3),Liti(4),4,'\0'};		
	static u8 Temp8[] = {1,Terrain_RFID(1),1,'\0'};													//Ԥ��λ4													
	
	Digital_Track_Table_Set(0, Temp);
	Digital_Track_Table_Set(1, Temp1);
	Digital_Track_Table_Set(2, Temp2);
	Digital_Track_Table_Set(3, Temp3);
	Digital_Track_Table_Set(4, Temp4);
	Digital_Track_Table_Set(5, Temp5);
	Digital_Track_Table_Set(7, Temp7);
}

uint8_t Current = 1;//��ͷ����
uint8_t next = 0;

void Change_Direction(void)	//�ñ䳵ͷ����
{
	if ((Current + 1 == next) || ((Current + 1 == 5) && (next == 1))) 
		turn_right_trackQ7(90);
	else if ((Current - 1) == next || ((Current - 1 == 0) && (next == 4))) 	//�ı䳵ͷ����
		turn_left_trackQ7(90);
	else if (Current == next) {}
	else if (Current == 5) turn_right_trackQ7(90);
	else {
		turn_right_trackQ7(90);
		turn_right_trackQ7(90);}
		Current = next;				//���³�ͷ״̬
}

/*
���ܣ���������·������
Table_Num��·�߱��0-10 ���10��·��
Table_Data������·����������
*/
void Digital_Track_Table_Set(uint8_t Table_Num, uint8_t* Table_Data) {
	memset(Task_Table_Data[Table_Num], 0, sizeof(Task_Table_Data[Table_Num]));//����0
	memcpy(Task_Table_Data[Table_Num], Table_Data, strlen((const char *)Table_Data)+1);//��1��Ҫ��'/0'����
}

uint8_t* Task_Table;//·��
uint8_t RuKu_Flag=false;
uint8_t Auto_Run_Flag=false;
/*
���ܣ�������������
Table_Num��·�߱��0-10 ���10��·��
Is_Init_Current����������ǰ�Ƿ��������ó�ͷ��������ķ���Ϊ·�ߵĵ�0������
Is_RuKu_Flag���Ƿ����
*/
void Start_Digital_Track_Table(u8 Table_Num,bool Is_Init_Current,bool Is_RuKu_Flag) {
	//led_timerOpen();//2022��11��14��
	Task_Table = Task_Table_Data[Table_Num];
	if (Is_Init_Current == true) {
		Current = *Task_Table;//��ȡ��ǰ����
	}	
	RuKu_Flag=Is_RuKu_Flag;
	Auto_Run_Flag = true;
}
/******************************************/
static uint8_t Init_Flag = true;
static uint8_t Task_Number = 0;
static uint8_t Task_Index = 0;//��ǰ�ڼ�������
static uint8_t Marker_Value = 0;
//��ȡ��һ������
void rTask_Next()
{
	next = Task_Table[Task_Index+1];//��ȡ��һ������
	Marker_Value = Task_Table[Task_Index+1] & 0x3F;//��ȡ��־����ֵ
	pNum_index = 1;
}
//�ı䷽��
void CDirection()
{
	if (((next & 0x3F) >= Task_First_Dat) && ((next & 0x3F) < Special_Task_First_Dat)) 
					{
						next = ((next >> 6) & 0x03) + 1;		//��ȡ����
					}

					Change_Direction();			//�ı䷽��

				//����ģʽ(�ж�����ʲôģʽ:�Ƿ�ǰ������������ǰ�����Ƿ����)

				if (Task_Index == Task_Number) {//���������Ƿ�Ϊ���һ������
					
					if (RuKu_Flag == true) {//��������������		
						pNum_index = 5;
					}
					
					else {
						pNum_index = 4;//����ͣ�����
					}
				}
				else {
					if (Marker_Value < 5) {
						pNum_index = 3; //����ʻ
					}
					else {
						pNum_index = 2;
					}
				}
}
//��������
void PTask()
{
		Process_Task(Marker_Value);
		pNum_index = 6;
}
//�н�
void GForward()
{
	track_PID(60);
	go_forward(65, Mylib.go_value);
	pNum_index = 6;
}
//�����
void OFF_Garage()
{
	Auto_Run_Flag = false;
	Init_Flag = true;
				
	//StartAuto_Flag=1;
	//AotoFirstTime=gt_get();    /****���Զ�����·��****/
}
//���
void ON_Garage()
{
	uint8_t state=0;//����������ݳ�ʼ��
	printf_LCD((char *)"%d",Marker_InitStruct.Light.FirstGear);
	Floor=(MO6%Marker_InitStruct.Light.FirstGear)+1;
	printf_LCD((char *)"%d",Floor);

	//Init_Garage();
	//state = 1;
	//state=Garage_Reduction();			//���⸴λ
	//if(state)
	//{
		Garage_Track();							//�������
		delay_ms(200);
		//Garage_Rise();							//��������
	//}
	Finish_Task();								//�ر�LED���������߳��
	Auto_Run_Flag = false;
	Init_Flag = true;
}
//���һ������
void OK_Task()
{
	Task_Index += 1;	//����������������1
	printf_LCD("FinishTaskNum:%d\r\n",Task_Index);
	pNum_index = 0;			
}
void Digital_Tracking(void){
	
  void (*pArr[])() = {rTask_Next,CDirection,PTask,GForward,OFF_Garage,ON_Garage,OK_Task};
	if(Auto_Run_Flag == true)
	{
		if (Init_Flag == true) {    	//�жϳ�ʼ��flag
			Init_Flag = false;                   //��֤��ʼ��һ��
			Task_Number = strlen((char*)Task_Table)-2;//��������
			Task_Index = 0;
			pNum_index = 0;
		}
		for(int i = 0; i < 8; i++){
			pArr[pNum_index]();
		}
	}
}

/**
�������ܣ��������
��    ������
�� �� ֵ����
*/

void KEY_Check()
{
	if(S1 == 0)
	{
		delay_ms(10);
		if(S1 == 0)
		{
			LED1 = !LED1;
			while(!S1);
		//Init_Task();
			ring rt_buff = fifo_init();
			ring_buff_insert(rt_buff,'K');
			uint8_t ring_num[] = {ring_buff_get(rt_buff)};
			char Debug_Temp[] = "OK";
			Send_InfoData_To_Fifo((u8 *)ring_num,strlen(ring_num));
			ring_buff_destory(rt_buff);
		}
	}
	if(S2 == 0)
	{
		delay_ms(10);
		if(S2 == 0)
		{
			LED2 = !LED2;
			while(!S2);
			Start_Digital_Track_Table(1,true,false);
		}
	}

	if(S3 == 0)
	{
		delay_ms(10);
		if(S3 == 0)
		{
			LED3 = !LED3;
			while(!S3);
			Start_Digital_Track_Table(2,true,false);
		}
	}
	if(S4 == 0)
	{
		delay_ms(10);
		if(S4 == 0)
		{
			LED4 = !LED4;
			while(!S4);
		}
	}
}
void Scheduler_run()
{
	
	KEY_Check();// ��������ִ��
	
	Can_WifiRx_Check(); // wifi���ݽ��ռ��
	
	if(gt_get_sub(RFID_Init_Check_times) == 0) // ���rfid�Ƿ�ͨ������ �����������ظ���ʼ�� �ظ�ȡ����������
	{
		static FlagStatus Is_ResetMP_Spk_Flag = RESET;
		RFID_Init_Check_times = gt_get() + 200; // RFID��ʼ�����
		if(Rc522_GetLinkFlag() == 0)                            // �ж���RC522�Ƿ�ͨ�ųɹ�
		{
			Readcard_daivce_Init();
			MP_SPK = !MP_SPK; // ������ȡ��
			Is_ResetMP_Spk_Flag = SET;
		}
		else
		{
			if(Is_ResetMP_Spk_Flag == SET)
			{
				Is_ResetMP_Spk_Flag = RESET;
				if(MP_SPK == 1)
				{
						MP_SPK = 0; // �����⵽ͨ�ųɹ����ͽ���������0
				}
			}
			LED4 = !LED4;
			Rc522_LinkTest();
		 }
		}
			if(gt_get_sub(Power_check_times) == 0){
			Power_check_times = gt_get() + 200; // ��ص������
			Power_Check();
		}

		if(gt_get_sub(function_init_times) == 0){
				function_init_times = gt_get() +  200;
				Digital_Tracking(); // ��������
		}
		/*
		�����޷����յ�ʱ�ӳ����͵������ź�ʱ���Զ�����һ��·��
		*/
		if(StartAuto_Flag==1){
			if(WaitingTime<((gt_get()-AotoFirstTime)/1000))
			{
				StartAuto_Flag=0;//·��:
			}
		}

}