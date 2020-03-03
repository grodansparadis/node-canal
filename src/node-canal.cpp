///////////////////////////////////////////////////////////////////////////
// node-canal.cpp
//
// VSCP to CAN conversion node.
//
// This file is part of the VSCP (https://www.vscp.org)
//
// The MIT License (MIT)
//
// Copyright © 2020 Ake Hedman, Grodans Paradis AB
// <info@grodansparadis.com>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//

#include <chrono>
#include <thread>

#include "node-canal.h"

// Workerthreads
void *deviceReceiveThread(void *pData);

Napi::FunctionReference CNodeCanal::constructor;

Napi::Object CNodeCanal::Init(Napi::Env env, Napi::Object exports) {

  Napi::HandleScope scope(env);

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
       InstanceMethod("getDriverInfo", &CNodeCanal::getDriverInfo)
       });

  constructor = Napi::Persistent(func);
  constructor.SuppressDestruct();

  exports.Set("CNodeCanal", func);

  // Error constants
  exports.Set("CANAL_ERROR_SUCCESS", Napi::Number::New(env,            0 )); /* All is OK */
  exports.Set("CANAL_ERROR_BAUDRATE", Napi::Number::New(env,           1 )); /* Baud rate error */
  exports.Set("CANAL_ERROR_BUS_OFF", Napi::Number::New(env,            2 )); /* Bus off error */
  exports.Set("CANAL_ERROR_BUS_PASSIVE", Napi::Number::New(env,        3 )); /* Bus Passive error */
  exports.Set("CANAL_ERROR_BUS_WARNING", Napi::Number::New(env,        4 )); /* Bus warning error */
  exports.Set("CANAL_ERROR_CAN_ID", Napi::Number::New(env,             5 )); /* Invalid CAN ID */
  exports.Set("CANAL_ERROR_CAN_MESSAGE", Napi::Number::New(env,        6 )); /* Invalid CAN message */
  exports.Set("CANAL_ERROR_CHANNEL", Napi::Number::New(env,            7 )); /* Invalid channel */
  exports.Set("CANAL_ERROR_FIFO_EMPTY", Napi::Number::New(env,         8 )); /* FIFO is empty */
  exports.Set("CANAL_ERROR_FIFO_FULL", Napi::Number::New(env,          9 )); /* FIFO is full */
  exports.Set("CANAL_ERROR_FIFO_SIZE", Napi::Number::New(env,         10 )); /* FIFO size error */
  exports.Set("CANAL_ERROR_FIFO_WAIT", Napi::Number::New(env,         11 ));
  exports.Set("CANAL_ERROR_GENERIC", Napi::Number::New(env,           12 )); /* Generic error */
  exports.Set("CANAL_ERROR_HARDWARE", Napi::Number::New(env,          13 )); /* Hardware error */
  exports.Set("CANAL_ERROR_INIT_FAIL", Napi::Number::New(env,         14 )); /* Initialization failed */
  exports.Set("CANAL_ERROR_INIT_MISSING", Napi::Number::New(env,      15 ));
  exports.Set("CANAL_ERROR_INIT_READY", Napi::Number::New(env,        16 ));
  exports.Set("CANAL_ERROR_NOT_SUPPORTED", Napi::Number::New(env,     17 )); /* Not supported */
  exports.Set("CANAL_ERROR_OVERRUN", Napi::Number::New(env,           18 )); /* Overrun */
  exports.Set("CANAL_ERROR_RCV_EMPTY", Napi::Number::New(env,         19 )); /* Register value error */
  exports.Set("CANAL_ERROR_TRM_FULL", Napi::Number::New(env,          21 ));
  exports.Set("CANAL_ERROR_ERRFRM_STUFF", Napi::Number::New(env,      22 )); /* Error frame: stuff error detected */
  exports.Set("CANAL_ERROR_ERRFRM_FORM", Napi::Number::New(env,       23 )); /* Error frame: form error detected */
  exports.Set("CANAL_ERROR_ERRFRM_ACK", Napi::Number::New(env,        24 )); /* Error frame: acknowledge error */
  exports.Set("CANAL_ERROR_ERRFRM_BIT1", Napi::Number::New(env,       25 )); /* Error frame: bit 1 error */
  exports.Set("CANAL_ERROR_ERRFRM_BIT0", Napi::Number::New(env,       26 )); /* Error frame: bit 0 error */
  exports.Set("CANAL_ERROR_ERRFRM_CRC", Napi::Number::New(env,        27 )); /* Error frame: CRC error */
  exports.Set("CANAL_ERROR_LIBRARY", Napi::Number::New(env,           28 )); /* Unable to load library */
  exports.Set("CANAL_ERROR_PROCADDRESS", Napi::Number::New(env,       29 )); /* Unable get library proc. address */
  exports.Set("CANAL_ERROR_ONLY_ONE_INSTANCE", Napi::Number::New(env, 30 )); /* Only one instance allowed */
  exports.Set("CANAL_ERROR_SUB_DRIVER", Napi::Number::New(env,        31 )); /* Problem with sub driver call */
  exports.Set("CANAL_ERROR_TIMEOUT", Napi::Number::New(env,           32 )); /* Blocking call time-out */
  exports.Set("CANAL_ERROR_NOT_OPEN", Napi::Number::New(env,          33 )); /* The device is not open. */
  exports.Set("CANAL_ERROR_PARAMETER", Napi::Number::New(env,         34 )); /* A parameter is invalid. */
  exports.Set("CANAL_ERROR_MEMORY", Napi::Number::New(env,            35 )); /* Memory exhausted. */
  exports.Set("CANAL_ERROR_INTERNAL", Napi::Number::New(env,          36 )); /* Some kind of internal program error */
  exports.Set("CANAL_ERROR_COMMUNICATION", Napi::Number::New(env,     37 ));
  
  /* ID flags             */
  exports.Set("CANAL_IDFLAG_STANDARD", Napi::Number::New(env, 0x00000000 ));  /* Standard message id (11-bit) */
  exports.Set("CANAL_IDFLAG_EXTENDED", Napi::Number::New(env, 0x00000001 ));  /* Extended message id (29-bit) */
  exports.Set("CANAL_IDFLAG_RTR", Napi::Number::New(env,      0x00000002 ));  /* RTR-Frame */
  exports.Set("CANAL_IDFLAG_STATUS", Napi::Number::New(env,   0x00000004 ));  /* This package is a status indication (id holds error code) */
  exports.Set("CANAL_IDFLAG_SEND", Napi::Number::New(env,     0x80000000 ));  /* Reserved for use by application software to indicate send */

  /* Communication speeds */
  exports.Set("CANAL_BAUD_USER", Napi::Number::New(env, 0 ));  /* User specified (In CANAL i/f DLL). */
  exports.Set("CANAL_BAUD_1000", Napi::Number::New(env, 1 ));  /*   1 Mbit */
  exports.Set("CANAL_BAUD_800", Napi::Number::New(env,  2 ));  /* 800 Kbit */
  exports.Set("CANAL_BAUD_500", Napi::Number::New(env,  3 ));  /* 500 Kbit */
  exports.Set("CANAL_BAUD_250", Napi::Number::New(env,  4 ));  /* 250 Kbit */
  exports.Set("CANAL_BAUD_125", Napi::Number::New(env,  5 ));  /* 125 Kbit */
  exports.Set("CANAL_BAUD_100", Napi::Number::New(env,  6 ));  /* 100 Kbit */
  exports.Set("CANAL_BAUD_50", Napi::Number::New(env,   7 ));  /*  50 Kbit */
  exports.Set("CANAL_BAUD_20", Napi::Number::New(env,   8 ));  /*  20 Kbit */
  exports.Set("CANAL_BAUD_10", Napi::Number::New(env,   9 ));  /*  10 Kbit */

  return exports;
}

