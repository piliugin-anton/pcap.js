#ifndef NODE_PCAPJS_DEV_H
#define NODE_PCAPJS_DEV_H

#include "util.h"

class PCap : public Napi::ObjectWrap<PCap> {
  public:
    static Napi::Object Init(Napi::Env env, Napi::Object exports);
    PCap(const Napi::CallbackInfo& info);
    void Finalize(Napi::Env env);
    static Napi::Value findDevice(const Napi::CallbackInfo& info);
    static void ipStringHelper(const char* key, sockaddr *addr, Napi::Object *Address);
    void startCapture(const Napi::CallbackInfo& info);
  private:
    pcap_t* _pcapHandle = nullptr;
    const char* _deviceName;
    int _dataLinkType;
    int _fd;
    uv_poll_t* _pollHandle = nullptr;
    const uint16_t _bufferSize = 65535;
    bool _handlingPackets = false;
    bool _closing = false;
    Napi::FunctionReference _cb;
    char* _bufferData;
    static void onPackets(uv_poll_t* handle, int status, int events);
    static void emitPacket(u_char* user, const struct pcap_pkthdr* pktHdr, const u_char* pktData);
};

#endif
