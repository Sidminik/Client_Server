/*
Оптимизация строковых операций :
Проверка строк с помощью cstring более эффективна, чем использование std::string::find или std::string::append.

Нет обработки ошибок при создании потока в методе createNewClient(). Рекомендуется добавить проверку на успешное создание потока.

Нет проверки на успешное создание сокета (serverSocket) в конструкторе Server(). Рекомендуется добавить проверку 
на значение serverSocket до использования.

Использование raw указателей, таких как char* buffer, можно заменить на использование std::string 
для безопасного и удобного управления данными.
*/

#include "Server.h"   

using namespace std;

int main()
{
    system("chcp 1251");
    system("CLS");

    unique_ptr<Server> server(new Server);

    return 0;
}


/*
Оптимизация работы с буферами:
Вместо использования фиксированного размера буфера попробуйте использовать динамический буфер (например, std::vector<char>) 
для получения данных от клиента. Это поможет избежать переполнения буфера.

// Constructor and destructor remain unchanged...

void Server::createNewClient()
{
    while (true)
    {
        SOCKET connectingUserSocket = accept(serverSocket, (SOCKADDR*)&serverAddr, &serverAddrSize);
        if (connectingUserSocket == INVALID_SOCKET)
        {
            std::cerr << "Ошибка приема нового соединения с сокетом\n";
            continue; // Попробуйте еще раз.
        }
        {
            std::lock_guard<std::mutex> lock(mtxNewConnect);
            allUsersCounter++;
            User currentUser(connectingUserSocket, allUsersCounter);
            usersConnections.push_back(currentUser);
            std::cout << "Клиент " << currentUser.getNumber() << " подключен!\n";
            connectedUsersCounter++;
            std::cout << "Всего клиентов зарегистрировано " << allUsersCounter << std::endl;
            std::cout << "Всего клиентов подключено " << connectedUsersCounter << std::endl;
        }
        std::thread(&Server::serverReceive, this, connectingUserSocket).detach();
    }
}
*/