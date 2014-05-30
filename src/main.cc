#include <node.h>
#include "rollsumobj.h"

using namespace v8;

void InitAll(Handle<Object> exports) {
  RollsumObj::Init(exports);
}

NODE_MODULE(rollsum, InitAll)