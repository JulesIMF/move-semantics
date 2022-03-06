/*++

Copyright (c) 2022 JulesIMF, MIPT

Module Name:

    Console.cpp

Abstract:

    

Author / Creation date:

    JulesIMF / 24.02.22

Revision History:

--*/


//
// Includes / usings
//

#include <Colors.h>
#include <Tracker.h>

//
// Defines
//

namespace Tracker
{
    void ConsoleLogger::printAllign()
    {
        static int const tabsize = 4;
        printf("%*s", depth() * tabsize, "");
    }

    void ConsoleLogger::printColor(TextLogger::Color color, std::string const& str)
    {
        switch (color)
        {
        #define CONSOLE_LOGGER_COLOR(c) \
        case TextLogger::Color::c: \
            printf("%s", TerminalColor::c); \
            break;

        CONSOLE_LOGGER_COLOR(BlackB);
        CONSOLE_LOGGER_COLOR(RedB);
        CONSOLE_LOGGER_COLOR(GreenB);
        CONSOLE_LOGGER_COLOR(YellowB);
        CONSOLE_LOGGER_COLOR(BlueB);
        CONSOLE_LOGGER_COLOR(PurpleB);
        CONSOLE_LOGGER_COLOR(CyanB);
        CONSOLE_LOGGER_COLOR(WhiteB);
        
        default:
        CONSOLE_LOGGER_COLOR(Default);

        #undef CONSOLE_LOGGER_COLOR
        }

        printf("%s", str.c_str());
        printf("%s", TerminalColor::Default);
    }

    ConsoleLogger::~ConsoleLogger()
    {
        auto total = mainLogger.getTotal();
        printf("\n\n%d object%s created, %d copied, %d moved\n", total.obj, (total.obj != 1 ? "s" : ""), total.copy, total.move);
    }
}