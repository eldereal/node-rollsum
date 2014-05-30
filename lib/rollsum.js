var util = require('util');
var Writable = require('stream').Writable;
var rollsum = require('../build/Release/rollsum.node');
 
/*
 * Create a rollsum object.
 *   chunkSize: how many bytes to calculate a digest.
 *   digestSize: how many digests to notify callback.
 */ 
function Rollsum(chunkSize, digestSize, options) {
  if(chunkSize<=0){
    throw new Error("Invalid arguments for rollinghash, chunkSize must be positive");
  }
  Writable.call(this, options);
  this.chunkSize = chunkSize;
  this.digestSize = digestSize;
  this.hashBuffer = new Buffer(digestSize*4);
  this.buffer = new Buffer(chunkSize + digestSize - 1);
  this.rollsum = new rollsum.RollsumObj(this.buffer, chunkSize, this.hashBuffer, digestSize);
  var $this = this;
  this.on('finish', function(){
    $this.rollsum.end(function(digestSize){
      $this.emit("digest", digestSize);
    });
    $this.emit('end');
  });
}
util.inherits(Rollsum, Writable);

function processBuffer($this, buf){
  $this.rollsum.feed(buf);
}

Rollsum.prototype._write = function (chunk, enc, cb) {
  var buf;
  if(Buffer.isBuffer(chunk)){
    buf = chunk;
  }else{
    buf = new Buffer(chunk, enc);
  }
  var $this = this;
  this.rollsum.feed(buf, function(digestSize){
    $this.emit("digest", digestSize);
  });
  cb();
};

Rollsum.prototype.getLength = function(){
  return this.rollsum.getLength();
}

Rollsum.prototype.getHash = function(i){
  var b = this.hashBuffer;
  return b[i*4] + b[1+i*4]*256 + b[2+i*4]*256*256 + b[3+i*4]*256*256*256;
}

Rollsum.prototype.getHashData = function(i){
  return this.buffer.slice(i, i+this.chunkSize);
}
