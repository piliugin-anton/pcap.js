const { PCap } = require("./build/Debug/pcapjs.node");

const instance = new PCap("lo", (buffer, isTruncated, timestamp) => console.log("[JS_onPacket]", instance.getStats()));
try {
    while (true) instance.sendPacket(Buffer.alloc(60));
} catch (ex) {
    console.error(ex);
}

