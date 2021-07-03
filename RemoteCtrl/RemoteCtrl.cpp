// RemoteCtrl.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include "framework.h"
#include "RemoteCtrl.h"
#include "ServerSocket.h"
#include <direct.h>
#include <io.h>
#include <list>
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

int MakeDriverInfo() {//1:A   2:B   3:C   AB是软盘
    string result = "";
    for (int i = 1; i <= 26; ++i) {
        if (_chdrive(i) == 0) {
            if (result.size() > 0) result += ',';
            result += i + 'A' - 1;
        }
    }
    
    CPacket pack(1, (BYTE*)result.c_str(), result.size());
    Dump((BYTE*)pack.Data(), pack.Size());//pack.nLength + 6);
    //CServerSocket::getInstance()->Send(pack); 无连接，先注释
    return 0;
}


typedef struct file_info {
    file_info() {
        isInvalid = 0;
        isDirectory = 1;
        hasNext = TRUE;
        memset(szFileName, 0, sizeof(szFileName));
    }//结构体，不用就没了，不用搞析构函数，也不用分配内存空间
    BOOL isInvalid;//是否是有效目录 （快捷键无效）
    BOOL isDirectory;//是否为目录
    BOOL hasNext;
    char szFileName[256];
}FILEINFO, * PFILEINFO;

int MakeDirectoryInfo() {
    std::string strPath;
    //std::list<FILEINFO> lstFileInfos;
    if (CServerSocket::getInstance()->GetFilePath(strPath) == false) {
        OutputDebugString(_T("当前命令非获取文件命令，解析错误"));
        return -1;
    }
    if (_chdir(strPath.c_str())!= 0) {//切换失败，maybe没有权限
        FILEINFO finfo;
        finfo.isInvalid = 1;
        finfo.isDirectory = TRUE;
        finfo.hasNext = FALSE;
        memcpy(finfo.szFileName, strPath.c_str(), strPath.size());
        //lstFileInfos.push_back(finfo); 

        CPacket pack(2, (BYTE*)&finfo, sizeof(finfo));
        CServerSocket::getInstance()->Send(pack);
        OutputDebugString(_T("没有权限访问目录"));
        return -2;
    }
    _finddata_t fdata;
    int hfind = 0;
    if ((hfind = _findfirst("*", &fdata)) == -1) {//io.h，遍历文件，可以写通配符     结构体 ;查找失败，返回错误
        OutputDebugString(_T("没有找到任何文件"));
        return -3;
    }
    do {
        FILEINFO finfo;
        //finfo.isInvalid = 0; 默认就是0
        finfo.isDirectory = (fdata.attrib & _A_SUBDIR) != 0; //判断是文件夹吗？
        memcpy(finfo.szFileName, fdata.name, strlen(fdata.name));
        //lstFileInfos.push_back(finfo);
        CPacket pack(2, (BYTE*)&finfo, sizeof(finfo));
        if (CServerSocket::getInstance()->Send(pack)) {
            OutputDebugString(_T("发送失败"));
            //return -4;
        }
    } while (!_findnext(hfind, &fdata));
    //发送信息到控制端    注：如果文件数量太多【日志、临时文件夹】，上面的while会遍历很久，连接可能断开。
    FILEINFO finfo;
    finfo.hasNext = FALSE;
    CPacket pack(2, (BYTE*)&finfo, sizeof(finfo));
    if (CServerSocket::getInstance()->Send(pack)) {
        OutputDebugString(_T("发送失败"));
        //return -4;
    }
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
                char* buffer = new char[4096];
                memset(buffer, 0, 4096);
                int cli_size = sizeof(Client_Addr);
                //SOCKET client = accept(Server_Socket, (sockaddr*)&Client_Addr, &cli_size);
                //recv(Server_Socket, buffer, sizeof(buffer), 0); //Win的read  = recv
                //send(Server_Socket, buffer, sizeof(buffer), 0); //win的write = send
                closesocket(Server_Socket); //与startup成对出现，清理网络//静态变量的初始化在首次调用时，销毁则是程序销毁时候销毁。如果是全局静态变量，则main函数前构造，main结束后析构。
            }*/
            
            int nCmd = 1;
            switch (nCmd)
            {
            case 1://查看磁盘分区
                MakeDriverInfo();
                break;
            case 2://查看指定目录下的文件
                MakeDirectoryInfo();
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
