#include <userint.h>
#include "CVIDLL.h" 
#include "function.h"    
#include "TxCalibration.h"
#include "global.h"
#include "radioGroup.h"
#include "toolbox.h"

int CVICALLBACK OnCaliLimitSet (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:

			MyDLL_License_Load ();
			showphID = License_Flag_panCALI;  
			
			break;
	}
	return 0;
}

int CVICALLBACK OnCaliLimitSet_Min (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:

			MyDLL_License_Load ();
			showphID = License_Flag_panCALI;  
			
			break;
	}
	return 0;
}

int CVICALLBACK On_Cali_Limit_OK (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int err;
	int channel;
	
	switch (event)
	{
		case EVENT_COMMIT:

			for (channel=0; channel<CHANNEL_MAX; channel++)
			{
		//		GetTableCellVal (panel, PAN_CALI_L_TABLE, MakePoint (3, channel*4+1), &my_struTxCal.power_min[channel]); 
		//		GetTableCellVal (panel, PAN_CALI_L_TABLE, MakePoint (4, channel*4+1), &my_struTxCal.power_max[channel]); 
				my_struTxCal.power_array[channel*4+1] = my_struTxCal.power_min[channel]; 
				my_struTxCal.power_array[channel*4+2] = my_struTxCal.power_max[channel]; 
	
		//		GetTableCellVal (panel, PAN_CALI_L_TABLE, MakePoint (2, channel*4+2), &sRxCal.power_in[channel]); 
		//		GetTableCellVal (panel, PAN_CALI_L_TABLE, MakePoint (3, channel*4+2), &sRxCal.power_min[channel]); 
		//		GetTableCellVal (panel, PAN_CALI_L_TABLE, MakePoint (4, channel*4+2), &sRxCal.power_max[channel]); 
				sRxCal.power_array[channel*4+0] = sRxCal.power_in[channel]; 
				sRxCal.power_array[channel*4+1] = sRxCal.power_min[channel];
				sRxCal.power_array[channel*4+2] = sRxCal.power_max[channel];
			}
			
			err = SetCaliConfigFile ();
			if (err) return -1;
			
			HidePanel (phCaliLimit); 
			
			break;
	}
	return 0;
}

int CVICALLBACK On_Cali_Limit_Quit (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:

			HidePanel (phCaliLimit);
			
			break;
	}
	return 0;
}