CNodeCanal::CNodeCanal(const Napi::CallbackInfo &info)
    : Napi::ObjectWrap<CNodeCanal>(info) {

  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);

  // Napi::Number value = info[0].As<Napi::Number>();
  this->m_pcanalif = new CCanalIf();
}


///////////////////////////////////////////////////////////////////////////////
// init
//

Napi::Value CNodeCanal::init(const Napi::CallbackInfo &info) {

  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);

  if (info.Length() < 3) {
    Napi::TypeError::New(env, "Three or four arguments expected (path, param, flags[,function])")
        .ThrowAsJavaScriptException();
  }

  if ((3 == info.Length()) && 
      (!info[0].IsString() || !info[1].IsString() || !info[2].IsNumber())) {
    Napi::TypeError::New(env, "Three or four arguments expected (path, param, flags[,function])")
        .ThrowAsJavaScriptException();
  }

  if ((4 == info.Length()) && !info[3].IsFunction()) {
    Napi::TypeError::New(env, "Four arguments expected (path, param, flags, function)")
        .ThrowAsJavaScriptException();      
  }

  Napi::String path = info[0].As<Napi::String>();
  Napi::String param = info[1].As<Napi::String>();
  Napi::Number flags = info[2].As<Napi::Number>();

  if (4 == info.Length()) {
    m_callback = info[3].As<Napi::Function>();
  } 
  
  int rv = this->m_pcanalif->init(path.ToString(), 
                                  param.ToString(),
                                  (uint32_t)flags.ToNumber());

  // Start listener if init succeeded and we have a callback 
  // function. Poll otherwise
  if ( (CANAL_ERROR_SUCCESS == rv) && 
      (4 == info.Length()) ) {
     addListener(env, m_callback);
  }

  return Napi::Number::New(info.Env(), rv);
}

