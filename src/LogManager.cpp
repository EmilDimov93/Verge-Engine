// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include "LogManager.hpp"

#include <fstream>
#include <iomanip>
#include <ctime>
#include <chrono>

#include "version.hpp"

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
    for (size_t i = entries.size() / 4; i >= 0; i--)
    {
        if (entries[i].number / 100 != 2)
        {
            entries.erase(entries.begin() + i);
            clearedEntriesCount++;
        }
    }
}

void LogManager::add(char letter, uint16_t number)
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
    for (size_t i = entries.size() - newMessageCount; i < entries.size(); i++)
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

void LogManager::writeToLogFile(){
    std::ofstream file("log.txt");

    auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    file << "Verge Engine Log - " << std::put_time(std::localtime(&now), "%Y-%m-%d - %H:%M:%S") << " - Version " << VERGE_ENGINE_VERSION << "\n\n";

    for (auto& entry : entries){
        file << entry.letter << std::setfill('0') << std::setw(3) << entry.number << ": " << entry.getMessage() << '\n';
    }
}