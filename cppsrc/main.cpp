/* cppsrc/main.cpp */
#include <napi.h>
// #include <canal.h>
// #include <canal_macro.h>
// #include <canaldlldef.h>
#include "samples/functionexample.h"
#include "samples/classexample.h"

Napi::Object InitAll(Napi::Env env, Napi::Object exports) {
  functionexample::Init(env, exports);
  return ClassExample::Init(env, exports);
}

NODE_API_MODULE(NODE_GYP_MODULE_NAME, InitAll)