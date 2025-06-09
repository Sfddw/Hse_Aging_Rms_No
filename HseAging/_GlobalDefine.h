#pragma once

#ifndef _GOLBAL_DEFINE_
#define _GOLBAL_DEFINE_

#define DEFAULT_FONT						_T("Tahoma")

/////////////////////////////////////////////////////////////////////////////
#define MLOG_MAX_LENGTH						16*1024
#define ETHERNET_MLOG_ONOFF					0	// 0:OFF, 1:ON

/////////////////////////////////////////////////////////////////////////////Đs
#define PGM_VERSION							_T("1.0.0_R1")

/////////////////////////////////////////////////////////////////////////////
#define DEBUG_TCP_RECEIVE_OK				0
#define DEBUG_DIO_ALARM_DISABLE				0
#define	DEBUG_GMES_TEST_SERVER				0
#define DEBUG_COMM_LOG						0
#define DEBUG_DOOR_OPEN_TEST				0

/////////////////////////////////////////////////////////////////////////////
#define MAX_RACK							6
#define MAX_LAYER							5
#define MAX_LAYER_CHANNEL					16
#define MAX_CHANNEL							80

#define MAX_TEMP_SENSOR						6
/////////////////////////////////////////////////////////////////////////////
// ON/OFF와 TRUE/FALSE에 관련된 Define은 여기에서 한다.
/////////////////////////////////////////////////////////////////////////////
#define OFF									FALSE
#define ON									TRUE

#define NG									0
#define OK									1

#define DISABLE								0
#define ENABLE								1

#define POWER_OFF							0
#define POWER_ON							1

#define AGING_STOP							0
#define AGING_START							1

#define NACK								0
#define ACK									1

#define S1									0
#define S2									1

/////////////////////////////////////////////////////////////////////////////
#define LIMIT_NONE							0
#define LIMIT_HIGH							1
#define LIMIT_LOW							2

#define CHANNEL_USE							0
#define CHANNEL_UNUSE						1

#define CABLE_CHECK_OK						0
#define CABLE_CHECK_NG						1

#define DOOR_OPEN							0
#define DOOR_CLOSE							1
////////////////////////////////////////////////////////////////////////////
#define DIO_OUT_RED							0x01
#define DIO_OUT_YELLOW						0x02
#define DIO_OUT_GREEN						0x04
#define DIO_OUT_BUZZER						0x08

#define DIO_OUT_RED_BLINK					0x01
#define DIO_OUT_YELLOW_BLINK				0x02
#define DIO_OUT_GREEN_BLINK					0x04
#define DIO_OUT_BUZZER_BLINK				0x08


#define TOWER_LAMP_READY					0x01
#define TOWER_LAMP_RUNNING					0x02
#define TOWER_LAMP_COMPLETE					0x04
#define TOWER_LAMP_ERROR					0x08

////////////////////////////////////////////////////////////////////////////
enum
{
	DIO_IN_DOOR1 = 0,
	DIO_IN_DOOR2,
	DIO_IN_DOOR3,
	DIO_IN_DOOR4,
	DIO_IN_DOOR5,
	DIO_IN_DOOR6,
	DIO_IN_MAX
};


enum
{
	RACK_1 = 0,
	RACK_2,
	RACK_3,
	RACK_4,
	RACK_5,
	RACK_6
};

enum
{
	LAYER_1 = 0,
	LAYER_2,
	LAYER_3,
	LAYER_4,
	LAYER_5
};

enum
{
	DOOR_1 = 0,
	DOOR_2,
	DOOR_3,
	DOOR_4,
	DOOR_5,
	DOOR_6,
	DOOR_MAX
};

enum
{
	CH_1 = 0,
	CH_2,
	CH_3,
	CH_4,
	CH_5,
	CH_6,
	CH_7,
	CH_8,
	CH_9,
	CH_10,
	CH_11,
	CH_12,
	CH_13,
	CH_14,
	CH_15,
	CH_16

};

enum
{
	CONNECT_PG = 0,
	CONNECT_DIO,
	CONNECT_TEMP,
	CONNECT_BARCODE,
	CONNECT_MES,
	CONNECT_EAS,
	CONNECT_MAX
};

