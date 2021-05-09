// *****************************************************************************
// * AppEvents.h
// *
// * Application event code 
// *
// * Copyright 2014 by hugh. All rights reserved.              
// *****************************************************************************
#ifndef __APPEVENTS_H__
#define __APPEVENTS_H__

#include <stdint.h>
#define MAX_INT16U_VALUE 0XFFFF 
#define UINT8  uint8_t
#define UINT32 uint32_t
#define UINT16 uint16_t
#define PUINT8 uint8_t*

#define EVENT_DATA_LEN  100  // 1S
#define EVENT_MAX_SIZE  64
#define EVENT_MSG_TIME  10
typedef UINT8 Eventstatus;

enum 
{
	  /**事件未激活 */
	  EVENT_INACTIVE = 0,
	  /** 事件已激活且以ms为单位计量*/
	  EVENT_MS_TIME,
	  /** 事件激活并马上执行 */
	  EVENT_ZERO_DELAY
};


//事件参数
typedef  struct  _EventControl
{
       Eventstatus   status;//event status
       UINT32 timeToExecute;//time to execute the event       
       UINT32 timeStart;//time to execute the event
} EventControl;

//事件结构
typedef  struct  
{
       EventControl   *control;//event 
       void(*handler)(void);//event handler
} EventData;

//Generel operation
UINT32 GetUINT32NowtimeMs(void);

//event operation
void EventSetDelayMs(EventControl *EVENT,UINT32 delayms);
void EventSetActive(EventControl *EVENT);
void EventSetInactive(EventControl *EVENT);
void EventSetMsg(EventControl *EVENT,int type,UINT32 delayms);
UINT8 EventGetActiveFlag(EventControl *EVENT);
void AppMainRun(void);
void AppEvenInit(void);
#endif
