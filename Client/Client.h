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

    void clientReceive();           // получение сообщений
    void clientSend();              // отправление сообщений
    void cleanup();                 // очистка ресурсов
};

Client::Client(const char* inetAddress)
{
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 7), &wsaData);

    server = socket(AF_INET, SOCK_STREAM, 0);
    if (server == INVALID_SOCKET)
    {
        cerr << "Сбой создания сокета завершился с ошибкой: " << WSAGetLastError() << endl;
        exit(EXIT_FAILURE);
    }

    SOCKADDR_IN addr;
    addr.sin_addr.s_addr = inet_addr(inetAddress);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(15000); 

    if (connect(server, (SOCKADDR*)&addr, sizeof(addr)) == SOCKET_ERROR)
    {
        cerr << "Соединение с сервером завершилось с ошибкой: " << WSAGetLastError() << endl;
        cleanup();
        exit(EXIT_FAILURE);
    }
    else
    {
        connectionFlag = true;
        cout << "Установлено соединение с сервером!" << endl;
        cout << "Для разрыва соединения введите \"/exit\"" << endl;
    }
};

Client::~Client()
{
    cleanup();
    cout << "Деструктор" << endl;
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
            cout << "Соединение было закрыто сервером" << endl;
            cleanup();
            return;
        }
        else if (recvRead < 0)
        {
            if (WSAGetLastError() == 10053)
            {
                cout << "Соединение было закрыто клиентом" << endl;
            }
            else if (WSAGetLastError() == 10054)
            {
                cout << "Удаленным хостом было принудительно закрыто существующее соединение" << endl;
            }
            else
            {
                cerr << "Получение завершилось с ошибкой: " << WSAGetLastError() << endl;
            }
            cout << "Поток получения отключился от сервера" << endl;
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
                        cout << "Поток получения отключился от сервера" << endl;
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
            cerr << "Ошибка отправки данных: " << WSAGetLastError() << endl;
            return;
        }

        if (stringBuffer == "/exit\n")
        {
            Sleep(3000);            // ожидание сигнала со стороны сервера на завершение clientReceive 
            if (connectionFlag)
            {
                cleanup();          // принудительное освобождение ресурсов при отсутсвии ответа от сервера
            }
            cout << "Поток отправки отключился от сервера" << endl;
            return;
        }
    }
}

void Client::cleanup() {
    if (server != INVALID_SOCKET) {
        closesocket(server);        // закрытие сокета
        server = INVALID_SOCKET;
    }
    WSACleanup();                   // освобождение ресурсов WinSock
}
