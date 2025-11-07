// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include <iostream> // temporary
#include <fstream>

#include <cstdint>

struct Size { 
    uint32_t w;
    uint32_t h; 
};

static std::vector<char> readFile(const std::string &fileName){
    std::ifstream file(fileName, std::ios::binary | std::ios::ate);

    if(!file.is_open()){
        return {};
    }

    size_t fileSize = (size_t)file.tellg();

    std::vector<char> fileBuffer(fileSize);

    file.seekg(0);

    file.read(fileBuffer.data(), fileSize);

    file.close();

    return fileBuffer;
}