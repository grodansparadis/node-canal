/* cppsrc/Samples/CNodeCanal.cpp */

#include "node-canal.h"

Napi::FunctionReference CNodeCanal::constructor;

Napi::Object CNodeCanal::Init(Napi::Env env, Napi::Object exports) {
  Napi::HandleScope scope(env);

  Napi::Function func = DefineClass(
      env, "CNodeCanal",
      {
          InstanceMethod("init", &CNodeCanal::init),
          InstanceMethod("open", &CNodeCanal::open),
          InstanceMethod("close", &CNodeCanal::close),
          InstanceMethod("send", &CNodeCanal::send),
          InstanceMethod("receive", &CNodeCanal::receive),
          InstanceMethod("dataAvailable", &CNodeCanal::dataAvailable),
      });

  constructor = Napi::Persistent(func);
  constructor.SuppressDestruct();

  exports.Set("CNodeCanal", func);
  return exports;
}

CNodeCanal::CNodeCanal(const Napi::CallbackInfo &info)
    : Napi::ObjectWrap<CNodeCanal>(info) {

  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);

  // int length = info.Length();
  // if (length != 1 || !info[0].IsNumber()) {
  //   // Napi::TypeError::New(env, "Number
  //   // expected").ThrowAsJavaScriptException();
  // }

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
                                      flags.ToNumber());

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

  // flags, canid, data
  if (info.Length() != 4 || !info[0].IsNumber() || !info[1].IsNumber() ||
      !info[2].IsNumber() || !info[3].IsObject()) {
    Napi::TypeError::New(env,
                         "Three arguments expected (flags,canid,data-array)")
        .ThrowAsJavaScriptException();
  }

  Napi::Number flags = info[0].As<Napi::Number>();
  Napi::Number timestamp = info[1].As<Napi::Number>();
  Napi::Number canid = info[2].As<Napi::Number>();
  Napi::Array data_array = info[3].As<Napi::Array>();

  canalMsg canmsg;
  memset(&canmsg, 0, sizeof(canmsg));
  canmsg.flags = (uint32_t)flags.ToNumber();
  canmsg.timestamp = (uint32_t)timestamp.ToNumber();
  canmsg.id = (uint32_t)canid.ToNumber();
  canmsg.sizeData = data_array.Length();
  for (uint32_t i = 0; i < data_array.Length(); i++) {
    Napi::Value val = data_array[i];
    if (val.IsNumber()) {
      canmsg.data[i] = (int)val.As<Napi::Number>();
    }
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
    Napi::TypeError::New(env, "Invalid argument type")
        .ThrowAsJavaScriptException();
    return Napi::Number::New(env, CANAL_ERROR_PARAMETER);
  }

  Napi::Object obj = Napi::Object::New(env);

  canalMsg canmsg;
  memset(&canmsg, 0, sizeof(canmsg));

  double rv = this->m_pcanalif->CanalReceive(&canmsg);
  if (CANAL_ERROR_SUCCESS == rv) {
    Napi::Array dataArray = Napi::Array::New(Env(), canmsg.sizeData);
    for ( uint32_t i=0; i<canmsg.sizeData; i++ ) {
      dataArray[uint32_t(i)] =  Napi::Number::New(info.Env(), canmsg.data[i]);  
    }
    obj.Set("flags", uint32_t(canmsg.flags));
    obj.Set("canid", uint32_t(canmsg.id));
    obj.Set("obid", uint32_t(canmsg.obid));
    obj.Set("typestamp", uint32_t(canmsg.timestamp));
    obj.Set("sizeData", uint32_t(canmsg.sizeData));
    obj.Set("data", dataArray);
  }

  Napi::Function cb = info[0].As<Napi::Function>();
  cb.MakeCallback(env.Global(), { obj } ); // {env.Null(), obj}

  return Napi::Number::New(env, rv);
}
