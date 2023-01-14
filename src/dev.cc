#include "dev.h"

Napi::Object PCap::Init(Napi::Env env, Napi::Object exports) {
    // This method is used to hook the accessor and method callbacks
    Napi::Function func = DefineClass(env, "PCap", {
        InstanceMethod<&PCap::GetValue>("GetValue", static_cast<napi_property_attributes>(napi_writable | napi_configurable)),
        InstanceMethod<&PCap::SetValue>("SetValue", static_cast<napi_property_attributes>(napi_writable | napi_configurable)),
        InstanceMethod<&PCap::listDevices>("listDevices", static_cast<napi_property_attributes>(napi_writable | napi_configurable)),
        StaticMethod<&PCap::CreateNewItem>("CreateNewItem", static_cast<napi_property_attributes>(napi_writable | napi_configurable)),
        //InstanceMethod<&PCap::ipStringHelper>("ipStringHelper", static_cast<napi_property_attributes>(napi_writable | napi_configurable)),
    });

    Napi::FunctionReference* constructor = new Napi::FunctionReference();

    // Create a persistent reference to the class constructor. This will allow
    // a function called on a class prototype and a function
    // called on instance of a class to be distinguished from each other.
    *constructor = Napi::Persistent(func);
    exports.Set("PCap", func);

    // Store the constructor as the add-on instance data. This will allow this
    // add-on to support multiple instances of itself running on multiple worker
    // threads, as well as multiple instances of itself running in different
    // contexts on the same thread.
    //
    // By default, the value set on the environment here will be destroyed when
    // the add-on is unloaded using the `delete` operator, but it is also
    // possible to supply a custom deleter.
    env.SetInstanceData<Napi::FunctionReference>(constructor);

    return exports;
}

PCap::PCap(const Napi::CallbackInfo& info) :
    Napi::ObjectWrap<PCap>(info) {
  Napi::Env env = info.Env();
  // ...
  Napi::Number value = info[0].As<Napi::Number>();
  this->_value = value.DoubleValue();
}

Napi::Value PCap::GetValue(const Napi::CallbackInfo& info){
    Napi::Env env = info.Env();
    return Napi::Number::New(env, this->_value);
}

Napi::Value PCap::SetValue(const Napi::CallbackInfo& info){
    Napi::Env env = info.Env();
    // ...
    Napi::Number value = info[0].As<Napi::Number>();
    this->_value = value.DoubleValue();
    return this->GetValue(info);
}

// Create a new item using the constructor stored during Init.
Napi::Value PCap::CreateNewItem(const Napi::CallbackInfo& info) {
  // Retrieve the instance data we stored during `Init()`. We only stored the
  // constructor there, so we retrieve it here to create a new instance of the
  // JS class the constructor represents.
  Napi::FunctionReference* constructor =
      info.Env().GetInstanceData<Napi::FunctionReference>();
  return constructor->New({ Napi::Number::New(info.Env(), 42) });
}

void PCap::ipStringHelper(const char* key, sockaddr *addr, Napi::Object *Address) {
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

Napi::Value PCap::listDevices(const Napi::CallbackInfo& info) {
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
          ipStringHelper("address", address->addr, &addressObject);
          ipStringHelper("netmask", address->netmask, &addressObject);
          ipStringHelper("broadcastAddress", address->broadaddr, &addressObject);
          ipStringHelper("destinationAddress", address->dstaddr, &addressObject);
          addresses.Set(j++, addressObject);
        }
      }
    }

		devObject.Set("addresses", (addresses.Length() != 0) ? addresses : env.Null());
    devices.Set(i, devObject);
	}

  return devices;
}