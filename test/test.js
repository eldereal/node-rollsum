if(process.argv.length<3){
	var path =  require("path");
	var p = path.relative(process.cwd(), process.argv[1]);
	console.error("Usage", process.argv[0], p, "filename");
	process.exit(1);
}

var fs = require('fs');
var crypto = require('crypto');
var rollsum = require('../lib/rollsum.js');

var digest = new rollsum(512, 512);
var digests = [];
var digestCount = 0;

digest.on('digest', function(length){
  digestCount+=length;
  var h = digest.getDigest(0).toString(16);
  var md5 = crypto.createHash('md5').update(digest.getDigestData(0)).digest("hex");
  digests.push({weak: h, md5: md5});
}).on('end', function(){
  console.info("digests count:", digestCount);
  console.info("digests for each 512 block:");
  console.info(digests);
  console.info("file length:",digest.getLength());
});

var r = fs.createReadStream(process.argv[2]);
r.pipe(digest);