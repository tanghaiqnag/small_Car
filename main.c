/**
�������ƣ�2023�����ۺϳ���
�޸�ʱ�䣺2023.9.5
*/
#include <stdio.h>
#include "stm32f4xx.h"
#include <stdbool.h>
#include "canp_hostcom.h"
#include "delay.h"
#include "Timer.h"
#include "Digital_track.h"
#include "Bsp_init.h"

int main(void)
{
	Power_check_times      = gt_get() + 200;
	WIFI_Upload_data_times = gt_get() + 200;
	RFID_Init_Check_times  = gt_get() + 200;
	function_init_times    = gt_get() + 200;
	Hardware_Init();//Ӳ����ʼ��
	Send_UpMotor(0, 0);
	Delay_ms_02(1000);
	while(1)
	{
			Scheduler_run();//����ִ��
	}
}