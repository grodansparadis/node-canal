/* cppsrc/Samples/classexample.h */

#include "canalif.h"
#include <napi.h>

class CNodeCanal : public Napi::ObjectWrap<CNodeCanal> {
public:
  static Napi::Object
  Init(Napi::Env env,
       Napi::Object exports); // Init function for setting the export key to JS
  CNodeCanal(const Napi::CallbackInfo &info); // Constructor to initialise

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

  CCanalIf *m_pcanalif; // internal instance of CCanalIf used to perform actual
                        // operations.
};