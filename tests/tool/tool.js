var dfu = require('web-bluetooth-dfu').dfu;
var hex2bin = require('web-bluetooth-dfu').hex2bin;
var fs = require('fs');

var log = console.log;
dfu.addLogger(log);
hex2bin.addLogger(log);

var fileName = null;

dfu.findDevice({ name: 'DfuTarg' })
.then(device => {
    fileName = process.argv[2]; 
	log("found device: " + device.name);
    log("using file name: " + fileName);

	return dfu.writeMode(device);
})
.then(() => dfu.findDevice({ name: "DfuTarg" }))
.then(device => {
    var file = fs.readFileSync(fileName);
    var hex = file.toString();
    var buffer = hex2bin.convert(hex);
    log("file length: " + buffer.byteLength);

    return dfu.provision(device, buffer);
})
.then(() => process.exit())
.catch(error => {
    log(error);
    process.exit();
});
