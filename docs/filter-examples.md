# Filter Examples

```javascript
// TCP packets
tcp

// TCP packets except for IPv6
tcp && !ipv6

// TCP packets with ACK flag
tcp.flags.ack

// IPv4 packets with source address `192.168.2.101`
ipv4.src == `192.168.2.101`

// IPv4 packets with source address `192.168.2.*`
ipv4.src >= `192.168.2.0` && ipv4.src <= `192.168.2.255`
```
