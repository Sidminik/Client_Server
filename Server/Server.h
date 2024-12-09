#pragma once

#define _WINSOCK_DEPRECATED_NO_WARNINGS     
#include <iostream>
#include <thread>
#include <vector>
#include <string>
#include <winsock2.h>
#include <mutex>
#include "User.h"
#include "Contact.h"

#pragma comment(lib, "WS2_32.lib")          

class Server
{
    SOCKET serverSocket;                    // сокет сервера
    SOCKADDR_IN serverAddr;                 // адрес сокета сервера
    std::vector<User> usersConnections;     // вектор хранения подключений пользователя (клиентов)
    int allUsersCounter;                    // счетчик всех клиентов
    int connectedUsersCounter;              // счетчик подключенных клиентов
    std::recursive_mutex mtxNewConnect;     // синхронизация в методе serverReceive
    std::recursive_mutex mtxDisconnect;     // синхронизация в методе socketError
    std::recursive_mutex mtxNewMessage;     // синхронизация в методе sendCommandProcessing
public:
    Server();
    ~Server();

    void createNewClient();                                                         // создание потока нового клиента
    void serverReceive(SOCKET transmittedSocket);                                   // подключение нового клиента
    void socketError(User currentUser);                                             // обработка ошибки сокета
    void exitProcessing(User currentUser);                                          // обработка команды /exit
    void startProcessing(User currentUser);                                         // обработка команды /start
    void clientCountProcessing(User currentUser) const;                             // обработка команды /client_count
    void sendCommandProcessing(User currentUser, std::string sendCommandString);    // обработка команды /send_message
    void showCommandProcessing(User currentUser, std::string showCommandString);    // обработка команды /show_message 
};

