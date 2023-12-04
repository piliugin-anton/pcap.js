# pcap.js
Packet manipulation library for Node.js

Capture privileges:

``sudo setcap cap_net_raw,cap_net_admin+eip $(eval readlink -f `which node`)``

or

``chown root $(eval readlink -f `which node`) && chmod u+s $(eval readlink -f `which node`)``

### License [GNU General Public License v3.0](./LICENSE)