#include "pch.h"
#include "Temp2xxx.h"

/////////////////////////////////////////////////////////////////////////////


CTemp2xxx::CTemp2xxx(void)
{
	pR232Port = new CRs232Port;

	m_bPortOpen1 = FALSE;
	m_bPortOpen2 = FALSE;
	m_bIsReceiveACK1 = FALSE;
	m_bIsReceiveACK2 = FALSE;
}

CTemp2xxx::~CTemp2xxx(void)
{
	delete pR232Port;
}

//////////////////////////////////////////////////////////////////////////////
// Protected
BOOL CTemp2xxx::rs232_OpenPort1 (CString sPortName, unsigned long dwBaud)
{
	return pR232Port->OpenPort1(sPortName, dwBaud, NULL);
}

BOOL CTemp2xxx::rs232_OpenPort2 (CString sPortName, unsigned long dwBaud)
{
	return pR232Port->OpenPort2(sPortName, dwBaud, NULL);
}

BOOL CTemp2xxx::rs232_OpenPort3(CString sPortName, unsigned long dwBaud)
{
	return pR232Port->OpenPort3(sPortName, dwBaud, NULL);
}


void CTemp2xxx::rs232_ClosePort1 ()
{
	pR232Port->ClosePort1();
}

void CTemp2xxx::rs232_ClosePort2 ()
{
	pR232Port->ClosePort2();
}

ULONG CTemp2xxx::rs232_WritePort1 (unsigned char* wrbuff, unsigned long nToWriteLen)
{
	DWORD dwWritten=0;

	dwWritten = pR232Port->WritePort1(wrbuff, nToWriteLen);

	return dwWritten;
}

