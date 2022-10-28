/*++

Copyright (c) 2022 JulesIMF, MIPT

Module Name:

    Html.cpp

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
#include <stdexcept>
#include <chrono>
#include <ctime>

//
// Defines
//

namespace Tracker
{
    HtmlLogger::HtmlLogger()
    {
        file = fopen(filename, "w");
        if (file == nullptr)
            throw std::runtime_error(std::string("cant open \"") + filename + "\"");
        
        fprintf(file, "<pre style = \"background-color: #1e1e1e; color: #FFFFFF\"><span class=\"inner-pre\" style=\"font-size: %dpx\">\n", fontSize);
        printColor(Color::RedB, "Tracker log ");
        auto currentTime = time(NULL);
        printColor(Color::Default, "generated on " + std::string(std::ctime(&currentTime)) + "\n\n");
    }

    HtmlLogger::~HtmlLogger()
    {
        auto total = mainLogger.getTotal();
        fprintf(file, "\n\n%d object%s created, %d copied, %d moved\n", total.obj, (total.obj != 1 ? "s" : ""), total.copy, total.move);
        fprintf(file, "</span></pre>");
        fclose(file);
    }

    void HtmlLogger::printAllign()
    {
        // static int n = 0;
        // fprintf(file, "%d ", ++n);
        static int const tabsize = 4;
        fprintf(file, "%*s", depth() * tabsize, "");
    }

    void HtmlLogger::printColor(TextLogger::Color color, std::string const& str)
    {
        switch (color)
        {
        case TextLogger::Color::BlackB:
            fprintf(file, "<font color=#%s>", "666666");
            fprintf(file, "%s</font></b>", str.c_str());
            break;
        
        case TextLogger::Color::RedB:
            fprintf(file, "<b><font color=#%s>", "f14c4c");
            fprintf(file, "%s</font></b>", str.c_str());
            break;
        
        case TextLogger::Color::GreenB:
            fprintf(file, "<b><font color=#%s>", "23d18b");
            fprintf(file, "%s</font></b>", str.c_str());
            break;

        case TextLogger::Color::YellowB:
            fprintf(file, "<b><font color=#%s>", "f5f543");
            fprintf(file, "%s</font></b>", str.c_str());
            break;
        
        case TextLogger::Color::BlueB:
            fprintf(file, "<b><font color=#%s>", "3b8eea");
            fprintf(file, "%s</font></b>", str.c_str());
            break;
        
        case TextLogger::Color::PurpleB:
            fprintf(file, "<b><font color=#%s>", "d670d6");
            fprintf(file, "%s</font></b>", str.c_str());
            break;
        
        case TextLogger::Color::CyanB:
            fprintf(file, "<b><font color=#%s>", "28b5d8");
            fprintf(file, "%s</font></b>", str.c_str());
            break;

        case TextLogger::Color::WhiteB:
            fprintf(file, "<b><font color=#%s>", "e5e5e5");
            fprintf(file, "%s</font></b>", str.c_str());
            break;

        default:
            fprintf(file, "%s", str.c_str());
        }
    }
}