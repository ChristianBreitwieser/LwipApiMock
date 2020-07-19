
#ifndef __LWIPOPTIONS_H__
#define __LWIPOPTIONS_H__

#define TCPIP_THREAD_STACKSIZE 512
#define SYS_LIGHTWEIGHT_PROT    1
#define NO_SYS                  0
#define MEM_ALIGNMENT           4
#define MEM_SIZE                (4*1024)
#define MEMP_NUM_PBUF           4
#define MEMP_NUM_UDP_PCB        5
#define MEMP_NUM_TCP_PCB        5
#define MEMP_NUM_TCP_PCB_LISTEN 5
#define MEMP_NUM_TCP_SEG        8
#define MEMP_NUM_SYS_TIMEOUT    10
#define PBUF_POOL_SIZE          4
#define PBUF_POOL_BUFSIZE       1520
#define LWIP_TCP                1
#define TCP_TTL                 255
#define TCP_QUEUE_OOSEQ         1
#define TCP_MSS                 (1500 - 40)
#define TCP_SND_BUF             (2*TCP_MSS)
#define TCP_SND_QUEUELEN        4*( TCP_SND_BUF)/TCP_MSS
#define TCP_WND                 (12*TCP_MSS)
#define LWIP_ICMP                       1
#define LWIP_UDP                1
#define UDP_TTL                 255
#define LWIP_STATS 0
#define LWIP_PROVIDE_ERRNO 0
#define LWIP_DEBUG 1
#define NETIF_DEBUG                     LWIP_DBG_ON
#define PBUF_DEBUG                      LWIP_DBG_OFF
#define SOCKETS_DEBUG                   LWIP_DBG_ON | LWIP_DBG_TRACE | LWIP_DBG_STATE 
#define ICMP_DEBUG                      LWIP_DBG_ON | LWIP_DBG_TRACE 
#define MEM_DEBUG                       LWIP_DBG_ON
#define MEMP_DEBUG                      LWIP_DBG_ON
#define IP_DEBUG                        LWIP_DBG_ON
#define TCPIP_DEBUG                     LWIP_DBG_ON
#define UDP_DEBUG                       LWIP_DBG_ON | LWIP_DBG_TRACE   
#define TCP_QLEN_DEBUG                  LWIP_DBG_ON
#define TCP_RST_DEBUG                   LWIP_DBG_ON
#define MEMP_DEBUG                      LWIP_DBG_ON
#define SYS_DEBUG                       LWIP_DBG_ON
#define TCP_DEBUG                       LWIP_DBG_ON | LWIP_DBG_TRACE 
#define TCP_INPUT_DEBUG                 LWIP_DBG_ON
#define TCP_FR_DEBUG                    LWIP_DBG_ON
#define TCP_RTO_DEBUG                   LWIP_DBG_ON
#define TCP_CWND_DEBUG                  LWIP_DBG_ON
#define TCP_WND_DEBUG                   LWIP_DBG_ON
#define TCP_OUTPUT_DEBUG                LWIP_DBG_ON
#define RTSP_DEBUG                      LWIP_DBG_ON
#define LWIP_DBG_TYPES_ON (LWIP_DBG_TRACE | LWIP_DBG_STATE | LWIP_DBG_FRESH)
#define CHECKSUM_GEN_IP                 1
#define CHECKSUM_GEN_UDP                1
#define CHECKSUM_GEN_TCP                1
#define CHECKSUM_CHECK_IP               1
#define CHECKSUM_CHECK_UDP              1
#define CHECKSUM_CHECK_TCP              1
#define LWIP_NETIF_API                   1
#define NUM_NETCONN                      128
#define LWIP_NETCONN                    0
#define LWIP_SOCKET                     0

#endif 

