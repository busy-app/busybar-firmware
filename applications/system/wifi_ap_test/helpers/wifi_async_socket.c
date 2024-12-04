#include "wifi_async_socket.h"

// #include <sl_wifi.h>
// #include <sl_si91x_driver.h>
// #include <sl_wifi_callback_framework.h>

#include <sl_net.h>
//#include <socket.h>
#include <sl_si91x_socket.h>
#include <sl_si91x_socket_constants.h>
#include <sl_si91x_socket_utility.h>
//#include <sl_si91x_socket_support.h>
#include "errno.h"

#define TAG "WifiApTestAppScan"
#define SL_HIGH_PERFORMANCE_SOCKET BIT(7)
#define LISTENING_PORT 5005
#define BACK_LOG       1
// #define WIFI_SCAN_TIMEOUT 10000
// #define MAX_SCANNED_AP    20

// typedef struct {
//     sl_wifi_extended_scan_result_parameters_t extended_scan_result;
//     sl_wifi_extended_scan_result_t extended_scan_result_info[MAX_SCANNED_AP];
//     sl_status_t callback_status;
//     uint16_t scan_count;
//     FuriSemaphore* scan_complete;
// } WifiApTestAppScan;
#define TOTAL_SOCKETS                   1  //@ Total number of sockets. TCP TX + TCP RX + UDP TX + UDP RX
#define TOTAL_TCP_SOCKETS               1  //@ Total TCP sockets. TCP TX + TCP RX
#define TOTAL_UDP_SOCKETS               0  //@ Total UDP sockets. UDP TX + UDP RX
#define TCP_TX_ONLY_SOCKETS             0  //@ Total TCP TX only sockets. TCP TX
#define TCP_RX_ONLY_SOCKETS             1  //@ Total TCP RX only sockets. TCP RX
#define UDP_TX_ONLY_SOCKETS             0  //@ Total UDP TX only sockets. UDP TX
#define UDP_RX_ONLY_SOCKETS             0  //@ Total UDP RX only sockets. UDP RX
#define TCP_RX_HIGH_PERFORMANCE_SOCKETS 1  //@ Total TCP RX High Performance sockets
#define TCP_RX_WINDOW_SIZE_CAP          44 //@ TCP RX Window size
#define TCP_RX_WINDOW_DIV_FACTOR        44 //@ TCP RX Window division factor

static sl_si91x_socket_config_t socket_config = {
  TOTAL_SOCKETS,                   // Total sockets
  TOTAL_TCP_SOCKETS,               // Total TCP sockets
  TOTAL_UDP_SOCKETS,               // Total UDP sockets
  TCP_TX_ONLY_SOCKETS,             // TCP TX only sockets
  TCP_RX_ONLY_SOCKETS,             // TCP RX only sockets
  UDP_TX_ONLY_SOCKETS,             // UDP TX only sockets
  UDP_RX_ONLY_SOCKETS,             // UDP RX only sockets
  TCP_RX_HIGH_PERFORMANCE_SOCKETS, // TCP RX high performance sockets
  TCP_RX_WINDOW_SIZE_CAP,          // TCP RX window size
  TCP_RX_WINDOW_DIV_FACTOR         // TCP RX window division factor
};

void data_callback(uint32_t sock_no,
                   uint8_t *buffer,
                   uint32_t length,
                   const sl_si91x_socket_metadata_t *firmware_socket_response)
{
  UNUSED_PARAMETER(sock_no);
  UNUSED_PARAMETER(length);
  UNUSED_PARAMETER(buffer);
  UNUSED_PARAMETER(firmware_socket_response);

printf ("data_callback\r\n");
for (uint32_t i = 0; i < length; i++) {
  printf ("%c", buffer[i]);
}
printf ("\r\n");
//   if (first_data_frame) {
//     start = osKernelGetTickCount();
//     printf("\r\nClient Socket ID : %ld\r\n", sock_no);
//     switch (THROUGHPUT_TYPE) {
//       case UDP_RX:
//         printf("\r\nUDP_RX Throughput test start\r\n");
//         break;
//       case TCP_RX:
//         printf("\r\nTCP_RX Throughput test start\r\n");
//         break;
//     }
//     first_data_frame = 0;
//   }

//   bytes_read += length;
//   now = osKernelGetTickCount();
//   if ((bytes_read > BYTES_TO_RECEIVE) || ((now - start) > TEST_TIMEOUT)) {
//     has_data_received = 1;
//   }
}

void wifi_async_socket_server_init(void) {

   int server_socket                 = -1;
   int client_socket                 = -1;
   int socket_return_value           = 0;
   //struct sockaddr_in server_address = { 0 };
   //socklen_t socket_length           = sizeof(struct sockaddr_in);
   uint8_t high_performance_socket   = SL_HIGH_PERFORMANCE_SOCKET;

  sl_status_t status = sl_si91x_config_socket(socket_config);
  if (status != SL_STATUS_OK) {
    printf("Socket config failed: %ld\r\n", status);
  }
  printf("\r\nSocket config Done\r\n");

  // Create Server socket
  server_socket = sl_si91x_socket_async(AF_INET, SOCK_STREAM, IPPROTO_TCP, &data_callback);
  if (server_socket < 0) {
    printf("\r\nSocket creation failed with BSD error: %d\r\n", errno);
    return;
  }
  printf("\r\nServer Socket ID : %d\r\n", server_socket);

  //Set socket
  socket_return_value = sl_si91x_setsockopt_async(server_socket,
                                                  SOL_SOCKET,
                                                  SL_SI91X_SO_HIGH_PERFORMANCE_SOCKET,
                                                  &high_performance_socket,
                                                  sizeof(high_performance_socket));
  if (socket_return_value < 0) {
    printf("\r\nSet Socket option failed with BSD error: %d\r\n", errno);
    close(client_socket);
    return;
  }
//   server_address.sin_family = AF_INET;
//   server_address.sin_port   = LISTENING_PORT;

//   // Bind socket
//   socket_return_value = sl_si91x_bind(server_socket, (struct sockaddr *)&server_address, socket_length);
//   if (socket_return_value < 0) {
//     printf("\r\nSocket bind failed with BSD error: %d\r\n", errno);
//     close(server_socket);
//     return;
//   }

//   // Listen socket
//   socket_return_value = sl_si91x_listen(server_socket, BACK_LOG);
//   if (socket_return_value < 0) {
//     printf("\r\nSocket listen failed with BSD error:%d\r\n", errno);
//     close(server_socket);
//     return;
//   }
//   printf("\r\nListening on Local Port : %d\r\n", LISTENING_PORT);

//   // Accept socket
//   client_socket = sl_si91x_accept(server_socket, NULL, 0);
//   if (client_socket < 0) {
//     printf("\r\nSocket accept failed with BSD error: %d\r\n", errno);
//     close(server_socket);
//     return;
//   }

// //   while (!has_data_received) {
// //     osThreadYield();
// //   }

//   //now = osKernelGetTickCount();

//   printf("\r\nTCP_RX Throughput test finished\r\n");
//   //printf("\r\nTotal bytes received : %ld\r\n", bytes_read);

//   // Close socket
//   close(server_socket);
//   close(client_socket);

//   //measure_and_print_throughput(bytes_read, (now - start));
}