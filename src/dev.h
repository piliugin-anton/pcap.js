#ifndef NODE_PCAPJS_DEV_H
#define NODE_PCAPJS_DEV_H

#include "util.h"

struct Packet {
  bpf_u_int32 len;
  bpf_u_int32 capLen;
  u_char* data;
  bool truncated;
  double timestamp;
  Packet(const struct pcap_pkthdr* pktHdr, const u_char* pktData) {
    len = pktHdr->len;
    capLen = pktHdr->caplen;
    data = (u_char*)pktData;
    truncated = capLen < len;
    timestamp = (pktHdr->ts.tv_sec + (1.0/1000000000) * pktHdr->ts.tv_usec) * 1000.0;
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
    void Finalize(Napi::Env env);
  private:
    pcap_t* _pcapHandle = nullptr;
    std::string _deviceName;
    int _dataLinkType;
    int _fd;
    uv_poll_t _pollHandle;
    const uint16_t _bufferSize = 65535;
    const int _bufferTimeout = 1000;
    bool _handlingPackets = false;
    bool _closing = false;
    static void packetCallbackJS(Napi::Env env, Napi::Function callback, Context *context, Packet *data);
    Napi::TypedThreadSafeFunction<Context, Packet, PCap::packetCallbackJS> _cb;
    static void onPackets(uv_poll_t* handle, int status, int events);
    static void emitPacket(u_char* user, const struct pcap_pkthdr* pktHdr, const u_char* pktData);
};

#endif
