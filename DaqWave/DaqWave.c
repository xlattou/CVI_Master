#include <cvirte.h>		
#include <userint.h>
#include "DaqWave.h"

static int panelHandle;

int main (int argc, char *argv[])
{
	if (InitCVIRTE (0, argv, 0) == 0)
		return -1;	/* out of memory */
	if ((panelHandle = LoadPanel (0, "DaqWave.uir", PANEL)) < 0)
		return -1;
	DisplayPanel (panelHandle);
	RunUserInterface ();
	DiscardPanel (panelHandle);
	return 0;
}

int CVICALLBACK AcquireCallback (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	double ScanRate;
	char ChanelString[20];
	int value;
	switch (event)
	{
		case EVENT_COMMIT:
			GetCtrlVal (panelHandle, PANEL_ACQUIRE, &value);
			switch(value)
			{
				case 1:
					GetCtrlVal (panelHandle, PANEL_CHANNEL_STRING, ChanelString);
					GetCtrlVal (panelHandle, PANEL_SCANRATE, &ScanRate);
					
					
					break;
				
					case 0:
						break;
						
					default:
						break;
			}
			break;
	}
	return 0;
}

int CVICALLBACK AITimerCallback (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_TIMER_TICK:
			int a = 3;
			break;
	}
	return 0;
}

int CVICALLBACK ClearCallback (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:

			break;
	}
	return 0;
}

int CVICALLBACK QuitCallback (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			QuitUserInterface (0);
			break;
	}
	return 0;
}
