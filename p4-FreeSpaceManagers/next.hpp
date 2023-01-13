#pragma once

#include <stdio.h>
#include "fsm.hpp"


class NextFit : public FSM
{
public:
    NextFit(const uint32_t& sizeOfRam);
    MemAddr Alloc(const uint32_t& desiredSize);
private:
    uint32_t lastIndex;
};