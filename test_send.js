const { PCap } = require("./");

const instance = new PCap("lo", () => {});
try {
    while (true) instance.sendPacket(Buffer.allocUnsafe(1400));
} catch (ex) {
    console.error(ex);
}