ULONG CTemp2xxx::rs232_WritePort2 (unsigned char* wrbuff, unsigned long nToWriteLen)
{
	DWORD dwWritten=0;

	dwWritten = pR232Port->WritePort2(wrbuff, nToWriteLen);

	return dwWritten;
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
BOOL CTemp2xxx::Temp2xxx_rs232send(unsigned char* sendmsg, unsigned long dwsize)
{
	if(m_bPortOpen1 == FALSE)
		return FALSE;

#if (DEBUG_CHAMBER_COM_LOG==1)
	///////////////////////////////////////////////
	TCHAR szlog[4096]={0,};
	_stprintf_s(szlog, _T("%s"), char_To_wchar((char*)sendmsg));
	m_pApp->gf_fucWriteMLog(_T("<<PL4J-SEND>>"), szlog);
	///////////////////////////////////////////////
#endif

	rs232_WritePort1(sendmsg, dwsize);


	return TRUE;
}

BYTE CTemp2xxx::Temp2xxx_makeCheckSum(char* packet)
{
	BYTE sum=0;
	int  len;

	len = (int)strlen(packet);

	for(int i=0; i<len; i++)
	{
		sum += packet[i];
	}

	return sum;
}

BOOL CTemp2xxx::Temp2xxx_sendPacket(char* packet)
{
	BOOL bRet=TRUE;
	char szsendmsg[1024]={0,};
	ULONG dwsize=0;

#if 1
	BYTE sum;
	sum = Temp2xxx_makeCheckSum(packet);
	// 마지막에 CR코드가 들어간다.
	sprintf_s(szsendmsg, "%c%02d%s%02X%c%c", _STX_, TEMP2XXX_ADDR, packet, sum, 0x0D, 0x0A);
#else
	sprintf_s(szsendmsg, "%c%02d%s%c%c", _STX_, TEMP2XXX_ADDR, packet, 0x0D, 0x0A);
#endif
	dwsize = (ULONG)strlen(szsendmsg);

	m_bIsReceiveACK1=FALSE;
	Temp2xxx_rs232send((BYTE*)szsendmsg, dwsize);

	return bRet;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
void CTemp2xxx::Temp2xxx_ClosePort()
{
	rs232_ClosePort1();
	Sleep(200);

	m_bPortOpen1 = FALSE;
}

BOOL CTemp2xxx::Temp2xxx_Initialize()
{
	int port;
	CString sPort;

	rs232_ClosePort1();
	Sleep(200);

	Read_SysIniFile(_T("SYSTEM"), _T("TEMP_CONTROLLER_PORT"), &port);
	if(port==0)	return TRUE;

	sPort.Format(_T("COM%d"), port);
	if(rs232_OpenPort1(sPort, 57600) == FALSE)
	{
		CString err;
		err.Format(_T("Temperature Controller COM Port Open Error => [%s]"), sPort);
		AfxMessageBox(err, MB_ICONERROR);
		return FALSE;
	}

	m_bPortOpen1	= TRUE;

	return TRUE;
}

BOOL CTemp2xxx::Temp2xxx_readTemp()
{
	BOOL bRet=FALSE;
	char buff[100]={0,};

	sprintf_s(buff, "RRD,01,0001");
	bRet = Temp2xxx_sendPacket(buff);

	return bRet;
}


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
BOOL CTemp2xxx::TempSDR100_rs232send(unsigned char* sendmsg, unsigned long dwsize)
{
#if (DEBUG_CHAMBER_COM_LOG==1)
	///////////////////////////////////////////////
	TCHAR szlog[4096]={0,};
	_stprintf_s(szlog, _T("%s"), char_To_wchar((char*)sendmsg));
	m_pApp->gf_fucWriteMLog(_T("<<PL4J-SEND>>"), szlog);
	///////////////////////////////////////////////
#endif

	rs232_WritePort2(sendmsg, dwsize);


	return TRUE;
}

BOOL CTemp2xxx::TempSDR100_sendPacket(char* packet, int bACK)
{
	BOOL bRet=TRUE;
	char szsendmsg[1024]={0,};
	ULONG dwsize=0;

	// 마지막에 CR코드가 들어간다.
	sprintf_s(szsendmsg, "%c%02d%s%c%c", _STX_, TEMPSDR100_ADDR, packet, 0x0D, 0x0A);
	dwsize = (ULONG)strlen(szsendmsg);

	m_bIsReceiveACK2=FALSE;
	TempSDR100_rs232send((BYTE*)szsendmsg, dwsize);

	// Ack Receive
	if(bACK==TRUE)
	{
		bRet = TempSDR100_receiveACK();
	}

	return bRet;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
void CTemp2xxx::TempSDR100_ClosePort()
{
	rs232_ClosePort2();
	Sleep(200);
}

BOOL CTemp2xxx::TempSDR100_Initialize(int port)
{
	CString sPort;

	rs232_ClosePort2();
	Sleep(200);

	if (port == 0)	return TRUE;

	sPort.Format(_T("COM%d"), port);
	if(rs232_OpenPort2(sPort, 115200) == FALSE)
	//sPort.Format(_T("COM10"));
	if (rs232_OpenPort2(sPort, 115200) == FALSE)
	{
		CString err;
		err.Format(_T("Temperature Recorder COM Port Open Error => [%s]"), sPort);
		AfxMessageBox(err, MB_ICONERROR); // 온도 포트 확인
		return FALSE;
	}

	m_bPortOpen2	= TRUE;

	return TRUE;
}

BOOL CTemp2xxx::TempSDR100_receiveACK()
{
	DWORD stTick = ::GetTickCount();

	while(1)
	{
		DWORD edTick = ::GetTickCount();
		if((edTick-stTick) > RS232_WAIT_TIME)
			return  FALSE;				// Time Out

		if(m_bIsReceiveACK2==TRUE)		// ACK Receive
			break;

		ProcessMessage();
	}

	return TRUE;
}

BOOL CTemp2xxx::TempSDR100_readTemp()
{
	BOOL bRet=FALSE;
	char buff[100]={0,};

	sprintf_s(buff, "RSD,%02d,0001", MAX_TEMP_SENSOR);
	bRet = TempSDR100_sendPacket(buff);

	return bRet;
}
