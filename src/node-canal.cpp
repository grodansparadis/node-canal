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
          InstanceMethod("DataAvailable", &CNodeCanal::DataAvailable),
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

  int length = info.Length();
  if (length != 1 || !info[0].IsNumber()) {
    // Napi::TypeError::New(env, "Number
    // expected").ThrowAsJavaScriptException();
  }

  // Napi::Number value = info[0].As<Napi::Number>();
  this->m_pcanalif = new CCanalIf();
}

Napi::Value CNodeCanal::DataAvailable(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);

  double num = this->m_pcanalif->CanalDataAvailable();
  return Napi::Number::New(env, num);
}

Napi::Value CNodeCanal::open(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);

  double num = this->m_pcanalif->CanalOpen();
  return Napi::Number::New(env, num);
}

Napi::Value CNodeCanal::close(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);

  double num = this->m_pcanalif->CanalClose();
  return Napi::Number::New(env, num);
}

Napi::Value CNodeCanal::send(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  // flags, canid, data
  if (info.Length() != 3 || !info[0].IsNumber() || !info[1].IsNumber() ||
      !info[2].IsObject()) {
    Napi::TypeError::New(env,
                         "Three arguments expected (flags,canid,data-array)")
        .ThrowAsJavaScriptException();
  }

  Napi::Number flags = info[0].As<Napi::Number>();
  Napi::Number canid = info[1].As<Napi::Number>();
  Napi::Array data_array = info[2].As<Napi::Array>();
  // info[2].As<Napi::Object>();

  canalMsg canmsg;
  memset(&canmsg, 0, sizeof(canmsg));
  canmsg.flags = (uint32_t)flags.ToNumber();
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