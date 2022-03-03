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

#include <Tracker.h>
#include "Int.h"

//
// Defines
//


Int getLol()
{
    return 45;
}

int main()
{
    Tracker::mainLogger.addNewLogger(new Tracker::ConsoleLogger);
    TRACKER_ENTER;
    TRACKER_CREATE(Int, lol, 54);
    TRACKER_CREATE(Int, lol2, 0);
    lol2 = std::move(lol + lol);

    lol2 = getLol();
    return 0;
}