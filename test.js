const { PCap, CONSTANTS } = require("./");

const instance = new PCap({
  device: "lo",
  callback: (buffer, isTruncated, timestamp) => {
    const currentDate = Date.now();
    if (currentDate - date >= 1000) {
      const currentStats = instance.getStats();
      const dropped = currentStats.dropped - stats.dropped;
      const pps = currentStats.received - stats.received - dropped;
      console.log("[JS_onPacket]", `${pps}/sec, dropped: ${dropped}`);
      stats = currentStats;
      date = currentDate;
    }
  },
});
let stats = instance.getStats();
let date = Date.now();
try {
  instance.startCapture(CONSTANTS.DIRECTION.BOTH);
  setTimeout(() => instance.stopCapture(), 10000);
} catch (ex) {
  console.error(ex);
}
