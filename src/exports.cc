#include "dev.h"
#include "enums.h"

Napi::Object Init(Napi::Env env, Napi::Object exports) {
  InitializeEnums(env, exports);

  exports.Set("listDevices", Napi::Function::New(env, listDevices));

  return exports;
}

NODE_API_MODULE(pcapjs, Init)