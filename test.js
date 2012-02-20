
var Magic = require('./build/Release/magic');
var magic = Magic.create(Magic.MAGIC_MIME);
console.log(magic.file("magic.cc"));
