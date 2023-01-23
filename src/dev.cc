#include "dev.h"
#include <iostream>

Napi::Object PCap::Init(Napi::Env env, Napi::Object exports) {
  // This method is used to hook the accessor and method callbacks
  Napi::Function func = DefineClass(env, "PCap", {
    StaticMethod<&PCap::findDevice>("findDevice", napi_default),
    InstanceMethod<&PCap::setFilter>("setFilter", napi_default),
    InstanceMethod<&PCap::startCapture>("startCapture", napi_default),
    InstanceMethod<&PCap::stopCapture>("stopCapture", napi_default),
    InstanceMethod<&PCap::sendPacket>("sendPacket", napi_default),
    InstanceMethod<&PCap::getStats>("getStats", napi_default)
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

PCap::PCap(const Napi::CallbackInfo& info) : Napi::ObjectWrap<PCap>(info) {
  Napi::Env env = info.Env();
  if (info[0].IsString()) {
    Napi::Value device = findDevice(info).As<Napi::Value>();
    if (device.IsNull() || !device.IsObject()) throw Napi::Error::New(env, "No device with name " + info[0].As<Napi::String>().Utf8Value() + " found");
    
    this->_deviceName = device.As<Napi::Object>().Get("name").As<Napi::String>().Utf8Value();
  } else {
    this->_deviceName = "any";
  }

  if (info[0].IsFunction() || info[1].IsFunction()) {
    // Create a new context set to the receiver (ie, `this`) of the function call
    this->_context = new Napi::Reference<Napi::Value>(Napi::Persistent(info.This()));
    this->_onPacketFNREF = Napi::Persistent(info[info[0].IsFunction() ? 0 : 1].As<Napi::Function>());
  } else throw Napi::Error::New(env, "Callback function must be set");

  this->createDevice(env);
}

void PCap::Finalize(Napi::Env env) {
  std::cout << "Finalize...\n";
  if (this->_context) delete this->_context;
}

void PCap::createDevice(Napi::Env env) {
  if (this->_pcapHandle) return;

  char errbuf[PCAP_ERRBUF_SIZE];
  this->_pcapHandle = pcap_create(this->_deviceName.c_str(), errbuf);
  if (!this->_pcapHandle) throw Napi::Error::New(env, errbuf);
  if (pcap_set_promisc(this->_pcapHandle, 1) != 0) throw Napi::Error::New(env, "Unable to set promiscuous mode");
  if (pcap_set_buffer_size(this->_pcapHandle, this->_bufferSize) != 0) throw Napi::Error::New(env, "Unable to set buffer size");
  if (pcap_set_timeout(this->_pcapHandle, this->_bufferTimeout) != 0) throw Napi::Error::New(env, "Unable to set read timeout");
  if (pcap_set_snaplen(this->_pcapHandle, this->_snapshotLength) != 0) throw Napi::Error::New(env, "Unable to set snapshot length");
  pcap_set_immediate_mode(this->_pcapHandle, 1);
  if (pcap_set_tstamp_type(this->_pcapHandle, PCAP_TSTAMP_HOST) != 0) throw Napi::Error::New(env, "Unable to set timestamp type");
  if (pcap_set_tstamp_precision(this->_pcapHandle, PCAP_TSTAMP_PRECISION_NANO) != 0) throw Napi::Error::New(env, "Unable to set timestamp precision");
  this->_dataLinkType = pcap_datalink(this->_pcapHandle);
  int activated = pcap_activate(this->_pcapHandle);
  if (activated < 0) throw Napi::Error::New(env, pcap_statustostr(activated));
  if (pcap_setnonblock(this->_pcapHandle, 1, errbuf) == PCAP_ERROR) throw Napi::Error::New(env, errbuf);
}

void PCap::startEventLoop(Napi::Env env) {
  this->_fd = pcap_get_selectable_fd(this->_pcapHandle);
  int r = uv_poll_init(uv_default_loop(), &this->_pollHandle, this->_fd);
  if (r != 0) throw Napi::Error::New(env, "Unable to initialize UV polling");
  r = uv_poll_start(&this->_pollHandle, UV_READABLE, PCap::onPackets);
  if (r != 0) throw Napi::Error::New(env, "Unable to start UV polling");
}

void PCap::setFilter(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();

  if (!this->_pcapHandle) throw Napi::Error::New(env, "Capturing device is not created");

  const char* filterChar = info[0].IsString() ? info[0].As<Napi::String>().Utf8Value().data() : "";
  struct bpf_program fp;
  int compile = pcap_compile(this->_pcapHandle, &fp, filterChar, 1, PCAP_NETMASK_UNKNOWN);
  if (compile != 0) throw Napi::Error::New(env, pcap_statustostr(compile));
  int setFilter = pcap_setfilter(this->_pcapHandle, &fp);
  if (setFilter != 0) throw Napi::Error::New(env, pcap_statustostr(setFilter));

  pcap_freecode(&fp);
}

void PCap::startCapture(const Napi::CallbackInfo& info) {
  if (this->_capturing) return;

  Napi::Env env = info.Env();

  this->createDevice(env);
  this->startEventLoop(env);

  this->_onPacketTSFN = Napi::TypedThreadSafeFunction<Context, Packet, PCap::packetCallbackJS>::New(
    env,
    this->_onPacketFNREF.Value(), // JavaScript function called asynchronously
    "PacketCallback", // Name
    0, // Unlimited queue
    1, // Only one thread will use this initially
    this->_context
  );

  this->_closing = false;
  this->_pollHandle.data = this;
  this->_capturing = true;
}

Napi::Value PCap::stopCapture(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();

  if (this->_closing || !this->_capturing) return Napi::Boolean::New(env, false);

  this->_closing = true;

  if (uv_is_active((const uv_handle_t*)&this->_pollHandle) != 0) uv_poll_stop(&this->_pollHandle);
  if (this->_pcapHandle) pcap_close(this->_pcapHandle);
  this->_onPacketTSFN.Release();

  this->_handlingPackets = false;
  this->_capturing = false;

  return Napi::Boolean::New(env, true);
}

Napi::Value PCap::getStats(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();

  if (this->_pcapHandle) pcap_stats(this->_pcapHandle, &this->_stat);

  Napi::Object stats = Napi::Object::New(env);
  stats.Set("received", Napi::Number::New(env, this->_stat.ps_recv));
  stats.Set("dropped", Napi::Number::New(env, this->_stat.ps_drop));
  stats.Set("ifdropped", Napi::Number::New(env, this->_stat.ps_ifdrop));

  return stats;
}

Napi::Value PCap::sendPacket(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();

  if (!info[0].IsBuffer()) return Napi::Boolean::New(env, false);

  Napi::Buffer<u_char> buffer = info[0].As<Napi::Buffer<u_char>>();
  int status = pcap_sendpacket(this->_pcapHandle, buffer.Data(), buffer.Length());
  if (status != 0) throw Napi::Error::New(env, pcap_statustostr(status));

  return Napi::Boolean::New(env, true);
}

void PCap::onPackets(uv_poll_t* handle, int status, int events) {
  if (status != 0) return;

  PCap *obj = static_cast<PCap*>(handle->data);
  if (!obj->_closing && !obj->_handlingPackets && (events & UV_READABLE)) {
    obj->_handlingPackets = true;
    pcap_dispatch(obj->_pcapHandle, -1, PCap::emitPacket, (u_char*)obj);
    obj->_handlingPackets = false;
  }
}

void PCap::emitPacket(u_char* user, const struct pcap_pkthdr* pktHdr, const u_char* pktData) {
  PCap *obj = (PCap*)user;

  if (!obj->_closing) {
    Packet *packet = new Packet(pktHdr, pktData);
    obj->_onPacketTSFN.NonBlockingCall(packet);
  }
}

void PCap::packetCallbackJS(Napi::Env env, Napi::Function callback, Context *context, Packet *packet) {
  if (env != nullptr) {
    // On Node-API 5+, the `callback` parameter is optional; however, this example
    // does ensure a callback is provided.
    if (callback != nullptr)
      callback.Call(context->Value(), {
        Napi::Buffer<u_char>::New(env, packet->data, packet->capLen, [](Napi::Env /*env*/, u_char* finalizeData) {
          free(finalizeData);
        }),
        Napi::Boolean::New(env, packet->truncated),
        Napi::Number::New(env, packet->timestamp)
      });
  }
  // We're finished with the data.
  if (packet != nullptr) delete packet;
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

Napi::Value PCap::findDevice(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  char errbuf[PCAP_ERRBUF_SIZE];
  pcap_if_t *alldevsp = nullptr, *device;
  pcap_addr_t *address;
  int i, j, af;
  
  if (pcap_findalldevs(&alldevsp, errbuf) != 0) {
    throw Napi::Error::New(env, errbuf);
	}
	
  Napi::Array devices = Napi::Array::New(env);
  Napi::Value searchValue = info[0].As<Napi::Value>();
  bool doSearch = searchValue.IsString();
  Napi::Value found = env.Null();
	for (i = 0, device = alldevsp; device != nullptr; device = device->next, ++i) {
    Napi::Object devObject = Napi::Object::New(env);
    Napi::String name = Napi::String::New(env, device->name);
    devObject.Set("name", name);
    if (doSearch && found.IsNull() && searchValue.StrictEquals(name)) found = devObject;
    devObject.Set("description", (device->description != nullptr) ? Napi::String::New(env, device->description) : env.Null());
    devObject.Set("flags", Napi::Number::New(env, (double)device->flags));

    Napi::Array addresses = Napi::Array::New(env);
    for (j = 0, address = device->addresses; address != nullptr; address = address->next) {
      if (address->addr) {
        af = address->addr->sa_family;
        if (af == AF_INET || af == AF_INET6) {
          Napi::Object addressObject = Napi::Object::New(env);
          addressObject.Set("family", Napi::Number::New(env, af));
          ipStringHelper("address", address->addr, &addressObject);
          ipStringHelper("netmask", address->netmask, &addressObject);
          ipStringHelper("broadcastAddress", address->broadaddr, &addressObject);
          ipStringHelper("destinationAddress", address->dstaddr, &addressObject);
          addresses.Set(j++, addressObject);
          if (doSearch && found.IsNull() && searchValue.StrictEquals(addressObject.Get("address"))) found = devObject;
        }
      }
    }

		devObject.Set("addresses", (addresses.Length() != 0) ? addresses : env.Null());
    devices.Set(i, devObject);
	}

  pcap_freealldevs(alldevsp);

  return doSearch ? found : devices;
}