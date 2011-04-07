#include "packet.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

Packet::Packet()
{
    this->Size = 0;
    this->buffer = NULL;
}

Packet::~Packet()
{
    if (this->buffer)
        free(this->buffer);
}

void * Packet::getBuffer()
{
    return this->buffer;
}

uint16_t Packet::checksum(uint16_t *buf, int nwords)
{
    uint32_t sum;

    for (sum = 0; nwords > 0; nwords--)
        sum += *buf++;
    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);
    return ~sum;
}

Packet * Packet::getPseudoIPTCPDATA()
{
    tcp* pTCP = static_cast<tcp*>(this->buffer);
    Packet* p = new Packet();

    std::cout << "[ip.len]=" << htons(pTCP->ip_len) << " [ip+data]=" << this->Size - sizeof(eth) << "- ip->hl" << pTCP->ip_hl
            << " tcp.data_offset=" << pTCP->doff << std::endl;
    // pseudo header ip
    p->append(&pTCP->ip_src, 4);
    p->append(&pTCP->ip_dst, 4);
    uint8_t a = 0;
    p->append(&a, 1);
    p->append(&pTCP->ip_p, 1);
//    uint16_t b = htons(sizeof(struct tcphdr));
    uint16_t b = htons(pTCP->ip_len) - (sizeof(ip) - sizeof(eth));//this->Size - sizeof(ip); // must be wrong with padding and maybe options
    std::cout << "HEADER TCP + DATA SIZE = " << b << std::endl;
    b = htons(b);
    p->append(&b, 2);

    b = htons(b);
    // tcp header and data...
    p->append((uint8_t*)(this->buffer) + sizeof(ip), b);
    uint8_t odd = 0;
    if (p->Size % 2 != 0)
        p->append(&odd, 1);

    return p;
}

void Packet::computeChecksum()
{
    tcp* pTCP = static_cast<tcp*>(this->buffer);
    unsigned char * packet = ((unsigned char *)this->buffer) + sizeof(eth);

    pTCP->ip_sum = 0;
    pTCP->ip_sum = this->checksum((uint16_t *)(packet), 20 >> 1);


    Packet * p = this->getPseudoIPTCPDATA();
    checktcp * test = (checktcp*)(p->getBuffer());

    std::cout << "OLD (REAL) -" << pTCP->check << "-  (COPIED) " << test->tcp.check << std::endl;
    test->tcp.check = 0;
    pTCP->check = checksum((uint16_t*)p->getBuffer(), p->Size >> 1);
    std::cout << "NEW (REAL) -" << pTCP->check << "-" << std::endl;
    delete p;
}

void eth::craftETH(uint16_t type, uint8_t *srcmac, uint8_t *dstmac)
{
    this->type = htons(type);
    memcpy(this->ar_tha, dstmac, 6);
    memcpy(this->ar_sha, srcmac, 6);
}

void arp::craftARP(uint8_t *srcmac, uint8_t *srcip,
                   uint8_t *dstmac, uint8_t *dstip)
{
    this->ar_hrd = htons(1);
    this->ar_pro = htons(0x0800);
    this->ar_hln = 6;
    this->ar_pln = 4;
    this->ar_op = htons(ARP_REQUEST);
    memcpy(this->ar_sha, srcmac, 6);
    memcpy(this->ar_sip, srcip, 4);
    memcpy(this->ar_tha, dstmac, 6);
    memcpy(this->ar_tip, dstip, 4);
}

void ip::craftIP(uint8_t *srcmac, uint32_t srcip,
                 uint8_t *dstmac, uint32_t dstip)
{
    this->craftETH(0x0800, srcmac, dstmac);
    this->ip_hl = 5;
    this->ip_v = 4;
    this->ip_tos = 0;
    this->ip_len = htons(40 + 20); // 16 without options ??
    this->ip_id = htons(1337);
    this->ip_off = 0x40;
    this->ip_ttl = 255;
    this->ip_p = 0; // to set later before compute the checksum
    this->ip_sum = 0;
    this->ip_src = srcip;
    this->ip_dst = dstip;
}

void tcp::craftTCP(uint8_t *srcmac, uint32_t srcip,
                   uint8_t *dstmac, uint32_t dstip)
{

    this->craftIP(srcmac, srcip, dstmac, dstip);
    this->ip_p = IPPROTO_TCP;
    this->ip_sum = this->checksumIP((uint16_t *)( ((unsigned char *)this) + sizeof(eth)), 20 >> 1);

    this->source = htons(1337);
    this->dest = htons(1337);
    this->seq = htonl(40);
    this->ack_seq = htonl(5);
    this->res1 = 5;
    this->doff = 0x6;

    this->fin = 0;
    this->syn = 0;
    this->rst = 0;
    this->psh = 0;
    this->ack = 1;
    this->urg = 0;
    //this->res2 = 0;
    this->window = htons(20);
    this->check = 0;
    this->urg_ptr = 0;
    //this->options = 0;
}
