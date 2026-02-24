// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include "Log.hpp"

#include "LogCodes.hpp"
#include "version.hpp"

#include <fstream>
#include <chrono>

static constexpr size_t LOG_MESSAGE_LIMIT = 100000;

#define IS_ENTRY_ERROR(num) (((num) / 100) % 10 == 2)

std::vector<ErrorCode> Log::entries;
size_t Log::newMessageCount = 0;
bool Log::hasNewMessagesFlag = false;
size_t Log::clearedEntriesCount = 0;
LogOutputMode Log::outputMode = VE_LOG_OUTPUT_MODE_FILE_AND_CONSOLE;

const std::map<std::pair<char, uint16_t>, std::string> ErrorCode::messages = LOG_MESSAGES;

std::string ErrorCode::getMessage()
{
    auto it = messages.find({letter, number});
    if (it != messages.end())
    {
        return it->second;
    }
    return "Invalid error code";
}

void Log::init(LogOutputMode mode)
{
    outputMode = mode;

    if (mode == VE_LOG_OUTPUT_MODE_CONSOLE || mode == VE_LOG_OUTPUT_MODE_FILE_AND_CONSOLE)
    {
        for (std::string message : getNewMessages())
        {
            std::cout << "LOG: " << message << std::endl;
        }
    }

    Log::add('E', 000);
}

void Log::freeLogSpace()
{
    if (entries.empty()) return;

    // Remove non-error messages from the oldest quarter
    const size_t limit = entries.size() / 4;

    for (size_t i = limit + 1; i-- > 0; )
    {
        if (i >= entries.size()) continue;
        if (!IS_ENTRY_ERROR(entries[i].number))
        {
            entries.erase(entries.begin() + i);
            clearedEntriesCount++;
        }
    }
}

void Log::add(char letter, uint16_t number)
{
    entries.push_back(ErrorCode{letter, number});
    hasNewMessagesFlag = true;
    newMessageCount++;

    if (outputMode == VE_LOG_OUTPUT_MODE_CONSOLE || outputMode == VE_LOG_OUTPUT_MODE_FILE_AND_CONSOLE)
    {
        std::cout << "LOG: " << entries.back().getMessage() << std::endl;
    }

    if (IS_ENTRY_ERROR(number))
    {
        induceCrash();
    }

    if (entries.size() > LOG_MESSAGE_LIMIT)
    {
        freeLogSpace();
    }
}

std::vector<std::string> Log::getNewMessages()
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

bool Log::hasNewMessages()
{
    return hasNewMessagesFlag;
}

void Log::writeToLogFile()
{
    std::ofstream file("log.txt");

    auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    file << "Verge Engine Log - " << std::put_time(std::localtime(&now), "%Y-%m-%d - %H:%M:%S") << " - Version " << VERGE_ENGINE_VERSION << "\n\n";

    for (auto &entry : entries)
    {
        file << entry.letter << std::setfill('0') << std::setw(3) << entry.number << ": " << entry.getMessage() << '\n';
    }
}

void Log::induceCrash()
{
    entries.push_back(ErrorCode{'E', 200});

    if (outputMode == VE_LOG_OUTPUT_MODE_FILE || outputMode == VE_LOG_OUTPUT_MODE_FILE_AND_CONSOLE)
    {
        writeToLogFile();
    }

    throw EngineCrash{};
}

void Log::end()
{
    if (entries.empty() || entries.back() != ErrorCode{'E', 200})
        add('E', 001);

    if (outputMode == VE_LOG_OUTPUT_MODE_FILE || outputMode == VE_LOG_OUTPUT_MODE_FILE_AND_CONSOLE)
    {
        writeToLogFile();
    }
}