node-rollsum: rollsum library for node.js
===

This library is a node.js wrapper of **librsync**'s rollsum module.

see test/test.js for more info. You can run it with `node test/test.js <filename>`.

My email is `eldereal@gmail.com` and any suggestions and discussions are welcomed.

Module
---

    var Rollsum = require("node-rollsum")
    var s = new Rollsum(ChunkSize, DigestSize);

Rollsum is a writable stream which you can write data into it. The constructor has two arguments: 

* `ChunkSize`: The size of the data that calculate one digest.
* `DigestSize`: When the digest is rolling, it is not efficient to get notified in each byte. So I use a buffer to store calculated digests. The buffer's size is DigestSize. When the buffer is full (or the input stream ends), a "digest" event is fire.

Events
---
 * `on('digest', function(length){ })`: when digest buffer is full filled or the stream ends, you will get notified by this event. The argument `length` represents the number of digests in the buffer. In most cases it should be equal to `DigestSize`. Only when the stream ends, the event will be fired with the number of remaining digests.
 * `on('end', function(){ })`: Called after the stream ends and the last `digest` event is fired.
 * `Rollsum` is a writable stream so other events of stream is also availiable.

Methods
---
 * `getLength()`: Get length in bytes of the processed data.
 * `getDigest(index)`: When the event `digest` is fired, you can get digests in the event handler. The argument `index` should be non-negative and less than the callback argument `length`. This function will return a 32-bit non-negative integer as the digest.
 * `getDigestData(index)`: You should call it in the callback of event `digest` just as `getDigest(index)`. Sometimes you want to know the actual data that calculates the digest. For example, in *rsync* algorithm, You should calculate a stronger digest of the content when the rolling digest matches the input. This function returns a Buffer which contains the desired data.
 