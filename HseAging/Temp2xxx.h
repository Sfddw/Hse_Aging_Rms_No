#pragma once

#include "Rs232Port.h"
/////////////////////////////////////////////////////////////////////////////
// User Define
#define	RS232_WAIT_TIME		2000

#define TEMP2XXX_ADDR		1
#define TEMPSDR100_ADDR		1

#define RUNMODE_TIMEOUT		0
#define RUNMODE_SCENARIO	1

#define OPMODE_RUN			1
#define OPMODE_HOLD			2
#define OPMODE_STEP			3
#define OPMODE_STOP			4

#define _STX_				0x02


class CTemp2xxx
{
public:
	CTemp2xxx(void);
	~CTemp2xxx(void);

	BOOL m_bPortOpen1;
	BOOL m_bPortOpen2;
	BOOL m_bIsReceiveACK1;
	BOOL m_bIsReceiveACK2;

	BYTE  Temp2xxx_makeCheckSum(char* packet);
	void  Temp2xxx_ClosePort();
	BOOL  Temp2xxx_Initialize();
	BOOL  Temp2xxx_readTemp();
	BOOL  Temp2xxx_sendPacket(char* packet);
	BOOL  Temp2xxx_rs232send(unsigned char* sendmsg, unsigned long dwsize);

	BYTE  TempSDR100_makeCheckSum(char* packet);
	void  TempSDR100_ClosePort();
	BOOL  TempSDR100_Initialize(int port);
	BOOL  TempSDR100_receiveACK();
	BOOL  TempSDR100_readTemp();
	BOOL  TempSDR100_sendPacket(char* packet, BOOL bACK=FALSE);
	BOOL  TempSDR100_rs232send(unsigned char* sendmsg, unsigned long dwsize);

protected:
	CRs232Port*		pR232Port;

	BOOL  rs232_OpenPort1 (CString sPortName, unsigned long dwBaud);
	BOOL  rs232_OpenPort2 (CString sPortName, unsigned long dwBaud);

	BOOL  rs232_OpenPort3(CString sPortName, unsigned long dwBaud);

	void  rs232_ClosePort1 ();
	void  rs232_ClosePort2 ();

	void  rs232_ClosePort3();

	ULONG rs232_WritePort1 (unsigned char* wrbuff, unsigned long nToWriteLen);
	ULONG rs232_WritePort2 (unsigned char* wrbuff, unsigned long nToWriteLen);

	ULONG rs232_WritePort3(unsigned char* wrbuff, unsigned long nToWriteLen);
};
