
#ifndef _BT_PROFILE_CFG_H_
#define _BT_PROFILE_CFG_H_

#include "app_config.h"
#include "btcontroller_modules.h"


#if (TRANS_DATA_EN || RCSP_BTMATE_EN || RCSP_ADV_EN || SMART_BOX_EN ||ANCS_CLIENT_EN)
#define    BT_FOR_APP_EN             1
#else
#define    BT_FOR_APP_EN             0
#ifndef AI_APP_PROTOCOL
#define    AI_APP_PROTOCOL             0
#endif
#endif


///---sdp service record profile- 用户选择支持协议--///
#if (BT_FOR_APP_EN || APP_ONLINE_DEBUG)
#undef USER_SUPPORT_PROFILE_SPP
#define USER_SUPPORT_PROFILE_SPP   0
#endif

//ble demo的例子
#define DEF_BLE_DEMO_NULL                 0 //ble 没有使能
#define DEF_BLE_DEMO_ADV                  1 //only adv,can't connect
#define DEF_BLE_DEMO_TRANS_DATA           2 //
#define DEF_BLE_DEMO_DUEROS_DMA           3 //
#define DEF_BLE_DEMO_RCSP_DEMO            4 //
#define DEF_BLE_DEMO_ADV_RCSP             5
#define DEF_BLE_DEMO_GMA                  6
#define DEF_BLE_DEMO_CLIENT               7 //
#define DEF_BLE_TME_ADV					  8
#define DEF_BLE_ANCS_ADV				  9
#define DEF_BLE_DEMO_MI 				  10
#define DEF_BLE_DEMO_TUYA				  11
#define DEF_BLE_DEMO_MULTI				  12
#define DEF_BLE_DEMO_TUYA_MULTI			  13

//配置选择的demo
#if TCFG_USER_BLE_ENABLE

#if AI_APP_PROTOCOL
#define TCFG_BLE_DEMO_SELECT          DEF_BLE_DEMO_NULL

#elif (SMART_BOX_EN | RCSP_BTMATE_EN)
#define TCFG_BLE_DEMO_SELECT          DEF_BLE_DEMO_RCSP_DEMO

#elif TRANS_DATA_EN
#define TCFG_BLE_DEMO_SELECT          DEF_BLE_DEMO_TRANS_DATA

#elif RCSP_ADV_EN
#define TCFG_BLE_DEMO_SELECT          DEF_BLE_DEMO_ADV_RCSP

#elif TUYA_DEMO_EN
#define TCFG_BLE_DEMO_SELECT          DEF_BLE_DEMO_TUYA

#elif BLE_CLIENT_EN
#define TCFG_BLE_DEMO_SELECT          DEF_BLE_DEMO_CLIENT

#elif TRANS_MULTI_BLE_EN
#define TCFG_BLE_DEMO_SELECT          DEF_BLE_DEMO_MULTI

#elif TUYA_MULTI_BLE_EN
#define TCFG_BLE_DEMO_SELECT          DEF_BLE_DEMO_TUYA_MULTI

#elif ANCS_CLIENT_EN
#define TCFG_BLE_DEMO_SELECT          DEF_BLE_ANCS_ADV

#else
#define TCFG_BLE_DEMO_SELECT          DEF_BLE_DEMO_ADV
#endif

#else
#define TCFG_BLE_DEMO_SELECT          DEF_BLE_DEMO_NULL//ble is closed
#endif

//配对加密使能
#define TCFG_BLE_SECURITY_EN          0



#endif
