const { PCap } = require("./build/Debug/pcapjs.node");

const instance = new PCap("lo", () => {});
try {
    while (true) instance.sendPacket(Buffer.allocUnsafe(1400));
} catch (ex) {
    console.error(ex);
}

