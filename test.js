const pcapjs = require("./");
console.log(pcapjs)

try {
    const devices = pcapjs.listDevices();
    comsole.log(devices);
} catch (ex) {
    console.error(ex);
}
