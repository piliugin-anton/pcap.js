#include "dev.h"

void SetAddrStringHelper(const char* key, sockaddr *addr, Napi::Object *Address) {
  if (key && addr) {
    char dst_addr[INET6_ADDRSTRLEN + 1] = {0};
    char* src = 0;
    socklen_t size = 0;
    if (addr->sa_family == AF_INET) {
      struct sockaddr_in* saddr = (struct sockaddr_in*) addr;
      src = (char*) &(saddr->sin_addr);
      size = INET_ADDRSTRLEN;
    } else {
      struct sockaddr_in6* saddr6 = (struct sockaddr_in6*) addr;
      src = (char*) &(saddr6->sin6_addr);
      size = INET6_ADDRSTRLEN;
    }

    const char* address = inet_ntop(addr->sa_family, src, dst_addr, size);
    if (address) {
      Address->Set(key, address);
    }
  }
}

Napi::Array listDevices(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  char errbuf[PCAP_ERRBUF_SIZE];
  pcap_if_t *alldevsp = nullptr, *device;
  pcap_addr_t *address;
  int i, j, af;
  
  if (pcap_findalldevs(&alldevsp, errbuf) != 0) {
    throw Napi::Error::New(env, Napi::String::New(env, errbuf));
	}
	
  Napi::Array devices = Napi::Array::New(env);
	for (i = 0, device = alldevsp; device != nullptr; device = device->next, ++i) {
    Napi::Object devObject = Napi::Object::New(env);
    devObject.Set("name", Napi::String::New(env, device->name));
    devObject.Set("description", (device->description != nullptr) ? Napi::String::New(env, device->description) : env.Null());
    devObject.Set("flags", Napi::Number::New(env, (double)device->flags));

    Napi::Array addresses = Napi::Array::New(env);
    for (j = 0, address = device->addresses; address != nullptr; address = address->next) {
      if (address->addr) {
        af = address->addr->sa_family;
        if (af == AF_INET || af == AF_INET6) {
          Napi::Object addressObject = Napi::Object::New(env);
          SetAddrStringHelper("address", address->addr, &addressObject);
          SetAddrStringHelper("netmask", address->netmask, &addressObject);
          SetAddrStringHelper("broadcastAddress", address->broadaddr, &addressObject);
          SetAddrStringHelper("destinationAddress", address->dstaddr, &addressObject);
          addresses.Set(j++, addressObject);
        }
      }
    }

		devObject.Set("addresses", addresses);
    devices.Set(i, devObject);
	}

  return devices;
}