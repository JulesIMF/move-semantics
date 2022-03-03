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
    void TextLogger::enterFunction(std::string name)
    {
        printAllign();
        printColor(Color::Default, name.c_str());
        printAllign();
        printColor(Color::Default, "{\n");
        pushFunction(name);
    }

    void TextLogger::exitFunction()
    {
        Logger::exitFunction();
        printAllign();
        printColor(Color::Default, "}\n");
    }

    void TextLogger::printInfo(TrackedInfo const& info)
    {
        char hex[17];
        sprintf(hex, "%08x", info.address);

        printColor(Color::YellowB, "\"" + info.name + "\" ");
        printColor(Color::Default, "(id: ");
        printColor(Color::PurpleB, std::to_string(info.id));
        printColor(Color::Default, ", val: ");
        printColor(Color::PurpleB, info.value);
        printColor(Color::Default, ", addr: ");
        printColor(Color::PurpleB, hex);
        printColor(Color::Default, ")");
        printColor(Color::Default, "\n");
    }

    void TextLogger::enterCtr(TrackedInfo& info)
    {
        printAllign();
        printColor(Color::WhiteB, "CTR ");
        printInfo(info);
    }

    void TextLogger::enterCtrCopy(TrackedInfo& info)
    {
        printAllign();
        printColor(Color::RedB, "COPY ");
        printInfo(info);
    }

    void TextLogger::enterCtrMove(TrackedInfo& info)
    {
        printAllign();
        printColor(Color::GreenB, "MOVE ");
        printInfo(info);
    }

    void TextLogger::enterAsg(TrackedInfo& info)
    {
        printAllign();
        printColor(Color::WhiteB, "= ");
        printInfo(info);
    }

    void TextLogger::enterAsgCopy(TrackedInfo& info)
    {
        printAllign();
        printColor(Color::RedB, "COPY= ");
        printInfo(info);
    }

    void TextLogger::enterAsgMove(TrackedInfo& info)
    {
        printAllign();
        printColor(Color::GreenB, "MOVE= ");
        printInfo(info);
    }

    void TextLogger::enterAsgOper(TrackedInfo& info, std::string const& oper)
    {
        printAllign();
        printColor(Color::WhiteB, oper + "= ");
        printInfo(info);
    }

    void TextLogger::enterDtr(TrackedInfo& info)
    {
        printAllign();
        printColor(Color::BlueB, "DTR ");
        printInfo(info);
    }
}