enum
{
	STATUS_NOT_CONNECT = 0,
	STATUS_IDLE,
	STATUS_RUN,
	STATUS_ERROR,
	STATUS_UNUSE
};

enum
{
	PIXEL_SINGLE = 0,
	PIXEL_DUAL,
	PIXEL_QUAD
};

enum
{
	AGING_IDLE = 0,
	AGING_RUNNING,
	AGING_COMPLETE,
	AGING_ERROR,
	AGING_COMPLETE_DOORCLOSE
};

enum
{
	ERR_INFO_NONE = 0,
	ERR_INFO_VCC,
	ERR_INFO_ICC,
	ERR_INFO_VBL,
	ERR_INFO_IBL
};


////////////////////////////////////////////
//------------GMES DEFINE-------------------
#define SERVER_MES					0
#define SERVER_EAS					1

#define MES_ID_TYPE_PID				0
#define MES_ID_TYPE_SERIAL			1

#define MES_DMOU_MODE_MANUAL		0
#define MES_DMOU_MODE_AUTO			1

#define MES_AGN_IN_AUTO				2

enum
{
	HOST_EAYT = 0,
	HOST_UCHK,
	HOST_EDTI,
	HOST_LPIR,
	HOST_PCHK,
	HOST_EPCR,
	HOST_FLDR,
	HOST_EICR,
	HOST_AGN_INSP,
	HOST_AGN_IN,
	HOST_AGN_OUT,
	HOST_DSPM,
	HOST_DMIN,
	HOST_DMOU,
	HOST_EQCC,
	HOST_APDR,
	HOST_ERCP,
	HOST_UNDO,
};

/////////////////////////////////////////////////////////////////////////////
// COLORREF
/////////////////////////////////////////////////////////////////////////////
#define COLOR_USER_BACKGROUND		RGB(240, 240, 255)
#define COLOR_BLACK					RGB(0,0,0)
#define COLOR_WHITE					RGB(255,255,255)
#define COLOR_RED					RGB(255,0,0)
#define COLOR_GREEN					RGB(0,255,0)
#define COLOR_BLUE					RGB(0,0,255)
#define COLOR_YELLOW				RGB(255,255,0)
#define COLOR_CYAN					RGB(0,255,255)
#define COLOR_SKYBLUE				RGB(199,199,255)
#define COLOR_SEABLUE				RGB(31,78,121)
#define COLOR_ORANGE				RGB(245,136,22)
#define COLOR_VERDANT				RGB(112,146,108)
#define COLOR_VERDANT2				RGB(240,255,240)
#define COLOR_GRAY16				RGB(16,16,16)
#define COLOR_GRAY32				RGB(32,32,32)
#define COLOR_GRAY48				RGB(48,48,48)
#define COLOR_GRAY64				RGB(64,64,64)
#define COLOR_GRAY80				RGB(80,80,80)
#define COLOR_GRAY94				RGB(94,94,94)
#define COLOR_GRAY96				RGB(96,96,96)
#define COLOR_GRAY128				RGB(128,128,128)
#define COLOR_GRAY159				RGB(159,159,159)
#define COLOR_GRAY192				RGB(192,192,192)
#define COLOR_GRAY224				RGB(224,224,224)
#define COLOR_GRAY240				RGB(240,240,240)
#define COLOR_GREEN128				RGB(0,128,0)
#define COLOR_GREEN224				RGB(0,224,0)
#define COLOR_RED128				RGB(128,0,0)
#define COLOR_BLUE128				RGB(0,0,128)
#define COLOR_PINK					RGB(255,130,169)
#define COLOR_MAGENTA				RGB(224,0,224)
#define COLOR_DARK_RED				RGB(169,100,100)
#define COLOR_DARK_GREEN			RGB(63,128,63)
#define COLOR_DARK_BLUE				RGB(63,63,255)
#define COLOR_DARK_ORANGE			RGB(230,160,0)
#define COLOR_DARK_YELLOW			RGB(224,224,128)
#define COLOR_DARK_MAGENTA			RGB(64,16,64)
#define COLOR_DARK_NAVY				RGB(68, 75, 83)
#define COLOR_BLUISH				RGB(65, 105, 225)
#define COLOR_LIGHT_RED				RGB(255,224,224)
#define COLOR_LIGHT_GREEN			RGB(113,255,170)
#define COLOR_LIGHT_BLUE			RGB(224,224,255)
#define COLOR_LIGHT_ORANGE			RGB(255,234,218)
#define COLOR_LIGHT_CYAN			RGB(150,230,220)
#define COLOR_LIGHT_YELLOW			RGB(231,250,205)
#define COLOR_LIGHT_BROWN			RGB(196,189,151)
#define COLOR_JADEGREEN				RGB(192,220,192)
#define COLOR_JADEBLUE				RGB(0,156,220)
#define COLOR_JADERED				RGB(236,112,112)
#define COLOR_DEEP_BLUE				RGB(77,97,128)
#define COLOR_GREENISH				RGB(146,208,80)
#define COLOR_PURPLE				RGB(112,48,160)
#define COLOR_DEF_LEVEL0_5			RGB(250,240,239)
#define COLOR_DEF_LEVEL1_0			RGB(243,225,224)
#define COLOR_DEF_LEVEL1_5			RGB(236,210,209)
#define COLOR_DEF_LEVEL2_0			RGB(229,195,194)
#define COLOR_DEF_LEVEL2_5			RGB(222,180,179)
#define COLOR_DEF_LEVEL3_0			RGB(215,175,174)
#define COLOR_DEF_LEVEL3_5			RGB(208,160,159)
#define COLOR_DEF_LEVEL4_0			RGB(201,145,144)
#define COLOR_DEF_LEVEL4_5			RGB(194,130,129)
#define COLOR_DEF_LEVEL5_0			RGB(187,115,114)
#define COLOR_DARK_BG				RGB(51,54,63)
#define COLOR_BUTTON_SEL			RGB(102,157,178)
#define COLOR_BUTTON_DARK			RGB(28,44,68)
#define COLOR_MAIN_TITLE			RGB(132,187,207)
#define COLOR_ITEM_HEAD				RGB(126,177,86)
#define COLOR_ITEM_TITLE			RGB(62,69,88)


