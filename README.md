# pcap.js
Packet manipulation library for Node.js

Capture privileges:

sudo setcap cap_net_raw,cap_net_admin+eip $(eval readlink -f `which node`)

or

chown root `which node` && chmod u+s `which node`

### License [GNU General Public License v3.0](./LICENSE)