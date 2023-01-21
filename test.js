const { PCap } = require("./build/Debug/pcapjs.node");

const instance = new PCap("wlp3s0f0", (buffer, isTruncated, timestamp) => console.log("[JS_onPacket]", buffer, isTruncated, timestamp));
try {
    instance.startCapture();
    //setInterval(() => console.log(Date.now()), 1000);
    setTimeout(() => instance.stopCapture(), 10000);
} catch (ex) {
    console.error(ex);
}

