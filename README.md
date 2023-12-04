# pcap.js

### Packet manipulation library for Node.js

1. Set capture privileges for Node.js:

``sudo setcap cap_net_raw,cap_net_admin+eip $(eval readlink -f `which node`)``

or

``chown root $(eval readlink -f `which node`) && chmod u+s $(eval readlink -f `which node`)``

2. `npm install`

### License [GNU General Public License v3.0](./LICENSE)