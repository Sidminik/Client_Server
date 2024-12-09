#pragma once

#define _WINSOCK_DEPRECATED_NO_WARNINGS     // ������ ��� ���������� ���������� �������������� API
#include <iostream>
#include <thread>
#include <vector>
#include <string>
#include <winsock2.h>
#include <mutex>
#include "User.h"
#include "Contact.h"

#pragma comment(lib, "WS2_32.lib")          // ����������� ���������� �������

class Server
{
    SOCKET serverSocket;                    // ����� �������
    SOCKADDR_IN serverAddr;                 // ����� ������ �������
    std::vector<User> usersConnections;     // ������ �������� ����������� ������������ (��������)
    int allUsersCounter;                    // ������� ���� ��������
    int connectedUsersCounter;              // ������� ������������ ��������
    std::recursive_mutex mtxNewConnect;     // ������������� ����������� ������ ������ serverReceive
    std::recursive_mutex mtxDisconnect;     // ������������� ����������� ������ ������ socketError
    std::recursive_mutex mtxNewMessage;     // ������������� ����������� ������ ������ sendCommandProcessing
public:
    Server();
    ~Server();

    void createNewClient();                                                         // �������� ������ ������ �������
    void serverReceive(SOCKET transmittedSocket);                                   // ����������� ������ �������
    void socketError(User currentUser);                                             // ��������� ������ ������
    void exitProcessing(User currentUser);                                          // ��������� ������� /exit
    void startProcessing(User currentUser);                                         // ��������� ������� /start
    void clientCountProcessing(User currentUser) const;                             // ��������� ������� /client_count
    void sendCommandProcessing(User currentUser, std::string sendCommandString);    // ��������� ������� /send_message
    void showCommandProcessing(User currentUser, std::string showCommandString);    // ��������� ������� /show_message 
};

