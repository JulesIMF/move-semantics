/*++

Copyright (c) 2022 JulesIMF, MIPT

Module Name:

    Dot.cpp

Abstract:



Author / Creation date:

    JulesIMF / 24.02.22

Revision History:

--*/


//
// Includes / usings
//

#include <Tracker.h>
#include <cassert>
#include <cstdarg>
#include <stdexcept>
#include <Colors.h>

//
// Defines
//

namespace Tracker
{
    void DotLogger::enterFunction(std::string name)
    {
        write(nodes, "subgraph cluster_%d {\n", hypergraphs++);
        write(nodes, "label=\"%s\"\n", name.c_str());

        int const step = 20;
        int color = 0xFF - ((depth() + 1) * step);
        write(nodes, "style=filled; color=\"#%2x%2x%2x\"\n", color, color, color);
        endPrintNode();
        pushFunction(name);
    }

    void DotLogger::exitFunction()
    {
        write(nodes, "}\n");
        Logger::exitFunction();
    }

    void DotLogger::enterDtr(TrackedInfo const& info)
    {
        auto node = allocNode(info.id);
        linkExec(node);
        assert(node.index);
        printNodeName(nodes, node);
        write(nodes, "[label = \"DTR \\\"%s\\\"\", "
                       "shape = \"diamond\", "
                       "style = \"filled, bold\", "
                       "fillcolor = \"#3b8eea\", "
                       "fontsize = 15]\n",
                       info.name.c_str());
        endPrintNode();

        link({ info.id, 0 }, node, LinkType::Dtr);
    }

    void DotLogger::enterCtr(TrackedInfo const& info)
    {
        auto node = allocNode(info.id);
        logInfo(info, currentNode(info.id), "CTR");
        linkExec(node);
        assert(!node.index);
    }

    void DotLogger::enterCtrCopy(TrackedInfo const& infoTo, TrackedInfo const& infoFrom)
    {
        auto node = allocNode(infoTo.id);
        logInfo(infoTo, currentNode(infoTo.id), "COPY", "f14c4c");
        linkExec(node);
        assert(!node.index);
        link(currentNode(infoFrom.id), node, LinkType::Copy);
    }

    void DotLogger::enterCtrMove(TrackedInfo const& infoTo, TrackedInfo const& infoFrom)
    {
        auto node = allocNode(infoTo.id);
        logInfo(infoTo, currentNode(infoTo.id), "MOVE", "23d18b");
        linkExec(node);
        assert(!node.index);
        link(currentNode(infoFrom.id), node, LinkType::Move);
    }

    void DotLogger::enterAsg(TrackedInfo const& info)
    {
        auto node = allocNode(info.id);
        logInfo(info, currentNode(info.id), "Literal assign");
        linkExec(node);
    }

    void DotLogger::enterAsgCopy(TrackedInfo const& infoTo, TrackedInfo const& infoFrom)
    {
        auto node = allocNode(infoTo.id);
        logInfo(infoTo, currentNode(infoTo.id), "COPY assign", "f14c4c");
        linkExec(node);
        link(currentNode(infoFrom.id), node, LinkType::Copy);
    }

    void DotLogger::enterAsgMove(TrackedInfo const& infoTo, TrackedInfo const& infoFrom)
    {
        auto node = allocNode(infoTo.id);
        logInfo(infoTo, currentNode(infoTo.id), "MOVE assign", "23d18b");
        linkExec(node);
        link(currentNode(infoFrom.id), node, LinkType::Move);
    }

    void DotLogger::enterAsgOper(TrackedInfo const& infoTo, TrackedInfo const& infoFrom, std::string const& oper)
    {
        auto node = allocNode(infoTo.id);
        logInfo(infoTo, currentNode(infoTo.id), oper + "=");
        linkExec(node);
        link(currentNode(infoFrom.id), node, LinkType::Asg);
    }

    DotLogger::Node DotLogger::allocNode(int id)
    {
        if (id < 0)
            return { -1, -1 };

        return { id, nodeById[id]++ };
    }

    DotLogger::Node DotLogger::getAsgNode(int id, std::string const& oper)
    {
        int index = ++nOpers;
        printNodeName(nodes, { id, -index });

        write(nodes, "[shape=rectangle, style=filled, label=\"%s=\", fillcolor=\"mediumspringgreen\"]\n", oper.c_str());
        endPrintNode();

        return { id, -index };
    }

    DotLogger::Node DotLogger::currentNode(int id)
    {
        int index = nodeById[id] - 1;
        return { id, index };
    }

