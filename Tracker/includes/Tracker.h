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
#include <map>
#include <set>

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
        std::string function;
        bool isTemp = false;
    };

    struct Logger
    {
        virtual void enterFunction(std::string name) = 0;
        virtual void exitFunction();
        virtual void enterDtr(TrackedInfo const& info) = 0;
        virtual void enterCtr(TrackedInfo const& info) = 0;
        virtual void enterCtrCopy(TrackedInfo const& infoTo, TrackedInfo const& infoFrom) = 0;
        virtual void enterCtrMove(TrackedInfo const& infoTo, TrackedInfo const& infoFrom) = 0;
        virtual void enterAsg(TrackedInfo const& info) = 0;
        virtual void enterAsgCopy(TrackedInfo const& infoTo, TrackedInfo const& infoFrom) = 0;
        virtual void enterAsgMove(TrackedInfo const& infoTo, TrackedInfo const& infoFrom) = 0;
        virtual void enterAsgOper(TrackedInfo const& infoTo, TrackedInfo const& infoFrom, std::string const& oper) = 0;
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
        virtual void enterDtr(TrackedInfo const& info);
        virtual void enterCtr(TrackedInfo const& info);
        virtual void enterCtrCopy(TrackedInfo const& infoTo, TrackedInfo const& infoFrom);
        virtual void enterCtrMove(TrackedInfo const& infoTo, TrackedInfo const& infoFrom);
        virtual void enterAsg(TrackedInfo const& info);
        virtual void enterAsgCopy(TrackedInfo const& infoTo, TrackedInfo const& infoFrom);
        virtual void enterAsgMove(TrackedInfo const& infoTo, TrackedInfo const& infoFrom);
        virtual void enterAsgOper(TrackedInfo const& infoTo, TrackedInfo const& infoFrom, std::string const& oper);
        virtual ~MainLogger();

    protected:
        int currentId = 0;
        int tmpCounter = 0;
        struct
        {
            int obj = 0;
            int copy = 0;
            int move = 0;
        } total;
        

        std::vector<Logger*> loggers;
        int getId();
    
    public:
        decltype(total) getTotal() { return total; }
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
        virtual void enterDtr(TrackedInfo const& info);
        virtual void enterCtr(TrackedInfo const& info);
        virtual void enterCtrCopy(TrackedInfo const& infoTo, TrackedInfo const& infoFrom);
        virtual void enterCtrMove(TrackedInfo const& infoTo, TrackedInfo const& infoFrom);
        virtual void enterAsg(TrackedInfo const& info);
        virtual void enterAsgCopy(TrackedInfo const& infoTo, TrackedInfo const& infoFrom);
        virtual void enterAsgMove(TrackedInfo const& infoTo, TrackedInfo const& infoFrom);
        virtual void enterAsgOper(TrackedInfo const& infoTo, TrackedInfo const& infoFrom, std::string const& oper);

    protected:
        virtual void printInfo(TrackedInfo const& info);
        virtual void printAllign() = 0;
        virtual void printColor(Color color, std::string const& str) = 0;
    };

    struct ConsoleLogger : public TextLogger
    {
        static int const width = 8;
        virtual ~ConsoleLogger();

    protected:
        virtual void printAllign();
        virtual void printColor(TextLogger::Color color, std::string const& str);
    };

    struct HtmlLogger : public TextLogger
    {
        static int const width = 8;
        static int const fontSize = 14;
        HtmlLogger();
        virtual ~HtmlLogger();

    protected:
        virtual void printAllign();
        virtual void printColor(TextLogger::Color color, std::string const& str);
        char const* filename = "trackerlog.html";
        FILE* file;
    };

    struct DotLogger : public Logger
    {
        DotLogger();
        virtual ~DotLogger();

        virtual void enterFunction(std::string name);
        virtual void exitFunction() override;
        virtual void enterDtr(TrackedInfo const& info);
        virtual void enterCtr(TrackedInfo const& info);
        virtual void enterCtrCopy(TrackedInfo const& infoTo, TrackedInfo const& infoFrom);
        virtual void enterCtrMove(TrackedInfo const& infoTo, TrackedInfo const& infoFrom);
        virtual void enterAsg(TrackedInfo const& info);
        virtual void enterAsgCopy(TrackedInfo const& infoTo, TrackedInfo const& infoFrom);
        virtual void enterAsgMove(TrackedInfo const& infoTo, TrackedInfo const& infoFrom);
        virtual void enterAsgOper(TrackedInfo const& infoTo, TrackedInfo const& infoFrom, std::string const& oper);

    protected:
        char const* nodesFilename = "dotfiles/trackerlog.nodes.dot";
        char const* linksFilename = "dotfiles/trackerlog.links.dot";
        char const* finalFilename  ="dotfiles/trackedlog.dot";
        char const* imageFilename = "trackedlog.png";
        FILE* nodes; // goes 1st
        FILE* links; // goes 2nd
        

        // Dot
        enum class LinkType
        {
            Exec,
            Dtr,
            Copy,
            Move,
            Asg,
        };

        struct Node
        {
            int id;
            int index;
        };

        struct FileEntry
        {
            FILE* file;
            bool isResolved = true;
            std::string content;
        };

        int hypergraphs = 0;
        std::map<int, int> nodeById;
        // std::map<int, std::map<std::string, int>> operById;
        int nOpers = 0;
        std::map<std::string, std::string> entryContentByName;
        std::vector<FileEntry> entriesFlow;
        Node last = { -1, -1 };

        Node allocNode(int id);
        Node getAsgNode(int id, std::string const& oper);
        Node currentNode(int id);
        void logInfo(TrackedInfo const& info, Node node, std::string const& reason, char const* color = "000000");
        void link(Node from, Node to, LinkType type);
        void linkExec(Node to);
        void setLast(Node last);
        void printNodeName(FILE* file, Node node);
        void endPrintNode();
        void pushEntry(FILE* file, std::string const& entry);
        void setEntryContent(std::string const& entryName, char const* fmt, ...);
        void write(FILE* file, char const* fmt, ...);
        void flushEntries();
        void message(char const* fmt, ...);
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

#define TRACKER_DEFAULT_INITIALIZATION  Tracker::mainLogger.addNewLogger(new Tracker::ConsoleLogger); \
                                        Tracker::mainLogger.addNewLogger(new Tracker::HtmlLogger); \
                                        Tracker::mainLogger.addNewLogger(new Tracker::DotLogger)
#define TRACKER_ENTER Tracker::FncEnvoy __envoy(__PRETTY_FUNCTION__)
#define TRACKER_CREATE(type, name, init) type name(init, #name)

#endif