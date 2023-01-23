#include "enums.h"

void InitializeEnums(const Napi::Env &env, Napi::Object &exports) {
  Napi::Object enums = Napi::Object::New(env);

  Napi::Object devFlags = Napi::Object(env);
  devFlags.Set("LOOPBACK", static_cast<unsigned long>(PCAP_IF_LOOPBACK));
  devFlags.Set("UP", static_cast<unsigned long>(PCAP_IF_UP));
  devFlags.Set("RUNNING", static_cast<unsigned long>(PCAP_IF_RUNNING));
  devFlags.Set("WIRELESS", static_cast<unsigned long>(PCAP_IF_WIRELESS));
  devFlags.Set("CONNECTION_UKNOWN", static_cast<unsigned long>(PCAP_IF_CONNECTION_STATUS_UNKNOWN));
  devFlags.Set("CONNECTED", static_cast<unsigned long>(PCAP_IF_CONNECTION_STATUS_CONNECTED));
  devFlags.Set("DISCONNECTED", static_cast<unsigned long>(PCAP_IF_CONNECTION_STATUS_DISCONNECTED));
  devFlags.Set("CONNECTION_NA", static_cast<unsigned long>(PCAP_IF_CONNECTION_STATUS_NOT_APPLICABLE));
  enums.set("DEV", devFlags);

  enums.Freeze();
  exports.set("CONSTANTS", enums);
}
