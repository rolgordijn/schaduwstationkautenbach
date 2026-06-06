#pragma once
#include "IO.h"

// Software-only IO — no hardware. Used as test doubles and (future) as the
// web-side leaf in InputOrIO / OutputOrIO.
class VirtualIO : public IO {
public:
    VirtualIO();
    explicit VirtualIO(bool initialState);

    void init(int dir, int level) override;
    bool getValue() override;
    void setValue(bool val) override;
    void setInput()         override {}
    void setOutput()        override {}
    void setPinMode(int)    override {}
};
