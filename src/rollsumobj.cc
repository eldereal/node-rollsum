#include <node.h>
#include "rollsumobj.h"

using namespace v8;
using namespace node;

Persistent<Function> RollsumObj::constructor;

RollsumObj::RollsumObj(unsigned char * buffer, uint32_t size, uint32_t* hashBuffer, uint32_t hashSize) : 
	hashSize(hashSize),
	size(size),
	bufferSize(size + hashSize - 1){
	this->buffer = buffer;
	memset(this->buffer, 0, this->bufferSize);
	this->pos = 0;
	this->len = 0;
	this->hashes = hashBuffer;
	this->hashPos = 0;
	RollsumInit(&this->sum);
}

RollsumObj::~RollsumObj() {
	
}

void RollsumObj::Init(Handle<Object> exports) {
  // Prepare constructor template
  Local<FunctionTemplate> tpl = FunctionTemplate::New(New);
  tpl->SetClassName(String::NewSymbol("RollsumObj"));
  tpl->InstanceTemplate()->SetInternalFieldCount(1);
  // Prototype
  tpl->PrototypeTemplate()->Set(String::NewSymbol("feed"),
      FunctionTemplate::New(Feed)->GetFunction());
  tpl->PrototypeTemplate()->Set(String::NewSymbol("end"),
      FunctionTemplate::New(End)->GetFunction());
  tpl->PrototypeTemplate()->Set(String::NewSymbol("getLength"),
      FunctionTemplate::New(GetLength)->GetFunction());
  tpl->PrototypeTemplate()->Set(String::NewSymbol("getHashLength"),
      FunctionTemplate::New(GetHashLength)->GetFunction());
  constructor = Persistent<Function>::New(tpl->GetFunction());
  exports->Set(String::NewSymbol("RollsumObj"), constructor);
}

Handle<Value> RollsumObj::New(const Arguments& args) {
	HandleScope scope;

	if (args.IsConstructCall()) {
	// Invoked as constructor: `new RollsumObj(...)`
		if (args.Length() < 4) {
			ThrowException(Exception::TypeError(String::New("Wrong number of arguments")));
			return scope.Close(Undefined());
		}
		if(!Buffer::HasInstance(args[0]) || !Buffer::HasInstance(args[2])) { //判断是否是Buffer对象
			ThrowException(v8::Exception::TypeError(v8::String::New("Argument 1 and 3 must be Buffer objects")));  //抛出js异常 
		}
		if (!args[1]->IsNumber() || args[1]->NumberValue() <= 0 || !args[3]->IsNumber() || args[3]->NumberValue() <= 0) {
			ThrowException(Exception::RangeError(String::New("Wrong buffer size")));
			return scope.Close(Undefined());
		} 
		uint32_t size = (uint32_t) args[1]->NumberValue();	
		uint32_t hashSize = (uint32_t) args[3]->NumberValue();
		uint32_t bufferSize = Buffer::Length(args[0]->ToObject());
		if(bufferSize < size + hashSize - 1){
			ThrowException(v8::Exception::RangeError(v8::String::New("Argument 1: buffer size must be (size + hashSize - 1)")));
		}
		uint32_t hashBufferSize = Buffer::Length(args[2]->ToObject());
		if(hashBufferSize < sizeof(uint32_t)*hashSize){
			ThrowException(v8::Exception::RangeError(v8::String::New("Argument 3: buffer size is less than 4 times of hash size")));
		}
		unsigned char *buf = reinterpret_cast<unsigned char *>(Buffer::Data(args[0]->ToObject()));		
		uint32_t *hashbuf = reinterpret_cast<uint32_t *>(Buffer::Data(args[2]->ToObject()));		
		RollsumObj* obj = new RollsumObj(buf, size, hashbuf, hashSize);
		obj->Wrap(args.This());
		return args.This();
	} else {
		// Invoked as plain function `RollsumObj(...)`, turn into construct call.
		const int argc = 4;
		Local<Value> argv[argc] = { args[0], args[1], args[2], args[3]};
		return scope.Close(constructor->NewInstance(argc, argv));
	}
}

void RollsumObj::ProcessBuffer(char* buf, uint32_t beginIndex, uint32_t length, v8::Handle<v8::Function> &cb){
	uint32_t bufRemained = this->bufferSize - this->pos;
	while(beginIndex + bufRemained < length){
		this->ProcessBufferInSingleChunk(buf, beginIndex, bufRemained, cb);
		beginIndex += bufRemained;
		bufRemained = this->bufferSize - this->pos;
	}
	if(length>beginIndex){
		this->ProcessBufferInSingleChunk(buf, beginIndex, length - beginIndex, cb);
	}
}

