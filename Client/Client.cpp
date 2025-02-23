#include <memory>
#include "Client.h"

int main(int argc, char* argv[])
{
    const char* inetAddress;
    if (argc == 1)
    {
        inetAddress = "127.0.0.1";
    }
    else
    {
        inetAddress = argv[1];
    }

    system("chcp 1251");
    system("CLS");

    std::unique_ptr<Client> client(new Client(inetAddress));

    std::thread sendThread(&Client::clientSend, client.get());
    std::thread receiveThread(&Client::clientReceive, client.get());

    sendThread.join();
    receiveThread.join();

    std::cout << "Клиент отключился от сервера (оба потока завершены)" << std::endl;

    return 0;
}
