#include "dev.h"
#include <iostream>

Napi::Object PCap::Init(Napi::Env env, Napi::Object exports) {
  // This method is used to hook the accessor and method callbacks
  Napi::Function func = DefineClass(env, "PCap", {
    StaticMethod<&PCap::findDevice>("findDevice", napi_default),
    InstanceMethod<&PCap::startCapture>("startCapture", napi_default)
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
  Napi::Function cb = info[1].As<Napi::Function>();
  this->_cb = Napi::Persistent(cb);
  Napi::Value device = findDevice(info).As<Napi::Value>();
  if (device.IsObject()) {
    Napi::Object deviceObject = device.As<Napi::Object>();
    this->_deviceName = deviceObject.Get("name").As<Napi::String>().Utf8Value().data();
  } else {
    this->_deviceName = "any";
  }
}

void PCap::startCapture(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  char errbuf[PCAP_ERRBUF_SIZE];
  this->_pcapHandle = pcap_create(this->_deviceName, errbuf);
  if (this->_pcapHandle == nullptr) throw Napi::Error::New(env, Napi::String::New(env, errbuf));
  if (pcap_set_promisc(this->_pcapHandle, 1) != 0) throw Napi::Error::New(env, "Unable to set promiscuous mode");
  if (pcap_set_buffer_size(this->_pcapHandle, this->_bufferSize) != 0) throw Napi::Error::New(env, "Unable to set buffer size");
  if (pcap_set_timeout(this->_pcapHandle, 1000) != 0) throw Napi::Error::New(env, "Unable to set read timeout");
  pcap_set_immediate_mode(this->_pcapHandle, 1);
  if (pcap_set_tstamp_type(this->_pcapHandle, PCAP_TSTAMP_HOST) != 0) throw Napi::Error::New(env, "Unable to set timestamp type");
  if (pcap_set_tstamp_precision(this->_pcapHandle, PCAP_TSTAMP_PRECISION_NANO) != 0) throw Napi::Error::New(env, "Unable to set timestamp precision");
  this->_dataLinkType = pcap_datalink(this->_pcapHandle);
  Napi::Value filter = info[0].As<Napi::Value>();
  if (filter.IsString()) {
    const char* filterChar = filter.As<Napi::String>().Utf8Value().data();
    struct bpf_program fp;
    if (pcap_compile(this->_pcapHandle, &fp, filterChar, 1, PCAP_NETMASK_UNKNOWN) == -1) throw Napi::Error::New(env, pcap_geterr(this->_pcapHandle));
    if (pcap_setfilter(this->_pcapHandle, &fp) == -1) throw Napi::Error::New(env, pcap_geterr(this->_pcapHandle));

    pcap_freecode(&fp);
  }
  if (pcap_activate(this->_pcapHandle) < 0) throw Napi::Error::New(env, Napi::String::New(env, pcap_geterr(this->_pcapHandle)));
  if (pcap_setnonblock(this->_pcapHandle, 1, errbuf) == -1) throw Napi::Error::New(env, Napi::String::New(env, errbuf));
  this->_fd = pcap_get_selectable_fd(this->_pcapHandle);
  int r = uv_poll_init(uv_default_loop(), this->_pollHandle, this->_fd);
  if (r != 0) throw Napi::Error::New(env, "Unable to initialize UV polling");
  r = uv_poll_start(this->_pollHandle, UV_READABLE, PCap::onPackets);
  if (r != 0) throw Napi::Error::New(env, "Unable to start UV polling");
  this->_pollHandle->data = this;
}

void PCap::onPackets(uv_poll_t* handle, int status, int events) {
  std::cout << "onPackets() status: " << status << "\n";
  if (status != 0) return;

  PCap *obj = static_cast<PCap*>(handle->data);
  if (!obj->_closing && (events & UV_READABLE)) {
    obj->_handlingPackets = true;
    pcap_dispatch(obj->_pcapHandle, 32, PCap::emitPacket, (u_char*)obj);
    obj->_handlingPackets = false;
  }
}

void PCap::emitPacket(u_char* user, const struct pcap_pkthdr* pktHdr, const u_char* pktData) {
  PCap *obj = (PCap*)user;

  size_t copyLen = pktHdr->caplen;
  bool truncated = false;
  if (copyLen > obj->_bufferSize) {
    copyLen = obj->_bufferSize;
    truncated = true;
  }

  std::cout << "emitPacket(), len: " << copyLen << ", truncated: " << truncated << "\n";

  /*Napi::Buffer<char> Napi::Buffer::New(napi_env env, T* data, size_t length);
  memcpy(obj->_bufferData, pktData, copyLen);

  this->_cb::
  Local<Value> emit_argv[3] = {
    Nan::New<String>(packet_symbol),
    Nan::New<Number>(copy_len),
    Nan::New<Boolean>(truncated)
  };
  obj->async_res.runInAsyncScope(
    Nan::New<Object>(obj->persistent()),
    Nan::New<Function>(obj->Emit),
    3,
    emit_argv
  );*/
}

void PCap::Finalize(Napi::Env env) {
  if (this->_closing) return;

  this->_closing = true;

  if (this->_pollHandle != nullptr) uv_poll_stop(this->_pollHandle);
  if (this->_pcapHandle != nullptr) pcap_close(this->_pcapHandle);

  this->_pcapHandle = nullptr;
  this->_bufferData = nullptr;
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
    throw Napi::Error::New(env, Napi::String::New(env, errbuf));
	}
	
  Napi::Array devices = Napi::Array::New(env);
  Napi::Value searchValue = info[0].As<Napi::Value>();
  bool doSearch = searchValue.IsString();
  Napi::Value found = env.Null();
	for (i = 0, device = alldevsp; device != nullptr; device = device->next, ++i) {
    Napi::HandleScope scope(env);
    Napi::Object devObject = Napi::Object::New(env);
    devObject.Set("name", Napi::String::New(env, device->name));
    if (doSearch && found.IsNull() && devObject.Get("name").StrictEquals(searchValue)) found = devObject;
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
          if (doSearch && found.IsNull() && addressObject.Get("address").StrictEquals(searchValue)) found = devObject;
        }
      }
    }

		devObject.Set("addresses", (addresses.Length() != 0) ? addresses : env.Null());
    devices.Set(i, devObject);
	}

  pcap_freealldevs(alldevsp);

  return doSearch ? found : devices;
}