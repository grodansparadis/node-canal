/* cppsrc/main.cpp */
#include <canal.h>
#include <canal_macro.h>
#include <canaldlldef.h>
#include <canalif.h>
#include <napi.h>
#include <node-canal.h>

using namespace Napi;

Napi::Object InitAll(Napi::Env env, Napi::Object exports) {
  return CNodeCanal::Init(env, exports);
}

NODE_API_MODULE(NODE_GYP_MODULE_NAME, InitAll)