typedef enum _COLOR_IDX_{
	COLOR_IDX_USER_BACKGROUND = 0,
	COLOR_IDX_BLACK,
	COLOR_IDX_RED,
	COLOR_IDX_GREEN,
	COLOR_IDX_BLUE,
	COLOR_IDX_YELLOW,
	COLOR_IDX_ORANGE,
	COLOR_IDX_VERDANT,
	COLOR_IDX_VERDANT2,
	COLOR_IDX_CYAN,
	COLOR_IDX_MAGENTA,
	COLOR_IDX_SKYBLUE,
	COLOR_IDX_SEABLUE,
	COLOR_IDX_LIGHT_RED,
	COLOR_IDX_LIGHT_GREEN,
	COLOR_IDX_LIGHT_BLUE,
	COLOR_IDX_LIGHT_ORANGE,
	COLOR_IDX_LIGHT_CYAN,
	COLOR_IDX_LIGHT_YELLOW,
	COLOR_IDX_LIGHT_BROWN,
	COLOR_IDX_GRAY64,
	COLOR_IDX_GRAY94,
	COLOR_IDX_GRAY96,
	COLOR_IDX_GRAY128,
	COLOR_IDX_GRAY159,
	COLOR_IDX_GRAY192,
	COLOR_IDX_GRAY224,
	COLOR_IDX_GRAY240,
	COLOR_IDX_RED128,
	COLOR_IDX_GREEN128,
	COLOR_IDX_BLUE128,
	COLOR_IDX_PINK,
	COLOR_IDX_BLUISH,
	COLOR_IDX_JADEGREEN,
	COLOR_IDX_JADEBLUE,
	COLOR_IDX_JADERED,
	COLOR_IDX_DARK_RED,
	COLOR_IDX_DARK_GREEN,
	COLOR_IDX_DARK_BLUE,
	COLOR_IDX_DARK_ORANGE,
	COLOR_IDX_DARK_YELLOW,
	COLOR_IDX_DARK_MAGENTA,
	COLOR_IDX_DARK_NAVY,
	COLOR_IDX_DARK_BG,
	COLOR_IDX_WHITE,
	COLOR_IDX_DEEP_BLUE,
	COLOR_IDX_GREENISH,
	COLOR_IDX_PURPLE,
	COLOR_IDX_ITEM_HEAD,
	COLOR_IDX_ITEM_TITLE,
	COLOR_IDX_DEF_LEVEL0_5,
	COLOR_IDX_DEF_LEVEL1_0,
	COLOR_IDX_DEF_LEVEL1_5,
	COLOR_IDX_DEF_LEVEL2_0,
	COLOR_IDX_DEF_LEVEL2_5,
	COLOR_IDX_DEF_LEVEL3_0,
	COLOR_IDX_DEF_LEVEL3_5,
	COLOR_IDX_DEF_LEVEL4_0,
	COLOR_IDX_DEF_LEVEL4_5,
	COLOR_IDX_DEF_LEVEL5_0,
	COLOR_IDX_DEF_SIMPLIFY1,
	COLOR_IDX_DEF_SIMPLIFY2,
	COLOR_IDX_DEF_SIMPLIFY3,
	COLOR_IDX_DEF_SIMPLIFY4,
	COLOR_IDX_DEF_SIMPLIFY5,
	COLOR_IDX_MAX
}_COLOR_IDX_;

