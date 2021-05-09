// *****************************************************************************
// * AppEvents.c
// *
// * Application event code 
// *
// * Copyright 2014 by hugh. All rights reserved.              
// *****************************************************************************
//--- Include header file ----------------------------------------------------//
#include <string.h>
#include "stm32l0xx.h"
#include "AppEvents.h"
#include "config_Events.h"

#ifdef GENERATED_EVENT_CODE
GENERATED_EVENT_CODE
#endif //GENERATED_EVENT_CODE
//----------------------------------------------------------------------------//
//****************************************************************************//

EventData appevents[] ={appevent
                   {NULL,NULL}};
UINT8 RFTESTFlag=0;


//### #######################################################//
/*Generel operation begin*/
UINT32 GetUINT32NowtimeMs(void)
{
       return HAL_GetTick();
}


//--- Timer ------------------------------------------------------------------//
UINT32 Event_Timer_ElapsedTime(UINT32 a_u32PrevTime, UINT32 a_u32CurTime)
{
  UINT32  u32ElapsedTime;


  if (a_u32PrevTime > a_u32CurTime) {
    u32ElapsedTime = 0xffffffff - a_u32PrevTime;
    u32ElapsedTime += a_u32CurTime;
  }
  else {
    u32ElapsedTime = a_u32CurTime - a_u32PrevTime;
  }

  return u32ElapsedTime;
} // end of <PDK_Timer_ElapsedTime>

int Event_CheckTimeout(UINT32 a_u32Timeout,UINT32 a_u32timestart)
{
	UINT32	u32ElapsedTime =Event_Timer_ElapsedTime(a_u32timestart, GetUINT32NowtimeMs());
	
	if (u32ElapsedTime >= a_u32Timeout) 
	{
		return 0;
	}
	else
	{
		return a_u32Timeout-u32ElapsedTime;
	}
} // end of <ZHA_CheckTimeout>

//³õÊ¼»¯²Ù×÷
void AppEvenInit(void)
{ 
	EventData *nextEvent;
	for (nextEvent = appevents; ; nextEvent++) 
	{
	      EventControl *control = nextEvent->control;
	       if (control == NULL)
	             break;
				memset(control,0,sizeof(EventControl));
	}	
}


/*Generel operation end*/

void EventSetMsg(EventControl *EVENT,int type,UINT32 delayms)
{
    EVENT->status = type;
    EVENT->timeStart= GetUINT32NowtimeMs();
    EVENT->timeToExecute = delayms;  
//		log_info("<delayms=%d::EVENT=%p>\n",delayms,EVENT);
}

/*event operation begin*/
void EventSetDelayMs(EventControl *EVENT,UINT32 delayms)
{
		EventSetMsg(EVENT,EVENT_MS_TIME, delayms);
}

void EventSetActive(EventControl *EVENT)
{
		EventSetMsg(EVENT,EVENT_ZERO_DELAY, 0);
}

void EventSetInactive(EventControl *EVENT)
{
      EVENT->status = EVENT_INACTIVE;
      EVENT->timeToExecute = 0;
}

UINT8 EventGetActiveFlag(EventControl *EVENT)
{
      return   EVENT->status;
}

int eventgetsleeptime(void)
{
	int sleeptime=-1;
	
	EventData *nextEvent;
	for (nextEvent = appevents; ; nextEvent++) 
	{
	      EventControl *control = nextEvent->control;
	       if (control == NULL)
	       return sleeptime;
				 
	       if (control->status == EVENT_ZERO_DELAY)
				 {
						sleeptime=0;
				 }
				 else if(control->status == EVENT_MS_TIME)
				 {
					 int t=Event_CheckTimeout(control->timeToExecute,control->timeStart);
						if(t>=0 && sleeptime<0)
							sleeptime=t;
						
					 if(t>=0 && t<sleeptime)
						 sleeptime=t;
				 }
	}		
}

static void AppRunEvents(EventData *events)
{

	EventData *nextEvent=events;
	
	for (; ; nextEvent++) 
	{
	      EventControl *control = nextEvent->control;
	       if (control == NULL)
	             break;
				 if(control->status == EVENT_INACTIVE)
					 continue;
				 
	      if(control->status == EVENT_ZERO_DELAY || 
					((control->status == EVENT_MS_TIME)&& 
				  (!Event_CheckTimeout(control->timeToExecute,control->timeStart)))) 
	      {    
					EventSetInactive(control);
		  			nextEvent->handler();					
	      }
	}
}
/*event operation end*/
//ÂÖÑ¯
void AppMainRun(void)
{
	AppRunEvents(appevents);
}

//****************************************************************************//

