#include <chrono>
#include <thread>

#include "node-canal.h"

// Workerthreads
void *deviceReceiveThread(void *pData);

// Forward declaration
// static napi_value startThread(napi_env env, napi_callback_info info);
// static void thdata_is_unloading(napi_env env, void *data, void *hint);

std::thread nativeThread;
Napi::ThreadSafeFunction tsfn;

Napi::FunctionReference CNodeCanal::constructor;

Napi::Object CNodeCanal::Init(Napi::Env env, Napi::Object exports) {

  Napi::HandleScope scope(env);

  // threadData *thdata = (threadData *)malloc(sizeof(*thdata));
  // memset(thdata, 0, sizeof(*thdata));
  // thdata->thread = getIfPointer();

  // napi_property_descriptor export_properties[] = {{"startThread", NULL,
  //                                                  startThread, NULL, NULL,
  //                                                  NULL, napi_default,
  //                                                  thdata}};

  // Attach the addon data to the exports object to ensure that they are
  // destroyed together.
  // napi_wrap(env, exports, thdata, thdata_is_unloading, NULL, NULL);

  Napi::Function func = DefineClass(
      env, "CNodeCanal",
      {InstanceMethod("init", &CNodeCanal::init),
       InstanceMethod("open", &CNodeCanal::open),
       InstanceMethod("close", &CNodeCanal::close),
       InstanceMethod("send", &CNodeCanal::send),
       InstanceMethod("receive", &CNodeCanal::receive),
       InstanceMethod("dataAvailable", &CNodeCanal::dataAvailable),
       InstanceMethod("getStatus", &CNodeCanal::getStatus),
       InstanceMethod("getStatistics", &CNodeCanal::getStatistics),
       InstanceMethod("setFilter", &CNodeCanal::setFilter),
       InstanceMethod("setMask", &CNodeCanal::setMask),
       InstanceMethod("setBaudrate", &CNodeCanal::setBaudrate),
       InstanceMethod("getLevel", &CNodeCanal::getLevel),
       InstanceMethod("getVersion", &CNodeCanal::getVersion),
       InstanceMethod("getDllVersion", &CNodeCanal::getDllVersion),
       InstanceMethod("getVendorString", &CNodeCanal::getVendorString),
       InstanceMethod("getDriverInfo", &CNodeCanal::getDriverInfo),
       InstanceMethod("asyncReceive", &CNodeCanal::asyncReceive)});

  constructor = Napi::Persistent(func);
  constructor.SuppressDestruct();

  exports.Set("CNodeCanal", func);

  // napi_define_properties(
  //     env, exports, sizeof(export_properties) / sizeof(export_properties[0]),
  //     export_properties);
  // exports.Set("startThread",
  //               Napi::Function::New(env, startThread));
  return exports;
}

CNodeCanal::CNodeCanal(const Napi::CallbackInfo &info)
    : Napi::ObjectWrap<CNodeCanal>(info) {

  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);

  // Napi::Number value = info[0].As<Napi::Number>();
  this->m_pcanalif = new CCanalIf();
}

/*!
    init
*/

Napi::Value CNodeCanal::init(const Napi::CallbackInfo &info) {

  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);

  if (info.Length() != 3 || !info[0].IsString() || !info[1].IsString() ||
      !info[2].IsNumber()) {
    Napi::TypeError::New(env, "Three arguments expected (path, param, flags)")
        .ThrowAsJavaScriptException();
  }

  Napi::String path = info[0].As<Napi::String>();
  Napi::String param = info[1].As<Napi::String>();
  Napi::Number flags = info[2].As<Napi::Number>();
  int answer = this->m_pcanalif->init(path.ToString(), param.ToString(),
                                      (uint32_t)flags.ToNumber());

  return Napi::Number::New(info.Env(), answer);
}

/*!
    dataAvailable
*/
Napi::Value CNodeCanal::dataAvailable(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);

  uint32_t num = this->m_pcanalif->CanalDataAvailable();
  return Napi::Number::New(env, num);
}

/*!
    open
*/

Napi::Value CNodeCanal::open(const Napi::CallbackInfo &info) {

  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);

  double num = this->m_pcanalif->CanalOpen();
  return Napi::Number::New(env, num);
}

/*!
    close
*/

Napi::Value CNodeCanal::close(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);

  double num = this->m_pcanalif->CanalClose();
  return Napi::Number::New(env, num);
}

/*!
    send
*/

