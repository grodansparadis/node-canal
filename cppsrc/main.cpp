/* cppsrc/main.cpp */
#include "samples/classexample.h"
#include "samples/functionexample.h"
#include <canal.h>
#include <canal_macro.h>
#include <canaldlldef.h>
#include <canalif.h>
#include <napi.h>
#include <node-canal.h>

using namespace Napi;

Napi::Object InitAll(Napi::Env env, Napi::Object exports) {
  functionexample::Init(env, exports);
  ClassExample::Init(env, exports);
  return CNodeCanal::Init(env, exports);
}

// ----

static Value doSomethingUsefulWithData(Env env, void *data) {
  // Convert `data` into a JavaScript value and return it.
}

// Runs on the JS thread.
static void FinalizeTSFN(napi_env env, void *data, void *context) {
  // This is where you would wait for the threads to quit. This
  // function will only be called when all the threads are done using
  // the tsfn so, presumably, they can be joined here.
}

// Runs on the JS thread.
static void CallIntoJS(napi_env env, napi_value js_cb, void *context,
                       void *data) {
  if (env != nullptr && js_cb != nullptr) {

    // `data` was passed to `napi_call_threadsafe_function()` by one
    // of the threads. The order in which the threads add data as
    // they call `napi_call_threadsafe_function()` is the order in
    // which they will be given to this callback.
    //
    // `context` was passed to `napi_create_threadsafe_function()` and
    // is being provided here.
    //
    // Function::New(env, js_cb).Call({
    //   // DoSomethingUsefulWithData(env, data)
    // });

  } else {
    // The tsfn is in the process of getting cleaned up and there are
    // still items left on the queue. This function gets called with
    // each `void*` item, but with `env` and `js_cb` set to `NULL`,
    // because calls can no longer be made into JS, but the `void*`s
    // may still need to be freed.
  }
}

// Runs on the JS thread.
static void CreateThreadsafeCallback(const CallbackInfo &info) {
  if (!info[0].IsFunction()) {
    Error::New(info.Env(), "First argument must be a function")
        .ThrowAsJavaScriptException();
    return;
  }

  napi_threadsafe_function tsfn;

  NAPI_THROW_IF_FAILED_VOID(
      info.Env(),
      napi_create_threadsafe_function(
          info.Env(), info[0], nullptr,
          String::New(info.Env(), "My thread-safe function"),
          0,            // for an unlimited queue size
          1,            // initially only used from the main thread
          nullptr,      // data to make use of during finalization
          FinalizeTSFN, // gets called when the tsfn goes out of use
          nullptr,      // data that can be set here and retrieved on any thread
          CallIntoJS,   // function to call into JS
          &tsfn));

  // Now you can pass `tsfn` to any number of threads. Each one must
  // first call `napi_threadsafe_function_acquire()`. Then it may call
  // `napi_call_threadsafe_function()` any number of times. If on one
  // of those occasions the return value from
  // `napi_call_threadsafe_function()` is `napi_closing`, the thread
  // must make no more calls to any of the thread-safe function APIs.
  // If it never receives `napi_closing` from
  // `napi_call_threadsafe_function()` then, before exiting, the
  // thread must call `napi_release_threadsafe_function()`.
}

// ----

NODE_API_MODULE(NODE_GYP_MODULE_NAME, InitAll)