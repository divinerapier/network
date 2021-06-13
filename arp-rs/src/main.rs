use std::net::Ipv4Addr;
use std::thread;
use std::time::Duration;

use clap::{crate_authors, crate_version};
use clap::{AppSettings, Clap};
use pnet::datalink::Channel;
use pnet::datalink::{self, NetworkInterface};
use pnet::packet::arp::Arp;
use pnet::packet::arp::{ArpHardwareTypes, ArpOperation, ArpOperations};
use pnet::packet::ethernet::EtherTypes;
use pnet::packet::ethernet::Ethernet;
use pnet::packet::ethernet::MutableEthernetPacket;
use pnet::packet::arp::MutableArpPacket;
use pnet::util::MacAddr;
use pnet::packet::{MutablePacket, Packet};

#[derive(Clap)]
#[clap(version = crate_version!(), author = crate_authors!())]
#[clap(setting = AppSettings::ColoredHelp)]
struct Options {
    #[clap(short, long)]
    pub reply: bool,
    #[clap(short, long)]
    pub interface: Option<String>,
    #[clap(long = "sip")]
    pub source_ip: Option<Ipv4Addr>,
    #[clap(long = "smac")]
    pub source_mac: Option<MacAddr>,
    #[clap(long = "dip")]
    pub dest_ip: Option<Ipv4Addr>,
    #[clap(long = "dmac")]
    pub dest_mac: Option<MacAddr>,
}

fn main() {
    let opt: Options = Options::parse();
    loop {
        let interfaces = datalink::interfaces();
        // dbg!(interfaces);
        let interface = interfaces
            .into_iter()
            .filter(|iface| iface.name.eq(opt.interface.as_ref().unwrap()))
            .next()
            .unwrap();
        // dbg!(interface);
        send_arp(
            interface,
            opt.source_ip.as_ref().unwrap().to_owned(),
            opt.source_mac.as_ref().unwrap().to_owned(),
            opt.dest_ip.as_ref().unwrap().to_owned(),
            opt.dest_mac.as_ref().unwrap().to_owned(),
            opt.reply,
        );
        thread::sleep(Duration::from_secs(1));
    }
}

fn send_arp(
    iface: NetworkInterface,
    source_ip: Ipv4Addr,
    source_mac: MacAddr,
    destination_ip: Ipv4Addr,
    destination_mac: MacAddr,
    reply: bool,
) {
    let (mut tx, _) = match datalink::channel(&iface, Default::default()) {
        Ok(Channel::Ethernet(tx, rx)) => (tx, rx),
        Ok(_) => panic!("Unknown channel type"),
        Err(e) => panic!("Error happened {}", e),
    };
    let mut ethernet_buffer = [0u8; 42];
    let mut ethernet_packet = MutableEthernetPacket::new(&mut ethernet_buffer).unwrap();

    ethernet_packet.set_destination(destination_mac);
    ethernet_packet.set_source(source_mac);
    ethernet_packet.set_ethertype(EtherTypes::Arp);

    let mut arp_buffer = [0u8; 28];
    let mut arp_packet = MutableArpPacket::new(&mut arp_buffer).unwrap();

    arp_packet.set_hardware_type(ArpHardwareTypes::Ethernet);
    arp_packet.set_protocol_type(EtherTypes::Ipv4);
    arp_packet.set_hw_addr_len(6);
    arp_packet.set_proto_addr_len(4);
    arp_packet.set_operation(if reply {
        ArpOperations::Reply
    } else {
        ArpOperations::Request
    });
    arp_packet.set_sender_hw_addr(source_mac);
    arp_packet.set_sender_proto_addr(source_ip);
    arp_packet.set_target_hw_addr(destination_mac);
    arp_packet.set_target_proto_addr(destination_ip);

    ethernet_packet.set_payload(arp_packet.packet_mut());

    tx.send_to(&ethernet_packet.packet(), Some(iface));
}
