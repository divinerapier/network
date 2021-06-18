
#include <pcap.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/if_ether.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <netinet/ip_icmp.h>
#include <string.h>

int linkhdrlen(pcap_t *pd) {
    int linktype;

    // Determine the datalink layer type.
    if ((linktype = pcap_datalink(pd)) < 0) {
        printf("pcap_datalink(): %s\n", pcap_geterr(pd));
        return -1;
    }

    // Set the datalink layer header size.
    switch (linktype) {
        case DLT_NULL:
            return 4;

        case DLT_EN10MB: // dstmac(6) | srcmac(6) | type(2)
            return 14;

        case DLT_SLIP:
        case DLT_PPP:
            return 24;

        default:
            printf("Unsupported datalink (%d)\n", linktype);
            return -1;
    }

}

void printmac(const char *data, int begin, int end) {
    printf("%x", data[begin]);
    for (int i = begin + 1; i < end; i++) {
        printf(":%x", data[i]);
    }
}

void print_datalink(const char *data) {
    printf("[DATALINK LAYER]");
    printf(" dstmac: ");
    printmac(data, 0, 6);
    printf(" srcmac: ");
    printmac(data, 6, 12);

}

char *dump_mac(const unsigned char *mac, char *output) {
    sprintf(output, "%02x:%02x:%02x:%02x:%02x:%02x",
            mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    return output;
}

void parse_packet(u_char *user, const struct pcap_pkthdr *packethdr,
                  const u_char *packetptr) {
    int linkhdrlen = *(int *) user;
//    printf("linkhdrlen: %d\n", linkhdrlen);
    struct ip *iphdr;
    struct icmp *icmphdr;
    struct tcphdr *tcphdr;
    struct udphdr *udphdr;
    struct ethhdr *ethhdr;
    struct ether_header *ether_header;
    char src_mac[6 * 2 + 5 + 1] = {0}, dst_mac[6 * 2 + 5 + 1] = {0};

    char iphdrInfo[256], srcip[256], dstip[256];

//    ethhdr = (struct ethhdr *) packetptr;
//    printf("DATALINK. DST MAC: %s SRC: MAC: %s PROTO: %d\n", ethhdr->h_dest, ethhdr->h_source, ethhdr->h_proto);
    ether_header = (struct ether_header *) packetptr;
    printf("DATALINK. DST MAC: %s SRC: MAC: %s PROTO: %d\n", dump_mac(ether_header->ether_dhost, dst_mac),
           dump_mac(ether_header->ether_shost, src_mac),
           ether_header->ether_type);
    // Skip the datalink layer header and get the IP header fields.
    packetptr += linkhdrlen;
    iphdr = (struct ip *) packetptr;
    strcpy(srcip, inet_ntoa(iphdr->ip_src));
    strcpy(dstip, inet_ntoa(iphdr->ip_dst));
    int ip_off = ntohs(iphdr->ip_off);
//    int ip_off = iphdr->ip_off;
    sprintf(iphdrInfo, "IP. VER: %d HDRLEN: %d TOS:0x%x LEN: %d BODYLEN: %d\n"
                       "ID: %d RF: %d DF: %d MF: %d FragOff: %d \n"
                       "TTL:%d PROTO:%d CHECKSUM:%d \n"
                       "SRC: %s\n"
                       "DST: %s\n",
            iphdr->ip_v, iphdr->ip_hl << 2, iphdr->ip_tos, ntohs(iphdr->ip_len),
            ntohs(iphdr->ip_len) - (iphdr->ip_hl << 2),
            ntohs(iphdr->ip_id), (ip_off & IP_RF) != 0, (ip_off & IP_DF) != 0, (ip_off & IP_MF) != 0,
            ip_off & IP_OFFMASK,
            iphdr->ip_ttl, iphdr->ip_p, ntohs(iphdr->ip_sum),
            srcip, dstip);
    int ip_body_len = ntohs(iphdr->ip_len) - (iphdr->ip_hl << 2);
    // Advance to the transport layer header then parse and display
    // the fields based on the type of hearder: tcp, udp or icmp.
    packetptr += 4 * iphdr->ip_hl;
    switch (iphdr->ip_p) {
        case IPPROTO_TCP:
            tcphdr = (struct tcphdr *) packetptr;
            printf("TCP. %s:%d -> %s:%d\n",
                   srcip, ntohs(tcphdr->th_sport),
                   dstip, ntohs(tcphdr->th_dport));
            printf("%s\n", iphdrInfo);
            int tcp_body_len = ip_body_len - (tcphdr->th_off << 2);
            printf("%c%c%c%c%c%c SEQ: 0x%x ACK: 0x%x WIN: 0x%x THOFF: 0x%x TCP BODY: 0x%x EXP ACK: 0x%x\n",
                   (tcphdr->th_flags & TH_URG ? 'U' : '*'),
                   (tcphdr->th_flags & TH_ACK ? 'A' : '*'),
                   (tcphdr->th_flags & TH_PUSH ? 'P' : '*'),
                   (tcphdr->th_flags & TH_RST ? 'R' : '*'),
                   (tcphdr->th_flags & TH_SYN ? 'S' : '*'),
                   (tcphdr->th_flags & TH_FIN ? 'F' : '*'),
                   ntohl(tcphdr->th_seq), ntohl(tcphdr->th_ack),
                   ntohs(tcphdr->th_win), tcphdr->th_off << 2, tcp_body_len,
                   ntohl(tcphdr->th_seq) +
                   ((((tcphdr->th_flags & TH_SYN) != 0) || ((tcphdr->th_flags & TH_FIN) != 0)) ? 1 : 0) + tcp_body_len);
            break;

        case IPPROTO_UDP:
            udphdr = (struct udphdr *) packetptr;
            printf("UDP  %s:%d -> %s:%d\n", srcip, ntohs(udphdr->uh_sport),
                   dstip, ntohs(udphdr->uh_dport));
            printf("%s\n", iphdrInfo);
            break;

        case IPPROTO_ICMP:
            icmphdr = (struct icmp *) packetptr;
            printf("ICMP %s -> %s\n", srcip, dstip);
            printf("%s\n", iphdrInfo);
            printf("Type:%d Code:%d ID:%d Seq:%d\n", icmphdr->icmp_type, icmphdr->icmp_code,
                   ntohs(icmphdr->icmp_hun.ih_idseq.icd_id), ntohs(icmphdr->icmp_hun.ih_idseq.icd_seq));
            break;
    }
    printf(
            "+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+\n\n");
}

int main(int argc, char **argv) {
    if (argc < 3) {
        printf("exec <iface> <bpf>\n");
        exit(255);
    }
    int i;
    char *dev = argv[1];
    char errbuf[PCAP_ERRBUF_SIZE];
    pcap_t *descr;
    const u_char *packet;
    struct pcap_pkthdr hdr;     /* pcap.h */
    struct ether_header *eptr;  /* net/ethernet.h */
    struct bpf_program filter;

    for (i = 0; i < argc; i++) {
        printf("[%d:%s]\t", i, argv[i]);
    }
    printf("\n");

//    if (argc != 2) {
//        fprintf(stdout, "Usage: %s numpackets\n", argv[0]);
//        return 0;
//    }

//    /* grab a device to peak into... */
//    dev = pcap_lookupdev(errbuf);
//    if (dev == NULL) {
//        printf("%s\n", errbuf);
//        exit(1);
//    }
    /* open device for reading */
    descr = pcap_open_live(dev, BUFSIZ, 0, -1, errbuf);
    if (descr == NULL) {
        printf("pcap_open_live(): %s\n", errbuf);
        exit(1);
    }
    printf("open live: %s\n", dev);

    if (0 != pcap_compile(descr, &filter, argv[2], 1, 0)) {
        char *errmsg = pcap_geterr(descr);
        fprintf(stderr, "complie bpf error: %s\n", errmsg);
        exit(2);
    }

    if (0 != pcap_setfilter(descr, &filter)) {
        char *errmsg = pcap_geterr(descr);
        fprintf(stderr, "set filter error: %s\n", errmsg);
        exit(2);
    }

    /* allright here we call pcap_loop(..) and pass in our callback function */
    /* int pcap_loop(pcap_t *p, int cnt, pcap_handler callback, u_char *user)*/
    /* If you are wondering what the user argument is all about, so am I!!   */
    int dthdrlen = linkhdrlen(descr);
    if (dthdrlen <= 0) {
        printf("invalid datalink layer header length: %d\n", dthdrlen);
        exit(2);
    }
    pcap_loop(descr, 0, parse_packet, (u_char *) &dthdrlen);

    fprintf(stdout, "\nDone processing packets... wheew!\n");
    pcap_close(descr);
    return 0;
}