#ifndef NODE_PCAPJS_DEV_H
#define NODE_PCAPJS_DEV_H

#include "util.h"

struct Packet {
  pcap_pkthdr header;
  u_char* data;
  Packet(const pcap_pkthdr* pktHdr, const u_char* pktData) {
    header = *pktHdr;
    data = (u_char*)pktData;
  }
};

using Context = Napi::Reference<Napi::Value>;

class PCap : public Napi::ObjectWrap<PCap> {
  public:
    static Napi::Object Init(Napi::Env env, Napi::Object exports);
    PCap(const Napi::CallbackInfo& info);
    static Napi::Value findDevice(const Napi::CallbackInfo& info);
    static void ipStringHelper(const char* key, sockaddr *addr, Napi::Object *Address);
    void startCapture(const Napi::CallbackInfo& info);
    Napi::Value stopCapture(const Napi::CallbackInfo& info);
  private:
    pcap_t* _pcapHandle = nullptr;
    std::string _deviceName;
    int _dataLinkType;
    int _fd;
    uv_poll_t _pollHandle;
    const uint16_t _bufferSize = 65535;
    bool _handlingPackets = false;
    bool _closing = false;
    static void packetCallbackJS(Napi::Env env, Napi::Function callback, Context *context, Packet *data);
    Napi::TypedThreadSafeFunction<Context, Packet, PCap::packetCallbackJS> _cb;
    static void onPackets(uv_poll_t* handle, int status, int events);
    static void emitPacket(u_char* user, const struct pcap_pkthdr* pktHdr, const u_char* pktData);
};

#endif
