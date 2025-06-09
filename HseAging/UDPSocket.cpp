// UDPSocket.cpp : 구현 파일입니다.
//

#include "pch.h"
#include "UDPSocket.h"
#include "ipHlpapi.h"
#include <WS2tcpip.h>

#pragma comment(lib, "ipHlpapi.lib")


// CUDPSocket
CUDPSocket::CUDPSocket()
{
	ZeroMemory(m_sendBuf, sizeof(m_sendBuf));
	ZeroMemory(m_recvBuf, sizeof(m_recvBuf));

	m_bEthernetInit		= TRUE;
	m_bUDPSendFail		= FALSE;
	m_recvSize			= 0;
	m_bIsContinueRecv	= FALSE;

	m_recvSocketAddr = new SOCKADDR_IN;
	m_recvSocketAddr->sin_family = AF_INET;
	m_recvSocketAddr->sin_port = htons(UDP_SOCKET_PORT);	
	m_recvSocketAddr->sin_addr.s_addr = htonl(INADDR_ANY);
}

CUDPSocket::~CUDPSocket()
{
	free(m_recvSocketAddr);
}

BOOL CUDPSocket::CreatSocket(UINT nSocketPort, int nSocketType)
{
	CString m_stError;

	ZeroMemory(m_sendBuf, sizeof(m_sendBuf)); // 버퍼초기화

	//소켓을 생성
	if (!Create(nSocketPort, nSocketType))
	{ // 소켓 생성이 실패하면
		m_stError.Format(_T("Socket Create Fail -> %d"), GetLastError());
		AfxMessageBox(m_stError);
		return FALSE;
	}

	// 브로드 캐스트를 원하면 아래 추가 /////////////////////////////////////////////
	int flag=TRUE;
	int len, rflag;

	if (!SetSockOpt(SO_BROADCAST, &flag, sizeof(int)))
	{
		m_stError.Format(_T("SetSockOpt failed to set SO_BROADCAST:% d"), GetLastError());
		AfxMessageBox (m_stError);

		return FALSE;
	}

	len = sizeof(rflag);
	if (!GetSockOpt(SO_BROADCAST, &rflag, &len) && rflag == 0)
	{
		m_stError.Format(_T("GetSockOpt failed to get SO_BROADCAST:% d"), GetLastError());
		AfxMessageBox (m_stError);

		return FALSE;
	}
	//////////////////////////////////////////////////////////////////////////////

	return TRUE;
}

BOOL CUDPSocket::CloseSocket()
{
	return TRUE;
}

void CUDPSocket::getLocalIPAddress()
{
	CString LocalIP1, LocalIP2;

	PIP_ADAPTER_INFO pAdapterInfo;
	DWORD dwRetVal = 0;
	BOOL m_bBreak = FALSE;

	pAdapterInfo = (IP_ADAPTER_INFO *)malloc(sizeof(IP_ADAPTER_INFO));
	unsigned long ulOutBufLen = sizeof(IP_ADAPTER_INFO);

	if(GetAdaptersInfo(pAdapterInfo, &ulOutBufLen) != ERROR_SUCCESS)
	{
		//GlobalFree(pAdapterInfo);
		pAdapterInfo = (IP_ADAPTER_INFO*)malloc(ulOutBufLen);
	}

	if((dwRetVal=GetAdaptersInfo(pAdapterInfo, &ulOutBufLen)) == NO_ERROR)
	{
		m_strLocalIP1.Format (_T("%S"), pAdapterInfo->IpAddressList.IpAddress.String);

		if(m_strLocalIP1==_T("0.0.0.0"))
		{
			if(pAdapterInfo->IpAddressList.Next != NULL)
			{
				m_strLocalIP2.Format (_T("%S"), pAdapterInfo->IpAddressList.Next->IpAddress.String);
			}
		}
		else
		{
			if(pAdapterInfo->IpAddressList.Next != NULL)
			{
				m_strLocalIP2.Format (_T("%S"), pAdapterInfo->IpAddressList.Next->IpAddress.String);
			}
		}
	}
}

