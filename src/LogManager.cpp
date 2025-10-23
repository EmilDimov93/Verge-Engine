#include "LogManager.h"

#define LOG_MESSAGE_LIMIT 1000

std::string ErrorCode::GetMessage()
{
    auto it = messages.find({letter, number});
    if (it != messages.end())
    {
        return it->second;
    }
    return "";
}

void LogManager::FreeLogSpace(){
    for(int i = logList.size() / 4; i >= 0; i--){
        if(logList[i].number / 100 != 2){
            logList.erase(logList.begin() + i);
            clearedLogMessages++;
        }
    }
}

void LogManager::AddToLog(char letter, int number)
{
    logList.push_back(ErrorCode{letter, number});
    hasNewMessages = true;
    newMessageCount++;
    if (logList.size() > LOG_MESSAGE_LIMIT)
    {
        FreeLogSpace();
    }
}

std::vector<std::string> LogManager::GetNewMessages()
{
    std::vector<std::string> newMessages;
    for (int i = logList.size() - newMessageCount; i < logList.size(); i++)
    {
        newMessages.push_back(logList[i].GetMessage());
    }
    newMessageCount = 0;
    hasNewMessages = false;
    return newMessages;
}

bool LogManager::HasNewMessages(){
    return hasNewMessages;
}