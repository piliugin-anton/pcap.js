const { PCap } = require("./build/Debug/pcapjs.node");

const instance = new PCap("lo", () => {});
try {
    while (true) instance.sendPacket(Buffer.allocUnsafe(60).fill(0));
} catch (ex) {
    console.error(ex);
}

