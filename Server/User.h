#pragma once

#include <vector>
#include <string>
#include "Contact.h" 

class User
{
private:
    int userNumber;                                                         // номер пользователя
    SOCKET userSocket;                                                      // сокет пользователя
    std::vector<Contact> userContacts;                                      // вектор контактов конкретного пользователя

public:
    User(SOCKET newSocket, int newNumber)
        : userSocket(newSocket), userNumber(newNumber)
    {
    }
    ~User() = default;

    int getNumber() const;                                                  // геттер номера пользователя
    SOCKET getSocket() const;                                               // геттер сокета пользователя
    void addContact(int number, std::string message);                       // добавление контакта
    void addMessage(int senderNumber, std::string senderMessage);           // добавление сообщения
    std::string getMessage(int numberContact, int numberMessage) const;     // геттер сообщения по номеру от конкретного контакта
};

int User::getNumber() const
{
    return userNumber;
}

SOCKET User::getSocket() const
{
    return userSocket;
}

void User::addContact(int number, std::string message)
{
    Contact newContact(number, message);
    userContacts.push_back(newContact);
}

void User::addMessage(int senderNumber, std::string senderMessage)
{
    // добавление сообщения при наличии контакта с введенным номером
    for (Contact contact : userContacts)
    {
        if (contact.getNumber() == senderNumber)
        {
            contact.addMessage(senderMessage);
            return;
        }
    }
    // добавление сообщения при отсутствии контакта с введенным номером
    addContact(senderNumber, senderMessage);
}

std::string User::getMessage(int numberContact, int numberMessage) const
{
    for (Contact contact : userContacts)
    {
        if (contact.getNumber() == numberContact)
        {
            return contact.getMessage(numberMessage);
        }
    }
    return "Отсутствует клиент с введенным номером\n";
}
