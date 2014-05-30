#ifndef ROLLSUMOBJ_H
#define ROLLSUMOBJ_H

#include <node.h>
#include <node_buffer.h>
#include "rollsum.h"

class RollsumObj : public node::ObjectWrap {
 public:
  static void Init(v8::Handle<v8::Object> exports);

 private:
  RollsumObj(unsigned char * buffer, uint32_t size, uint32_t* hashBuffer, uint32_t hashSize);
  ~RollsumObj();

  static v8::Handle<v8::Value> New(const v8::Arguments& args);
  static v8::Handle<v8::Value> Feed(const v8::Arguments& args);
  static v8::Handle<v8::Value> End(const v8::Arguments& args);
  static v8::Handle<v8::Value> GetLength(const v8::Arguments& args);
  static v8::Handle<v8::Value> GetHashLength(const v8::Arguments& args);
  static v8::Persistent<v8::Function> constructor;
  uint32_t * hashes;
  const uint32_t hashSize;
  const uint32_t size;
  const uint32_t bufferSize;
  uint32_t pos;
  uint32_t hashPos;
  uint32_t len;
  unsigned char * buffer;
  Rollsum sum;
  void ProcessBuffer(char* buffer, uint32_t start, uint32_t length, v8::Handle<v8::Function> &cb);
  void ProcessBufferInSingleChunk(char* buffer, uint32_t start, uint32_t length, v8::Handle<v8::Function> &cb);
  void ProcessRemainedBuffer(v8::Handle<v8::Function> &cb);
};

#endif