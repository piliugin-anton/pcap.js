const pcapjs = require("./");
const { PCap } = pcapjs;
const instance = new PCap(11);
console.log(instance.listDevices())
instance.listDevices = null;
console.log(instance.listDevices())
/*
try {
    const devices = pcapjs.listDevices();
    console.log(devices);
} catch (ex) {
    console.error(ex);
}
*/