Server::Server()
{
    WSADATA wsaData; // ��������� ������, ���������� �������� � ���������� ������� Windows
    WSAStartup(MAKEWORD(2, 7), &wsaData); // ������� WSAStartup ���������� ������������� winsock DLL ���������
    serverSocket = socket(AF_INET, SOCK_STREAM, 0); // �������������� ������ �������
    if (serverSocket == INVALID_SOCKET)
    {
        std::cerr << "�������� ������ ����������� ����� � �������:" << WSAGetLastError() << std::endl;
    }

    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(15000);

    if (bind(serverSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
    {
        std::cerr << "������� ���������� ����������� ����� � �������: " << WSAGetLastError() << std::endl;
    }
    if (listen(serverSocket, 0) == SOCKET_ERROR)
    {
        std::cerr << "������� ������������� ����������� ����� � �������:" << WSAGetLastError() << std::endl;
    }
    std::cout << "�������������� �������� �����������...." << std::endl;
    createNewClient();
}

Server::~Server()
{
    closesocket(serverSocket);
    WSACleanup(); // ����������� ������� WinSock
};

void Server::createNewClient()
{
    while (true)
    {
        SOCKET connectingUserSocket = accept(serverSocket, (SOCKADDR*)&serverAddr, 0);
        if (connectingUserSocket == 0)
        {
            std::cerr << "������ ������ ������ ���������� � �������" << std::endl;
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
        std::cout << "������ " << currentUser.getNumber() << " ���������!" << std::endl;
    mtxNewConnect.unlock();

    char buffer[1024] = { 0 };
    std::string sendCommandString;
    std::string showCommandString;

    while (true) // ���� ������ �������
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
            std::cout << "������ " << currentUser.getNumber() << ": " << buffer;
        }
        memset(buffer, 0, sizeof(buffer));
    }
}

// ������� ��������� � ������������� ������� ��� � �������

/*
void Server::serverReceive(SOCKET transmittedSocket)
{
    mtxNewConnect.lock();
    allUsersCounter++;
    User currentUser(transmittedSocket, allUsersCounter);
    usersConnections.push_back(currentUser);
    connectedUsersCounter++;
    std::cout << "������ " << currentUser.getNumber() << " ���������!" << std::endl;
    mtxNewConnect.unlock();

    char buffer[1024];
    int recvRead = recv(transmittedSocket, buffer, sizeof(buffer), MSG_PEEK);
    int recvCount = 0;
    std::string sendCommandString;
    std::string showCommandString;
    std::string strBuffer;

    while (true) // ���� ������ �������
    {
        if (recvRead <= 0)
        {
            socketError(currentUser);
            return;
        }
        else
        {
            while ((recvCount = recv(transmittedSocket, buffer, sizeof(buffer), 0)) > 0) {
                if (recvCount >= sizeof(buffer))
                {
                    strBuffer.append(buffer, recvCount);
                } 
                else 
                if (strBuffer == "/exit\n")
                {
                    exitProcessing(currentUser);
                    return;
                }
                else if (strBuffer == "/start\n")
                {
                    startProcessing(currentUser);
                }
                else if (strBuffer == "/client_count\n")
                {
                    clientCountProcessing(currentUser);
                }
                else if (sendCommandString.append(strBuffer).find("/send_message_") == 0)
                {
                    sendCommandProcessing(currentUser, sendCommandString);
                    sendCommandString.clear();
                }
                else if (showCommandString.append(strBuffer).find("/show_message_") == 0
                    && showCommandString.find("_from_client_") < sizeof(strBuffer))
                {
                    showCommandProcessing(currentUser, showCommandString);
                    showCommandString.clear();
                }
                else
                {
                    std::cout << "������ " << currentUser.getNumber() << ": " << strBuffer;
                }
                strBuffer.clear();
                memset(buffer, 0, sizeof(buffer));




            }
            
            
                
        }
    }
}
*/

void Server::socketError(User currentUser)
{
    std::lock_guard<std::recursive_mutex> lock(mtxDisconnect);

    std::cerr << "���������� � �������� " << currentUser.getNumber() << " ����������� ����� � ������� : "
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
    std::cout << "������ " << currentUser.getNumber() << " ������������." << std::endl;
    connectedUsersCounter--;
    std::string localMessage = { "��������� ���������� �� �������\n" };
    send(currentUser.getSocket(), localMessage.c_str(), (int) localMessage.length(), 0);
}

void Server::startProcessing(User currentUser)
{
    std::string localMessage = { "������. ��� ���������� ����� - " + std::to_string(currentUser.getNumber()) + "\n"};
    send(currentUser.getSocket(), localMessage.c_str(), (int)localMessage.length(), 0);
    std::cout << localMessage;
}

void Server::clientCountProcessing(User currentUser) const
{
    std::string localMessage = { "���������� ��������, ������������ ������ - " 
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
    std::string localMessageLong = "����� ������ ��������� �� ������� " + std::to_string(senderNumber) + ": "
        + localMessageShort + "\n";

    std::cout << "����� ��������� ��� ������� " << recipientNumber << ": " << localMessageShort;
    
    for (int i = 0; i < usersConnections.size(); i++)
    {
        if (usersConnections[i].getNumber() == recipientNumber)
        {
            usersConnections[recipientNumber - 1].addMessage(senderNumber, localMessageShort);

            if (send(usersConnections[recipientNumber - 1].getSocket(), localMessageLong.c_str(),
                (int)localMessageLong.length() - 1, 0) == SOCKET_ERROR)
            {
                std::string errorMessage = "C�������� ���������� � �������\n";
                send(currentUser.getSocket(), errorMessage.c_str(), (int)errorMessage.length(), 0);
                std::cout << errorMessage;
            }
            else
            {
                std::string successfulMessage = "C�������� ���������� �������\n";
                send(currentUser.getSocket(), successfulMessage.c_str(), (int)successfulMessage.length(), 0);
                std::cout << successfulMessage;
            }
            return;
        }
        else
        {
            std::string errorUser = "����������� ���������� � ��������� �������\n";
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