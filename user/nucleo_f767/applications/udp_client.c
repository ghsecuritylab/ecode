#include "udp_client.h"
#include "ecode.h"

#define UDP_PORT	1025

static char udp_data[]="Hello wrold";
static struct pbuf * p_buf;
static ip_addr_t ipaddr;
static struct udp_pcb *udppcb;

static void udp_receive(void *arg, struct udp_pcb *upcb, struct pbuf *p,const ip_addr_t * addr, e_uint16_t port){
	ip_addr_t rmtaddr = *addr;

	if(p!=NULL){
		udp_sendto(upcb, p, &rmtaddr, port);
		pbuf_free(p);
	}
}

//UDP 客户端
void udp_client_init(void){
	p_buf = pbuf_alloc(PBUF_RAW, sizeof(udp_data), PBUF_REF);
	p_buf->payload = (void *)udp_data;

	IP4_ADDR(&ipaddr, 192, 168, 1, 100);
	udppcb = udp_new();

	udp_bind(udppcb, IP_ADDR_ANY, 1023);
	udp_recv(udppcb, udp_receive, NULL);
	udp_connect(udppcb, &ipaddr, UDP_PORT);
	
}

void udp_client(void){
	udp_send(udppcb, p_buf);
}