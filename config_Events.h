// *****************************************************************************
// * AppEvents.h
// *
// * Application event code 
// *
// * Copyright 2014 by hugh. All rights reserved.              
// *****************************************************************************
#ifndef __CONFIG_EVENTS_H__
#define __CONFIG_EVENTS_H__
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "AppEvents.h"
#include "glb_log.h"

/*event handler begin*/
#define GENERATED_EVENT_CODE  \
extern EventControl finger_msg_EVT;\
extern void finger_msg_event_handler(void);\
extern EventControl key_msg_EVT;\
extern void key_msg_event_handler(void);\
extern EventControl buzzer_msg_EVT;\
extern void buzzer_msg_event_handler(void);\
extern EventControl core_msg_EVT;\
extern void core_msg_event_handler(void);\
extern EventControl audio_msg_EVT;\
extern void audio_msg_event_handler(void);\
extern	EventControl audio_timer_msg_EVT;\
extern	void audio_timer_msg_event_handler(void);\
extern EventControl moter_msg_EVT;\
extern void moter_msg_event_handler(void);\
extern EventControl led_msg_EVT;\
extern void led_msg_event_handler(void);\
extern EventControl nfc_msg_EVT;\
extern void nfc_msg_event_handler(void);\
extern	EventControl zigbee_msg_EVT;\
extern	void zigbee_msg_event_handler(void);\
extern	EventControl zigbee_time_msg_EVT;\
extern	void zigbee_time_msg_event_handler(void);\
extern	EventControl key_time_msg_EVT;\
extern	void key_time_msg_event_handler(void);\
extern	EventControl zigbee_time_rtc_EVT;\
extern	void zigbee_time_rtc_event_handler(void);\
extern	EventControl nfc_time_msg_EVT;\
extern	void nfc_time_msg_event_handler(void);\
extern  EventControl adc_msg_EVT;\
extern  void adc_msg_event_handler(void);\
extern	EventControl adc_time_msg_EVT;\
extern	void adc_time_msg_event_handler(void);\
extern EventControl finger_timer_msg_EVT;\
extern void finger_timer_msg_event_handler(void);\


#define appevent {&finger_msg_EVT,finger_msg_event_handler},\
								 {&key_msg_EVT,key_msg_event_handler},\
								 {&buzzer_msg_EVT,buzzer_msg_event_handler},\
								 {&core_msg_EVT,core_msg_event_handler},\
								 {&audio_msg_EVT,audio_msg_event_handler},\
								 {&audio_timer_msg_EVT,audio_timer_msg_event_handler},\
								 {&moter_msg_EVT,moter_msg_event_handler},\
								 {&led_msg_EVT,led_msg_event_handler},\
								 {&nfc_msg_EVT,nfc_msg_event_handler},\
								 {&zigbee_msg_EVT,zigbee_msg_event_handler},\
								 {&zigbee_time_msg_EVT,zigbee_time_msg_event_handler},\
								 {&key_time_msg_EVT,key_time_msg_event_handler},\
								{&zigbee_time_rtc_EVT,zigbee_time_rtc_event_handler},\
								{&nfc_time_msg_EVT,nfc_time_msg_event_handler},\
								{&adc_msg_EVT,adc_msg_event_handler},\
								{&adc_time_msg_EVT,adc_time_msg_event_handler},\
								{&finger_timer_msg_EVT,finger_timer_msg_event_handler},\
								 
#endif