void CUDPSocket::getLocalGateWay()
{
	int gateway;
    DWORD dwStatus;
    IP_ADAPTER_INFO *pAdapterInfo = NULL;
    IP_ADAPTER_INFO *pOriginalPtr = NULL;
    ULONG ulSizeAdapterInfo = 0;
 
    dwStatus = GetAdaptersInfo(pAdapterInfo,&ulSizeAdapterInfo);
    //***********************************************************************
    //버퍼 오버 플로우 일때 ulSizeAdapterInfo 크기로 메모리를 할당하고 
    //다시 GetAdaptersInfo를 호출한다.
    if( dwStatus == ERROR_BUFFER_OVERFLOW)
    {
        if(!(pAdapterInfo = (PIP_ADAPTER_INFO)malloc(ulSizeAdapterInfo)))
        {
            AfxMessageBox(_T("Get Gateway Address Error"));
        }
 
        dwStatus = GetAdaptersInfo(pAdapterInfo,&ulSizeAdapterInfo);
    }
 
    char Gageway_Addr[16];
 
	if(pAdapterInfo!=NULL)
	{
		for(int x=0; x<16; x++)
			Gageway_Addr[x] = pAdapterInfo->GatewayList.IpAddress.String[x];
	 
		//gateway = inet_addr(pAdapterInfo->GatewayList.IpAddress.String);
		inet_pton(AF_INET, pAdapterInfo->GatewayList.IpAddress.String, &gateway);
		m_strGateway.Format(_T("%d.%d.%d.%d"), (gateway&0xFF), (gateway>>8&0xFF), (gateway>>16&0xFF), (gateway>>24&0xFF));
		free(pAdapterInfo);
	}
}

// CUDPSocket 멤버 함수
void CUDPSocket::parseReceivePacket(CString recvIP, int nRead, char* buf)
{
	int source, dest;
	CString recvpacket=_T("");

	// Receive Buff의 마지막은 NULL을 넣어준다.		
	buf[nRead] = NULL;

	if(m_bIsContinueRecv==FALSE)
	{
		sscanf_s(&buf[PACKET_PT_SOURCE],"%02X", &source);
		sscanf_s(&buf[PACKET_PT_DEST],	"%02X", &dest);
		sscanf_s(&buf[PACKET_PT_CMD],	"%02X", &m_recvCommand);
		sscanf_s(&buf[PACKET_PT_LEN],	"%04X", &m_recvTotalLength);

		// 목적지가 PC가 아니면 Return한다.
		if(dest != TARGET_PC)	return;

		m_recvSize += nRead;
		if(m_recvSize >= (m_recvTotalLength+16))
		{
			memcpy(m_recvBuf, buf, m_recvSize);
			m_recvBuf[m_recvSize] = NULL;
		}
		else
		{
			memcpy(m_recvBuf, buf, m_recvSize);
			m_bIsContinueRecv = TRUE;

			// 더 받을 Data가 있다면 Message Send하지 않고Retrun한다.
			return;
		}
	}
	else
	{
		memcpy(&m_recvBuf[m_recvSize], buf, nRead);
		m_recvSize += nRead;
		if(m_recvSize < (m_recvTotalLength+16))
		{
			// 더 받을 Data가 있다면 Message Send하지 않고Retrun한다.
			return;
		}
	}

	// CString IP Address를 char로 convert한다.
	char szipa[100] = {0,};
	int nMultiByteLen = WideCharToMultiByte(CP_ACP, 0, recvIP.GetBuffer(0), -1, NULL, 0, NULL,NULL);
	WideCharToMultiByte(CP_ACP, 0, recvIP.GetBuffer(0), -1, szipa, nMultiByteLen, NULL, NULL);

	// IPAddress와 Message 같이 전달한다.
	char szmsg[1024*8] = {0,};
	sprintf_s(szmsg, "%s#%s", szipa, m_recvBuf);
	if(recvIP == IP_ADDRESS_DIO)
		AfxGetApp()->GetMainWnd()->SendMessage(WM_ETH_UDP_RECEIVE_DIO, (WPARAM)szmsg, NULL);
	else
		AfxGetApp()->GetMainWnd()->SendMessage(WM_ETH_UDP_RECEIVE, (WPARAM)szmsg, NULL);

	// 변수를 초기화 한다.
	m_bIsContinueRecv = FALSE;
	m_recvSize = 0;
	m_recvCommand = 0;
	m_recvTotalLength = 0;
	ZeroMemory(m_recvBuf, sizeof(m_recvBuf));

}

