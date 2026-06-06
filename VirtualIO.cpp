#include "VirtualIO.h"

VirtualIO::VirtualIO()              : VirtualIO(true) {}
VirtualIO::VirtualIO(bool initial)  { pin = 0; pinState = initial; changed = false; }

void VirtualIO::init(int /*dir*/, int level) {
    pinState = level;
    changed  = false;
}

bool VirtualIO::getValue() {
    return pinState;
}

void VirtualIO::setValue(bool val) {
    if (val != pinState) { pinState = val; changed = true; }
}