//  content [0 ... size-1 ... size+hashSize-2] size+hashSize-1
//  hash          [0      ... hashSize-1]      hashSize
//  content [hashSize  size+hashSize-2] size-1
void RollsumObj::ProcessBufferInSingleChunk(char* buf, uint32_t beginIndex, uint32_t length, v8::Handle<v8::Function> &cb){
	memcpy(this->buffer + this->pos, buf + beginIndex, length);
	this->len += length;
	this->pos += length;
	if(this->pos == this->bufferSize){
		if(this->len == this->bufferSize){//rollin first size-1 bits 
			RollsumInit(&this->sum);
			for(uint32_t i=0;i<this->size-1;i++){
				RollsumRollin(&this->sum, this->buffer[i]);
			}
		}
		for(uint32_t i=0; i<this->hashSize;i++){
			unsigned char rollin = this->buffer[this->size+i-1];
			RollsumRollin(&this->sum, rollin);
			this->hashes[i]=htonl(RollsumDigest(&this->sum));
			unsigned char rollout = this->buffer[i];
			RollsumRollout(&this->sum, rollout);
		}
		const unsigned argc = 1;
  		Local<Value> argv[argc] = { Local<Value>::New(Number::New(this->hashSize)) };
		cb->Call(Context::GetCurrent()->Global(), argc, argv);
		this->pos = this->size-1;
		for(uint32_t i=0;i<this->size-1;i++){//roll buffers
			this->buffer[i] = this->buffer[i+this->hashSize];
		}
	}
}

void RollsumObj::ProcessRemainedBuffer(v8::Handle<v8::Function> &cb){
	if(this->len < this->size){
		//no enough bytes feed, so there is no output;
		return;
	}
	uint32_t remainCount = this->pos - this->size + 1;
	for(uint32_t i=0; i<remainCount;i++){
		unsigned char rollin = this->buffer[this->size+i-1];
		RollsumRollin(&this->sum, rollin);
		this->hashes[i]=htonl(RollsumDigest(&this->sum));
		unsigned char rollout = this->buffer[i];
		RollsumRollout(&this->sum, rollout);
	}
	const unsigned argc = 1;
	Local<Value> argv[argc] = { Local<Value>::New(Number::New(remainCount)) };
	cb->Call(Context::GetCurrent()->Global(), argc, argv);
}

Handle<Value> RollsumObj::Feed(const Arguments& args) {
	HandleScope scope;
	RollsumObj* obj = ObjectWrap::Unwrap<RollsumObj>(args.This());
	if (args.Length() < 1) {
		ThrowException(Exception::TypeError(String::New("Wrong number of arguments")));
		return scope.Close(Undefined());
	}
	Local<Value> arg0 = args[0];
	if(!Buffer::HasInstance(arg0)) { //判断是否是Buffer对象
		ThrowException(v8::Exception::TypeError(v8::String::New("Argument 1 must be a Buffer object")));  //抛出js异常 
	}
	Local<Function> cb = Local<Function>::Cast(args[1]);

	uint32_t size = Buffer::Length(arg0->ToObject());
	char *buf = Buffer::Data(arg0->ToObject()); 
	obj->ProcessBuffer(buf, 0, size, cb);
	return scope.Close(Number::New(obj->pos));
}


Handle<Value> RollsumObj::End(const Arguments& args) {
	HandleScope scope;
	RollsumObj* obj = ObjectWrap::Unwrap<RollsumObj>(args.This());
	if (args.Length() < 1) {
		ThrowException(Exception::TypeError(String::New("Wrong number of arguments")));
		return scope.Close(Undefined());
	}
	Local<Function> cb = Local<Function>::Cast(args[0]);
	obj->ProcessRemainedBuffer(cb);
	return scope.Close(Number::New(obj->pos));
}

Handle<Value> RollsumObj::GetLength(const Arguments& args) {
	HandleScope scope;
	RollsumObj* obj = ObjectWrap::Unwrap<RollsumObj>(args.This());
	return scope.Close(Number::New(obj->len));
}

Handle<Value> RollsumObj::GetHashLength(const Arguments& args) {
	HandleScope scope;
	RollsumObj* obj = ObjectWrap::Unwrap<RollsumObj>(args.This());
	return scope.Close(Number::New(obj->hashPos));
}