Napi::Value CNodeCanal::send(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  canalMsg canmsg;
  memset(&canmsg, 0, sizeof(canalMsg));

  if (4 == info.Length()) {
    // flags, id, data
    if (info[0].IsNumber() || info[1].IsNumber() || info[2].IsNumber() ||
        info[3].IsObject()) {

      Napi::Number flags = info[0].As<Napi::Number>();
      Napi::Number timestamp = info[1].As<Napi::Number>();
      Napi::Number id = info[2].As<Napi::Number>();
      Napi::Array data_array = info[3].As<Napi::Array>();

      memset(&canmsg, 0, sizeof(canmsg));
      canmsg.flags = (uint32_t)flags.ToNumber();
      canmsg.timestamp = (uint32_t)timestamp.ToNumber();
      canmsg.id = (uint32_t)id.ToNumber();
      canmsg.sizeData = data_array.Length();
      for (uint32_t i = 0; i < data_array.Length(); i++) {
        Napi::Value val = data_array[i];
        if (val.IsNumber()) {
          canmsg.data[i] = (int)val.As<Napi::Number>();
        }
      }
    } else {
      Napi::TypeError::New(
          env, "Four arguments expected (flags,id,data-array) or object")
          .ThrowAsJavaScriptException();
    }
  }
  // { id: 12132, ... }
  else if (1 == info.Length() && info[0].IsObject()) {
    Napi::Object msg = info[0].As<Napi::Object>();
    canmsg.flags = (uint32_t)msg.Get("flags").ToNumber();
    bool ext = (bool)msg.Get("ext").ToBoolean();
    if (ext)
      canmsg.flags |= CANAL_IDFLAG_EXTENDED;
    bool rtr = (bool)msg.Get("rtr").ToBoolean();
    if (rtr)
      canmsg.flags |= CANAL_IDFLAG_RTR;
    canmsg.timestamp = (uint32_t)msg.Get("timestamp").ToNumber();
    canmsg.obid = (uint32_t)msg.Get("obid").ToNumber();
    canmsg.id = (uint32_t)msg.Get("id").ToNumber();
    Napi::Array data_array = msg.Get("data").ToObject().As<Napi::Array>();
    canmsg.sizeData = data_array.Length();
    for (uint32_t i = 0; i < data_array.Length(); i++) {
      Napi::Value val = data_array[i];
      if (val.IsNumber()) {
        canmsg.data[i] = (int)val.As<Napi::Number>();
      }
    }
  } else {
    Napi::TypeError::New(
        env, "Four arguments expected (flags,id,data-array) or object")
        .ThrowAsJavaScriptException();
  }

  double num = this->m_pcanalif->CanalSend(&canmsg);
  return Napi::Number::New(env, num);
}

/*!
    receive
*/

Napi::Value CNodeCanal::receive(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  if (1 != info.Length()) {
    Napi::TypeError::New(env, "Invalid argument count")
        .ThrowAsJavaScriptException();
    return Napi::Number::New(env, CANAL_ERROR_PARAMETER);
  }

  if (!info[0].IsFunction()) {
    Napi::TypeError::New(env, "Invalid argument type (expected function)")
        .ThrowAsJavaScriptException();
    return Napi::Number::New(env, CANAL_ERROR_PARAMETER);
  }

  Napi::Object obj = Napi::Object::New(env);

  canalMsg canmsg;
  memset(&canmsg, 0, sizeof(canmsg));

  uint32_t rv = this->m_pcanalif->CanalReceive(&canmsg);
  if (CANAL_ERROR_SUCCESS == rv) {
    Napi::Array dataArray = Napi::Array::New(Env(), canmsg.sizeData);
    for (uint32_t i = 0; i < canmsg.sizeData; i++) {
      dataArray[uint32_t(i)] = Napi::Number::New(info.Env(), canmsg.data[i]);
    }
    obj.Set("flags", uint32_t(canmsg.flags));
    obj.Set("id", uint32_t(canmsg.id));
    obj.Set("obid", uint32_t(canmsg.obid));
    obj.Set("typestamp", uint32_t(canmsg.timestamp));
    obj.Set("sizeData", uint32_t(canmsg.sizeData));
    obj.Set("data", dataArray);
  }

  Napi::Function cb = info[0].As<Napi::Function>();
  cb.MakeCallback(env.Global(), {obj});

  return Napi::Number::New(env, rv);
}

/*!
    getStatus
*/

