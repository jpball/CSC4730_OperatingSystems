#pragma once

#include "fsm.hpp"
#include <algorithm>

class BestFit : public FSM
{
public:
    BestFit(const uint32_t& memSize);
    MemAddr Alloc(const uint32_t& desiredSize);
private:
};