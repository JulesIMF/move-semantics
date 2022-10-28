/*++

Copyright (c) 2022 JulesIMF, MIPT

Module Name:

    Int.h

Abstract:

    Int abstraction class

Author / Creation date:

    JulesIMF / 04.02.22

Revision History:

--*/


//
// Includes / usings
//

#include <Tracker.h>

//
// Defines
//

struct Int
{
public:
    // Get
    int get() const
    {
        return value;
    }

    #define ENTER_CTOR(type, other) \
    Tracker::mainLogger.setName(info, name); \
    info.value = std::to_string(value); \
    info.address = this; \
    Tracker::mainLogger.setHistory(Tracker::ModificationType::type, info, other);

    #define ENTER_ASG(type, other) \
    info.value = std::to_string(value); \
    Tracker::mainLogger.setHistory(Tracker::ModificationType::type, info, other);

    // Constructors
    Int(int value = 0, std::string name = "") : 
        value(value)
    {
        ENTER_CTOR(CTOR, nullptr)
        Tracker::mainLogger.enterCTOR(info);
    }

    Int(Int const& a, std::string name = "") : 
        value(a.value)
    {
        ENTER_CTOR(CTORCopy, &a.info)
        Tracker::mainLogger.enterCTORCopy(info, a.info);
    }
#ifndef INT_NO_MOVE
    Int(Int&& a, std::string name = "") :
        value(a.value)
    {
        ENTER_CTOR(CTORMove, &a.info)
        Tracker::mainLogger.enterCTORMove(info, a.info);
    }
#endif

    Int& operator=(int a)
    {
        value = a;
        ENTER_ASG(Asg, nullptr)
        Tracker::mainLogger.enterAsg(info);
        return *this;
    }

    Int& operator=(Int const& a)
    {
        value = a.value;
        ENTER_ASG(AsgCopy, &a.info)
        Tracker::mainLogger.enterAsgCopy(info, a.info);
        return *this;
    }
#ifndef INT_NO_MOVE
    Int& operator=(Int&& a)
    {
        value = a.value;
        ENTER_ASG(AsgMove, &a.info)
        Tracker::mainLogger.enterAsgMove(info, a.info);
        return *this;
    }
#endif

    ~Int()
    {
        Tracker::mainLogger.enterDTOR(info);
    }

    // Arithmetics
    #define OPER(name) \
    Int& operator name##=(Int const& b) \
    { \
        value name##= b.value; \
        info.value = std::to_string(value); \
        Tracker::mainLogger.setHistory(Tracker::ModificationType::AsgOper, info, &b.info, #name); \
        Tracker::mainLogger.enterAsgOper(info, b.info, #name); \
        return *this; \
    } \
    const Int operator name(Int const& b) const \
    { \
        TRACKER_ENTER; \
        return Int(*this) name##= b; \
    }

    OPER(+);
    OPER(-);
    OPER(*);
    OPER(/);
    OPER(|);
    OPER(&);
    OPER(^);
    OPER(%);
    OPER(>>);
    OPER(<<);


    const Int operator+() const
    {
        TRACKER_ENTER;
        return Int(*this);
    }

    const Int operator-() const
    {
        TRACKER_ENTER;
        return Int(*this) * Int(-1);
    }

    Int& operator++()
    {
        TRACKER_ENTER;
        value++;
        return *this;
    }

    Int operator++(int)
    {
        TRACKER_ENTER;
        TRACKER_CREATE(Int, currentValueCopy, *this);
        ++(*this);
        return currentValueCopy;
    }

    Int& operator--()
    {
        TRACKER_ENTER;
        value--;
        return *this;
    }

    Int operator--(int)
    {
        TRACKER_ENTER;
        TRACKER_CREATE(Int, currentValueCopy, *this);
        --(*this);
        return currentValueCopy;
    }

    // Cast 
    operator int() const 
    {
        return value;
    }

    // Comparators
    bool operator==(Int const& b)
    {
        return value == b.value;
    }

    bool operator<(Int const& b)
    {
        return value < b.value;
    }

    bool operator>(Int const& b)
    {
        return !(*this < b || *this == b);
    }

    bool operator<=(Int const& b)
    {
        return *this < b || *this == b;
    }

    bool operator>=(Int const& b)
    {
        return !(*this < b);
    }

    bool operator!=(Int const& b)
    {
        return !(*this == b);
    }


protected:
    int value;
    Tracker::TrackedInfo info;
};
