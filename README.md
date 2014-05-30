node-rollsum: rollsum library for node.js
---

This library is a node.js wrapper of **librsync**'s rollsum module.

	var fs = require('fs');
	var Rollsum = require('node-rollsum');
	
	var rollsum = new Rollsum();
	var fileReadStream = fs.createReadStream(process.argv[2]);
	fileReadStream.pipe(rollsum);
	
	rollsum.on('hash', function(length)){
	
	}