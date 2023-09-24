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

extern uint32_t Power_check_times;      // �����������
extern uint32_t WIFI_Upload_data_times; // ͨ��Wifi�ϴ���������
extern uint32_t RFID_Init_Check_times;
extern uint32_t function_init_times; // ����������
/*
���ܣ�����·������
Table_Num��·�߱��0-10 ���10��·��
Table_Data���ӳ�·����������
*/
void Digital_Track_Table_Set(uint8_t Table_Num, uint8_t* Table_Data);

/*
���ܣ���������
Table_Num��·�߱��0-10 ���10��·��
Is_Init_Current������ǰ�Ƿ���賵ͷ��������ķ���Ϊ·�ߵĵ�0������
Is_RuKu_Flag���Ƿ����
*/
void Start_Digital_Track_Table(u8 Table_Num, bool Is_Init_Current, bool Is_RuKu_Flag);

void Digital_Track_Init(void);

extern uint8_t Current;//��ͷ����

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

#define Terrain(x) 			(((x - 1) << 6) | Terrain_ENUM)			//���α�־��	
#define RFID(x)		 			(((x - 1) << 6) | RFID_ENUM)			  //RFID��־��
#define TFTA(x)		 			(((x - 1) << 6) | TFTA_ENUM)			  //A TFT��־��
#define TFTB(x)		 			(((x - 1) << 6) | TFTB_ENUM)			  //B TFT��־��
#define TFTC(x)		 			(((x - 1) << 6) | TFTC_ENUM)			  //C TFT��־��
#define Liti(x)		 			(((x - 1) << 6) | Liti_ENUM)		 		//������ʾ��־��
#define SYN(x)		 			(((x - 1) << 6) | SYN_ENUM)					//����������־��
#define Light(x)	 			(((x - 1) << 6) | Light_ENUM)				//����·�Ʊ�־��
#define TrafficA(x)			(((x - 1) << 6) | TrafficA_ENUM)		//A ��ͨ�Ʊ�־��
#define TrafficB(x)			(((x - 1) << 6) | TrafficB_ENUM)		//B ��ͨ�Ʊ�־��
#define TrafficC(x)			(((x - 1) << 6) | TrafficC_ENUM)		//C ��ͨ�Ʊ�־��
#define TrafficD(x)			(((x - 1) << 6) | TrafficD_ENUM)		//D ��ͨ�Ʊ�־��
#define Alarm(x)	 			(((x - 1) << 6) | Alarm_ENUM)				//����̨��־��
#define Code(x)		 			(((x - 1) << 6) | Code_ENUM)				//��ά���־��
#define NoRFID(x)	 			(((x - 1) << 6) | NoRFID_ENUM)			//��ʶ��RFID
#define Range(x)	 			(((x - 1) << 6) | Range_ENUM)				//���
#define Task(x)		 			(((x - 1) << 6) | Task_ENUM)				//ִ���������
#define Static(x)				(((x - 1) << 6) | Static_ENUM)			//ʶ��̬��־��ͼ��
#define Park_Correct(x) (((x - 1) << 6) | Park_Correct_ENUM)	//ͣ������
#define Gate(x) 				(((x - 1) << 6) | Gate_ENUM)				//��բ��־��
#define ETC(x)		 			(((x - 1) << 6) | ETC_ENUM)					//ETC��־��
#define WaitSTrack(x)		 	(((x - 1) << 6) | WaitSTrack_ENUM)				//����С������
#define Terrain_RFID(x)		 	(((x - 1) << 6) | Terrain_RFID_ENUM)			//���κ�RFID���

#define Task_First_Dat					Terrain_ENUM								//���������׵�ַ

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
#define L_Mapan				L_Mapan_ENUM													//��������ת��
#define R_Mapan				R_Mapan_ENUM													//��������ת��
#define Avoid					Avoid_ENUM														//��ܺ�·��
#define LED_Tba				LED_Tba_ENUM													//�������������
#define Go_ENUM				Go_ENUM															//ǰ��
#define Back_ENUM 		Back_ENUM														//����

#define Special_Task_First_Dat			L_Mapan					//��ͨ�����׵�ַ
#endif
