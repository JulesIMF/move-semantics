/*++

Copyright (c) 2022 JulesIMF, MIPT

Module Name:

    main.cpp

Abstract:

    

Author / Creation date:

    JulesIMF / 24.02.22

Revision History:

--*/


//
// Includes / usings
//
// #define INT_NO_MOVE
#include <numeric>
#include <Tracker.h>
#include "Int.h"

//
// Defines
//
template <typename T>
T&& forward(T& arg)
{
    return static_cast<T&&>(arg);
}

template<typename T>
T construct_from(T&& origin)
{
    TRACKER_ENTER;
    return T(origin);
}

int main()
{
    TRACKER_DEFAULT_INITIALIZATION;
    TRACKER_ENTER;
    TRACKER_CREATE(Int, origin, 0);
    auto copy = construct_from(origin);
    auto move = construct_from(std::move(origin));
    TRACKER_OFF;
}