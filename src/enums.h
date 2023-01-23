#ifndef NODE_PCAPJS_ENUMS_H
#define NODE_PCAPJS_ENUMS_H

#include "util.h"

void InitializeEnums(const Napi::Env &env, Napi::Object &exports);

namespace PCapEnums {
    enum PCapDirection {
	    /** Capture traffics both incoming and outgoing */
	    INOUT = 0,
	    /** Only capture incoming traffics */
	    IN,
	    /** Only capture outgoing traffics */
	    OUT
    };
    pcap_direction_t directionTypeMap(int32_t direction);
}

#endif
