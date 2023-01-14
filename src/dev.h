#ifndef NODE_PCAPJS_DEV_H
#define NODE_PCAPJS_DEV_H

#include "util.h"

class PCap : public Napi::ObjectWrap<PCap> {
  public:
    static Napi::Object Init(Napi::Env env, Napi::Object exports);
    PCap(const Napi::CallbackInfo& info);
    static Napi::Value CreateNewItem(const Napi::CallbackInfo& info);
    Napi::Value listDevices(const Napi::CallbackInfo& info);

  private:
    double _value;
    Napi::Value GetValue(const Napi::CallbackInfo& info);
    Napi::Value SetValue(const Napi::CallbackInfo& info);
    void ipStringHelper(const char* key, sockaddr *addr, Napi::Object *Address);
};

#endif
