#pragma once

// CUDPSocket 명령 대상입니다.
#define WM_ETH_UDP_RECEIVE		(WM_USER+200)
#define WM_ETH_UDP_RECEIVE_DIO	(WM_USER+201)

class CUDPSocket : public CAsyncSocket
{
public:
	CUDPSocket();
	virtual ~CUDPSocket();

/////////////////////////////////////////////////////////////////////////////
// 사용자 정의 Function/Variable
/////////////////////////////////////////////////////////////////////////////
public:
	BOOL m_bEthernetInit;
	BOOL m_bUDPSendFail;
	char m_sendBuf[PACKET_SIZE];	// 보낼 데이터를 저장

	char m_recvBuf[PACKET_SIZE*4];	// 받은 데이터를 저장
	int  m_recvSize;				// 받은 데이터의 Size
	int  m_recvCommand;				// 받은 Command
	int  m_recvTotalLength;			// 전체 받을 Packet의 Length
	BOOL m_bIsContinueRecv;			// 연속된 Packet인지 새로운 Packet인지 구분하는 Flag

	CString m_strLocalIP1, m_strLocalIP2, m_strGateway;

	SOCKADDR_IN *m_recvSocketAddr;	//받는 주소정보를 담고있음

	CString _hex_to_string(int size, char* hexStr);

	void getLocalIPAddress();
	void getLocalGateWay();

	void parseReceivePacket(CString recvIP, int nRead, char* buf);

	BOOL CreatSocket(UINT nSocketPort, int nSocketType);
	BOOL CloseSocket();
	BOOL SendToUDP(CString remoteIP, int length, char* m_sendPacket);

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////


	// Overrides
public:
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CUDPSocket)
public:
	virtual void OnReceive(int nErrorCode);
	//}}AFX_VIRTUAL

	// Generated message map functions
	//{{AFX_MSG(CUDPSocket)
	// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG

	// Implementation
protected:
};


