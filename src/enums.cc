#include "enums.h"

void InitializeEnums(const Napi::Env &env, Napi::Object &exports) {
  exports.Set("DEV_LOOPBACK", static_cast<unsigned long>(PCAP_IF_LOOPBACK));
  exports.Set("DEV_UP", static_cast<unsigned long>(PCAP_IF_UP));
  exports.Set("DEV_RUNNING", static_cast<unsigned long>(PCAP_IF_RUNNING));
  exports.Set("DEV_WIRELESS", static_cast<unsigned long>(PCAP_IF_WIRELESS));
  exports.Set("DEV_CONNECTION_UKNOWN", static_cast<unsigned long>(PCAP_IF_CONNECTION_STATUS_UNKNOWN));
  exports.Set("DEV_CONNECTED", static_cast<unsigned long>(PCAP_IF_CONNECTION_STATUS_CONNECTED));
  exports.Set("DEV_DISCONNECTED", static_cast<unsigned long>(PCAP_IF_CONNECTION_STATUS_DISCONNECTED));
  exports.Set("DEV_CONNECTION_NA", static_cast<unsigned long>(PCAP_IF_CONNECTION_STATUS_NOT_APPLICABLE));
}
