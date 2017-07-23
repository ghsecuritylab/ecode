#include "udp_server.h"
#include "ecode.h"

#include "lwip/udp.h"
#include "lwip/stats.h"
#include "lwip/pbuf.h"
#include "lwip/ip_addr.h"

#define UDP_PORT	1025

static struct udp_pcb *udppcb;
//struct ip_addr ipaddr;
static struct pbuf *p_buf;

static char udp_data[]="Hello wrold";

//UDP服务端
static void udp_receive(void *arg, struct udp_pcb *upcb, struct pbuf *p, struct ip_addr * addr, e_uint16_t port){
	struct ip_addr rmtaddr = *addr;

	if(p!=NULL){
		udp_sendto(upcb, p, &rmtaddr, port);
		pbuf_free(p);
	}
}

void udp_server_init(void){
	struct udp_pcb *upcb;
	upcb = udp_new();
	udp_bind(upcb, IP_ADDR_ANY, UDP_PORT);
	udp_recv(upcb, udp_receive, NULL);
}