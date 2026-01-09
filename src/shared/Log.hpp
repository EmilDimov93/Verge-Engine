// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include "definitions.hpp"

#include <map>
#include <string>
#include <vector>

class ErrorCode
{
public:
    char letter;
    uint16_t number;

    std::string getMessage();

    ErrorCode(char newLetter, uint16_t newNumber)
    {
        if (messages.find({newLetter, newNumber}) == messages.end())
        {
            letter = 'O';
            number = 100;
            return;
        }
        letter = newLetter;
        number = newNumber;
    }

private:
    static const std::map<std::pair<char, uint16_t>, std::string> messages;
};

inline bool operator!=(const ErrorCode &lhs, const ErrorCode &rhs)
{
    return lhs.letter != rhs.letter || lhs.number != rhs.number;
}

struct EngineCrash : public std::exception
{
};

enum LogOutputMode
{
    VE_LOG_OUTPUT_MODE_NONE,
    VE_LOG_OUTPUT_MODE_FILE,
    VE_LOG_OUTPUT_MODE_CONSOLE,
    VE_LOG_OUTPUT_MODE_FILE_AND_CONSOLE
};

class Log
{
public:
    static void init(LogOutputMode mode);
    static void add(char letter, uint16_t number);
    static std::vector<std::string> getNewMessages();
    static bool hasNewMessages();
    static void end();

private:
    static std::vector<ErrorCode> entries;
    static size_t newMessageCount;
    static bool hasNewMessagesFlag;
    static size_t clearedEntriesCount;
    static LogOutputMode outputMode;

    static void writeToLogFile();
    static void freeLogSpace();
    static void induceCrash();
};