Napi::Value CNodeCanal::getStatus(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  if (1 != info.Length()) {
    Napi::TypeError::New(env, "Invalid argument count")
        .ThrowAsJavaScriptException();
    return Napi::Number::New(env, CANAL_ERROR_PARAMETER);
  }

  if (!info[0].IsFunction()) {
    Napi::TypeError::New(env, "Invalid argument type (expect object)")
        .ThrowAsJavaScriptException();
    return Napi::Number::New(env, CANAL_ERROR_PARAMETER);
  }

  Napi::Object obj = Napi::Object::New(env);

  canalStatus canStatus;
  uint32_t rv = this->m_pcanalif->CanalGetStatus(&canStatus);
  if (CANAL_ERROR_SUCCESS == rv) {
    obj.Set("channel_status", uint32_t(canStatus.channel_status));
    obj.Set("lasterrorcode", uint32_t(canStatus.lasterrorcode));
    obj.Set("lasterrorsubcode", uint32_t(canStatus.lasterrorsubcode));
  }

  Napi::Function cb = info[0].As<Napi::Function>();
  cb.MakeCallback(env.Global(), {obj});

  return Napi::Number::New(env, rv);
}

/*!
    getStatistics
*/

Napi::Value CNodeCanal::getStatistics(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  if (1 != info.Length()) {
    Napi::TypeError::New(env, "Invalid argument count")
        .ThrowAsJavaScriptException();
    return Napi::Number::New(env, CANAL_ERROR_PARAMETER);
  }

  if (!info[0].IsFunction()) {
    Napi::TypeError::New(env, "Invalid argument type (expect object)")
        .ThrowAsJavaScriptException();
    return Napi::Number::New(env, CANAL_ERROR_PARAMETER);
  }

  Napi::Object obj = Napi::Object::New(env);

  canalStatistics canStatistics;
  uint32_t rv = this->m_pcanalif->CanalGetStatistics(&canStatistics);
  if (CANAL_ERROR_SUCCESS == rv) {
    obj.Set("cntReceiveFrames", uint32_t(canStatistics.cntReceiveFrames));
    obj.Set("cntTransmitFrames", uint32_t(canStatistics.cntTransmitFrames));
    obj.Set("cntReceiveData", uint32_t(canStatistics.cntReceiveData));
    obj.Set("cntReceiveData", uint32_t(canStatistics.cntTransmitData));
    obj.Set("cntReceiveData", uint32_t(canStatistics.cntOverruns));
    obj.Set("cntReceiveData", uint32_t(canStatistics.cntBusWarnings));
    obj.Set("cntReceiveData", uint32_t(canStatistics.cntBusOff));
  }

  Napi::Function cb = info[0].As<Napi::Function>();
  cb.MakeCallback(env.Global(), {obj});

  return Napi::Number::New(env, rv);
}

/*!
    setFilter
*/

Napi::Value CNodeCanal::setFilter(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  if (1 != info.Length()) {
    Napi::TypeError::New(env, "Invalid argument count")
        .ThrowAsJavaScriptException();
    return Napi::Number::New(env, CANAL_ERROR_PARAMETER);
  }

  if (!info[0].IsNumber()) {
    Napi::TypeError::New(env, "Invalid argument type (expect object)")
        .ThrowAsJavaScriptException();
    return Napi::Number::New(env, CANAL_ERROR_PARAMETER);
  }

  uint32_t filter = (uint32_t)info[0].As<Napi::Number>();
  uint32_t rv = this->m_pcanalif->CanalSetFilter(filter);
  return Napi::Number::New(env, rv);
}

/*!
    setMask
*/

Napi::Value CNodeCanal::setMask(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  if (1 != info.Length()) {
    Napi::TypeError::New(env, "Invalid argument count")
        .ThrowAsJavaScriptException();
    return Napi::Number::New(env, CANAL_ERROR_PARAMETER);
  }

  if (!info[0].IsNumber()) {
    Napi::TypeError::New(env, "Invalid argument type (expect object)")
        .ThrowAsJavaScriptException();
    return Napi::Number::New(env, CANAL_ERROR_PARAMETER);
  }

  uint32_t mask = (uint32_t)info[0].As<Napi::Number>();
  uint32_t rv = this->m_pcanalif->CanalSetMask(mask);
  return Napi::Number::New(env, rv);
}

/*!
    setBaudrate
*/

