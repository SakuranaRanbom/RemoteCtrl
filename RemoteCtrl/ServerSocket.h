#pragma once
#include "pch.h"
#include "framework.h"

#pragma pack(push)
#pragma pack(1)
class CPacket {
public:
	 CPacket():sHead(0), nLength(0), sCmd(0), sSum(0) {}
	 CPacket(WORD nCmd, const BYTE* pData, size_t nSize) {
		 sHead = 0xFEFF;
		 nLength = nSize + 2 + 2;
		 sCmd = nCmd;
		 if (nSize > 0) {
			 strData.resize(nSize);
			 memcpy((void*)strData.c_str(), pData, nSize);
		 }
		 sSum = 0;
		 for (int j = 0; j < strData.size(); ++j) {

			 sSum += BYTE(strData[j]) & 0xFF;
		 }
	 }
	 CPacket(const CPacket& pack) {
		 sHead = pack.sHead;
		 nLength = pack.nLength;
		 sCmd = pack.sCmd;
		 strData = pack.strData;
		 sSum = pack.sSum;
	 }
	 CPacket(const BYTE* pData, size_t& nSize) { //������ع�
		 size_t i = 0;
		 for (; i < nSize; ++i) {
			 if (*(WORD*)(pData + i) == 0xFEFF) {
				 sHead = *(WORD*)(pData + i);
				 i += 2;//������ݱ�ֻ�а�ͷ����ȷ����ôi+=2֮�������nSize���Գ���
				 break;
			 }
		 }
		 if (i + 8 >= nSize) { //���������ˣ�ֱ��ʧ��  4+ 2 + 2 + data
			 nSize = 0;//�õ���0�ֽ�
			 return;
		 }
		 nLength = *(DWORD*)(pData + i); i += 4;
		 if (nLength + i > nSize) {			//��ֻ����һ��
			 nSize = 0;
			 return;
		 }
		 sCmd = *(WORD*)(pData + i); i += 2;
		 if (nLength > 4) {
			 strData.resize(nLength - 2 - 2); //nlength  = sCmd + strData + sSum 
			 memcpy((void*)strData.c_str(), pData + i, nLength - 4);
			 i += nLength - 4; //ʼ����i����������λ
		 }
		 sSum = *(WORD*)(pData + i); i += 2;
		 WORD sum = 0;
		 for (int j = 0; j < strData.size(); ++j) {

			 sum += BYTE(strData[j]) & 0xFF;
		 }
		 if (sum == sSum) {
			 nSize = i; // head:2  length:4 data:nLength
			 return;
		 }
		 else {
			 nSize = 0;
		 }
	 }
	 CPacket& operator=(const CPacket& pack) {
		 if (this != &pack) {
			 sHead = pack.sHead;
			 nLength = pack.nLength;
			 sCmd = pack.sCmd;
			 strData = pack.strData;
			 sSum = pack.sSum;
		 }
		 return *this;
	 }
	 ~CPacket() {}

	 int Size() {
		 return nLength + 6;
	 }
	 const char* Data() {
		 strOut.resize(nLength + 6);
		 BYTE* pData = (BYTE*)strOut.c_str();
		 *(WORD*)pData = sHead; pData += 2;
		 *(DWORD*)pData = nLength; pData += 4;
		 *(WORD*)pData = sCmd; pData += 2;
		 memcpy(pData, strData.c_str(), strData.size()); pData += strData.size();
		 *(WORD*)pData = sSum;
		 return strOut.c_str();
	 }

public:
	WORD sHead;//��ͷ -�̶�λ����FE FF������
	DWORD nLength;//�����ȣ��ӿ������ʼ������У�������
	WORD sCmd;//�������� ��Ϊ�˶��룬��WORD����256�����͹���
	std::string strData;//������
	WORD sSum;//��У��
	std::string strOut;

};
#pragma pack(pop)


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
#define BUFFER_SIZE 4096
	int DealCommand() {
		char* buffer = new char[BUFFER_SIZE];
		memset(buffer, 0, sizeof(buffer));
		size_t index = 0;
		if (m_client == -1) return -1;
		while (true) {

			size_t len = recv(m_client, buffer, sizeof(buffer), 0);
			if (len <= 0) {
				return -1;
			}
			//TODO: ��������
			index += len;
			len = index;
			m_packet = CPacket((BYTE*)buffer, len);
			if (len > 0) { //�����ɹ���
				memmove(buffer, buffer + len, BUFFER_SIZE - len);
				index -= len;
				return m_packet.sCmd;
			}
		}
	}

	bool Send(const char* pData, size_t nSize) {
		if (m_client == -1) return false;
		return send(m_client, pData, nSize, 0) > 0;
	}

	bool Send(const CPacket& pack) {

		if (m_client == -1) return false;
		return send(m_client, (const char*)&pack, pack.nLength+6, 0) > 0;

	}

	bool GetFilePath(std::string& strPath) {
		if (m_packet.sCmd == 2 || m_packet.sCmd == 3) { //�鿴�ļ�  ��  �����ļ� ָ��

			strPath = m_packet.strData;
			return true;

		}
		return false;

	}
private:
	SOCKET m_client = INVALID_SOCKET;//��ʼ��ֵ��Ч�׽���
	SOCKET m_socket = INVALID_SOCKET;
	CPacket m_packet;
	CServerSocket& operator=(const CServerSocket& ss) {

	} 
	CServerSocket(const CServerSocket& ss) {
		m_socket = ss.m_socket;
		m_client = ss.m_client;
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
