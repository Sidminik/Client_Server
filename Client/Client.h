#pragma once

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <thread>
#include <iostream> 
#include <winsock2.h> 
#include <string>
#include <vector>
#pragma comment(lib, "WS2_32.lib")

class Client
{
    SOCKET server;
    bool connectionFlag;
public:
    Client(const char* inetAddress);
    ~Client();

    void clientReceive();               // получение сообщений
    void clientSend();                  // отправление сообщений
    void cleanup();                     // очистка ресурсов
};

Client::Client(const char* inetAddress)
{
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 7), &wsaData);

    server = socket(AF_INET, SOCK_STREAM, 0);
    if (server == INVALID_SOCKET)
    {
        std::cerr << "Сбой создания сокета завершился с ошибкой: " << WSAGetLastError() << std::endl;
        exit(EXIT_FAILURE);
    }

    SOCKADDR_IN addr;
    addr.sin_addr.s_addr = inet_addr(inetAddress);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(15000);

    if (connect(server, (SOCKADDR*)&addr, sizeof(addr)) == SOCKET_ERROR)
    {
        std::cerr << "Соединение с сервером завершилось с ошибкой: " << WSAGetLastError() << std::endl;
        cleanup();
        exit(EXIT_FAILURE);
    }
    else
    {
        connectionFlag = true;
        std::cout << "Установлено соединение с сервером!" << std::endl;
        std::cout << "Для разрыва соединения введите \"/exit\"" << std::endl;
    }
};

Client::~Client()
{
    cleanup();
    std::cout << "Деструктор" << std::endl;
}

void Client::clientReceive()
{
    std::vector<char> buffer(1024);

    while (true)
    {
        int recvRead = recv(server, buffer.data(), sizeof(buffer), 0);
        if (recvRead == SOCKET_ERROR)
        {
            int errorCode = WSAGetLastError();
            if (errorCode == 10053)
            {
                std::cout << "Соединение было закрыто клиентом" << std::endl;
            }
            else if (errorCode == 10054)
            {
                std::cout << "Удаленным хостом было принудительно закрыто существующее соединение" << std::endl;
            }
            else
            {
                std::cout << "Ошибка получения данных: " << errorCode << std::endl;
            }
            cleanup();
            return;
        }
        else if (recvRead == 0)
        {
            std::cout << "Соединение было закрыто сервером" << std::endl;
            cleanup();
            return;
        }
        else
        {
            std::cout.write(buffer.data(), recvRead);
            std::cout.flush();
        }
    }
}

void Client::clientSend()
{
    std::string stringBuffer;
    while (true)
    {
        std::getline(std::cin, stringBuffer);
        stringBuffer += "\n";

        if (send(server, stringBuffer.c_str(), (int)stringBuffer.length(), 0) == SOCKET_ERROR)
        {
            std::cerr << "Ошибка отправки данных: " << WSAGetLastError() << std::endl;
            return;
        }

        if (stringBuffer == "/exit\n")
        {
            Sleep(3000);                // ожидание сигнала со стороны сервера на завершение clientReceive (3 секунды)
            if (connectionFlag)
            {
                cleanup();              // принудительное освобождение ресурсов при отсутсвии ответа от сервера
            }
            std::cout << "Поток отправки отключился от сервера" << std::endl;
            return;
        }
    }
}

void Client::cleanup() {
    if (server != INVALID_SOCKET) {
        closesocket(server);
        server = INVALID_SOCKET;
    }
    WSACleanup();
}
