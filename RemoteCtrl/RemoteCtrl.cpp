// RemoteCtrl.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include "framework.h"
#include "RemoteCtrl.h"
#include "ServerSocket.h"
#include <direct.h>
#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// The one and only application object

CWinApp theApp;

using namespace std;
void Dump(BYTE* pData, size_t nSize) {
    std::string strOut;
    for (size_t i = 0; i < nSize; ++i) {
        char buf[8] = "";
        if (i > 0 && i % 16 == 0) strOut += "\n";
        snprintf(buf, sizeof(buf), "%02X ", pData[i] & 0xFF);
        strOut += buf;
    }
    strOut += "\n";
    OutputDebugStringA(strOut.c_str());
}

int MakeDriverInfo() {//1:A   2:B   3:C   AB������
    string result = "";
    for (int i = 1; i <= 26; ++i) {
        if (_chdrive(i) == 0) {
            if (result.size() > 0) result += ',';
            result += i + 'A' - 1;
        }
    }
    
    CPacket pack(1, (BYTE*)result.c_str(), result.size());
    Dump((BYTE*)pack.Data(), pack.Size());//pack.nLength + 6);
    //CServerSocket::getInstance()->Send(pack); �����ӣ���ע��
    return 0;
}
int main()
{
    int nRetCode = 0;

    HMODULE hModule = ::GetModuleHandle(nullptr);

    if (hModule != nullptr)
    {
        // initialize MFC and print and error on failure
        if (!AfxWinInit(hModule, nullptr, ::GetCommandLine(), 0))
        {
            // TODO: code your application's behavior here.
            wprintf(L"Fatal Error: MFC initialization failed\n");
            nRetCode = 1;
            
        }
        else
        {
            /* {
                // TODO: code your application's behavior here.
                //У��
                //server; 
                CServerSocket* pserver = CServerSocket::getInstance();//��̬����ֱ�ӵ�
                int count = 0;

                if (pserver->InitSocket() == false) {
                    MessageBox(NULL, _T("�����ʼ���쳣��δ�ܳɹ���ʼ������������״̬"), _T("�����ʼ��ʧ��"), MB_OK | MB_ICONERROR);
                    exit(0);
                }
                while (CServerSocket::getInstance() != NULL) {

                    if (pserver->AcceptClient() == false) {
                        if (count >= 3) {
                            exit(0);
                        }
                        MessageBox(NULL, _T("�޷����������û����Զ�����"), _T("�����û�ʧ��"), MB_OK | MB_ICONERROR);
                        count++;
                    }

                    int ret = pserver->DealCommand();
                    //TODO:��������
                }

                SOCKET Server_Socket = socket(PF_INET, SOCK_STREAM, 0);
                sockaddr_in Server_Address, Client_Addr;
                memset(&Server_Address, 0, sizeof(Server_Address));
                Server_Address.sin_family = AF_INET;
                Server_Address.sin_addr.s_addr = INADDR_ANY;//Server_Address.sin_addr.S_un.S_addr;��������ip
                Server_Address.sin_port = htons(5433);
                //��
                bind(Server_Socket, (sockaddr*)&Server_Address, sizeof(Server_Address));
                //ǰ�ڴ���
                listen(Server_Socket, 1);//ֻ��һ���˿���,ֻ����һ����
                char* buffer = new char[4096];
                memset(buffer, 0, 4096);
                int cli_size = sizeof(Client_Addr);
                //SOCKET client = accept(Server_Socket, (sockaddr*)&Client_Addr, &cli_size);
                //recv(Server_Socket, buffer, sizeof(buffer), 0); //Win��read  = recv
                //send(Server_Socket, buffer, sizeof(buffer), 0); //win��write = send
                closesocket(Server_Socket); //��startup�ɶԳ��֣���������//��̬�����ĳ�ʼ�����״ε���ʱ���������ǳ�������ʱ�����١������ȫ�־�̬��������main����ǰ���죬main������������
            }*/
            
            int nCmd = 1;
            switch (nCmd)
            {
            case 1://�鿴���̷���
                MakeDriverInfo();
                break;
            default:
                break;
            }
            

        }
    }
    else
    {
        // TODO: change error code to suit your needs
        wprintf(L"Fatal Error: GetModuleHandle failed\n");
        nRetCode = 1;
    }

    return nRetCode;
}
