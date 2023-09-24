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
volatile uint32_t WaitingTime;     //�ӳ��������ʱ��+20�룬��Ҫ�����Զ�����·��
uint8_t StartAuto_Flag=0;

uint8_t Task_Table_Data[10][30] = {0};//ȫ��·��

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
/*
	����ָ���������'switch case'
	ʹ�ó����ģ�黯������ά��
*/
static uint8_t Task_Index = 0;//��ǰ�ڼ�������
static uint8_t Task_Number = 0;
static uint8_t Init_Flag = true;
static uint8_t Marker_Value = 0;
uint8_t next_task()
{
	next = Task_Table[Task_Index+1];//��ȡ��һ������
	Marker_Value = Task_Table[Task_Index+1] & 0x3F;//��ȡ��־����ֵ
	return  1;	
}
uint8_t change_direction()
{
	if (((next & 0x3F) >= Task_First_Dat) && ((next & 0x3F) < Special_Task_First_Dat)) 
		{
			next = ((next >> 6) & 0x03) + 1;		//��ȡ����
		}

		Change_Direction();			//�ı䷽��

	//����ģʽ(�ж�����ʲôģʽ:�Ƿ�ǰ������������ǰ�����Ƿ����)

	if (Task_Index == Task_Number) {//���������Ƿ�Ϊ���һ������
		
		if (RuKu_Flag == true) {//��������������		
			return  5;
		}
		
		else {
			return  4;//����ͣ�����
		}
	}
	else {
		if (Marker_Value < 5) {
			return  3; //����ʻ
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
	uint8_t state=0;//����������ݳ�ʼ��
	printf_LCD((char *)"%d",Marker_InitStruct.Light.FirstGear);
	Floor=(MO6%Marker_InitStruct.Light.FirstGear)+1;
	printf_LCD((char *)"%d",Floor);
	Garage_Track();					//�������
	delay_ms(200);
	Finish_Task();					//�ر�LED���������߳��
	Auto_Run_Flag = false;
	Init_Flag = true;
}
uint8_t task_ok()
{
	Task_Index += 1;	//����������������1
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
		if (Init_Flag == true) {    //�жϳ�ʼ��flag
			Init_Flag = false;                   //��֤��ʼ��һ��
			Task_Number = strlen((char*)Task_Table)-2;//��������
			Task_Index = 0;
			Mode = 0;
		}
		if(Mode >= 0 && Mode <= 6)
		{
			p[Mode]();
		}
	}
}
//void Digital_Tracking(void){//��ʹ�ú���ָ���������
//	
//	static uint8_t Init_Flag = true;
//	static uint8_t Task_Number = 0;
//	static uint8_t Task_Index = 0;//��ǰ�ڼ�������
//	static uint8_t Mode = 0;
//	static uint8_t Marker_Value = 0;
//	if(Auto_Run_Flag == true)
//	{
//		if (Init_Flag == true) {    //�жϳ�ʼ��flag
//			Init_Flag = false;                   //��֤��ʼ��һ��
//			Task_Number = strlen((char*)Task_Table)-2;//��������
//			Task_Index = 0;
//			Mode = 0;
//		}
//		switch (Mode)
//		{
//			case 0:
//			{
//				next = Task_Table[Task_Index+1];//��ȡ��һ������
//				Marker_Value = Task_Table[Task_Index+1] & 0x3F;//��ȡ��־����ֵ
//				Mode = 1;	
//			}
//			break;
//			case 1: 									//�ı䷽��
//			{
//				if (((next & 0x3F) >= Task_First_Dat) && ((next & 0x3F) < Special_Task_First_Dat)) 
//					{
//						next = ((next >> 6) & 0x03) + 1;		//��ȡ����
//					}

//					Change_Direction();			//�ı䷽��

//				//����ģʽ(�ж�����ʲôģʽ:�Ƿ�ǰ������������ǰ�����Ƿ����)

//				if (Task_Index == Task_Number) {//���������Ƿ�Ϊ���һ������
//					
//					if (RuKu_Flag == true) {//��������������		
//						Mode = 5;
//						break;
//					}
//					
//					else {
//						Mode = 4;//����ͣ�����
//						break;
//					}
//				}
//				else {
//					if (Marker_Value < 5) {
//						Mode = 3; //����ʻ
//					}
//					else {
//						Mode = 2;
//					}
//				}
//			
//			}		
//			break;
//			case 2://��������
//			{
//				Process_Task(Marker_Value);
//				Mode = 6;
//			}		
//			break;
//			case 3://�н�
//			{
//				track_PID(60);
//				go_forward(65, Mylib.go_value);
//				Mode = 6;
//			}			
//			break;
//			case 4://�����
//			{
//				Auto_Run_Flag = false;
//				Init_Flag = true;
//				
//				//StartAuto_Flag=1;
// 			  //AotoFirstTime=gt_get();    /****���Զ�����·��****/

//			}			
//			break;
//			case 5://���     
//			{
//				uint8_t state=0;//����������ݳ�ʼ��
//				printf_LCD((char *)"%d",Marker_InitStruct.Light.FirstGear);
//				Floor=(MO6%Marker_InitStruct.Light.FirstGear)+1;
//				printf_LCD((char *)"%d",Floor);
//				
//				//Init_Garage();
//				//state = 1;
//				//state=Garage_Reduction();			//���⸴λ
//				//if(state)
//				//{
//					Garage_Track();							//�������
//					delay_ms(200);
//					//Garage_Rise();							//��������
//				//}
//				Finish_Task();								//�ر�LED���������߳��
//				Auto_Run_Flag = false;
//				Init_Flag = true;
//			}	
//			break;
//			case 6://�����һ������
//			{
//				Task_Index += 1;	//����������������1
//				printf_LCD("FinishTaskNum:%d\r\n",Task_Index);
//				Mode = 0;			
//			}			
//			break;
//			default://���н�����Ϊ���һ������	
//			{

//			}
//			break;
//			}
//	}
//}

