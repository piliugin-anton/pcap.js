const { PCap } = require("./");
const instance = new PCap("wlp3s0f0", (...args) => console.log("onPacket", args));

try {
    instance.startCapture();
} catch (ex) {
    console.error(ex);
}

