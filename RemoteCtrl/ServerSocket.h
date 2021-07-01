#pragma once
#include "pch.h"
#include "framework.h"

class CServerSocket
{
public:
	static CServerSocket* getInstance() {  //��̬���� �����������Server;��̬��thisָ�룬�޷�ֱ�ӷ��ʳ�Ա����->��Ա�����侲̬
		if (m_instance == NULL) {
			m_instance = new CServerSocket();
		}
		
		return m_instance;
	}

	bool InitSocket() {
		/*SOCKET Server_Socket = socket(PF_INET, SOCK_STREAM, 0);
		if (Server_Socket == -1) return false;*/
		if (m_socket == -1) return false;
		sockaddr_in Server_Address;
		memset(&Server_Address, 0, sizeof(Server_Address));
		Server_Address.sin_family = AF_INET;
		Server_Address.sin_addr.s_addr = INADDR_ANY;//Server_Address.sin_addr.S_un.S_addr;��������ip
		Server_Address.sin_port = htons(5433);
		//��
		if (bind(m_socket, (sockaddr*)&Server_Address, sizeof(Server_Address)) == -1) return false;
		//ǰ�ڴ���
		if (listen(m_socket, 1) == -1) return false;//ֻ��һ���˿���,ֻ����һ����
		
		return true;
	}

	bool AcceptClient() {
		char buffer[1024];
		sockaddr_in  Client_Addr;
		int cli_size = sizeof(m_socket);
		m_client = accept(m_socket, (sockaddr*)&Client_Addr, &cli_size);
		if (m_client == -1) {
			return false;
		}
		return true;
		//recv(Server_Socket, buffer, sizeof(buffer), 0); //Win��read  = recv
		//send(Server_Socket, buffer, sizeof(buffer), 0); //win��write = send
		
	}

	int DealCommand() {
		char buffer[1024];
		if (m_client == -1) return -1;
		while (true) {

			int len = recv(m_client, buffer, sizeof(buffer), 0);
			if (len <= 0) {
				return -1;
			}
			//TODO: ��������

		}
	}

	bool Send(const char* pData, size_t nSize) {
		if (m_client == -1) return false;
		return send(m_client, pData, nSize, 0) > 0;
	}
private:
	SOCKET m_client;
	SOCKET m_socket;
	CServerSocket& operator=(const CServerSocket& ss) {

	} 
	CServerSocket(const CServerSocket&) {

	}
	CServerSocket(){
		if (InitSockEnv() == FALSE) {
			MessageBox(NULL, _T("Cannot init socket environment"), _T("Init ERROR!!!"), MB_OK | MB_ICONERROR); 
			exit(0);
		}
		m_socket = socket(PF_INET, SOCK_STREAM, 0);
		
	}
	~CServerSocket(){
		closesocket(m_socket);
		WSACleanup();
	}

	BOOL InitSockEnv() {

		//�׽��ֳ�ʼ��
		WSADATA data;
		if (WSAStartup(MAKEWORD(1, 1), &data) != 0) {
			return FALSE;
		}
		return TRUE;
		
	}

	static void releaseInstance() {
		if (m_instance != NULL) {
			CServerSocket* tmp = m_instance;
			m_instance = NULL;
			delete tmp;
		}
	}

	static CServerSocket* m_instance;

	class CHelper {
	public:
		CHelper() {
			CServerSocket::getInstance();
		}

		~CHelper() {
			CServerSocket::releaseInstance();
		}
	};

	static CHelper m_helper;
};

extern CServerSocket server;
