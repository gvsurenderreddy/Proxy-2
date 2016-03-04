#ifndef PROXY_H
#define PROXY_H

/* dynamic config */
#define PROXYCONF "Proxy.conf"
#ifdef _WIN32
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
#define PORT_C 11234
#define PORT_L 11235
/* Layer >= Craft >= App */
#ifdef MAX_BUFF
#undef MAX_BUFF
#endif
#define MAX_BUFF 131072
#ifdef MIN_BUFF
#undef MIN_BUFF
#endif
#define MIN_BUFF 2048
#define SHID "_sid_\n"
#define CONFIG "_config_:"
#define COLLECT "_collect_"
#define DISMISS "_dismiss_"
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
/* Total number of IP addresses*/
#define TCADDRTOTAL 128
/* IPv6 tun address for server */
#define TSADDR6 "2001:3::1:201/64"
#ifdef DONOTDISABLEV4
/* IPv4 tun address for server */
#define TSADDR4 "192.168.7.201/24"
#endif
/* IPv6 tun address for client */
#define TCADDR6 "2001:3::3:203/64"
/* IPv6 pool */
#define TCADDR6P "2001:3::3:1/64"

#ifdef DONOTDISABLEV4
/* IPv4 tun address for client */
#define TCADDR4 "192.168.7.203/24"
/* IPv4 pool */
#define TCADDR4P "192.168.7.1/24"
#endif

/* key */
#define NESTKEY "certs/nest.politeping.com-key.pem"
/* certificate */
#define NESTX  "certs/nest.politeping.com-cert.pem"
/* cacertificate */
#define NESTCAX  "certs/cacert.pem"

#undef SetPort
#endif
