//const SegfaultHandler = require('segfault-handler');
//SegfaultHandler.registerHandler('crash.log');

const { PCap } = require("./build/Debug/pcapjs.node");
const instance = new PCap("wlp3s0f0", (...args) => console.log("[JS_onPacket]", args));
try {
    instance.startCapture();
} catch (ex) {
    console.error(ex);
}

