#pragma once

#include <vector>
#include <string>
#include "Contact.h" 

class User
{
private:
    int userNumber;                                                         // ����� ������������
    SOCKET userSocket;                                                      // ����� ������������
    std::vector<Contact> userContacts;                                      // ������ ��������� ����������� ������������

public:
    User(SOCKET newSocket, int newNumber) 
        : userSocket(newSocket), userNumber(newNumber)
    {}
    ~User() = default;

    int getNumber() const;                                                  // ������ ������ ������������
    SOCKET getSocket() const;                                               // ������ ������ ������������
    void addContact(int number, std::string message);                       // ���������� ��������
    void addMessage(int senderNumber, std::string senderMessage);           // ���������� ���������
    std::string getMessage(int numberContact, int numberMessage) const;     // ������ ��������� �� ������ �� ����������� ��������
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
    // ���������� ��������� ��� ������� �������� � ��������� �������
    for (Contact contact : userContacts)
    {
        if (contact.getNumber() == senderNumber)
        {
            contact.addMessage(senderMessage);
            return;
        }
    }
    // ���������� ��������� ��� ���������� �������� � ��������� �������
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
    return "����������� ������ � ��������� �������\n";
}
