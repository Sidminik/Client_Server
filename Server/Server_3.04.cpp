#include "Server.h"   

using namespace std;

int main()
{
    system("chcp 1251");
    system("CLS");

    unique_ptr<Server> server(new Server);

    return 0;
}
