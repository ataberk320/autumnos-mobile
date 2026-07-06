#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/if.h>
#include <linux/sockios.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <arpa/inet.h>

#define ADD_RTA(req, attr_type, data, len) \
    do { \
        struct rtattr *rta = (struct rtattr *)(((char *)req) + NLMSG_ALIGN((*req).n.nlmsg_len)); \
        rta->rta_type = attr_type; \
        rta->rta_len = RTA_LENGTH(len); \
        memcpy(RTA_DATA(rta), data, len); \
        (*req).n.nlmsg_len = NLMSG_ALIGN((*req).n.nlmsg_len) + RTA_ALIGN(rta->rta_len); \
    } while(0)


static int hal_ethinterfacectl(const char *ifname, unsigned int flag_mask, int set) {
	int sock = socket(AF_INET, SOCK_DGRAM, 0);
	struct ifreq ifr;
	
	if (sock < 0) return -1;

	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, ifname, IFNAMSIZ);

	if (ioctl(sock, SIOCGIFFLAGS, &ifr) < 0) {
		close(sock);
		return -1;
	}

	if (set) ifr.ifr_flags |= flag_mask;
	else     ifr.ifr_flags &= ~flag_mask;

	int ret = ioctl(sock, SIOCSIFFLAGS, &ifr);

	close(sock);
	return ret;
}


void hal_setgateway(int ifindex, const char *ip_addr, const char *gw) {
	int fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);

	struct {
		struct nlmsghdr n;
		struct ifaddrmsg i;
		char bud[256];
	} addr_req;

	memset(&addr_req, 0, sizeof(addr_req));
    	addr_req.n.nlmsg_len = NLMSG_LENGTH(sizeof(struct ifaddrmsg));
    	addr_req.n.nlmsg_flags = NLM_F_REQUEST | NLM_F_CREATE | NLM_F_EXCL;
    	addr_req.n.nlmsg_type = RTM_NEWADDR;
    	addr_req.i.ifa_family = AF_INET;
    	addr_req.i.ifa_prefixlen = 24;
    	addr_req.i.ifa_index = ifindex;

    	uint32_t ip;
    	inet_pton(AF_INET, ip_addr, &ip);
    	ADD_RTA(&addr_req, IFA_LOCAL, &ip, 4);
    	ADD_RTA(&addr_req, IFA_ADDRESS, &ip, 4);
    	send(fd, &addr_req, addr_req.n.nlmsg_len, 0);

    	struct {
        	struct nlmsghdr n;
        	struct rtmsg r;
        	char buf[256];
    	} route_req;

    	memset(&route_req, 0, sizeof(route_req));
    	route_req.n.nlmsg_len = NLMSG_LENGTH(sizeof(struct rtmsg));
    	route_req.n.nlmsg_flags = NLM_F_REQUEST | NLM_F_CREATE;
    	route_req.n.nlmsg_type = RTM_NEWROUTE;
    	route_req.r.rtm_family = AF_INET;
    	route_req.r.rtm_table = RT_TABLE_MAIN;
    	route_req.r.rtm_scope = RT_SCOPE_UNIVERSE;
    	route_req.r.rtm_type = RTN_UNICAST;
    	route_req.r.rtm_dst_len = 0;

    	uint32_t gateway;
    	inet_pton(AF_INET, gw, &gateway);
    	ADD_RTA(&route_req, RTA_GATEWAY, &gateway, 4);
    	ADD_RTA(&route_req, RTA_OIF, &ifindex, 4);
    	send(fd, &route_req, route_req.n.nlmsg_len, 0);

    	close(fd);
}
