
#include "functionexample.h"

std::string functionexample::hello()
{
    // long ll =  vscphlp_newSession();
    // vscphlp_closeSession(123 );
    return "Hello World----";
}

int functionexample::add(int a, int b)
{
    return a + b;
}

Napi::String functionexample::HelloWrapped (const Napi::CallbackInfo& info )
{
  Napi::Env env = info.Env();
  Napi::String returnValue = Napi::String::New(env, functionexample::hello());

  return returnValue;
}

Napi::Number functionexample::AddWrapped(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    if (info.Length() < 2 || !info[0].IsNumber() || !info[1].IsNumber()) {
        Napi::TypeError::New(env, "Number expected").ThrowAsJavaScriptException();
    }

    Napi::Number first = info[0].As<Napi::Number>();
    Napi::Number second = info[1].As<Napi::Number>();

    int returnValue = functionexample::add(first.Int32Value(), second.Int32Value());

    return Napi::Number::New(env, returnValue);
}


Napi::Object functionexample::Init( Napi::Env env, Napi::Object exports )
{
    exports.Set( "hello",
                    Napi::Function::New( env, 
                        functionexample::HelloWrapped ) );
    exports.Set( "add",
                    Napi::Function::New(env, 
                        functionexample::AddWrapped));
    return exports;
}



/*

// https://github.com/nodejs/node-addon-api/issues/432

FunctionReference r_log;
void emitLogInJS(char* msg) {
  if (r_log != nullptr) {
    Env env = r_log.Env();
    String message = String::New(env, msg);
    std::vector<napi_value> args = {message}; 
    r_log.Call(args);
  } 
}
void register_logger(const CallbackInfo& info) {
  r_log = Persistent(info[0].As<Function>());
  myclibrary_register_logger(emitLogInJS);
}
*/