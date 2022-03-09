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
#include <Tracker.h>
#include "Int.h"

//
// Defines
//

Int level1()
{
    TRACKER_ENTER;
    TRACKER_CREATE(Int, valueInLevel1, 0);
    return valueInLevel1;
}

Int level2()
{
    TRACKER_ENTER;
    TRACKER_CREATE(Int, valueInLevel2, level1());
    return valueInLevel2;
}

Int level3()
{
    TRACKER_ENTER;
    TRACKER_CREATE(Int, valueInLevel3, level2());
    return valueInLevel3;
}

int main()
{
    TRACKER_DEFAULT_INITIALIZATION;
    TRACKER_ENTER;
    TRACKER_CREATE(Int, returnValueLevel1, level1());
    TRACKER_CREATE(Int, returnValueLevel2, level2());
    TRACKER_CREATE(Int, returnValueLevel3, level3());
}