Server::Server()
{
    WSADATA wsaData; 
    WSAStartup(MAKEWORD(2, 7), &wsaData); 
    serverSocket = socket(AF_INET, SOCK_STREAM, 0); 
    if (serverSocket == INVALID_SOCKET)
    {
        std::cerr << "Создание сокета завершилось сбоем с ошибкой:" << WSAGetLastError() << std::endl;
    }

    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(15000);

    if (bind(serverSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
    {
        std::cerr << "Функция соединения завершилась сбоем с ошибкой: " << WSAGetLastError() << std::endl;
    }
    if (listen(serverSocket, 0) == SOCKET_ERROR)
    {
        std::cerr << "Функция прослушивания завершилась сбоем с ошибкой:" << WSAGetLastError() << std::endl;
    }
    std::cout << "Прослушиваются входящие подключения...." << std::endl;
    createNewClient();
}

Server::~Server()
{
    closesocket(serverSocket);
    WSACleanup();
};

void Server::createNewClient()
{
    while (true)
    {
        SOCKET connectingUserSocket = accept(serverSocket, (SOCKADDR*)&serverAddr, 0);
        if (connectingUserSocket == 0)
        {
            std::cerr << "Ошибка приема нового соединения с сокетом" << std::endl;
        }
        else
        {
            std::thread thread(&Server::serverReceive, this, connectingUserSocket);
            thread.detach();
        }
    }
}


void Server::serverReceive(SOCKET transmittedSocket)
{
    mtxNewConnect.lock();
        allUsersCounter++;
        User currentUser(transmittedSocket, allUsersCounter);
        usersConnections.push_back(currentUser);
        connectedUsersCounter++;
        std::cout << "Клиент " << currentUser.getNumber() << " подключен!" << std::endl;
    mtxNewConnect.unlock();

    char buffer[1024] = { 0 };
    std::string sendCommandString;
    std::string showCommandString;

    while (true) // цикл работы сервера
    {
        if (recv(currentUser.getSocket(), buffer, sizeof(buffer), 0) == SOCKET_ERROR)
        {
            socketError(currentUser);
            return;
        }
        else if (strcmp(buffer, "/exit\n") == 0)
        {
            exitProcessing(currentUser);
            return;
        }
        else if (strcmp(buffer, "/start\n") == 0)
        {
            startProcessing(currentUser);
        }
        else if (strcmp(buffer, "/client_count\n") == 0)
        {
            clientCountProcessing(currentUser);
        }
        else if (sendCommandString.append(buffer).find("/send_message_") == 0)
        {
            sendCommandProcessing(currentUser, sendCommandString);
            sendCommandString.clear();
        }
        else if (showCommandString.append(buffer).find("/show_message_") == 0
            && showCommandString.find("_from_client_") < sizeof(buffer))
        {
            showCommandProcessing(currentUser, showCommandString);
            showCommandString.clear();
        }
        else
        {
            std::cout << "Клиент " << currentUser.getNumber() << ": " << buffer;
        }
        memset(buffer, 0, sizeof(buffer));
    }
}

void Server::socketError(User currentUser)
{
    std::lock_guard<std::recursive_mutex> lock(mtxDisconnect);

    std::cerr << "Соединение с клиентом " << currentUser.getNumber() << " завершилось сбоем с ошибкой : "
        << WSAGetLastError() << std::endl;

    for (int i = 0; i < usersConnections.size(); i++)
    {
        if (usersConnections[i].getNumber() == currentUser.getNumber())
        {
            usersConnections.erase(usersConnections.begin() + i);
        }
    }
    connectedUsersCounter--;
}

void Server::exitProcessing(User currentUser)
{
    std::cout << "Клиент " << currentUser.getNumber() << " отсоединился." << std::endl;
    connectedUsersCounter--;
    std::string localMessage = { "Произошло отключение от сервера\n" };
    send(currentUser.getSocket(), localMessage.c_str(), (int) localMessage.length(), 0);
}

void Server::startProcessing(User currentUser)
{
    std::string localMessage = { "Привет. Ваш порядковый номер - " + std::to_string(currentUser.getNumber()) + "\n"};
    send(currentUser.getSocket(), localMessage.c_str(), (int)localMessage.length(), 0);
    std::cout << localMessage;
}

void Server::clientCountProcessing(User currentUser) const
{
    std::string localMessage = { "Количество клиентов, подключенных сейчас - " 
        + std::to_string(connectedUsersCounter) + "\n" };
    send(currentUser.getSocket(), localMessage.c_str(), (int) localMessage.length(), 0);
    std::cout << localMessage;
}

void Server::sendCommandProcessing(User currentUser, std::string sendCommandString)
{
    std::lock_guard<std::recursive_mutex> lock(mtxNewMessage);

    std::string command = "/send_message_";
    int senderNumber = currentUser.getNumber();
    int recipientNumber = stoi(sendCommandString.substr(command.length(), sendCommandString.length()));
    int endCommandIndex = (int) sendCommandString.find(":");

    std::string localMessageShort = sendCommandString.substr(endCommandIndex + 1, sendCommandString.length());
    std::string localMessageLong = "Текст нового сообщения от клиента " + std::to_string(senderNumber) + ": "
        + localMessageShort + "\n";

    std::cout << "Текст сообщения для клиента " << recipientNumber << ": " << localMessageShort;
    
    for (int i = 0; i < usersConnections.size(); i++)
    {
        if (usersConnections[i].getNumber() == recipientNumber)
        {
            usersConnections[recipientNumber - 1].addMessage(senderNumber, localMessageShort);

            if (send(usersConnections[recipientNumber - 1].getSocket(), localMessageLong.c_str(),
                (int)localMessageLong.length() - 1, 0) == SOCKET_ERROR)
            {
                std::string errorMessage = "Cообщение отправлено с ошибкой\n";
                send(currentUser.getSocket(), errorMessage.c_str(), (int)errorMessage.length(), 0);
                std::cout << errorMessage;
            }
            else
            {
                std::string successfulMessage = "Cообщение отправлено успешно\n";
                send(currentUser.getSocket(), successfulMessage.c_str(), (int)successfulMessage.length(), 0);
                std::cout << successfulMessage;
            }
            return;
        }
        else
        {
            std::string errorUser = "Отсутствует получатель с указанным номером\n";
            send(currentUser.getSocket(), errorUser.c_str(), (int)errorUser.length(), 0);
            std::cout << errorUser;
            return;
        }
    }
}

void Server::showCommandProcessing(User currentUser, std::string showCommandString)
{
    std::string commandPart1 = "/show_message_";
    std::string commandPart2 = "_from_client_";

    int indexPart2 = (int) showCommandString.find("_from_client_");
    int messageNumber = stoi(showCommandString.substr(commandPart1.length(), indexPart2));

    if ((indexPart2 + commandPart2.length()) < (showCommandString.length() - 1))
    {
        int recipientNumber = currentUser.getNumber();
        int senderNumber = stoi(showCommandString.substr(
            indexPart2 + commandPart2.length(), showCommandString.length()));

        for (int i = 0; i < usersConnections.size(); i++)
        {
            if (usersConnections[i].getNumber() == recipientNumber)
            {
                std::string localMessageShort = usersConnections[i].getMessage(senderNumber, messageNumber);
                std::string localMessageLong = localMessageShort + "\n";
                send(currentUser.getSocket(), localMessageLong.c_str(), (int)localMessageLong.length() - 1, 0);
                std::cout << localMessageShort;
                break;
            }
        }
    }
}
