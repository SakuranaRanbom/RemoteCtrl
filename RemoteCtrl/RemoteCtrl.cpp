// RemoteCtrl.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include "framework.h"
#include "RemoteCtrl.h"
#include "ServerSocket.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// The one and only application object

CWinApp theApp;

using namespace std;

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

            // TODO: code your application's behavior here.
            //校验
            //server; 
            CServerSocket* pserver = CServerSocket::getInstance();//静态方法直接调
            int count = 0;

            if (pserver->InitSocket() == false) {
                MessageBox(NULL, _T("网络初始化异常，未能成功初始化，请检测网络状态"), _T("网络初始化失败"), MB_OK | MB_ICONERROR);
                exit(0);
            }
            while (CServerSocket::getInstance() != NULL) {

                if (pserver->AcceptClient() == false) {
                    if (count >= 3) {
                        exit(0);
                    }
                    MessageBox(NULL, _T("无法正常接入用户，自动重试"), _T("接入用户失败"), MB_OK | MB_ICONERROR);
                    count++;
                }

                int ret = pserver->DealCommand();
                //TODO:处理命令
            }
        
            SOCKET Server_Socket = socket(PF_INET, SOCK_STREAM, 0);
            sockaddr_in Server_Address, Client_Addr;
            memset(&Server_Address, 0, sizeof(Server_Address));
            Server_Address.sin_family = AF_INET;
            Server_Address.sin_addr.s_addr = INADDR_ANY;//Server_Address.sin_addr.S_un.S_addr;监听所有ip
            Server_Address.sin_port = htons(5433);
            //绑定
            bind(Server_Socket, (sockaddr*)&Server_Address, sizeof(Server_Address));
            //前期处理
            listen(Server_Socket, 1);//只有一个人控制,只监听一个人
            char *buffer = new char[4096];
            memset(buffer, 0, 4096);
            int cli_size = sizeof(Client_Addr);
            //SOCKET client = accept(Server_Socket, (sockaddr*)&Client_Addr, &cli_size);
            //recv(Server_Socket, buffer, sizeof(buffer), 0); //Win的read  = recv
            //send(Server_Socket, buffer, sizeof(buffer), 0); //win的write = send
            closesocket(Server_Socket);

            //与startup成对出现，清理网络

            //静态变量的初始化在首次调用时，销毁则是程序销毁时候销毁。如果是全局静态变量，则main函数前构造，main结束后析构。

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
