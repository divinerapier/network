# ARP

## ARP Request

### Header

``` pcap
Ethernet II, Src: IntelCor_40:ca:18 (74:d8:3e:40:ca:18), Dst: Broadcast (ff:ff:ff:ff:ff:ff)
    Destination: Broadcast (ff:ff:ff:ff:ff:ff)
    Source: IntelCor_40:ca:18 (74:d8:3e:40:ca:18)
    Type: ARP (0x0806)
```

### Body

``` pcap
Address Resolution Protocol (request)
    Hardware type: Ethernet (1)
    Protocol type: IPv4 (0x0800)
    Hardware size: 6
    Protocol size: 4
    Opcode: request (1)
    Sender MAC address: IntelCor_40:ca:18 (74:d8:3e:40:ca:18)
    Sender IP address: 192.168.50.85
    Target MAC address: 00:00:00_00:00:00 (00:00:00:00:00:00)
    Target IP address: 192.168.50.22
```

## ARP Reply

### Header

``` pcap
Ethernet II, Src: 9e:4e:34:df:47:58 (9e:4e:34:df:47:58), Dst: IntelCor_40:ca:18 (74:d8:3e:40:ca:18)
    Destination: IntelCor_40:ca:18 (74:d8:3e:40:ca:18)
    Source: 9e:4e:34:df:47:58 (9e:4e:34:df:47:58)
    Type: ARP (0x0806)
    Padding: 000000000000000000000000000000000000
```

### Body

``` pcap
Address Resolution Protocol (reply)
    Hardware type: Ethernet (1)
    Protocol type: IPv4 (0x0800)
    Hardware size: 6
    Protocol size: 4
    Opcode: reply (2)
    Sender MAC address: 9e:4e:34:df:47:58 (9e:4e:34:df:47:58)
    Sender IP address: 192.168.50.22
    Target MAC address: IntelCor_40:ca:18 (74:d8:3e:40:ca:18)
    Target IP address: 192.168.50.85
```
