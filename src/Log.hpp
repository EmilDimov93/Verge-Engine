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

    ErrorCode(char newLetter, uint16_t newNumber){
        if(messages.find({newLetter, newNumber}) == messages.end()){
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

inline bool operator!=(const ErrorCode& lhs, const ErrorCode& rhs) {
    return lhs.letter != rhs.letter || lhs.number != rhs.number;
}

struct EngineCrash : public std::exception {};

class Log
{
public:
    static void add(char letter, uint16_t number);
    static std::vector<std::string> getNewMessages();
    static bool hasNewMessages();
    static void printNewMessages();
    static void end();

private:
    static std::vector<ErrorCode> entries;
    static size_t newMessageCount;
    static bool hasNewMessagesFlag;
    static size_t clearedEntriesCount;

    static void writeToLogFile();
    static void freeLogSpace();
    static void induceCrash();
};