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

    #define ENTER_CTR(type, other) \
    Tracker::mainLogger.setName(info, name); \
    info.value = std::to_string(value); \
    info.address = this; \
    Tracker::mainLogger.setHistory(Tracker::ModificationType::type, info, other); \
    Tracker::mainLogger.enter##type(info);

    #define ENTER_ASG(type, other) \
    info.value = std::to_string(value); \
    Tracker::mainLogger.setHistory(Tracker::ModificationType::type, info, other); \
    Tracker::mainLogger.enter##type(info);

    // Constructors
    Int(int value = 0, std::string name = "") : 
        value(value)
    {
        ENTER_CTR(Ctr, nullptr)
    }

    Int(Int const& a, std::string name = "") : 
        value(a.value)
    {
        ENTER_CTR(CtrCopy, &a.info)
    }

    Int(Int&& a, std::string name = "") :
        value(a.value)
    {
        ENTER_CTR(CtrMove, &a.info)
    }

    Int& operator=(int a)
    {
        value = a;
        ENTER_ASG(Asg, nullptr)
        return *this;
    }

    Int& operator=(Int const& a)
    {
        value = a.value;
        ENTER_ASG(AsgCopy, &a.info)
        return *this;
    }

    Int& operator=(Int&& a)
    {
        value = a.value;
        ENTER_ASG(AsgMove, &a.info)
        return *this;
    }

    ~Int()
    {
        Tracker::mainLogger.enterDtr(info);
    }

    // Arithmetics
    #define OPER(name) \
    Int& operator name##=(Int const& b) \
    { \
        value name##= b.value; \
        info.value = std::to_string(value); \
        Tracker::mainLogger.setHistory(Tracker::ModificationType::AsgOper, info, &b.info, #name); \
        Tracker::mainLogger.enterAsgOper(info, #name); \
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


    // const Int operator+() const
    // {
    //     return Int(*this);
    // }

    // const Int operator-() const
    // {
    //     return Int(*this) * Int(-1);
    // }

    // Int& operator++()
    // {
    //     value++;
    //     return *this;
    // }

    // Int operator++(int)
    // {
    //     Int copy = *this;
    //     ++(*this);
    //     return copy;
    // }

    // Int& operator--()
    // {
    //     value--;
    //     return *this;
    // }

    // Int operator--(int)
    // {
    //     Int copy = *this;
    //     --(*this);
    //     return copy;
    // }

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
