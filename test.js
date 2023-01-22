const { PCap } = require("./build/Debug/pcapjs.node");

const instance = new PCap("wlp3s0f0", (buffer, isTruncated, timestamp) => console.log("[JS_onPacket]", buffer, isTruncated, timestamp));
try {
    console.log("Starting capture...")
    instance.startCapture();
    const interval = setInterval(() => console.log("TS", Date.now()), 1000);
    setTimeout(() => {
        console.log("Stopping capture...")
        instance.stopCapture()
    }, 10000);
    setTimeout(() => {
        console.log("Starting capture...")
        instance.startCapture()
    }, 20000);
    setTimeout(() => {
        console.log("Stopping capture... Clearing interval...")
        instance.stopCapture()
        clearInterval(interval)
    }, 30000);
} catch (ex) {
    console.error(ex);
}

