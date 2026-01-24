// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include "../shared/Log.hpp"
#include "../shared/definitions.hpp"

template <typename HandleT>
class HandleFactory{
public:
    static HandleT getNewHandle(){
        if(last >= UINT64_MAX){
            Log::add('S', 200);
        }

        return HandleT{++last};
    }
    
private:
    static inline uint64_t last = 0;
};