    void DotLogger::logInfo(TrackedInfo const& info, Node node, std::string const& reason, char const* color)
    {
        printNodeName(nodes, node);
        write(nodes,  "[shape=none, label=<"
                        "<TABLE BORDER=\"2\">\n"
                        "<TR>\n"
                        "<TD COLSPAN = \"3\"><b><FONT COLOR=\"#707009\">\"%s\"</FONT></b></TD>\n"
                        "</TR>\n"
                        "<TR>\n"
                        "<TD COLSPAN = \"3\">Reason: <b><FONT COLOR=\"#%s\">%s</FONT></b></TD>\n"
                        "</TR>\n"
                        "<TR>\n"
                        "<TD>id: <b><FONT COLOR=\"#d670d6\">\"%d\"</FONT></b></TD>\n"
                        "<TD>val: <b><FONT COLOR=\"#d670d6\">\"%s\"</FONT></b></TD>\n"
                        "<TD>addr: <b><FONT COLOR=\"#d670d6\">\"%x\"</FONT></b></TD>\n"
                        "</TR>\n"
                        "</TABLE>\n"
                        ">];\n\n",
                        
                        info.name.c_str(),
                        color,
                        reason.c_str(),
                        info.id,
                        info.value.c_str(),
                        info.address);
        
        endPrintNode();
    }

    void DotLogger::link(Node from, Node to, LinkType type)
    {
        printNodeName(links, from);
        write(links, " -> ");
        printNodeName(links, to);
        switch (type)
        {
        case LinkType::Asg:
            write(links, "[color=black]");
            break;
        
        case LinkType::Copy:
            write(links, "[color=\"#f14c4c\"]");
            break;
        
        case LinkType::Move:
            write(links, "[color=\"#23d18b\"]");
            break;
        
        case LinkType::Exec:
            write(links, "[weight=100, style=dashed, color=\"#d670d6\"]");
            break;
        
        case LinkType::Dtr:
            write(links, "[style=dashed, color=\"#3b8eea\"]");
            break;
        
        default:
            assert(!"unknown type");
            break;
        }

        write(links, ";\n");
    }

    void DotLogger::linkExec(Node to)
    {
        if (last.id != -1)
            link(last, to, LinkType::Exec);       
        
        setLast(to);
    }

    void DotLogger::setLast(Node to)
    {
        last = to;
    }

    void DotLogger::printNodeName(FILE* file, Node node)
    {
        if (node.id < 0)
            return;
        
        if (node.index < 0)
        {
            node.index *= -1;
            write(file, "asg_id_%d_operindex_%d", node.id, node.index);
        }

        else
        {
            write(file, "node_id_%d_index_%d", node.id, node.index);
        }
    }

    DotLogger::DotLogger()
    {
        links = fopen(linksFilename, "w");
        if (links == nullptr)
            throw std::runtime_error(std::string("cant open \"") + linksFilename + "\"");

        nodes = fopen(nodesFilename, "w");
        if (nodes == nullptr)
            throw std::runtime_error(std::string("cant open \"") + nodesFilename + "\"");

        write(nodes, "digraph\n"
                       "{\n"
                       "dpi = 400;\n");
    }

    DotLogger::~DotLogger()
    {
        write(links, "}\n");
        flushEntries();
        fclose(nodes);
        fclose(links);
        char request[1024];

        snprintf(request, sizeof(request) - 1, "cat %s %s > %s", nodesFilename, linksFilename, finalFilename);
        system(request);

        message("rendering graph log...\n");
        snprintf(request, sizeof(request) - 1, "dot -Tpng %s -o %s 2> dotfiles/dotlog.txt", finalFilename, imageFilename);
        system(request);
        message("graph log saved to %s\n", imageFilename);
    }

    void DotLogger::endPrintNode()
    {
        write(nodes, "\n"
                       "// ---------------------------------------------------"
                       "--------------------------\n");
    }

    void DotLogger::write(FILE* file, char const* fmt, ...)
    {
        static char buf[1024];
        va_list va;
        va_start(va, fmt);
        vsnprintf(buf, sizeof(buf) - 1, fmt, va);
        va_end(va);
        entriesFlow.push_back(FileEntry{ .file = file,
                                       .isResolved = true,
                                       .content = buf });
    }

    void DotLogger::setEntryContent(std::string const& entryName, char const* fmt, ...)
    {
        static char buf[1024];
        va_list va;
        va_start(va, fmt);
        vsnprintf(buf, sizeof(buf) - 1, fmt, va);
        va_end(va);
        entryContentByName[entryName] = buf;
    }

    void DotLogger::pushEntry(FILE* file, std::string const& entryName)
    {
        entriesFlow.push_back(FileEntry{ .file = file,
                                         .isResolved = false,
                                         .content = entryName });
    }

    void DotLogger::flushEntries()
    {
        for (auto const& entry : entriesFlow)
        {
            if (entry.isResolved)
                fprintf(entry.file, "%s", entry.content.c_str());
            
            else
                fprintf(entry.file, "%s", entryContentByName[entry.content].c_str());
        }

        entriesFlow.clear();
        entryContentByName.clear();
    }

    void DotLogger::message(char const* fmt, ...)
    {
        fprintf(stderr, "%sDotLogger: %s", TerminalColor::PurpleB, TerminalColor::Default);
        va_list va;
        va_start(va, fmt);
        vfprintf(stderr, fmt, va);
        va_end(va);
    }
}