///////////////////////////////////////////////////////////////////////////////
// dataAvailable
// 

Napi::Value CNodeCanal::dataAvailable(const Napi::CallbackInfo &info) {
  
  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);

  uint32_t count = this->m_pcanalif->CanalDataAvailable();
  return Napi::Number::New(env, count);
}


///////////////////////////////////////////////////////////////////////////////
// open
//

Napi::Value CNodeCanal::open(const Napi::CallbackInfo &info) {

  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);

  int rv = this->m_pcanalif->CanalOpen();

  // if ( NULL != m_callback ) {
  //     addListener(env, m_callback);
  // }

  return Napi::Number::New(env, rv);
}

///////////////////////////////////////////////////////////////////////////////
// close
//

Napi::Value CNodeCanal::close(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);

  this->m_pcanalif->m_bQuit = true; // Quit the main loop
  std::this_thread::sleep_for(std::chrono::milliseconds(500));
  int rv = this->m_pcanalif->CanalClose();
  return Napi::Number::New(env, rv);
}

///////////////////////////////////////////////////////////////////////////////
// send
//

Napi::Value CNodeCanal::send(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  canalMsg canmsg;
  memset(&canmsg, 0, sizeof(canalMsg));

  if (5 == info.Length()) {
    // flags, id, data
    if (info[0].IsNumber() || info[1].IsNumber() || info[2].IsNumber() ||
        info[3].IsObject()) {

      Napi::Number flags = info[0].As<Napi::Number>();
      Napi::Number obid = info[1].As<Napi::Number>();
      Napi::Number timestamp = info[2].As<Napi::Number>();
      Napi::Number id = info[3].As<Napi::Number>();
      Napi::Array data_array = info[4].As<Napi::Array>();

      memset(&canmsg, 0, sizeof(canmsg));
      canmsg.flags = (uint32_t)flags.ToNumber();
      canmsg.obid = (uint32_t)obid.ToNumber();
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
          env, "Five arguments expected (flags,obid,timestamp,id,data-array) or object")
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

  int rv = this->m_pcanalif->CanalSend(&canmsg);
  return Napi::Number::New(env, rv);
}

///////////////////////////////////////////////////////////////////////////////
// receive
//

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

///////////////////////////////////////////////////////////////////////////////
// getStatus
//

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
  memset(&canStatus,0,sizeof(canalStatus));
  uint32_t rv = this->m_pcanalif->CanalGetStatus(&canStatus);
  if (CANAL_ERROR_SUCCESS == rv) {
    obj.Set("Channel_Status", uint32_t(canStatus.channel_status));
    obj.Set("LastErrorCode", uint32_t(canStatus.lasterrorcode));
    obj.Set("LastErrorSubCode", uint32_t(canStatus.lasterrorsubcode));
    obj.Set("LastErrorStr", canStatus.lasterrorstr);
  }

  Napi::Function cb = info[0].As<Napi::Function>();
  cb.MakeCallback(env.Global(), {obj});

  return Napi::Number::New(env, rv);
}

