
var Magic = require('magic');
var magic = Magic.create(Magic.MAGIC_MIME);

magic.file("magic.cc", function(err, result) {
	console.log(result);
})
