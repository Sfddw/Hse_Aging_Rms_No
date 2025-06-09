#pragma once

// CUDPSocket ��� ����Դϴ�.
#define WM_ETH_UDP_RECEIVE		(WM_USER+200)
#define WM_ETH_UDP_RECEIVE_DIO	(WM_USER+201)

class CUDPSocket : public CAsyncSocket
{
public:
	CUDPSocket();
	virtual ~CUDPSocket();

/////////////////////////////////////////////////////////////////////////////
// ����� ���� Function/Variable
/////////////////////////////////////////////////////////////////////////////
public:
	BOOL m_bEthernetInit;
	BOOL m_bUDPSendFail;
	char m_sendBuf[PACKET_SIZE];	// ���� �����͸� ����

	char m_recvBuf[PACKET_SIZE*4];	// ���� �����͸� ����
	int  m_recvSize;				// ���� �������� Size
	int  m_recvCommand;				// ���� Command
	int  m_recvTotalLength;			// ��ü ���� Packet�� Length
	BOOL m_bIsContinueRecv;			// ���ӵ� Packet���� ���ο� Packet���� �����ϴ� Flag

	CString m_strLocalIP1, m_strLocalIP2, m_strGateway;

	SOCKADDR_IN *m_recvSocketAddr;	//�޴� �ּ������� �������

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


