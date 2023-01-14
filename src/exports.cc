#include "dev.h"
#include "enums.h"

Napi::Object Init(Napi::Env env, Napi::Object exports) {
  InitializeEnums(env, exports);
  PCap::Init(env, exports);

  return exports;
}

NODE_API_MODULE(pcapjs, Init)