///////////////////////////////////////////////////////////////////////////////
// getStatistics
//

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
  memset(&canStatistics,0,sizeof(canalStatistics));
  uint32_t rv = this->m_pcanalif->CanalGetStatistics(&canStatistics);
  if (CANAL_ERROR_SUCCESS == rv) {
    obj.Set("cntReceiveFrames", uint32_t(canStatistics.cntReceiveFrames));
    obj.Set("cntTransmitFrames", uint32_t(canStatistics.cntTransmitFrames));
    obj.Set("cntReceiveData", uint32_t(canStatistics.cntReceiveData));
    obj.Set("cntTransmitData", uint32_t(canStatistics.cntTransmitData));
    obj.Set("cntOverruns", uint32_t(canStatistics.cntOverruns));
    obj.Set("cntBusWarnings", uint32_t(canStatistics.cntBusWarnings));
    obj.Set("cntBusOff", uint32_t(canStatistics.cntBusOff));
  }

  Napi::Function cb = info[0].As<Napi::Function>();
  cb.MakeCallback(env.Global(), {obj});

  return Napi::Number::New(env, rv);
}

///////////////////////////////////////////////////////////////////////////////
// setFilter
//

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

///////////////////////////////////////////////////////////////////////////////
// setMask
//

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

///////////////////////////////////////////////////////////////////////////////
// setBaudrate
//

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

///////////////////////////////////////////////////////////////////////////////
// getLevel
//

Napi::Value CNodeCanal::getLevel(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  uint32_t level = this->m_pcanalif->CanalGetLevel();
  return Napi::Number::New(env, level);
}

///////////////////////////////////////////////////////////////////////////////
// getVersion
//

Napi::Value CNodeCanal::getVersion(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  uint32_t version = this->m_pcanalif->CanalGetVersion();
  return Napi::Number::New(env, version);
}

///////////////////////////////////////////////////////////////////////////////
// getDllVersion
//

Napi::Value CNodeCanal::getDllVersion(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  uint32_t version = this->m_pcanalif->CanalGetDllVersion();
  return Napi::Number::New(env, version);
}

///////////////////////////////////////////////////////////////////////////////
// getVendorString
//

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

// The thread-safe function finalizer callback. This callback executes
// at destruction of thread-safe function, taking as arguments the finalizer
// data and threadsafe-function context.
void finalizerCallback( Napi::Env env, 
                          void *finalizeData,
                          tsfnContext *context ) {
  // Join the thread
  context->workThread.join();

  // Resolve the Promise previously returned to JS via the CreateTSFN method.
  context->deferred.Resolve(Napi::Boolean::New(env, true));
  delete context;
};

///////////////////////////////////////////////////////////////////////////////
// addListener
//

bool CNodeCanal::addListener(Napi::Env &env,
                                    Napi::Function &callback) 
{
  napi_value work_name;

  // This string describes the asynchronous work.
  napi_create_string_utf8( env, 
                            "addListner",
                            NAPI_AUTO_LENGTH, 
                            &work_name);

  // Construct context data
  auto context = new tsfnContext(env); 
  context->m_pif = m_pcanalif;                   

  // Create a ThreadSafeFunction
  context->tsfn = Napi::ThreadSafeFunction::New(
      env,
      callback,              // JavaScript function called asynchronously
      work_name,             // Name
      0,                     // Unlimited queue
      1,                     // Only one thread will use this initially
      context,               // Context,
      finalizerCallback,
      (void *)nullptr 
    );

  // Create a native thread

  void *data = (void *)context;
  context->workThread = std::thread([data] {

    tsfnContext *ctx = (tsfnContext *)data;

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
    while (!ctx->m_pif->m_bQuit) {

      // Sit and wait for connection if were not connected
      if ( 0 == ctx->m_pif->m_openHandle ) {
        std::this_thread::sleep_for(std::chrono::microseconds(500));
        continue;
      }

      if ( ctx->m_pif->m_openHandle && 
            (CANAL_ERROR_SUCCESS ==
          ctx->m_pif->m_proc_CanalBlockingReceive(ctx->m_pif->m_openHandle, 
                                                    &msg, 
                                                    500))) {
        canalMsg *pmsg = new canalMsg();
        memcpy(pmsg, &msg, sizeof(canalMsg));
        napi_status status = ctx->tsfn.BlockingCall(pmsg, callback);
        if (status != napi_ok) {
          // Handle error
          delete pmsg;
        }
        
      }
    }

    ctx->tsfn.Release();

  });

  return true;
}

