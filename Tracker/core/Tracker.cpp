/*++

Copyright (c) 2022 JulesIMF, MIPT

Module Name:

    Tracker.cpp

Abstract:

    

Author / Creation date:

    JulesIMF / 24.02.22

Revision History:

--*/


//
// Includes / usings
//

#define TRACKER_CPP
#include <Tracker.h>

//
// Defines
//

namespace Tracker
{
    void Logger::exitFunction()
    {
        calls.pop();
    }

    void Logger::pushFunction(std::string const& name)
    {
        calls.push(name);
    }

    int Logger::depth()
    {
        return calls.size();
    }

    // ----------------------------------------------------

    void MainLogger::addNewLogger(Logger* logger)
    {
        loggers.push_back(logger);
    }

    void MainLogger::setName(TrackedInfo& info, std::string const& name)
    {
        info.id = getId();

        if (name == "")
        {
            info.name = "tmp";
            info.name += std::to_string(++tmpCounter);
            info.isTemp = true;
        }

        else
            info.name = name;
        
        info.function = calls.size() ? calls.top() : "???";
    }

    void MainLogger::setHistory(ModificationType type, TrackedInfo& info, TrackedInfo const* other, std::string const& oper)
    {
        switch (type)
        {
        case ModificationType::Ctr:
            info.history = "<id" + std::to_string(info.id) + "|" + info.value + ">";
            break;
        
        case ModificationType::CtrCopy:
            info.history = "COPY(" + other->name + ", " + other->history + ")";
            break;
        
        case ModificationType::CtrMove:
            info.history = "MOVE(" + other->name + ", " + other->history + ")";
            break;
        
        case ModificationType::Asg:
            info.history = "ASG(" + info.value + ", " + info.history + ")";
            break;

        case ModificationType::AsgCopy:
            info.history = "ASGCOPY(" + other->name + ", " + other->history + ", " + info.history + ")";
            break;

        case ModificationType::AsgMove:
            info.history = "ASGMOVE(" + other->name + ", " + other->history + ", " + info.history + ")";
            break;
        
        case ModificationType::AsgOper:
            info.history = "ASG" + oper + "(" + other->name + ", " + other->history + ", " + info.history + ")";
            break;
        
        default:
            break;
        }
    }

    int MainLogger::getId()
    {
        return currentId++;
    }

    void MainLogger::enterFunction(std::string name)
    {
        Logger::pushFunction(name);
        for (auto logger : loggers)
        {
            logger->enterFunction(name);
        }
    }

    void MainLogger::exitFunction()
    {
        Logger::exitFunction();
        for (auto logger : loggers)
            logger->exitFunction();
    }

    void MainLogger::enterDtr(TrackedInfo const& info)
    {
        for (auto logger : loggers)
            logger->enterDtr(info);
    }

    void MainLogger::enterCtr(TrackedInfo const& info)
    {
        total.obj++;
        for (auto logger : loggers)
            logger->enterCtr(info);
    }

    void MainLogger::enterCtrCopy(TrackedInfo const& infoTo, TrackedInfo const& infoFrom)
    {
        total.obj++;
        total.copy++;
        for (auto logger : loggers)
            logger->enterCtrCopy(infoTo, infoFrom);
    }

    void MainLogger::enterCtrMove(TrackedInfo const& infoTo, TrackedInfo const& infoFrom)
    {
        total.obj++;
        total.move++;
        for (auto logger : loggers)
            logger->enterCtrMove(infoTo, infoFrom);
    }

    void MainLogger::enterAsg(TrackedInfo const& info)
    {
        for (auto logger : loggers)
            logger->enterAsg(info);
    }

    void MainLogger::enterAsgCopy(TrackedInfo const& infoTo, TrackedInfo const& infoFrom)
    {
        total.copy++;
        for (auto logger : loggers)
            logger->enterAsgCopy(infoTo, infoFrom);
    }

    void MainLogger::enterAsgMove(TrackedInfo const& infoTo, TrackedInfo const& infoFrom)
    {
        total.move++;
        for (auto logger : loggers)
            logger->enterAsgMove(infoTo, infoFrom);
    }
    
    void MainLogger::enterAsgOper(TrackedInfo const& infoTo, TrackedInfo const& infoFrom, std::string const& oper) 
    {
        for (auto logger : loggers) 
            logger->enterAsgOper(infoTo, infoFrom, oper);
    }

    MainLogger::~MainLogger()
    {
        for (auto logger : loggers)
            delete logger;
    }

    // ----------------------------------------------------

    MainLogger mainLogger;
}