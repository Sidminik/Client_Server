#pragma once

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <thread>
#include <iostream> 
#include <winsock2.h> 
#include <string>
#pragma comment(lib, "WS2_32.lib")

using namespace std;

class Client
{
    SOCKET server;
    bool connectionFlag;
public:
    Client(const char* inetAddress);
    ~Client();

    void clientReceive();           // ��������� ���������
    void clientSend();              // ����������� ���������
    void cleanup();                 // ������� ��������
};

Client::Client(const char* inetAddress)
{
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 7), &wsaData);

    server = socket(AF_INET, SOCK_STREAM, 0);
    if (server == INVALID_SOCKET)
    {
        cerr << "���� �������� ������ ���������� � �������: " << WSAGetLastError() << endl;
        exit(EXIT_FAILURE);
    }

    SOCKADDR_IN addr;
    addr.sin_addr.s_addr = inet_addr(inetAddress);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(15000); 

    if (connect(server, (SOCKADDR*)&addr, sizeof(addr)) == SOCKET_ERROR)
    {
        cerr << "���������� � �������� ����������� � �������: " << WSAGetLastError() << endl;
        cleanup();
        exit(EXIT_FAILURE);
    }
    else
    {
        connectionFlag = true;
        cout << "����������� ���������� � ��������!" << endl;
        cout << "��� ������� ���������� ������� \"/exit\"" << endl;
    }
};

Client::~Client()
{
    cleanup();
    cout << "����������" << endl;
}

void Client::clientReceive()
{
    while (true)
    {
        char buffer[1024];
        int recvCount = 0;
        string strBuffer;

        int recvRead = recv(server, buffer, sizeof(buffer), MSG_PEEK);

        if (recvRead == 0)
        {
            cout << "���������� ���� ������� ��������" << endl;
            cleanup();
            return;
        }
        else if (recvRead < 0)
        {
            if (WSAGetLastError() == 10053)
            {
                cout << "���������� ���� ������� ��������" << endl;
            }
            else if (WSAGetLastError() == 10054)
            {
                cout << "��������� ������ ���� ������������� ������� ������������ ����������" << endl;
            }
            else
            {
                cerr << "��������� ����������� � �������: " << WSAGetLastError() << endl;
            }
            cout << "����� ��������� ���������� �� �������" << endl;
            cleanup();
            return;
        }
        else
        {
            while ((recvCount = recv(server, buffer, sizeof(buffer), 0)) > 0) {
                strBuffer.append(buffer, recvCount);
                if (recvCount < sizeof(buffer))
                {
                    if (strBuffer == "/exit\n")
                    {
                        connectionFlag = false;
                        cout << "����� ��������� ���������� �� �������" << endl;
                        return;
                    }
                    cout << strBuffer << endl;
                    strBuffer.clear();
                }
            }
        }
    }
}

void Client::clientSend()
{
    string stringBuffer;
    while (true)
    {
        getline(cin, stringBuffer);
        stringBuffer += "\n";

        if (send(server, stringBuffer.c_str(), stringBuffer.length(), 0) == SOCKET_ERROR)
        {
            cerr << "������ �������� ������: " << WSAGetLastError() << endl;
            return;
        }

        if (stringBuffer == "/exit\n")
        {
            Sleep(3000);            // �������� ������� �� ������� ������� �� ���������� clientReceive 
            if (connectionFlag)
            {
                cleanup();          // �������������� ������������ �������� ��� ��������� ������ �� �������
            }
            cout << "����� �������� ���������� �� �������" << endl;
            return;
        }
    }
}

void Client::cleanup() {
    if (server != INVALID_SOCKET) {
        closesocket(server);        // �������� ������
        server = INVALID_SOCKET;
    }
    WSACleanup();                   // ������������ �������� WinSock
}
