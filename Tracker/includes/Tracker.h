/*++

Copyright (c) 2022 JulesIMF, MIPT

Module Name:

    Tracker.h

Abstract:

    

Author / Creation date:

    JulesIMF / 23.02.22

Revision History:

--*/

#ifndef TRACKER
#define TRACKER

//
// Includes / usings
//

#include <string>
#include <stack>
#include <vector>

//
// Defines
//

namespace Tracker
{
    enum class ModificationType
    {
        Dtr,
        Ctr,
        CtrCopy,
        CtrMove,
        Asg,
        AsgCopy,
        AsgMove,
        AsgOper,
    };

    struct TrackedInfo
    {
        int id;
        void* address;
        std::string name;
        std::string value;
        std::string history;
        bool isTemp = false;
    };

    struct Logger
    {
        virtual void enterFunction(std::string name) = 0;
        virtual void exitFunction();
        virtual void enterDtr(TrackedInfo& info) = 0;
        virtual void enterCtr(TrackedInfo& info) = 0;
        virtual void enterCtrCopy(TrackedInfo& info) = 0;
        virtual void enterCtrMove(TrackedInfo& info) = 0;
        virtual void enterAsg(TrackedInfo& info) = 0;
        virtual void enterAsgCopy(TrackedInfo& info) = 0;
        virtual void enterAsgMove(TrackedInfo& info) = 0;
        virtual void enterAsgOper(TrackedInfo& info, std::string const& oper) = 0;
        virtual ~Logger() = default;

    protected:
        std::stack<std::string> calls;
        int depth();
        void pushFunction(std::string const& name);
    };

    struct MainLogger : public Logger
    {
        void addNewLogger(Logger* logger);
        void setHistory(ModificationType type, TrackedInfo& info, TrackedInfo const* other = nullptr, std::string const& oper = "");
        void setName(TrackedInfo& info, std::string const& name);

        virtual void enterFunction(std::string name);
        virtual void exitFunction() override;
        virtual void enterDtr(TrackedInfo& info);
        virtual void enterCtr(TrackedInfo& info);
        virtual void enterCtrCopy(TrackedInfo& info);
        virtual void enterCtrMove(TrackedInfo& info);
        virtual void enterAsg(TrackedInfo& info);
        virtual void enterAsgCopy(TrackedInfo& info);
        virtual void enterAsgMove(TrackedInfo& info);
        virtual void enterAsgOper(TrackedInfo& info, std::string const& oper);
        virtual ~MainLogger();

    protected:
        int currentId = 0;

        std::vector<Logger*> loggers;
        int getId();
    };

    struct TextLogger : public Logger
    {
        enum class Color
        {
            Default,
            BlackB,
            RedB,
            GreenB,
            YellowB,
            BlueB,
            PurpleB,
            CyanB,
            WhiteB,
        };

        virtual void enterFunction(std::string name);
        virtual void exitFunction() override;
        virtual void enterDtr(TrackedInfo& info);
        virtual void enterCtr(TrackedInfo& info);
        virtual void enterCtrCopy(TrackedInfo& info);
        virtual void enterCtrMove(TrackedInfo& info);
        virtual void enterAsg(TrackedInfo& info);
        virtual void enterAsgCopy(TrackedInfo& info);
        virtual void enterAsgMove(TrackedInfo& info);
        virtual void enterAsgOper(TrackedInfo& info, std::string const& oper);

    protected:
        virtual void printInfo(TrackedInfo const& info);
        virtual int printAllign() = 0;
        virtual void printColor(Color color, std::string const& str) = 0;
    };

    struct ConsoleLogger : public Logger
    {
        static int const width = 8;

    protected:
        virtual int printAllign();
        virtual void printColor(char const* str);
    }

    struct DotLogger : public Logger
    {
        static int const width = 8;
        virtual void enterFunction(std::string name);
        virtual void exitFunction() override;
        virtual void enterDtr(TrackedInfo& info);
        virtual void enterCtr(TrackedInfo& info);
        virtual void enterCtrCopy(TrackedInfo& info);
        virtual void enterCtrMove(TrackedInfo& info);
        virtual void enterAsg(TrackedInfo& info);
        virtual void enterAsgCopy(TrackedInfo& info);
        virtual void enterAsgMove(TrackedInfo& info);
        virtual void enterAsgOper(TrackedInfo& info, std::string const& oper);

    protected:
        int printfAlligned(char const* format, ...);
    };

    extern MainLogger mainLogger;

    struct FncEnvoy
    {
        FncEnvoy(std::string name)
        {
            mainLogger.enterFunction(name);
        }

        ~FncEnvoy()
        {
            mainLogger.exitFunction();
        }
    };
}

#define TRACKER_ENTER Tracker::FncEnvoy __envoy(__PRETTY_FUNCTION__)
#define TRACKER_CREATE(type, name, init) type name(init, #name)

#endif