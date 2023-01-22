const { PCap } = require("./build/Debug/pcapjs.node");

const instance = new PCap("any", (buffer, isTruncated, timestamp) => console.log("[JS_onPacket]", buffer, isTruncated, timestamp, instance.getStats()));
try {
    const interval = setInterval(() => console.log(Date.now()), 1000)
    console.log("Starting capture...")
    instance.startCapture();
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