#define FONT_IDX_MAX			10

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
#define UDP_SOCKET_PORT							50001
#define PACKET_SIZE								(16*1024)	// Firmware Download 때문에 최소 3K 이상 필요
#define LOCAL_HOST_IP							_T("127.0.0.1")
#define IP_ADDRESS_PG							_T("192.168.10.1")
#define IP_ADDRESS_DIO							_T("192.168.10.64")

#define TARGET_PC								0xA1
#define TARGET_MAIN								0xA2

#define PACKET_STX								0x02
#define PACKET_ETX								0x03

#define PACKET_PT_SOURCE						1
#define PACKET_PT_DEST							3
#define PACKET_PT_ID							5
#define PACKET_PT_CMD							7
#define PACKET_PT_LEN							9
#define PACKET_PT_RET							13
#define PACKET_PT_DATA							14

/////////////////////////////////////////////////////////////////////////////
// PG Command Packet
/////////////////////////////////////////////////////////////////////////////
// Command Define
#define CMD_DIO_OUTPUT							0x20
#define CMD_DIO_BOARD_INITIAL					0x21
#define CMD_DIO_INPUT							0xAB

#define CMD_SET_FUSING							0xA0
#define CMD_SET_CABLE_OPEN_CHECK				0xA5
#define CMD_SET_CHANNEL_USE_UNUSE				0xA6
#define CMD_GET_AGING_STATUS					0xAB
#define CMD_SET_POWER_SEQUENCE_ONOFF			0xAC
#define CMD_SET_AGING_START_STOP				0xAF
#define CMD_GET_POWER_MEASURE					0xBD
#define CMD_GET_FPGA_VERSION					0xFC
#define CMD_GET_FW_VERSION						0xFE
#define CMD_BOOTCODE_DOWNLOAD					0xF0	// CTRL Firmware Data Download Packet
#define CMD_BOOTCODE_COMPLETE					0xF2	// CTRL Firmware Data Download Complete
#define CMD_GOTOBOOT_DSECTION					0xF3	// CTRL Jump to Firmware Download Section
#define CMD_GOTOBOOT_USECTION					0xF4	// CTRL Jump to Firmware Upload Section



/////////////////////////////////////////////////////////////////////////////
// Macro Define
/////////////////////////////////////////////////////////////////////////////
#define	WM_UPDATE_SYSTEM_INFO					(WM_USER+102)
#define WM_UPDATE_QUANTITY_INFO					(WM_USER+103)
#define	WM_BCR_RACK_ID_INPUT					(WM_USER+104)




#endif