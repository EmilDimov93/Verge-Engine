#include "LogManager.h"

#define LOG_MESSAGE_LIMIT 1000

std::string ErrorCode::getMessage()
{
    auto it = messages.find({letter, number});
    if (it != messages.end())
    {
        return it->second;
    }
    return "";
}

void LogManager::freeLogSpace()
{
    for (int i = entries.size() / 4; i >= 0; i--)
    {
        if (entries[i].number / 100 != 2)
        {
            entries.erase(entries.begin() + i);
            clearedEntriesCount++;
        }
    }
}

void LogManager::add(char letter, int number)
{
    entries.push_back(ErrorCode{letter, number});
    hasNewMessagesFlag = true;
    newMessageCount++;
    if (entries.size() > LOG_MESSAGE_LIMIT)
    {
        freeLogSpace();
    }
}

std::vector<std::string> LogManager::getNewMessages()
{
    std::vector<std::string> newMessages;
    for (int i = entries.size() - newMessageCount; i < entries.size(); i++)
    {
        newMessages.push_back(entries[i].getMessage());
    }
    newMessageCount = 0;
    hasNewMessagesFlag = false;
    return newMessages;
}

bool LogManager::hasNewMessages()
{
    return hasNewMessagesFlag;
}