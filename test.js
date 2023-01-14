const pcapjs = require("./build/Debug/pcapjs.node");
//console.log(pcapjs)

try {
    const devices = pcapjs.listDevices();
    console.log(devices);
} catch (ex) {
    console.error(ex);
}
