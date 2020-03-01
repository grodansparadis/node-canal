///////////////////////////////////////////////////////////////////////////
// node-canal.h
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

#include <pthread.h>
#include "canalif.h"
#include <napi.h>

#include <thread>

struct tsfnContext {

  tsfnContext(Napi::Env env) : deferred(Napi::Promise::Deferred::New(env)) {
    ;
  };

  // Native Promise returned to JavaScript
  Napi::Promise::Deferred deferred;

  // CANAL interface
  CCanalIf *m_pif;

  // Native thread
  std::thread workThread;

  Napi::ThreadSafeFunction tsfn;
};

class CNodeCanal : public Napi::ObjectWrap<CNodeCanal> {
public:
  static Napi::Object
  Init(Napi::Env env,
       Napi::Object exports); // Init function for setting the export key to JS
  CNodeCanal(const Napi::CallbackInfo &info); // Constructor to initialise

  CCanalIf *getIfPointer() { return m_pcanalif; };

private:
  static Napi::FunctionReference
      constructor; // reference to store the class definition that needs to be
                   // exported to JS

  // Wrapper for init
  Napi::Value init(const Napi::CallbackInfo &info); 

  // Wrapper for CanalOpen
  Napi::Value open(const Napi::CallbackInfo &info);

  // Wrapper for CanalClose
  Napi::Value close(const Napi::CallbackInfo &info);

  // Wrapper for CanalSend
  Napi::Value send(const Napi::CallbackInfo &info);

  // Wrapper for CanalReceive
  Napi::Value receive(const Napi::CallbackInfo &info);
  
  Napi::Value
  dataAvailable(const Napi::CallbackInfo &info); // wrapped add function
  
  // Wrapper for CanalGetStatus
  Napi::Value getStatus(const Napi::CallbackInfo &info);

  // Wrapper for CanalGetStatistics
  Napi::Value getStatistics(const Napi::CallbackInfo &info);

  // Wrapper for CanalSetFilter
  Napi::Value setFilter(const Napi::CallbackInfo &info);

  // Wrapper for CanalSetMask
  Napi::Value setMask(const Napi::CallbackInfo &info);

  // Wrapper for CanalSetBaudrate
  Napi::Value setBaudrate(const Napi::CallbackInfo &info);

  // Wrapper for CanalGetLevel
  Napi::Value getLevel(const Napi::CallbackInfo &info);

  // Wrapper for CanalGetVersion
  Napi::Value getVersion(const Napi::CallbackInfo &info);

  // Wrapper for CanalGetDllVersion
  Napi::Value getDllVersion(const Napi::CallbackInfo &info);

  // Wrapper for CanalGetVendorString
  Napi::Value getVendorString(const Napi::CallbackInfo &info);

  // Wrapper for CanalGetDriverInfo
  Napi::Value getDriverInfo(const Napi::CallbackInfo &info);

  // Wrapper for CanalGetDriverInfo
  bool addListener(Napi::Env &env, Napi::Function &callback);
  //Napi::Value addListener(const Napi::CallbackInfo &info);

  Napi::Function m_callback;

  CCanalIf *m_pcanalif; // internal instance of CCanalIf used to perform actual
                        // operations.                        
};