int CVICALLBACK On_CheckT_Quit (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{	  
	int  channel;
	switch (event)
	{
		case EVENT_COMMIT:

			HidePanel (phCheckT); 
			//关闭所有通道电源    
			for(channel=0;channel<CHANNEL_MAX;channel++)
			{   
				if (INSTR[channel].ChannelEn)
				{	 
					SET_EVB_SHDN(channel, 0);
				}
			}
			break;
	}
	return 0;
}

void CVICALLBACK On_CheckT (int menuBar, int menuItem, void *callbackData,
		int panel)
{
	int progresshandle;
	int channel;
	int error;
	
	DisplayPanel (phCheckT); 
	
	progresshandle = CreateProgressDialog ("设备配置状态", "设备配置进度", 1, VAL_NO_INNER_MARKERS, "");
	
	//for BERT
	for (channel=0;channel<CHANNEL_MAX;channel++)
	{
		UpdateProgressDialog (progresshandle, (int)(100.*channel/CHANNEL_MAX), 0);
	
		if (INSTR[channel].ChannelEn)
		{
			SetTableCellRangeAttribute (phCheckT, PAN_CHE_T_TABLE, MakeRect (channel+1, 2, 1, 3), ATTR_CELL_DIMMED, 0);
			
			//Power on
			errChk(SET_EVB_SHDN(channel, 1));
	
			//清除控件显示信息
			SetTableCellVal (phCheckT, PAN_CHE_T_TABLE, MakePoint (2, channel+1), ""); 
			SetTableCellVal (phCheckT, PAN_CHE_T_TABLE, MakePoint (3, channel+1), 0.); 
		}
		else
		{
			SetTableCellRangeAttribute (phCheckT, PAN_CHE_T_TABLE, MakeRect (channel+1, 2, 1, 3), ATTR_CELL_DIMMED, 1); 	
		}
	}
	
Error:
		
	DiscardProgressDialog (progresshandle); 
}

int CVICALLBACK On_CheckT_OK (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int error=0;
	int channel;
	char buf[1024];
	
	switch (event)
	{
		case EVENT_COMMIT:

			for (channel=0; channel<CHANNEL_MAX; channel++)
			{
				if (INSTR[channel].ChannelEn)
				{
					GetTableCellVal (phCheckT, PAN_CHE_T_TABLE, MakePoint (2, channel+1), sTxCheck.sn_array[channel]);
					GetTableCellVal (phCheckT, PAN_CHE_T_TABLE, MakePoint (3, channel+1), &sTxCheck.power_sys[channel]); 
					
					if (0==strlen(sTxCheck.sn_array[channel]))
					{
						sprintf (buf, "请先读取通道%d的标准模块序列号    ", channel);
						MessagePopup ("提示", buf); 
						return -1;
					}
			
					//读取点检门限
					error = DB_get_txchecklimit(channel);
					if (error)
					{
						return -1;
					}
			
					if (sTxCheck.power_sys[channel]>sTxCheck.power_max[channel] || sTxCheck.power_sys[channel]<sTxCheck.power_min[channel])
					{
						sprintf (buf, "通道%d点检模块功率读数超出设定范围，点检失败    \n请重新校准发端光路", channel);
						MessagePopup ("提示", buf); 
						return -1;
					}
			
					error = DB_save_txcheckup (channel, sTxCheck.sn_array[channel], sTxCheck.power_sys[channel]);
					if (error)
					{
						MessagePopup ("提示", "保存点检数据错误   ");        
						return -1;
					}
				}
			}
			
			HidePanel (phCheckT); 
			//关闭所有通道电源    
			for(channel=0;channel<CHANNEL_MAX;channel++)
			{   
				if (INSTR[channel].ChannelEn)
				{	 
					SET_EVB_SHDN(channel, 0);
				}
			}
			break;
	}
	return 0;
}

int CVICALLBACK On_Check_Table (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	char sn[30];
	double val;  
	int channel;
	Point focus; 
	
	switch (event)
	{
		case EVENT_COMMIT:

			GetActiveTableCell(panel, control, &focus);
			
			if (focus.x == 4) 
			{
				channel = focus.y-1;
				
				MyDLLReadModuleSN (INST_EVB[channel], sn);
			
				MyDLLCheckSN (sn);
			
				if (Get_Power(channel, &val)<0) 
				{
					MessagePopup ("ERROR", "Read power meter error !"); 
					return -1;
				} 
			
				val += my_struTxCal.power[channel]; 
					  
				SetTableCellVal (panel, PAN_CHE_T_TABLE, MakePoint (2, focus.y), sn); 
				SetTableCellVal (panel, PAN_CHE_T_TABLE, MakePoint (3, focus.y), val); 
			}
			
			break;
	}
	return 0;
}

int CVICALLBACK On_CaliLimit_Chan (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	static int channel=0;
	int temp;
	int i;
	
	switch (event)
	{
		case EVENT_COMMIT:

			Radio_GetMarkedOption(panel, PAN_CALI_L_RAD_CHAN, &temp);
			
			if (INSTR[temp].ChannelEn)
			{
				channel=temp;
				
				//加载当前通道光路校准参数
				SetTableCellVal (panel, PAN_CALI_L_TABLE, MakePoint (2, 1), 0.); 
				SetTableCellVal (panel, PAN_CALI_L_TABLE, MakePoint (3, 1), my_struTxCal.power_min[channel]); 
				SetTableCellVal (panel, PAN_CALI_L_TABLE, MakePoint (4, 1), my_struTxCal.power_max[channel]); 
			
				SetTableCellVal (panel, PAN_CALI_L_TABLE, MakePoint (2, 2), sRxCal.power_in[channel]);  
				SetTableCellVal (panel, PAN_CALI_L_TABLE, MakePoint (3, 2), sRxCal.power_min[channel]); 
				SetTableCellVal (panel, PAN_CALI_L_TABLE, MakePoint (4, 2), sRxCal.power_max[channel]); 
				
				SetTableCellVal (phCaliLimit, PAN_CALI_L_TABLE, MakePoint (2, 3), my_struCal_OLT.Power_In[channel]);  
				SetTableCellVal (phCaliLimit, PAN_CALI_L_TABLE, MakePoint (3, 3), my_struCal_OLT.Power_Min[channel]); 
				SetTableCellVal (phCaliLimit, PAN_CALI_L_TABLE, MakePoint (4, 3), my_struCal_OLT.Power_Max[channel]); 
			}
			else
			{
				MessagePopup ("提示", "被选通道未启用    ");
				Radio_SetMarkedOption(panel, PAN_CALI_L_RAD_CHAN, channel);	
			}
			
			break;
	}
	return 0;
}

int CVICALLBACK On_Cali_Limit_APPLICATION (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int channel;
	int error;
	
	switch (event)
	{
		case EVENT_COMMIT:
			
				Radio_GetMarkedOption(panel, PAN_CALI_L_RAD_CHAN, &channel);
			
				GetTableCellVal (panel, PAN_CALI_L_TABLE, MakePoint (3, 1), &my_struTxCal.power_min[channel]); 
				GetTableCellVal (panel, PAN_CALI_L_TABLE, MakePoint (4, 1), &my_struTxCal.power_max[channel]); 
				my_struTxCal.power_array[channel*4+1] = my_struTxCal.power_min[channel]; 
				my_struTxCal.power_array[channel*4+2] = my_struTxCal.power_max[channel]; 
				
				//发端小光
				GetTableCellVal (panel, PAN_CALI_L_TABLE, MakePoint (3, 1), &my_struTxCal.minpower_min[channel]); 
				GetTableCellVal (panel, PAN_CALI_L_TABLE, MakePoint (4, 1), &my_struTxCal.minpower_max[channel]); 
				my_struTxCal.minpower_array[channel*4+1] = my_struTxCal.minpower_min[channel]; 
				my_struTxCal.minpower_array[channel*4+2] = my_struTxCal.minpower_max[channel]; 
	
				GetTableCellVal (panel, PAN_CALI_L_TABLE, MakePoint (2, 2), &sRxCal.power_in[channel]); 
				GetTableCellVal (panel, PAN_CALI_L_TABLE, MakePoint (3, 2), &sRxCal.power_min[channel]); 
				GetTableCellVal (panel, PAN_CALI_L_TABLE, MakePoint (4, 2), &sRxCal.power_max[channel]); 
				sRxCal.power_array[channel*4+0] = sRxCal.power_in[channel]; 
				sRxCal.power_array[channel*4+1] = sRxCal.power_min[channel];
				sRxCal.power_array[channel*4+2] = sRxCal.power_max[channel];
				
				GetTableCellVal (phCaliLimit, PAN_CALI_L_TABLE, MakePoint (2, 3), &my_struCal_OLT.Power_In[channel]);
				GetTableCellVal (phCaliLimit, PAN_CALI_L_TABLE, MakePoint (3, 3), &my_struCal_OLT.Power_Min[channel]); 
				GetTableCellVal (phCaliLimit, PAN_CALI_L_TABLE, MakePoint (4, 3), &my_struCal_OLT.Power_Max[channel]); 
				my_struCal_OLT.Array[channel*5+0] = my_struCal_OLT.Power_In[channel]; 
				my_struCal_OLT.Array[channel*5+1] = my_struCal_OLT.Power_Min[channel]; 
				my_struCal_OLT.Array[channel*5+2] = my_struCal_OLT.Power_Max[channel]; 
						
				error = SetCaliConfigFile ();
				if (error) 
				{
					return -1;
				}
			

			break;
	}
	return 0;
}
