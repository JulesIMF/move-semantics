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


Int test(Int a, Int b, Int c)
{
    TRACKER_ENTER;
    return a + b + c;
}

int main()
{
    Tracker::mainLogger.addNewLogger(new Tracker::ConsoleLogger);
    Tracker::mainLogger.addNewLogger(new Tracker::HtmlLogger);
    Tracker::mainLogger.addNewLogger(new Tracker::DotLogger);
    TRACKER_ENTER;
    TRACKER_CREATE(Int, lol, 0);
    TRACKER_CREATE(Int, lol2, 2);
    lol = lol2;
    return 0;
}