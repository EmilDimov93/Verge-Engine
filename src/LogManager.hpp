// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include <map>
#include <string>
#include <vector>

#include "definitions.hpp"

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

class LogManager
{
private:
    std::vector<ErrorCode> entries;
    size_t newMessageCount = 0;
    bool hasNewMessagesFlag = false;
    size_t clearedEntriesCount = 0;

    void freeLogSpace();
    void InduceCrash();

public:
    void add(char letter, uint16_t number);
    std::vector<std::string> getNewMessages();
    bool hasNewMessages();
    void writeToLogFile();
};