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

#include <cstdarg>
#include <Colors.h>
#include <Tracker.h>

//
// Defines
//

namespace Tracker
{
    int ConsoleLogger::printAllign(char const* format, ...)
    {
        static int const tabsize = 4;
        int returned = printf("%*s", depth() * tabsize, "");
        if (returned < 0)
            return returned;

        va_list list = {};
        va_start(list, format);
        returned += vfprintf(stdout, format, list);
        va_end(list);

        return returned;
    }

    void ConsoleLogger::enterFunction(std::string name)
    {
        printfAlligned("%s\n", name.c_str());
        printfAlligned("{\n", name.c_str());
        pushFunction(name);
    }

    void ConsoleLogger::exitFunction()
    {
        Logger::exitFunction();
        printfAlligned("}\n");
    }

    void ConsoleLogger::enterCtr(TrackedInfo& info)
    {
        printColor()
        printInfo(info);
    }

    void ConsoleLogger::enterCtrCopy(TrackedInfo& info)
    {
        printfAlligned("%s%s %s", TerminalColor::RedB, "COPY", TerminalColor::Default);
        printInfo(info);
    }

    void ConsoleLogger::enterCtrMove(TrackedInfo& info)
    {
        printfAlligned("%s%s %s", TerminalColor::GreenB, "MOVE", TerminalColor::Default);
        printInfo(info);
    }

    void ConsoleLogger::enterAsg(TrackedInfo& info)
    {
        printfAlligned("%s%s%s ", TerminalColor::WhiteB, "=", TerminalColor::Default);
        printInfo(info);
    }

    void ConsoleLogger::enterAsgCopy(TrackedInfo& info)
    {
        printfAlligned("%s%s %s", TerminalColor::RedB, "COPY=", TerminalColor::Default);
        printInfo(info);
    }

    void ConsoleLogger::enterAsgMove(TrackedInfo& info)
    {
        printfAlligned("%s%s %s", TerminalColor::GreenB, "MOVE=", TerminalColor::Default);
        printInfo(info);
    }

    void ConsoleLogger::enterAsgOper(TrackedInfo& info, std::string const& oper)
    {
        printfAlligned("%s%s%s%s ", TerminalColor::WhiteB, oper.c_str(), "=", TerminalColor::Default);
        printInfo(info);
    }

    void ConsoleLogger::enterDtr(TrackedInfo& info)
    {
        printfAlligned("%s%s%s ", TerminalColor::BlueB, "DTR", TerminalColor::Default);
        printInfo(info);
    }
}