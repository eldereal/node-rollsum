var fs = require('fs');
var crypto = require('crypto');

var hash = new Adler32RollingHash(512, 512);
var hashes = [];
var hashCount = 0;

hash.on('hash', function(length){
  hashCount+=length;
  var h = hash.getHash(0);
  var md5 = crypto.createHash('md5').update(hash.getHashData(0)).digest();
  hashes.push({weak: h, md5: md5});
}).on('end', function(){
  //console.info("hash results ",hashes);
  console.info("hash count", hashCount);
  console.info("file length ",hash.getLength());
});

var r = fs.createReadStream(process.argv[2]);
r.pipe(hash);