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

#include <Tracker.h>

//
// Defines
//

namespace Tracker
{
    void TextLogger::enterFunction(std::string name)
    {
        printAllign();
        printColor(Color::Default, name + "\n");
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
    }

    void TextLogger::enterCTOR(TrackedInfo const& info)
    {
        printAllign();
        printColor(Color::WhiteB, "CTOR ");
        printInfo(info);
        printColor(Color::Default, "\n");
    }

    void TextLogger::enterCTORCopy(TrackedInfo const& infoTo, TrackedInfo const& infoFrom)
    {
        printAllign();
        printColor(Color::RedB, "COPY ");
        printInfo(infoTo);
        printColor(Color::Default, " from ");
        printInfo(infoFrom);
        printColor(Color::Default, "\n");
    }

    void TextLogger::enterCTORMove(TrackedInfo const& infoTo, TrackedInfo const& infoFrom)
    {
        printAllign();
        printColor(Color::GreenB, "MOVE ");
        printInfo(infoTo);
        printColor(Color::Default, " from ");
        printInfo(infoFrom);
        printColor(Color::Default, "\n");
    }

    void TextLogger::enterAsg(TrackedInfo const& info)
    {
        printAllign();
        printColor(Color::WhiteB, "= ");
        printInfo(info);
        printColor(Color::Default, "\n");
    }

    void TextLogger::enterAsgCopy(TrackedInfo const& infoTo, TrackedInfo const& infoFrom)
    {
        printAllign();
        printColor(Color::RedB, "COPY= ");
        printInfo(infoTo);
        printColor(Color::Default, " from ");
        printInfo(infoFrom);
        printColor(Color::Default, "\n");
    }

    void TextLogger::enterAsgMove(TrackedInfo const& infoTo, TrackedInfo const& infoFrom)
    {
        printAllign();
        printColor(Color::GreenB, "MOVE= ");
        printInfo(infoTo);
        printColor(Color::Default, " from ");
        printInfo(infoFrom);
        printColor(Color::Default, "\n");
    }

    void TextLogger::enterAsgOper(TrackedInfo const& infoTo, TrackedInfo const& infoFrom, std::string const& oper)
    {
        printAllign();
        printColor(Color::WhiteB, oper + "= ");
        printInfo(infoTo);
        printColor(Color::Default, " from ");
        printInfo(infoFrom);
        printColor(Color::Default, "\n");
    }

    void TextLogger::enterDTOR(TrackedInfo const& info)
    {
        printAllign();
        printColor(Color::BlueB, "DTOR ");
        printInfo(info);
        printColor(Color::Default, "\n");
    }
}