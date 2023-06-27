#ifndef	_DEFINES_CARROBOT_H
#define	_DEFINES_CARROBOT_H

#define UDP 0
#define PARTIAL_TCP 1
#define TCP 2

//wifi parameters note
#define SSID_W "EnryPc"
#define PASSWD_W "4%a3O975"
#define ACCSESS_POINT_IP "192.168.137.1"
#define PORT_AP 80

// Tempi invio approssimativi (media su 100 invii)
// UDP 			12.0 ms | 12 ms
// PARTIAL TCP	23.8 ms | 110 ms
// TCP			44.0 ms | 200 ms

#define PROTOCOL PARTIAL_TCP
//#define TIMING

#if PROTOCOL == UDP
#define Wifi_Transmit(index, length, data) Wifi_TcpIp_SendData_Udp(index, length, data)
#elif PROTOCOL == PARTIAL_TCP
#define Wifi_Transmit(index, length, data) Wifi_TcpIp_SendData_PartialTcp(index, length, data)
#else
#define Wifi_Transmit(index, length, data) Wifi_TcpIp_SendData_Tcp(index, length, data)
#endif

#endif
