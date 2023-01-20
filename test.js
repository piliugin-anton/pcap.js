//const SegfaultHandler = require('segfault-handler');
//SegfaultHandler.registerHandler('crash.log');

const { PCap } = require("./build/Debug/pcapjs.node");
console.log(PCap.findDevice())
const instance = new PCap("wlp3s0f0", (buffer) => console.log("[JS_onPacket]", buffer));
try {
    //instance.startCapture();
    setTimeout(() => console.log(Date.now(), instance.stopCapture()), 5000);
} catch (ex) {
    console.error(ex);
}

