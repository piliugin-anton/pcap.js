#include "enums.h"

void InitializeEnums(const Napi::Env &env, Napi::Object &exports) {
  Napi::Object enums = Napi::Object::New(env);

  Napi::Object devFlags = Napi::Object::New(env);
  devFlags.Set("LOOPBACK", static_cast<int>(PCAP_IF_LOOPBACK));
  devFlags.Set("UP", static_cast<int>(PCAP_IF_UP));
  devFlags.Set("RUNNING", static_cast<int>(PCAP_IF_RUNNING));
  devFlags.Set("WIRELESS", static_cast<int>(PCAP_IF_WIRELESS));
  devFlags.Set("CONNECTION_UKNOWN", static_cast<int>(PCAP_IF_CONNECTION_STATUS_UNKNOWN));
  devFlags.Set("CONNECTED", static_cast<int>(PCAP_IF_CONNECTION_STATUS_CONNECTED));
  devFlags.Set("DISCONNECTED", static_cast<int>(PCAP_IF_CONNECTION_STATUS_DISCONNECTED));
  devFlags.Set("CONNECTION_NA", static_cast<int>(PCAP_IF_CONNECTION_STATUS_NOT_APPLICABLE));
  enums.Set("DEV", devFlags);

  Napi::Object direction = Napi::Object::New(env);
  direction.Set("IN", static_cast<int>(PCapEnums::PCapDirection::IN));
  direction.Set("OUT", static_cast<int>(PCapEnums::PCapDirection::OUT));
  direction.Set("BOTH", static_cast<int>(PCapEnums::PCapDirection::INOUT));
  enums.Set("DIRECTION", direction);

  exports.Set("CONSTANTS", enums);
}

namespace PCapEnums {
  pcap_direction_t directionTypeMap(int32_t direction) {
	  switch (direction) {
		  case PCapDirection::IN:    return PCAP_D_IN;
		  case PCapDirection::OUT:   return PCAP_D_OUT;
		  case PCapDirection::INOUT: return PCAP_D_INOUT;
      default: return PCAP_D_INOUT;
	  }
  }
}