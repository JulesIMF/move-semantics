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
        case ModificationType::CTOR:
            info.history = "<id" + std::to_string(info.id) + "|" + info.value + ">";
            break;
        
        case ModificationType::CTORCopy:
            info.history = "COPY(" + other->name + ", " + other->history + ")";
            break;
        
        case ModificationType::CTORMove:
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

    void MainLogger::on()
    {
        isOn = true;
    }

    void MainLogger::off()
    {
        isOn = false;
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

    void MainLogger::enterDTOR(TrackedInfo const& info)
    {
        if (!isOn) return;
        if (!isOn) return;
        for (auto logger : loggers)
            logger->enterDTOR(info);
    }

    void MainLogger::enterCTOR(TrackedInfo const& info)
    {
        if (!isOn) return;
        total.obj++;
        for (auto logger : loggers)
            logger->enterCTOR(info);
    }

    void MainLogger::enterCTORCopy(TrackedInfo const& infoTo, TrackedInfo const& infoFrom)
    {
        if (!isOn) return;
        total.obj++;
        total.copy++;
        for (auto logger : loggers)
            logger->enterCTORCopy(infoTo, infoFrom);
    }

    void MainLogger::enterCTORMove(TrackedInfo const& infoTo, TrackedInfo const& infoFrom)
    {
        if (!isOn) return;
        total.obj++;
        total.move++;
        for (auto logger : loggers)
            logger->enterCTORMove(infoTo, infoFrom);
    }

    void MainLogger::enterAsg(TrackedInfo const& info)
    {
        if (!isOn) return;
        for (auto logger : loggers)
            logger->enterAsg(info);
    }

    void MainLogger::enterAsgCopy(TrackedInfo const& infoTo, TrackedInfo const& infoFrom)
    {
        if (!isOn) return;
        total.copy++;
        for (auto logger : loggers)
            logger->enterAsgCopy(infoTo, infoFrom);
    }

    void MainLogger::enterAsgMove(TrackedInfo const& infoTo, TrackedInfo const& infoFrom)
    {
        if (!isOn) return;
        total.move++;
        for (auto logger : loggers)
            logger->enterAsgMove(infoTo, infoFrom);
    }
    
    void MainLogger::enterAsgOper(TrackedInfo const& infoTo, TrackedInfo const& infoFrom, std::string const& oper) 
    {
        if (!isOn) return;
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