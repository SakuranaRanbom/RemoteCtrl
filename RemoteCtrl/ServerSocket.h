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
	 CPacket(const BYTE* pData, size_t& nSize) { //解包的重构
		 size_t i = 0;
		 for (; i < nSize; ++i) {
			 if (*(WORD*)(pData + i) == 0xFEFF) {
				 sHead = *(WORD*)(pData + i);
				 i += 2;//如果数据表只有包头且正确，那么i+=2之后其大于nSize，仍出来
				 break;
			 }
		 }
		 if (i + 8 >= nSize) { //包解析完了，直接失败  4+ 2 + 2 + data
			 nSize = 0;//用掉了0字节
			 return;
		 }
		 nLength = *(DWORD*)(pData + i); i += 4;
		 if (nLength + i > nSize) {			//包只收了一半
			 nSize = 0;
			 return;
		 }
		 sCmd = *(WORD*)(pData + i); i += 2;
		 if (nLength > 4) {
			 strData.resize(nLength - 2 - 2); //nlength  = sCmd + strData + sSum 
			 memcpy((void*)strData.c_str(), pData + i, nLength - 4);
			 i += nLength - 4; //始终让i保持在最新位
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
	WORD sHead;//包头 -固定位，用FE FF来代替
	DWORD nLength;//包长度（从控制命令开始，到和校验结束）
	WORD sCmd;//控制命令 （为了对齐，用WORD来，256本来就够了
	std::string strData;//包数据
	WORD sSum;//和校验
	std::string strOut;

};
#pragma pack(pop)


class CServerSocket
{
public:
	static CServerSocket* getInstance() {  //静态函数 ：从外面访问Server;静态无this指针，无法直接访问成员变量->成员变量变静态
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
		Server_Address.sin_addr.s_addr = INADDR_ANY;//Server_Address.sin_addr.S_un.S_addr;监听所有ip
		Server_Address.sin_port = htons(5433);
		//绑定
		if (bind(m_socket, (sockaddr*)&Server_Address, sizeof(Server_Address)) == -1) return false;
		//前期处理
		if (listen(m_socket, 1) == -1) return false;//只有一个人控制,只监听一个人
		
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
		//recv(Server_Socket, buffer, sizeof(buffer), 0); //Win的read  = recv
		//send(Server_Socket, buffer, sizeof(buffer), 0); //win的write = send
		
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
			//TODO: 处理命令
			index += len;
			len = index;
			m_packet = CPacket((BYTE*)buffer, len);
			if (len > 0) { //解析成功了
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
		if (m_packet.sCmd == 2 || m_packet.sCmd == 3) { //查看文件  或  运行文件 指令

			strPath = m_packet.strData;
			return true;

		}
		return false;

	}
private:
	SOCKET m_client = INVALID_SOCKET;//初始赋值无效套接字
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

		//套接字初始化
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