Napi::Value CNodeCanal::setBaudrate(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  if (1 != info.Length()) {
    Napi::TypeError::New(env, "Invalid argument count")
        .ThrowAsJavaScriptException();
    return Napi::Number::New(env, CANAL_ERROR_PARAMETER);
  }

  if (!info[0].IsNumber()) {
    Napi::TypeError::New(env, "Invalid argument type (expect object)")
        .ThrowAsJavaScriptException();
    return Napi::Number::New(env, CANAL_ERROR_PARAMETER);
  }

  uint32_t baud = (uint32_t)info[0].As<Napi::Number>();
  uint32_t rv = this->m_pcanalif->CanalSetBaudrate(baud);
  return Napi::Number::New(env, rv);
}

/*!
    getLevel
*/

Napi::Value CNodeCanal::getLevel(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  uint32_t level = this->m_pcanalif->CanalGetLevel();
  return Napi::Number::New(env, level);
}

/*!
    getVersion
*/

Napi::Value CNodeCanal::getVersion(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  uint32_t version = this->m_pcanalif->CanalGetVersion();
  return Napi::Number::New(env, version);
}

/*!
    getDllVersion
*/

Napi::Value CNodeCanal::getDllVersion(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  uint32_t version = this->m_pcanalif->CanalGetDllVersion();
  return Napi::Number::New(env, version);
}

/*!
    getVendorString
*/

Napi::Value CNodeCanal::getVendorString(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  const char *pVendorStr = this->m_pcanalif->CanalGetVendorString();
  return Napi::String::New(env, pVendorStr);
}

///////////////////////////////////////////////////////////////////////////////
// getDriverInfo
//

Napi::Value CNodeCanal::getDriverInfo(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  const char *pDriverInfoStr = this->m_pcanalif->CanalGetDriverInfo();
  return Napi::String::New(env, pDriverInfoStr);
}

///////////////////////////////////////////////////////////////////////////////
// asyncReceive
//

Napi::Value CNodeCanal::asyncReceive(const Napi::CallbackInfo &info) {

  napi_value work_name;

  // thdata->pif = m_pcanalif;
  Napi::Env env = info.Env();

  if (info.Length() < 2) {
    throw Napi::TypeError::New(env, "Expected two arguments");
  } else if (!info[0].IsFunction()) {
    throw Napi::TypeError::New(env, "Expected first arg to be function");
  } else if (!info[1].IsNumber()) {
    throw Napi::TypeError::New(env, "Expected second arg to be number");
  }

  // This string describes the asynchronous work.
  napi_create_string_utf8(env, "Thread-safe Function Round Trip Example",
                          NAPI_AUTO_LENGTH, &work_name);

  // Create a ThreadSafeFunction
  m_pcanalif->tsfn = Napi::ThreadSafeFunction::New(
      env,
      info[0].As<Napi::Function>(), // JavaScript function called asynchronously
      work_name,                    // Name
      0,                            // Unlimited queue
      1,                            // Only one thread will use this initially
      [](Napi::Env) {               // Finalizer used to clean threads up
        nativeThread.join();
      });

  // Create a native thread
  void *data = (void *)m_pcanalif;
  nativeThread = std::thread([data] {
    CCanalIf *pif = (CCanalIf *)data;

    auto callback = [](Napi::Env env, 
                        Napi::Function jsCallback,
                        canalMsg *pmsg) {

      Napi::Array dataArray = Napi::Array::New(env, pmsg->sizeData);
      for (uint32_t i = 0; i < pmsg->sizeData; i++) {
        dataArray[uint32_t(i)] =
            Napi::Number::New(env, pmsg->data[i]);
      }

      Napi::Object obj = Napi::Object::New(env);
      obj.Set("id", uint32_t(pmsg->id));
      obj.Set("flags", uint32_t(pmsg->flags));
      obj.Set("obid", uint32_t(pmsg->obid));
      obj.Set("sizeData", uint32_t(pmsg->sizeData));
      obj.Set("timestamp", uint32_t(pmsg->timestamp));
      obj.Set("data", dataArray );
      jsCallback.Call({obj});

      // We're finished with the data.
      delete pmsg;
    };

    canalMsg msg;
    while (!pif->m_bQuit) {

      if (CANAL_ERROR_SUCCESS ==
          pif->m_proc_CanalBlockingReceive(pif->m_openHandle, &msg, 500)) {
        canalMsg *pmsg = new canalMsg();
        memcpy(pmsg, &msg, sizeof(canalMsg));
        napi_status status = pif->tsfn.BlockingCall(pmsg, callback);
        if (status != napi_ok) {
          // Handle error
          delete pmsg;
        }
      }
    }

    tsfn.Release();

  });

  return Napi::Boolean::New(env, true);
}

