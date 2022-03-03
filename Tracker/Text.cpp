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
        // //  "tmp1" (id: 01, val: 108, addr: 0xb2d16f60) ASG+(lol, <id0|54>, COPY(lol, <id0|54>))
        // // printf("%s\"%s\"%s "
        // //        "(%sid:%s %02d, "
        // //        "%sval:%s %s, "
        // //        "%saddr:%s %08x)"
        // //        "%s%s%s"
        // //        "\n",
        // //        TerminalColor::BlueB, info.name.c_str(), TerminalColor::Default,
        // //        TerminalColor::PurpleB, TerminalColor::Default, info.id, 
        // //        TerminalColor::PurpleB, TerminalColor::Default, info.value.c_str(),
        // //        TerminalColor::PurpleB, TerminalColor::Default, info.address,
        // //        TerminalColor::Cyan, TerminalColor::Default, info.history.c_str());

        // printf("%s\"%s\"%s "
        //     "(id:%s %02d%s, "
        //     "val:%s %s%s, "
        //     "addr:%s 0x%08x%s) "
        //     //"%s%s%s"
        //     "\n",
        //     TerminalColor::YellowB, info.name.c_str(), TerminalColor::Default,
        //     TerminalColor::PurpleB, info.id, TerminalColor::Default,
        //     TerminalColor::PurpleB, info.value.c_str(), TerminalColor::Default,
        //     TerminalColor::PurpleB, info.address, TerminalColor::Default,
        //     TerminalColor::Cyan, info.history.c_str(), TerminalColor::Default);
        
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
        printfAlligned("%s%s%s ", TerminalColor::WhiteB, "CTR", TerminalColor::Default);
        printInfo(info);
    }

    void TextLogger::enterCtrCopy(TrackedInfo& info)
    {
        printfAlligned("%s%s %s", TerminalColor::RedB, "COPY", TerminalColor::Default);
        printInfo(info);
    }

    void TextLogger::enterCtrMove(TrackedInfo& info)
    {
        printfAlligned("%s%s %s", TerminalColor::GreenB, "MOVE", TerminalColor::Default);
        printInfo(info);
    }

    void TextLogger::enterAsg(TrackedInfo& info)
    {
        printfAlligned("%s%s%s ", TerminalColor::WhiteB, "=", TerminalColor::Default);
        printInfo(info);
    }

    void TextLogger::enterAsgCopy(TrackedInfo& info)
    {
        printfAlligned("%s%s %s", TerminalColor::RedB, "COPY=", TerminalColor::Default);
        printInfo(info);
    }

    void TextLogger::enterAsgMove(TrackedInfo& info)
    {
        printfAlligned("%s%s %s", TerminalColor::GreenB, "MOVE=", TerminalColor::Default);
        printInfo(info);
    }

    void TextLogger::enterAsgOper(TrackedInfo& info, std::string const& oper)
    {
        printfAlligned("%s%s%s%s ", TerminalColor::WhiteB, oper.c_str(), "=", TerminalColor::Default);
        printInfo(info);
    }

    void TextLogger::enterDtr(TrackedInfo& info)
    {
        printfAlligned("%s%s%s ", TerminalColor::BlueB, "DTR", TerminalColor::Default);
        printInfo(info);
    }
}