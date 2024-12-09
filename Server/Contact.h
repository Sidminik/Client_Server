#pragma once

#include <vector>
#include <string>
#include <stdexcept>

class Contact
{
private:
    int contactNumber;                                  // номер контакта
    std::vector<std::string> contactMessages;           // вектор сообщений для конкретного контакта  

public:
    Contact(int senderNumber, std::string& senderMessage)
    {
        contactNumber = senderNumber;
        addMessage(senderMessage);
    }
    ~Contact() = default;

    void setNumber(int senderNumber);                   // сеттер номера контакта-отправителя
    int getNumber() const;                              // геттер номера контакта-отправителя    
    void addMessage(std::string& senderMessage);        // добавление нового сообщения в вектор
    std::string getMessage(int index);                  // геттер сообщения по номеру
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
        return "Текст сообщения " + std::to_string(index) + " от клиента " + std::to_string(contactNumber) + ": "
            + contactMessages[index - 1];
    }
    else 
    {
        return "Отсутствует сообщение с введенным номером от данного клиента\n";
    }
}
