#include "Client.h"

using namespace std;

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

    Client client(inetAddress);

    thread sendThread(&Client::clientSend, &client);
    thread receiveThread(&Client::clientReceive, &client);

    sendThread.join();
    receiveThread.join();

    cout << "Клиент отключился от сервера (оба потока завершены)" << endl;

    return 0;
}
