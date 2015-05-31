#ifndef PROXY_H
#define PROXY_H

/* dynamic config */
#define PROXYCONF "Proxy.conf"
#ifdef _MSC_VER
/* service name */
#define SVCNAME "ProxyService"
#endif

/* comment line below to get rid of IPv4 */
#define DONOTDISABLEV4

/*
 * configuration parameters
 */
#define NUMPARAMS 3
/* server by default */
#define CLIENT_L 0
/* default addr */
#define ADDRANY_L "::"
/* default port */
#define PORT_L 11235
/* Layer >= Craft >= App */
#ifdef MAX_BUFF
#undef MAX_BUFF
#endif
#define MAX_BUFF 131072
#define SHID "_sid_\n"
#define CONFIG "_config_:"
#define CL 9
#define SOCKERROR -1
#define FILECONN "activeconn"

/*
 * threads timing
 */
#define TF 5
/* async recv/send timing */
#define AF 10
/* heartbeat timing */
#define HB 30
/* microseconds */
#define U 500000
/* platform dependent factor */
#define F 1000

/*
 * tun interface
 */
#define TUN "tun0"
/* IPv6 tun address for server */
#define TSADDR6 "2001:3::1:77/64"
#ifdef DONOTDISABLEV4
/* IPv4 tun address for server */
#define TSADDR4 "192.168.7.77/24"
#endif
/* IPv6 tun address for client */
#define TCADDR6 "2001:3::3:73/64"
#ifdef DONOTDISABLEV4
/* IPv4 tun address for client */
#define TCADDR4 "192.168.7.73/24"
#endif

/* key */
#define NESTKEY "certs/nest.politeping.com-key.pem"
/* certificate */
#define NESTX  "certs/nest.politeping.com-cert.pem"
/* cacertificate */
#define NESTCAX  "/certs/cacert.pem"

#undef SetPort
#endif