void CUDPSocket::OnReceive(int nErrorCode) 
{
	// TODO: Add your specialized code here and/or call the base class
	//int dd=0;
	char buf[PACKET_SIZE]={0,};
	CString recvpacket=_T("");
	CString recvIP=_T("");
	int nRead, addrlen=sizeof(SOCKADDR);

	nRead=ReceiveFrom(buf, PACKET_SIZE, (SOCKADDR*)m_recvSocketAddr, &addrlen, 0);
	recvIP.Format(_T("%d.%d.%d.%d"),
					m_recvSocketAddr->sin_addr.S_un.S_un_b.s_b1,
					m_recvSocketAddr->sin_addr.S_un.S_un_b.s_b2,
					m_recvSocketAddr->sin_addr.S_un.S_un_b.s_b3,
					m_recvSocketAddr->sin_addr.S_un.S_un_b.s_b4
				);

	// Local Host IP주소에서 전송된 Packet은 버린다.
	if((recvIP==m_strLocalIP1) || (recvIP==m_strLocalIP2) || (recvIP==LOCAL_HOST_IP) || (recvIP==_T("192.168.10.0")))
		return;

	switch(nRead)
	{
		case 0:
		{
			//Close();
			AfxMessageBox(_T("Unkwon pacheck received"), MB_ICONERROR);
			break;
		}
		case SOCKET_ERROR:
		{
			if (GetLastError() != WSAEWOULDBLOCK) // WSAEWOULDBLOCK이되면 걍 나간다
			{
				if(GetLastError() != WSAEMSGSIZE)
				{   // 일반적인 받기 에러처리
					CString stErr;
					stErr.Format(_T("받기에러, LastError() Code => %d"), GetLastError());
					recvpacket.Format(_T("<<SOCKET RECEVIE FAIL>>   %s#%s"), recvIP, stErr);
				}
				else
				{
					recvpacket.Format(_T("<<SOCKET RECEVIE FAIL>>   %s#%s"), recvIP, _T("데이터그램 사이즈가 너무커서 잘렸습니다."));
				}
				AfxMessageBox(recvpacket, MB_ICONERROR);
			}
			break;
		}
		default: // 에러가 아니면
		{
			if (nRead != SOCKET_ERROR && nRead != 0)
			{
				parseReceivePacket(recvIP, nRead, buf);
			}
		}
	}

	CAsyncSocket::OnReceive(nErrorCode);	
}

BOOL CUDPSocket::SendToUDP(CString remoteIP, int length, char* m_sendPacket)
{
	int m_nBytesSent=0;
	int m_nBytesBufSize;

	m_nBytesBufSize = length;

	while(m_nBytesSent < m_nBytesBufSize)
	{
		int dwBytes;
		int sendlen;

		if((m_nBytesBufSize-m_nBytesSent) < 1450)
			sendlen = m_nBytesBufSize-m_nBytesSent;
		else
			sendlen = 1450;

		// SendTo(보낼메시지, 메시지사이즈, 받을포트, 받을IP)
		dwBytes = SendTo((m_sendPacket+m_nBytesSent), sendlen, UDP_SOCKET_PORT, remoteIP);
		if(dwBytes == SOCKET_ERROR)
		{
			if (GetLastError() ==  WSAEWOULDBLOCK)
			{
				ProcessMessage();
				continue;;
			}
			else
			{
				Close();
				return FALSE;
			}
		}
		else // 에러가 안나면
		{
			m_nBytesSent += dwBytes;
		}
	}

	ZeroMemory(m_sendBuf, sizeof(m_sendBuf));;

	return TRUE;
}
