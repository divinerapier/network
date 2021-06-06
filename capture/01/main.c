/* ldev.c
   Martin Casado

   To compile:
   >gcc ldev.c -lpcap

   Looks for an interface, and lists the network ip
   and mask associated with that interface.
*/
#include <stdio.h>
#include <stdlib.h>
#include <pcap/pcap.h>  /* GIMME a libpcap plz! */
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

const char *sockaddr_string(struct sockaddr *addr) {
    if (addr == NULL) {
        return "";
    }
    struct sockaddr_in *addr_in = (struct sockaddr_in *) addr;
    char *s = inet_ntoa(addr_in->sin_addr);
    return s;
}

int main(int argc, char **argv) {
    char *dev; /* name of the device to use */
    char *net; /* dot notation of the network address */
    char *mask;/* dot notation of the network mask    */
    int ret;   /* return code */
    int i = 0;
    char errbuf[PCAP_ERRBUF_SIZE];
    bpf_u_int32 netp; /* ip          */
    bpf_u_int32 maskp;/* subnet mask */
    struct in_addr addr;

    /* ask pcap to find a valid device for use to sniff on */
    dev = pcap_lookupdev(errbuf);

    /* error checking */
    if (dev == NULL) {
        printf("pcap_lookupdev: %s\n", errbuf);
        exit(1);
    }

    /* print out device name */
    printf("DEV: %s\n", dev);

    struct pcap_if *interface;
    struct pcap_if *temp = interface;

    int rev = pcap_findalldevs(&interface, errbuf);
    if (rev == -1) {
        printf("pcap_findalldevs: %s\n", errbuf);
        exit(1);
    }
    printf("i = %d\n", i);
    for (i = 0, temp = interface; temp; temp = temp->next) {
        int j = 0;
        struct pcap_addr *local_addr = NULL;
        printf("\n%2d  name: %20s\tflag: %2d\tdescription: %s", i++, temp->name, temp->flags, temp->description);
        for (j = 0, local_addr = temp->addresses; local_addr; local_addr = local_addr->next) {
            printf("\n\t\t\t\t\t\t\t\t\t\t\t\taddr: %16s\tnetmask: %16s\tdstaddr: %16s\tbroadaddr: %16s",
                   sockaddr_string(local_addr->addr),
                   sockaddr_string(local_addr->netmask),
                   sockaddr_string(local_addr->dstaddr),
                   sockaddr_string(local_addr->broadaddr));
        }

    }

    pcap_freealldevs(interface);

    /* ask pcap for the network address and mask of the device */
    ret = pcap_lookupnet(dev, &netp, &maskp, errbuf);

    if (ret == -1) {
        printf("%s\n", errbuf);
        exit(1);
    }

    /* get the network address in a human readable form */
    addr.s_addr = netp;
    net = inet_ntoa(addr);

    if (net == NULL)/* thanks Scott :-P */
    {
        perror("inet_ntoa");
        exit(1);
    }

    printf("\nNET: %s\n", net);

    /* do the same as above for the device's mask */
    addr.s_addr = maskp;
    mask = inet_ntoa(addr);

    if (mask == NULL) {
        perror("inet_ntoa");
        exit(1);
    }

    printf("\nMASK: %s\n", mask);

    return 0;
}