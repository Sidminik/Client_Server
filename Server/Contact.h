#pragma once

#include <vector>
#include <string>
#include <stdexcept>

class Contact
{
private:
    int contactNumber;                                  // ����� ��������
    std::vector<std::string> contactMessages;           // ������ ��������� ��� ����������� ��������  

public:
    Contact(int senderNumber, std::string& senderMessage)
    {
        contactNumber = senderNumber;
        addMessage(senderMessage);
    }
    ~Contact() = default;

    void setNumber(int senderNumber);                   // ������ ������ ��������-�����������
    int getNumber() const;                              // ������ ������ ��������-�����������    
    void addMessage(std::string& senderMessage);        // ���������� ������ ��������� � ������
    std::string getMessage(int index);                  // ������ ��������� �� ������
};

void Contact::setNumber(int senderNumber)
{
    contactNumber = senderNumber;
}

int Contact::getNumber() const
{
    return contactNumber;
}

void Contact::addMessage(std::string& senderMessage)
{
    contactMessages.push_back(senderMessage);
}

std::string Contact::getMessage(int index) 
{
    if (index > 0 && index <= contactMessages.size()) 
    {
        return "����� ��������� " + std::to_string(index) + " �� ������� " + std::to_string(contactNumber) + ": "
            + contactMessages[index - 1];
    }
    else 
    {
        return "����������� ��������� � ��������� ������� �� ������� �������\n";
    }
}
