#pragma once

#include "Game/MapObj/BenefitItemObj.hpp"

class BenefitItemInvincible : public BenefitItemObj {
public:
    BenefitItemInvincible(const char*);

    virtual ~BenefitItemInvincible();
    virtual void init(const JMapInfoIter&);
    virtual void appearThrowUp();
    virtual void exeCatch();
    virtual void initEscape();
    virtual void doEscape();
    virtual bool attachSupportTico();

    /* 0x154 */ u16 _13A;
    /* 0x158 */ TVec3f mUpAxis;
    /* 0x164 */ TVec3f mFwdAxis;
};
