#include "console.h"
#include "console_argument_types.h"

extern sl_status_t console_variable_get(console_args_t *arguments);
static const char *_get_arg_help[] = {
  0,
};

static const console_descriptive_command_t _get_command = { .description   = "Get",
                                                            .argument_help = _get_arg_help,
                                                            .handler       = console_variable_get,
                                                            .argument_list = { CONSOLE_ARG_STRING, CONSOLE_ARG_END } };

extern sl_status_t help_command_handler(console_args_t *arguments);
static const char *_help_arg_help[] = {};

static const console_descriptive_command_t _help_command = { .description   = "Print help",
                                                             .argument_help = _help_arg_help,
                                                             .handler       = help_command_handler,
                                                             .argument_list = { CONSOLE_ARG_END } };

extern sl_status_t console_variable_list(console_args_t *arguments);
static const char *_list_arg_help[] = {};

static const console_descriptive_command_t _list_command = { .description   = "List variables",
                                                             .argument_help = _list_arg_help,
                                                             .handler       = console_variable_list,
                                                             .argument_list = { CONSOLE_ARG_END } };

extern sl_status_t net_deinit_command_handler(console_args_t *arguments);
static const char *_net_deinit_arg_help[] = {
  0,
};

static const console_descriptive_command_t _net_deinit_command = { .description   = "Deinit the network interface",
                                                                   .argument_help = _net_deinit_arg_help,
                                                                   .handler       = net_deinit_command_handler,
                                                                   .argument_list = { CONSOLE_ENUM_ARG(net_interface),
                                                                                      CONSOLE_ARG_END } };

extern sl_status_t net_down_command_handler(console_args_t *arguments);
static const char *_net_down_arg_help[] = {
  0,
};

static const console_descriptive_command_t _net_down_command = { .description   = "Down the network interface",
                                                                 .argument_help = _net_down_arg_help,
                                                                 .handler       = net_down_command_handler,
                                                                 .argument_list = { CONSOLE_ENUM_ARG(net_interface),
                                                                                    CONSOLE_ARG_END } };

extern sl_status_t net_init_command_handler(console_args_t *arguments);
static const char *_net_init_arg_help[] = {
  0,
};

static const console_descriptive_command_t _net_init_command = { .description   = "Init the network interface",
                                                                 .argument_help = _net_init_arg_help,
                                                                 .handler       = net_init_command_handler,
                                                                 .argument_list = { CONSOLE_ENUM_ARG(net_interface),
                                                                                    CONSOLE_ARG_END } };

extern sl_status_t net_join_multicast_address_command_handler(console_args_t *arguments);
static const char *_net_join_multicast_address_arg_help[] = {
  0,
  0,
};

static const console_descriptive_command_t _net_join_multicast_address_command = {
  .description   = "Join multicast address",
  .argument_help = _net_join_multicast_address_arg_help,
  .handler       = net_join_multicast_address_command_handler,
  .argument_list = { CONSOLE_OPTIONAL_ARG('i', CONSOLE_ENUM_ARG(net_interface)),
                     CONSOLE_OPTIONAL_ARG('p', CONSOLE_ARG_STRING),
                     CONSOLE_ARG_END }
};

extern sl_status_t net_leave_multicast_address_command_handler(console_args_t *arguments);
static const char *_net_leave_multicast_address_arg_help[] = {
  0,
  0,
};

static const console_descriptive_command_t _net_leave_multicast_address_command = {
  .description   = "Leave multicast address",
  .argument_help = _net_leave_multicast_address_arg_help,
  .handler       = net_leave_multicast_address_command_handler,
  .argument_list = { CONSOLE_OPTIONAL_ARG('i', CONSOLE_ENUM_ARG(net_interface)),
                     CONSOLE_OPTIONAL_ARG('p', CONSOLE_ARG_STRING),
                     CONSOLE_ARG_END }
};

extern sl_status_t net_up_command_handler(console_args_t *arguments);
static const char *_net_up_arg_help[] = {
  0,
  0,
};

static const console_descriptive_command_t _net_up_command = {
  .description   = "Up the network interface",
  .argument_help = _net_up_arg_help,
  .handler       = net_up_command_handler,
  .argument_list = { CONSOLE_ENUM_ARG(net_interface), CONSOLE_OPTIONAL_ARG('i', CONSOLE_ARG_UINT), CONSOLE_ARG_END }
};

extern sl_status_t rtt_command_handler(console_args_t *arguments);
static const char *_rtt_arg_help[] = {
  0,
};

static const console_descriptive_command_t _rtt_command = { .description   = "Print to RTT",
                                                            .argument_help = _rtt_arg_help,
                                                            .handler       = rtt_command_handler,
                                                            .argument_list = { CONSOLE_ARG_STRING, CONSOLE_ARG_END } };

extern sl_status_t console_variable_set(console_args_t *arguments);
static const char *_set_arg_help[] = {
  0,
  0,
};

static const console_descriptive_command_t _set_command = {
  .description   = "Set",
  .argument_help = _set_arg_help,
  .handler       = console_variable_set,
  .argument_list = { CONSOLE_ARG_STRING, CONSOLE_ARG_STRING, CONSOLE_ARG_END }
};

extern sl_status_t set_nvm_profile_command_handler(console_args_t *arguments);
static const char *_set_nvm_profile_arg_help[] = {
  0,
  0,
};

static const console_descriptive_command_t _set_nvm_profile_command = {
  .description   = "set profile and credential in nvm",
  .argument_help = _set_nvm_profile_arg_help,
  .handler       = set_nvm_profile_command_handler,
  .argument_list = { CONSOLE_ENUM_ARG(net_interface), CONSOLE_OPTIONAL_ARG('i', CONSOLE_ARG_UINT), CONSOLE_ARG_END }
};

extern sl_status_t sl_dns_hostgetbyname_command_handler(console_args_t *arguments);
static const char *_sl_net_dns_resolve_hostname_arg_help[] = {
  0,
  0,
  0,
};

static const console_descriptive_command_t _sl_net_dns_resolve_hostname_command = {
  .description   = "DNS resolution",
  .argument_help = _sl_net_dns_resolve_hostname_arg_help,
  .handler       = sl_dns_hostgetbyname_command_handler,
  .argument_list = { CONSOLE_ARG_STRING,
                     CONSOLE_OPTIONAL_ARG('t', CONSOLE_ARG_UINT),
                     CONSOLE_OPTIONAL_ARG('i', CONSOLE_ENUM_ARG(sl_net_dns_resolution_ip_type_t)),
                     CONSOLE_ARG_END }
};

extern sl_status_t sl_net_ping_command_handler(console_args_t *arguments);
static const char *_sl_net_ping_arg_help[] = {
  0,
  0,
  0,
};

static const console_descriptive_command_t _sl_net_ping_command = {
  .description   = "Send ping request",
  .argument_help = _sl_net_ping_arg_help,
  .handler       = sl_net_ping_command_handler,
  .argument_list = { CONSOLE_ARG_STRING, CONSOLE_ENUM_ARG(sl_ip_address_type_t), CONSOLE_ARG_UINT, CONSOLE_ARG_END }
};

extern sl_status_t sl_si91x_get_ram_log_command_handler(console_args_t *arguments);
static const char *_sl_si91x_get_ram_log_arg_help[] = {
  0,
  0,
};

static const console_descriptive_command_t _sl_si91x_get_ram_log_command = {
  .description   = "read the mentioned length of data from firmware RAM",
  .argument_help = _sl_si91x_get_ram_log_arg_help,
  .handler       = sl_si91x_get_ram_log_command_handler,
  .argument_list = { CONSOLE_OPTIONAL_ARG('a', CONSOLE_ARG_UINT),
                     CONSOLE_OPTIONAL_ARG('l', CONSOLE_ARG_UINT),
                     CONSOLE_ARG_END }
};

extern sl_status_t start_dhcp_command_handler(console_args_t *arguments);
static const char *_start_dhcp_arg_help[] = {
  0,
  0,
};

static const console_descriptive_command_t _start_dhcp_command = {
  .description   = "Start dhcp and get ip address",
  .argument_help = _start_dhcp_arg_help,
  .handler       = start_dhcp_command_handler,
  .argument_list = { CONSOLE_OPTIONAL_ARG('i', CONSOLE_ENUM_ARG(sl_ip_address_type_t)),
                     CONSOLE_OPTIONAL_ARG('m', CONSOLE_ENUM_ARG(sl_ip_management_t)),
                     CONSOLE_OPTIONAL_ARG('a', CONSOLE_ARG_IP_ADDRESS),
                     CONSOLE_OPTIONAL_ARG('g', CONSOLE_ARG_IP_ADDRESS),
                     CONSOLE_OPTIONAL_ARG('n', CONSOLE_ARG_IP_ADDRESS),
                     CONSOLE_ARG_END }
};

extern sl_status_t thread_command_handler(console_args_t *arguments);
static const char *_thread_arg_help[] = {
  0,
  0,
};

static const console_descriptive_command_t _thread_command = {
  .description   = "Spawn thread",
  .argument_help = _thread_arg_help,
  .handler       = thread_command_handler,
  .argument_list = { CONSOLE_ARG_UINT, CONSOLE_ARG_REMAINING_COMMAND_LINE, CONSOLE_ARG_END }
};

extern sl_status_t sl_wifi_set_11ax_config_command_handler(console_args_t *arguments);
static const char *_wifi_11ax_config_arg_help[] = {
  0,
};

static const console_descriptive_command_t _wifi_11ax_config_command = {
  .description   = "Configure the 11ax for WiFi 6",
  .argument_help = _wifi_11ax_config_arg_help,
  .handler       = sl_wifi_set_11ax_config_command_handler,
  .argument_list = { CONSOLE_ARG_UINT, CONSOLE_ARG_END }
};

extern sl_status_t sl_wifi_assert_command_handler(console_args_t *arguments);
static const char *_wifi_assert_arg_help[] = {};

static const console_descriptive_command_t _wifi_assert_command = { .description   = "Trigger an assert",
                                                                    .argument_help = _wifi_assert_arg_help,
                                                                    .handler       = sl_wifi_assert_command_handler,
                                                                    .argument_list = { CONSOLE_ARG_END } };

extern sl_status_t wifi_config_pll_mode_handler(console_args_t *arguments);
static const char *_wifi_config_pll_mode_arg_help[] = {
  "PLL mode (pll_mode_20mhz or pll_mode_40mhz)",
};

static const console_descriptive_command_t _wifi_config_pll_mode_command = {
  .description   = "Configure WiFi PLL mode (pll_mode_20mhz: 20MHz, pll_mode_40mhz: 40MHz)",
  .argument_help = _wifi_config_pll_mode_arg_help,
  .handler       = wifi_config_pll_mode_handler,
  .argument_list = { CONSOLE_ENUM_ARG(wifi_pll_mode), CONSOLE_ARG_END }
};

extern sl_status_t wifi_config_power_chain_handler(console_args_t *arguments);
static const char *_wifi_config_power_chain_arg_help[] = {
  "Power chain (hp_chain or lp_chain)",
};

static const console_descriptive_command_t _wifi_config_power_chain_command = {
  .description   = "Configure WiFi power chain (hp_chain: High Power, lp_chain: Low Power)",
  .argument_help = _wifi_config_power_chain_arg_help,
  .handler       = wifi_config_power_chain_handler,
  .argument_list = { CONSOLE_ENUM_ARG(wifi_power_chain), CONSOLE_ARG_END }
};

extern sl_status_t wifi_bsd_get_host_by_name_handler(console_args_t *arguments);
static const char *_wifi_bsd_get_host_by_name_arg_help[] = {
  0,
};

static const console_descriptive_command_t _wifi_bsd_get_host_by_name_command = {
  .description   = "BSD socket listen, \n parameters is \n 1 - hostName \n",
  .argument_help = _wifi_bsd_get_host_by_name_arg_help,
  .handler       = wifi_bsd_get_host_by_name_handler,
  .argument_list = { CONSOLE_ARG_STRING, CONSOLE_ARG_END }
};

extern sl_status_t wifi_bsd_get_peer_name(console_args_t *arguments);
static const char *_wifi_bsd_get_peer_name_arg_help[] = {
  0,
};

static const console_descriptive_command_t _wifi_bsd_get_peer_name_command = {
  .description   = "BSD get remote node address \n parameter \n1 - socket_fd",
  .argument_help = _wifi_bsd_get_peer_name_arg_help,
  .handler       = wifi_bsd_get_peer_name,
  .argument_list = { CONSOLE_ARG_INT, CONSOLE_ARG_END }
};

extern sl_status_t wifi_bsd_get_sock_name(console_args_t *arguments);
static const char *_wifi_bsd_get_sock_name_arg_help[] = {
  0,
};

static const console_descriptive_command_t _wifi_bsd_get_sock_name_command = {
  .description   = "BSD get local address of socket \n parameter \n1 - socket_fd",
  .argument_help = _wifi_bsd_get_sock_name_arg_help,
  .handler       = wifi_bsd_get_sock_name,
  .argument_list = { CONSOLE_ARG_INT, CONSOLE_ARG_END }
};

extern sl_status_t wifi_bsd_socket_get_opt_handler(console_args_t *arguments);
static const char *_wifi_bsd_get_sock_opt_arg_help[] = {
  0,
  0,
};

static const console_descriptive_command_t _wifi_bsd_get_sock_opt_command = {
  .description   = "BSD get the socket options \n parameters\n 1- socket_fd 2- option level\n",
  .argument_help = _wifi_bsd_get_sock_opt_arg_help,
  .handler       = wifi_bsd_socket_get_opt_handler,
  .argument_list = { CONSOLE_ARG_INT, CONSOLE_ENUM_ARG(option_name), CONSOLE_ARG_END }
};

extern sl_status_t wifi_bsd_socket_set_opt_handler(console_args_t *arguments);
static const char *_wifi_bsd_set_sock_opt_arg_help[] = {
  0,
  0,
  0,
  0,
};

static const console_descriptive_command_t _wifi_bsd_set_sock_opt_command = {
  .description =
    "BSD get the socket options \n parameters\n 1- socket_fd 2- option name  3- option value  4- option length\n",
  .argument_help = _wifi_bsd_set_sock_opt_arg_help,
  .handler       = wifi_bsd_socket_set_opt_handler,
  .argument_list = { CONSOLE_ARG_INT, CONSOLE_ENUM_ARG(option_name), CONSOLE_ARG_INT, CONSOLE_ARG_INT, CONSOLE_ARG_END }
};

extern sl_status_t wifi_bsd_socket_accept_handler(console_args_t *arguments);
static const char *_wifi_bsd_socket_accept_arg_help[] = {
  0,
};

static const console_descriptive_command_t _wifi_bsd_socket_accept_command = {
  .description   = "BSD socket listen, \n parameters is \n 1 - sock_fd \n",
  .argument_help = _wifi_bsd_socket_accept_arg_help,
  .handler       = wifi_bsd_socket_accept_handler,
  .argument_list = { CONSOLE_ARG_INT, CONSOLE_ARG_END }
};

extern sl_status_t wifi_bsd_socket_bind_handler(console_args_t *arguments);
static const char *_wifi_bsd_socket_bind_arg_help[] = {
  0,
  0,
  0,
  0,
};

static const console_descriptive_command_t _wifi_bsd_socket_bind_command = {
  .description =
    "BSD socket bind, \n parameters is \n 1 - sock_fd, \n 2 - ip_address, \n 3 - ip_address_len, \n 4 - port \n",
  .argument_help = _wifi_bsd_socket_bind_arg_help,
  .handler       = wifi_bsd_socket_bind_handler,
  .argument_list = { CONSOLE_ARG_INT, CONSOLE_ARG_STRING, CONSOLE_ARG_UINT, CONSOLE_ARG_UINT, CONSOLE_ARG_END }
};

extern sl_status_t wifi_bsd_socket_close_handler(console_args_t *arguments);
static const char *_wifi_bsd_socket_close_arg_help[] = {
  0,
};

static const console_descriptive_command_t _wifi_bsd_socket_close_command = {
  .description   = "BSD socket close, \n parameters is \n 1 - sock_fd \n",
  .argument_help = _wifi_bsd_socket_close_arg_help,
  .handler       = wifi_bsd_socket_close_handler,
  .argument_list = { CONSOLE_ARG_INT, CONSOLE_ARG_END }
};

extern sl_status_t wifi_bsd_socket_connect_handler(console_args_t *arguments);
static const char *_wifi_bsd_socket_connect_arg_help[] = {
  0,
  0,
  0,
  0,
};

static const console_descriptive_command_t _wifi_bsd_socket_connect_command = {
  .description =
    "BSD socket connect, \n parameters are \n 1 - sock_fd, \n 2 - ip_address, \n 3 - ip_address_len, \n 4 - port \n",
  .argument_help = _wifi_bsd_socket_connect_arg_help,
  .handler       = wifi_bsd_socket_connect_handler,
  .argument_list = { CONSOLE_ARG_INT, CONSOLE_ARG_STRING, CONSOLE_ARG_UINT, CONSOLE_ARG_UINT, CONSOLE_ARG_END }
};

extern sl_status_t wifi_bsd_socket_create_handler(console_args_t *arguments);
static const char *_wifi_bsd_socket_create_arg_help[] = {
  0,
  0,
  0,
};

static const console_descriptive_command_t _wifi_bsd_socket_create_command = {
  .description = "BSD socket creation, \n parameters are \n 1 - Domain, possible value: 1(AF_INET), 2(AF_INET6) .., \n "
                 "2 - Type, possible value: 1(SOCK_STREAM), 2(SOCK_DGRAM),  \n3 - protocol, possible "
                 "value:1(IPPROTO_TCP), 2(IPPROTO_UDP).. \n",
  .argument_help = _wifi_bsd_socket_create_arg_help,
  .handler       = wifi_bsd_socket_create_handler,
  .argument_list = { CONSOLE_ENUM_ARG(bsd_socket_family),
                     CONSOLE_ENUM_ARG(bsd_socket_type),
                     CONSOLE_ENUM_ARG(bsd_socket_protocol),
                     CONSOLE_ARG_END }
};

extern sl_status_t wifi_bsd_socket_listen_handler(console_args_t *arguments);
static const char *_wifi_bsd_socket_listen_arg_help[] = {
  0,
  0,
};

static const console_descriptive_command_t _wifi_bsd_socket_listen_command = {
  .description   = "BSD socket listen, \n parameters is \n 1 - sock_fd, \n 2 - backlog \n",
  .argument_help = _wifi_bsd_socket_listen_arg_help,
  .handler       = wifi_bsd_socket_listen_handler,
  .argument_list = { CONSOLE_ARG_INT, CONSOLE_ARG_INT, CONSOLE_ARG_END }
};

extern sl_status_t wifi_bsd_socket_receive_handler(console_args_t *arguments);
static const char *_wifi_bsd_socket_receive_arg_help[] = {
  0,
};

static const console_descriptive_command_t _wifi_bsd_socket_receive_command = {
  .description   = "BSD socket receive from, \n parameters is \n 1 - sock_fd \n",
  .argument_help = _wifi_bsd_socket_receive_arg_help,
  .handler       = wifi_bsd_socket_receive_handler,
  .argument_list = { CONSOLE_ARG_INT, CONSOLE_ARG_END }
};

extern sl_status_t wifi_bsd_socket_receive_from_handler(console_args_t *arguments);
static const char *_wifi_bsd_socket_receive_from_arg_help[] = {
  0,
};

static const console_descriptive_command_t _wifi_bsd_socket_receive_from_command = {
  .description   = "BSD socket receive from, \n parameters is \n 1 - sock_fd\n",
  .argument_help = _wifi_bsd_socket_receive_from_arg_help,
  .handler       = wifi_bsd_socket_receive_from_handler,
  .argument_list = { CONSOLE_ARG_INT, CONSOLE_ARG_END }
};

extern sl_status_t wifi_bsd_select_handler(console_args_t *arguments);
static const char *_wifi_bsd_socket_select_arg_help[] = {
  0,
  0,
};

static const console_descriptive_command_t _wifi_bsd_socket_select_command = {
  .description   = " nfd - number of file descriptors\n time - time in seconds",
  .argument_help = _wifi_bsd_socket_select_arg_help,
  .handler       = wifi_bsd_select_handler,
  .argument_list = { CONSOLE_ARG_INT, CONSOLE_ARG_INT, CONSOLE_ARG_END }
};

extern sl_status_t wifi_bsd_socket_send_handler(console_args_t *arguments);
static const char *_wifi_bsd_socket_send_arg_help[] = {
  0,
  0,
};

static const console_descriptive_command_t _wifi_bsd_socket_send_command = {
  .description   = "BSD socket send, \n parameters is \n 1 - sock_fd, \n 2 - data to be sent \n",
  .argument_help = _wifi_bsd_socket_send_arg_help,
  .handler       = wifi_bsd_socket_send_handler,
  .argument_list = { CONSOLE_ARG_INT, CONSOLE_ARG_STRING, CONSOLE_ARG_END }
};

extern sl_status_t wifi_bsd_socket_send_to_handler(console_args_t *arguments);
static const char *_wifi_bsd_socket_send_to_arg_help[] = {
  0, 0, 0, 0, 0,
};

static const console_descriptive_command_t _wifi_bsd_socket_send_to_command = {
  .description = "BSD socket send, \n parameters is \n 1 - sock_fd, \n 2 - data to be sent, \n 3 - ip_address, \n 4 - "
                 "ip_address_len (4 or 16 bytes), \n 5 - port \n",
  .argument_help = _wifi_bsd_socket_send_to_arg_help,
  .handler       = wifi_bsd_socket_send_to_handler,
  .argument_list = { CONSOLE_ARG_INT,
                     CONSOLE_ARG_STRING,
                     CONSOLE_ARG_STRING,
                     CONSOLE_ARG_UINT,
                     CONSOLE_ARG_UINT,
                     CONSOLE_ARG_END }
};

extern sl_status_t wifi_connect_command_handler(console_args_t *arguments);
static const char *_wifi_connect_arg_help[] = {
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

static const console_descriptive_command_t _wifi_connect_command = {
  .description   = "Start connecting to specified AP",
  .argument_help = _wifi_connect_arg_help,
  .handler       = wifi_connect_command_handler,
  .argument_list = { CONSOLE_ARG_STRING,
                     CONSOLE_OPTIONAL_ARG('p', CONSOLE_ARG_STRING),
                     CONSOLE_OPTIONAL_ARG('s', CONSOLE_ENUM_ARG(wifi_security)),
                     CONSOLE_OPTIONAL_ARG('u', CONSOLE_ARG_STRING),
                     CONSOLE_OPTIONAL_ARG('e', CONSOLE_ENUM_ARG(wifi_encryption)),
                     CONSOLE_OPTIONAL_ARG('t', CONSOLE_ARG_UINT),
                     CONSOLE_OPTIONAL_ARG('l', CONSOLE_ARG_UINT),
                     CONSOLE_OPTIONAL_ARG('c', CONSOLE_ARG_UINT),
                     CONSOLE_OPTIONAL_ARG('b', CONSOLE_ENUM_ARG(wifi_band)),
                     CONSOLE_OPTIONAL_ARG('w', CONSOLE_ENUM_ARG(wifi_bandwidth)),
                     CONSOLE_OPTIONAL_ARG('i', CONSOLE_ENUM_ARG(wifi_interface)),
                     CONSOLE_ARG_END }
};

extern sl_status_t wifi_deinit_command_handler(console_args_t *arguments);
static const char *_wifi_deinit_arg_help[] = {};

static const console_descriptive_command_t _wifi_deinit_command = { .description   = "Deinit Wi-Fi interface",
                                                                    .argument_help = _wifi_deinit_arg_help,
                                                                    .handler       = wifi_deinit_command_handler,
                                                                    .argument_list = { CONSOLE_ARG_END } };

extern sl_status_t sl_wifi_disable_twt(console_args_t *arguments);
static const char *_wifi_disable_target_wake_time_arg_help[] = {};

static const console_descriptive_command_t _wifi_disable_target_wake_time_command = {
  .description   = "Configures TWT parameters. Disables a TWT session. This is blocking API",
  .argument_help = _wifi_disable_target_wake_time_arg_help,
  .handler       = sl_wifi_disable_twt,
  .argument_list = { CONSOLE_ARG_END }
};

extern sl_status_t wifi_disconnect_command_handler(console_args_t *arguments);
static const char *_wifi_disconnect_arg_help[] = {
  0,
};

static const console_descriptive_command_t _wifi_disconnect_command = {
  .description   = "Disconnect",
  .argument_help = _wifi_disconnect_arg_help,
  .handler       = wifi_disconnect_command_handler,
  .argument_list = { CONSOLE_OPTIONAL_ARG('i', CONSOLE_ENUM_ARG(wifi_interface)), CONSOLE_ARG_END }
};

extern sl_status_t wifi_disconnect_ap_client_command_handler(console_args_t *arguments);
static const char *_wifi_disconnect_ap_client_arg_help[] = {
  0,
  0,
};

static const console_descriptive_command_t _wifi_disconnect_ap_client_command = {
  .description   = "Disconnect client connected to AP",
  .argument_help = _wifi_disconnect_ap_client_arg_help,
  .handler       = wifi_disconnect_ap_client_command_handler,
  .argument_list = { CONSOLE_ARG_MAC_ADDRESS,
                     CONSOLE_OPTIONAL_ARG('i', CONSOLE_ENUM_ARG(ap_client_deauth)),
                     CONSOLE_ARG_END }
};

extern sl_status_t sl_wifi_enable_twt(console_args_t *arguments);
static const char *_wifi_enable_target_wake_time_arg_help[] = {};

static const console_descriptive_command_t _wifi_enable_target_wake_time_command = {
  .description   = "Configures TWT parameters. Enables a TWT session. This is blocking API",
  .argument_help = _wifi_enable_target_wake_time_arg_help,
  .handler       = sl_wifi_enable_twt,
  .argument_list = { CONSOLE_ARG_END }
};

extern sl_status_t sl_wifi_filter_broadcast_command_handler(console_args_t *arguments);
static const char *_wifi_filter_broadcast_arg_help[] = {};

static const console_descriptive_command_t _wifi_filter_broadcast_command = {
  .description   = "control the amount of broadcast and multicast data that is transmitted",
  .argument_help = _wifi_filter_broadcast_arg_help,
  .handler       = sl_wifi_filter_broadcast_command_handler,
  .argument_list = { CONSOLE_ARG_END }
};

extern sl_status_t wifi_generate_wps_pin_command_handler(console_args_t *arguments);
static const char *_wifi_generate_wps_pin_arg_help[] = {};

static const console_descriptive_command_t _wifi_generate_wps_pin_command = {
  .description   = "Generate 8 digit wps pin",
  .argument_help = _wifi_generate_wps_pin_arg_help,
  .handler       = wifi_generate_wps_pin_command_handler,
  .argument_list = { CONSOLE_ARG_END }
};

extern sl_status_t sl_wifi_get_advanced_scan_configuration_command_handler(console_args_t *arguments);
static const char *_wifi_get_advanced_scan_configuration_arg_help[] = {};

static const console_descriptive_command_t _wifi_get_advanced_scan_configuration_command = {
  .description   = "Get advanced scan configuration",
  .argument_help = _wifi_get_advanced_scan_configuration_arg_help,
  .handler       = sl_wifi_get_advanced_scan_configuration_command_handler,
  .argument_list = { CONSOLE_ARG_END }
};

extern sl_status_t wifi_get_ap_client_count_command_handler(console_args_t *arguments);
static const char *_wifi_get_ap_client_count_arg_help[] = {};

static const console_descriptive_command_t _wifi_get_ap_client_count_command = {
  .description   = "Client count for get ap",
  .argument_help = _wifi_get_ap_client_count_arg_help,
  .handler       = wifi_get_ap_client_count_command_handler,
  .argument_list = { CONSOLE_ARG_END }
};

extern sl_status_t wifi_get_ap_client_list_command_handler(console_args_t *arguments);
static const char *_wifi_get_ap_client_list_arg_help[] = {};

static const console_descriptive_command_t _wifi_get_ap_client_list_command = {
  .description   = "Client list for ap",
  .argument_help = _wifi_get_ap_client_list_arg_help,
  .handler       = wifi_get_ap_client_list_command_handler,
  .argument_list = { CONSOLE_ARG_END }
};

extern sl_status_t wifi_get_ap_configuration_command_handler(console_args_t *arguments);
static const char *_wifi_get_ap_configuration_arg_help[] = {
  0,
};

static const console_descriptive_command_t _wifi_get_ap_configuration_command = {
  .description   = "Get ap configuration",
  .argument_help = _wifi_get_ap_configuration_arg_help,
  .handler       = wifi_get_ap_configuration_command_handler,
  .argument_list = { CONSOLE_OPTIONAL_ARG('i', CONSOLE_ENUM_ARG(wifi_interface)), CONSOLE_ARG_END }
};

extern sl_status_t wifi_get_channel_command_handler(console_args_t *arguments);
static const char *_wifi_get_channel_arg_help[] = {
  0,
};

static const console_descriptive_command_t _wifi_get_channel_command = {
  .description   = "Fetch channel info",
  .argument_help = _wifi_get_channel_arg_help,
  .handler       = wifi_get_channel_command_handler,
  .argument_list = { CONSOLE_ENUM_ARG(wifi_interface), CONSOLE_ARG_END }
};

extern sl_status_t wifi_get_ap_client_info_command_handler(console_args_t *arguments);
static const char *_wifi_get_client_info_arg_help[] = {};

static const console_descriptive_command_t _wifi_get_client_info_command = {
  .description   = "Fetch the client info that are currently connected to AP",
  .argument_help = _wifi_get_client_info_arg_help,
  .handler       = wifi_get_ap_client_info_command_handler,
  .argument_list = { CONSOLE_ARG_END }
};

extern sl_status_t wifi_get_default_interface_command_handler(console_args_t *arguments);
static const char *_wifi_get_default_interface_arg_help[] = {};

static const console_descriptive_command_t _wifi_get_default_interface_command = {
  .description   = "Fetch default interface",
  .argument_help = _wifi_get_default_interface_arg_help,
  .handler       = wifi_get_default_interface_command_handler,
  .argument_list = { CONSOLE_ARG_END }
};

extern sl_status_t wifi_get_firmware_version_command_handler(console_args_t *arguments);
static const char *_wifi_get_fw_version_arg_help[] = {};

static const console_descriptive_command_t _wifi_get_fw_version_command = {
  .description   = "Fetch firmware version",
  .argument_help = _wifi_get_fw_version_arg_help,
  .handler       = wifi_get_firmware_version_command_handler,
  .argument_list = { CONSOLE_ARG_END }
};

extern sl_status_t wifi_get_mac_address_command_handler(console_args_t *arguments);
static const char *_wifi_get_mac_address_arg_help[] = {
  0,
};

static const console_descriptive_command_t _wifi_get_mac_address_command = {
  .description   = "Fetch MAC address",
  .argument_help = _wifi_get_mac_address_arg_help,
  .handler       = wifi_get_mac_address_command_handler,
  .argument_list = { CONSOLE_OPTIONAL_ARG('i', CONSOLE_ARG_UINT), CONSOLE_ARG_END }
};

extern sl_status_t wifi_get_pairwise_master_key_command_handler(console_args_t *arguments);
static const char *_wifi_get_pairwise_master_key_arg_help[] = {
  0,
  0,
  0,
  0,
};

static const console_descriptive_command_t _wifi_get_pairwise_master_key_command = {
  .description   = "Generate PMK if PSK and SSID are provided This is a blocking API",
  .argument_help = _wifi_get_pairwise_master_key_arg_help,
  .handler       = wifi_get_pairwise_master_key_command_handler,
  .argument_list = { CONSOLE_ARG_UINT,
                     CONSOLE_ARG_STRING,
                     CONSOLE_ARG_STRING,
                     CONSOLE_OPTIONAL_ARG('i', CONSOLE_ENUM_ARG(wifi_interface)),
                     CONSOLE_ARG_END }
};

extern sl_status_t wifi_get_performance_profile_command_handler(console_args_t *arguments);
static const char *_wifi_get_performance_profile_arg_help[] = {};

static const console_descriptive_command_t _wifi_get_performance_profile_command = {
  .description   = "Get the current performance profile of the module",
  .argument_help = _wifi_get_performance_profile_arg_help,
  .handler       = wifi_get_performance_profile_command_handler,
  .argument_list = { CONSOLE_ARG_END }
};

extern sl_status_t wifi_get_signal_strength_command_handler(console_args_t *arguments);
static const char *_wifi_get_signal_strength_arg_help[] = {
  0,
};

static const console_descriptive_command_t _wifi_get_signal_strength_command = {
  .description   = "Queries signal strength",
  .argument_help = _wifi_get_signal_strength_arg_help,
  .handler       = wifi_get_signal_strength_command_handler,
  .argument_list = { CONSOLE_OPTIONAL_ARG('i', CONSOLE_ENUM_ARG(wifi_interface)), CONSOLE_ARG_END }
};

extern sl_status_t wifi_get_statistics_command_handler(console_args_t *arguments);
static const char *_wifi_get_statistics_arg_help[] = {};

static const console_descriptive_command_t _wifi_get_statistics_command = {
  .description   = "Get wifi statistics",
  .argument_help = _wifi_get_statistics_arg_help,
  .handler       = wifi_get_statistics_command_handler,
  .argument_list = { CONSOLE_ARG_END }
};

extern sl_status_t wifi_get_transmit_rate_command_handler(console_args_t *arguments);
static const char *_wifi_get_transmit_rate_arg_help[] = {};

static const console_descriptive_command_t _wifi_get_transmit_rate_command = {
  .description   = "Fetch configured rate protocol and data rate",
  .argument_help = _wifi_get_transmit_rate_arg_help,
  .handler       = wifi_get_transmit_rate_command_handler,
  .argument_list = { CONSOLE_ARG_END }
};

extern sl_status_t wifi_get_tx_power_command_handler(console_args_t *arguments);
static const char *_wifi_get_tx_power_arg_help[] = {};

static const console_descriptive_command_t _wifi_get_tx_power_command = { .description   = "get module TX power",
                                                                          .argument_help = _wifi_get_tx_power_arg_help,
                                                                          .handler = wifi_get_tx_power_command_handler,
                                                                          .argument_list = { CONSOLE_ARG_END } };

extern sl_status_t wifi_init_command_handler(console_args_t *arguments);
static const char *_wifi_init_arg_help[] = { 0, 0, 0 };

static const console_descriptive_command_t _wifi_init_command = {
  .description   = "Init Wi-Fi interface",
  .argument_help = _wifi_init_arg_help,
  .handler       = wifi_init_command_handler,
  .argument_list = { CONSOLE_OPTIONAL_ARG('i', CONSOLE_ENUM_ARG(wifi_init_mode)),
                     CONSOLE_OPTIONAL_ARG('b', CONSOLE_ARG_INT),
                     CONSOLE_OPTIONAL_ARG('r', CONSOLE_ENUM_ARG(wifi_init_region)),
                     CONSOLE_ARG_END }
};

extern sl_status_t wifi_iot_socket_accept_handler(console_args_t *arguments);
static const char *_wifi_iot_socket_accept_arg_help[] = {
  0,
};

static const console_descriptive_command_t _wifi_iot_socket_accept_command = {
  .description   = "IOT socket listen, \n parameters is \n 1 - sock_fd \n",
  .argument_help = _wifi_iot_socket_accept_arg_help,
  .handler       = wifi_iot_socket_accept_handler,
  .argument_list = { CONSOLE_ARG_INT, CONSOLE_ARG_END }
};

extern sl_status_t wifi_iot_socket_bind_handler(console_args_t *arguments);
static const char *_wifi_iot_socket_bind_arg_help[] = {
  0,
  0,
  0,
  0,
};

static const console_descriptive_command_t _wifi_iot_socket_bind_command = {
  .description =
    "IOT socket bind, \n parameters is \n 1 - sock_fd, \n 2 - ip_address, \n 3 - port \n 4 - ip_address_len \n",
  .argument_help = _wifi_iot_socket_bind_arg_help,
  .handler       = wifi_iot_socket_bind_handler,
  .argument_list = { CONSOLE_ARG_INT,
                     CONSOLE_ARG_IP_ADDRESS,
                     CONSOLE_ARG_UINT,
                     CONSOLE_OPTIONAL_ARG('l', CONSOLE_ARG_UINT),
                     CONSOLE_ARG_END }
};

extern sl_status_t wifi_iot_socket_close_handler(console_args_t *arguments);
static const char *_wifi_iot_socket_close_arg_help[] = {
  0,
};

static const console_descriptive_command_t _wifi_iot_socket_close_command = {
  .description   = "IOT closing socket, \n parameters is \n 1 - sock_fd \n",
  .argument_help = _wifi_iot_socket_close_arg_help,
  .handler       = wifi_iot_socket_close_handler,
  .argument_list = { CONSOLE_ARG_INT, CONSOLE_ARG_END }
};

extern sl_status_t wifi_iot_socket_connect_handler(console_args_t *arguments);
static const char *_wifi_iot_socket_connect_arg_help[] = {
  0,
  0,
  0,
  0,
};

static const console_descriptive_command_t _wifi_iot_socket_connect_command = {
  .description =
    "IOT socket connect, \n parameters is \n 1 - sock_fd, , \n 2 - ip_address, \n 3 - port, \n 4 - ip_address_len \n",
  .argument_help = _wifi_iot_socket_connect_arg_help,
  .handler       = wifi_iot_socket_connect_handler,
  .argument_list = { CONSOLE_ARG_INT,
                     CONSOLE_ARG_IP_ADDRESS,
                     CONSOLE_ARG_UINT,
                     CONSOLE_OPTIONAL_ARG('l', CONSOLE_ARG_UINT),
                     CONSOLE_ARG_END }
};

extern sl_status_t wifi_iot_socket_create_handler(console_args_t *arguments);
static const char *_wifi_iot_socket_create_arg_help[] = {
  0,
  0,
  0,
};

static const console_descriptive_command_t _wifi_iot_socket_create_command = {
  .description   = "IOT socket creation, \n parameters are \n 1 - Type, possible value: 1(stream), 2(dgram), 3(raw),  "
                   "\n2 - protocol, possible value:1(tcp), 2(udp).. \n  - Domain, possible value: 1(ipv4), 2(ipv6) \n",
  .argument_help = _wifi_iot_socket_create_arg_help,
  .handler       = wifi_iot_socket_create_handler,
  .argument_list = { CONSOLE_ENUM_ARG(socket_type),
                     CONSOLE_ENUM_ARG(ip_protocol),
                     CONSOLE_OPTIONAL_ARG('j', CONSOLE_ENUM_ARG(ipv4_or_ipv6)),
                     CONSOLE_ARG_END }
};

extern sl_status_t wifi_iot_get_host_by_name_handler(console_args_t *arguments);
static const char *_wifi_iot_socket_get_host_by_name_arg_help[] = {
  0,
  0,
  0,
};

static const console_descriptive_command_t _wifi_iot_socket_get_host_by_name_command = {
  .description = "IOT socket get host by name, \n parameters is \nstring - url, \nipv4_or_ipv6 - AF interface\nuint32 "
                 "- length of ip in bytes",
  .argument_help = _wifi_iot_socket_get_host_by_name_arg_help,
  .handler       = wifi_iot_get_host_by_name_handler,
  .argument_list = { CONSOLE_ARG_STRING,
                     CONSOLE_ENUM_ARG(ipv4_or_ipv6),
                     CONSOLE_OPTIONAL_ARG('l', CONSOLE_ARG_UINT),
                     CONSOLE_ARG_END }
};

extern sl_status_t wifi_iot_socket_get_opt_handler(console_args_t *arguments);
static const char *_wifi_iot_socket_get_opt_arg_help[] = {
  0,
  0,
};

static const console_descriptive_command_t _wifi_iot_socket_get_opt_command = {
  .description   = "IOT socket get opt  parameters are \n 1 - sock_fd \n 2 - option_id \n",
  .argument_help = _wifi_iot_socket_get_opt_arg_help,
  .handler       = wifi_iot_socket_get_opt_handler,
  .argument_list = { CONSOLE_ARG_INT, CONSOLE_ENUM_ARG(get_option_id), CONSOLE_ARG_END }
};

extern sl_status_t wifi_iot_socket_get_peer_name_handler(console_args_t *arguments);
static const char *_wifi_iot_socket_get_peer_name_arg_help[] = {
  0,
};

static const console_descriptive_command_t _wifi_iot_socket_get_peer_name_command = {
  .description   = "IOT socket get peer name, \n parameters is \n 1 - sock_fd \n",
  .argument_help = _wifi_iot_socket_get_peer_name_arg_help,
  .handler       = wifi_iot_socket_get_peer_name_handler,
  .argument_list = { CONSOLE_ARG_INT, CONSOLE_ARG_END }
};

extern sl_status_t wifi_iot_socket_get_socket_name_handler(console_args_t *arguments);
static const char *_wifi_iot_socket_get_sock_name_arg_help[] = {
  0,
};

static const console_descriptive_command_t _wifi_iot_socket_get_sock_name_command = {
  .description   = "IOT socket get socket name, \n parameters is \n 1 - sock_fd \n",
  .argument_help = _wifi_iot_socket_get_sock_name_arg_help,
  .handler       = wifi_iot_socket_get_socket_name_handler,
  .argument_list = { CONSOLE_ARG_INT, CONSOLE_ARG_END }
};

extern sl_status_t wifi_iot_socket_listen_handler(console_args_t *arguments);
static const char *_wifi_iot_socket_listen_arg_help[] = {
  0,
  0,
};

static const console_descriptive_command_t _wifi_iot_socket_listen_command = {
  .description   = "IOT socket listen, \n parameters is \n 1 - sock_fd, \n 2 - backlog \n",
  .argument_help = _wifi_iot_socket_listen_arg_help,
  .handler       = wifi_iot_socket_listen_handler,
  .argument_list = { CONSOLE_ARG_INT, CONSOLE_ARG_INT, CONSOLE_ARG_END }
};

extern sl_status_t wifi_iot_socket_receive_handler(console_args_t *arguments);
static const char *_wifi_iot_socket_receive_arg_help[] = {
  0,
};

static const console_descriptive_command_t _wifi_iot_socket_receive_command = {
  .description   = "IOT socket receive, \n parameters is \n 1 - sock_fd \n",
  .argument_help = _wifi_iot_socket_receive_arg_help,
  .handler       = wifi_iot_socket_receive_handler,
  .argument_list = { CONSOLE_ARG_INT, CONSOLE_ARG_END }
};

extern sl_status_t wifi_iot_socket_receive_from_handler(console_args_t *arguments);
static const char *_wifi_iot_socket_receive_from_arg_help[] = {
  0,
};

static const console_descriptive_command_t _wifi_iot_socket_receive_from_command = {
  .description   = "IOT socket receive from, \n parameters is \n 1 - sock_fd \n",
  .argument_help = _wifi_iot_socket_receive_from_arg_help,
  .handler       = wifi_iot_socket_receive_from_handler,
  .argument_list = { CONSOLE_ARG_INT, CONSOLE_ARG_END }
};

extern sl_status_t wifi_iot_socket_send_handler(console_args_t *arguments);
static const char *_wifi_iot_socket_send_arg_help[] = {
  0,
  0,
};

static const console_descriptive_command_t _wifi_iot_socket_send_command = {
  .description   = "IOT socket receieve, \n parameters is \n 1 - sock_fd, \n 2 - data to be sent \n",
  .argument_help = _wifi_iot_socket_send_arg_help,
  .handler       = wifi_iot_socket_send_handler,
  .argument_list = { CONSOLE_ARG_INT, CONSOLE_ARG_STRING, CONSOLE_ARG_END }
};

extern sl_status_t wifi_iot_socket_send_to_handler(console_args_t *arguments);
static const char *_wifi_iot_socket_send_to_arg_help[] = {
  0, 0, 0, 0, 0,
};

static const console_descriptive_command_t _wifi_iot_socket_send_to_command = {
  .description = "IOT socket send to, \n parameters are \n 1 - sock_fd, \n 2 - data to be sent \n 3 - ip_address, \n 4 "
                 "- port, \n 5 - ip_address_len (4 or 16 bytes) \n",
  .argument_help = _wifi_iot_socket_send_to_arg_help,
  .handler       = wifi_iot_socket_send_to_handler,
  .argument_list = { CONSOLE_ARG_INT,
                     CONSOLE_ARG_STRING,
                     CONSOLE_ARG_IP_ADDRESS,
                     CONSOLE_ARG_UINT,
                     CONSOLE_OPTIONAL_ARG('l', CONSOLE_ARG_UINT),
                     CONSOLE_ARG_END }
};

extern sl_status_t wifi_iot_socket_set_opt_handler(console_args_t *arguments);
static const char *_wifi_iot_socket_set_opt_arg_help[] = {
  0,
  0,
  0,
};

static const console_descriptive_command_t _wifi_iot_socket_set_opt_command = {
  .description   = "IOT socket set opt parameters are \n 1 - sock_fd \n 2 - option_id \n 3 - value \n",
  .argument_help = _wifi_iot_socket_set_opt_arg_help,
  .handler       = wifi_iot_socket_set_opt_handler,
  .argument_list = { CONSOLE_ARG_INT, CONSOLE_ENUM_ARG(set_option_id), CONSOLE_ARG_INT, CONSOLE_ARG_END }
};

extern sl_status_t wifi_is_interface_up_command_handler(console_args_t *arguments);
static const char *_wifi_is_interface_up_arg_help[] = {
  0,
};

static const console_descriptive_command_t _wifi_is_interface_up_command = {
  .description   = "Check if the specified interface is up",
  .argument_help = _wifi_is_interface_up_arg_help,
  .handler       = wifi_is_interface_up_command_handler,
  .argument_list = { CONSOLE_ENUM_ARG(wifi_interface), CONSOLE_ARG_END }
};

extern sl_status_t wifi_load_certificate_handler(console_args_t *arguments);
static const char *_wifi_load_certificate_arg_help[] = {
  0,
  0,
  0,
};

static const console_descriptive_command_t _wifi_load_certificate_command = {
  .description =
    "Load/Clear Certificate from Flash, \n parameters are \n 1 - load/clear, possible value: 0(Clear), 1(Load) .., \n "
    "2 - Certificate Index , possible value: 0, 1, 2 ..,  \n 3 - Type, possible value: 0(OPENSSL), 1(AWS) .. \n",
  .argument_help = _wifi_load_certificate_arg_help,
  .handler       = wifi_load_certificate_handler,
  .argument_list = { CONSOLE_OPTIONAL_ARG('l', CONSOLE_ARG_INT),
                     CONSOLE_OPTIONAL_ARG('i', CONSOLE_ARG_INT),
                     CONSOLE_OPTIONAL_ARG('t', CONSOLE_ARG_INT),
                     CONSOLE_ARG_END }
};

extern sl_status_t wifi_scan_command_handler(console_args_t *arguments);
static const char *_wifi_scan_arg_help[] = {
  0, 0, 0, 0, 0, 0,
};

static const console_descriptive_command_t _wifi_scan_command = {
  .description   = "Start scanning",
  .argument_help = _wifi_scan_arg_help,
  .handler       = wifi_scan_command_handler,
  .argument_list = { CONSOLE_OPTIONAL_ARG('s', CONSOLE_ARG_STRING),
                     CONSOLE_OPTIONAL_ARG('i', CONSOLE_ENUM_ARG(wifi_interface)),
                     CONSOLE_OPTIONAL_ARG('m', CONSOLE_ENUM_ARG(enable_bg_scan)),
                     CONSOLE_OPTIONAL_ARG('c', CONSOLE_ARG_UINT),
                     CONSOLE_OPTIONAL_ARG('a', CONSOLE_ARG_UINT),
                     CONSOLE_OPTIONAL_ARG('t', CONSOLE_ARG_UINT),
                     CONSOLE_ARG_END }
};

extern sl_status_t sl_wifi_send_raw_data_command_handler(console_args_t *arguments);
static const char *_wifi_send_raw_data_arg_help[] = {
  0,
};

static const console_descriptive_command_t _wifi_send_raw_data_command = {
  .description   = "send raw data frame",
  .argument_help = _wifi_send_raw_data_arg_help,
  .handler       = sl_wifi_send_raw_data_command_handler,
  .argument_list = { CONSOLE_ENUM_ARG(wifi_interface), CONSOLE_ARG_END }
};

extern sl_status_t wifi_set_advanced_client_config_command_handler(console_args_t *arguments);
static const char *_wifi_set_advanced_client_config_arg_help[] = {
  0, 0, 0, 0, 0,
};

static const console_descriptive_command_t _wifi_set_advanced_client_config_command = {
  .description   = "Set advanced client config",
  .argument_help = _wifi_set_advanced_client_config_arg_help,
  .handler       = wifi_set_advanced_client_config_command_handler,
  .argument_list = { CONSOLE_OPTIONAL_ARG('o', CONSOLE_ARG_UINT),
                     CONSOLE_OPTIONAL_ARG('r', CONSOLE_ARG_UINT),
                     CONSOLE_OPTIONAL_ARG('i', CONSOLE_ARG_UINT),
                     CONSOLE_OPTIONAL_ARG('b', CONSOLE_ARG_UINT),
                     CONSOLE_OPTIONAL_ARG('f', CONSOLE_ARG_UINT),
                     CONSOLE_ARG_END }
};

extern sl_status_t sl_wifi_set_advanced_scan_configuration_command_handler(console_args_t *arguments);
static const char *_wifi_set_advanced_scan_configuration_arg_help[] = {
  0, 0, 0, 0, 0,
};

static const console_descriptive_command_t _wifi_set_advanced_scan_configuration_command = {
  .description   = "Set advanced scan configuration",
  .argument_help = _wifi_set_advanced_scan_configuration_arg_help,
  .handler       = sl_wifi_set_advanced_scan_configuration_command_handler,
  .argument_list = { CONSOLE_OPTIONAL_ARG('i', CONSOLE_ARG_UINT),
                     CONSOLE_OPTIONAL_ARG('j', CONSOLE_ARG_UINT),
                     CONSOLE_OPTIONAL_ARG('k', CONSOLE_ARG_INT),
                     CONSOLE_OPTIONAL_ARG('l', CONSOLE_ARG_UINT),
                     CONSOLE_OPTIONAL_ARG('m', CONSOLE_ARG_UINT),
                     CONSOLE_ARG_END }
};

extern sl_status_t wifi_set_antenna_command_handler(console_args_t *arguments);
static const char *_wifi_set_antenna_arg_help[] = {
  0,
  0,
};

static const console_descriptive_command_t _wifi_set_antenna_command = {
  .description   = "Set Wi-Fi antenna",
  .argument_help = _wifi_set_antenna_arg_help,
  .handler       = wifi_set_antenna_command_handler,
  .argument_list = { CONSOLE_OPTIONAL_ARG('i', CONSOLE_ENUM_ARG(wifi_interface)),
                     CONSOLE_OPTIONAL_ARG('a', CONSOLE_ARG_UINT),
                     CONSOLE_ARG_END }
};

extern sl_status_t wifi_set_ap_configuration_command_handler(console_args_t *arguments);
static const char *_wifi_set_ap_configuration_arg_help[] = {
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

static const console_descriptive_command_t _wifi_set_ap_configuration_command = {
  .description = "set ap configuration -s ssid -c channel -o option -i credential_id -t client_timeout -d becon_count "
                 "-m max_clients",
  .argument_help = _wifi_set_ap_configuration_arg_help,
  .handler       = wifi_set_ap_configuration_command_handler,
  .argument_list = { CONSOLE_OPTIONAL_ARG('s', CONSOLE_ARG_STRING),
                     CONSOLE_OPTIONAL_ARG('p', CONSOLE_ENUM_ARG(wifi_security)),
                     CONSOLE_OPTIONAL_ARG('e', CONSOLE_ENUM_ARG(wifi_encryption)),
                     CONSOLE_OPTIONAL_ARG('c', CONSOLE_ARG_UINT),
                     CONSOLE_OPTIONAL_ARG('r', CONSOLE_ENUM_ARG(rate_protocol)),
                     CONSOLE_OPTIONAL_ARG('o', CONSOLE_ARG_UINT),
                     CONSOLE_OPTIONAL_ARG('i', CONSOLE_ARG_UINT),
                     CONSOLE_OPTIONAL_ARG('t', CONSOLE_ARG_UINT),
                     CONSOLE_OPTIONAL_ARG('d', CONSOLE_ARG_UINT),
                     CONSOLE_OPTIONAL_ARG('m', CONSOLE_ARG_UINT),
                     CONSOLE_ARG_END }
};

extern sl_status_t wifi_set_channel_command_handler(console_args_t *arguments);
static const char *_wifi_set_channel_arg_help[] = {
  0,
  0,
};

static const console_descriptive_command_t _wifi_set_channel_command = {
  .description   = "Set channel number in AP interface",
  .argument_help = _wifi_set_channel_arg_help,
  .handler       = wifi_set_channel_command_handler,
  .argument_list = { CONSOLE_ENUM_ARG(wifi_interface), CONSOLE_ARG_UINT, CONSOLE_ARG_END }
};

extern sl_status_t wifi_set_default_interface_command_handler(console_args_t *arguments);
static const char *_wifi_set_default_interface_arg_help[] = {
  0,
};

static const console_descriptive_command_t _wifi_set_default_interface_command = {
  .description   = "Set default interface",
  .argument_help = _wifi_set_default_interface_arg_help,
  .handler       = wifi_set_default_interface_command_handler,
  .argument_list = { CONSOLE_ENUM_ARG(wifi_interface), CONSOLE_ARG_END }
};

extern sl_status_t wifi_set_performance_profile_command_handler(console_args_t *arguments);
static const char *_wifi_set_performance_profile_arg_help[] = {
  0,
};

static const console_descriptive_command_t _wifi_set_performance_profile_command = {
  .description   = "Set the performance profile of the module",
  .argument_help = _wifi_set_performance_profile_arg_help,
  .handler       = wifi_set_performance_profile_command_handler,
  .argument_list = { CONSOLE_ENUM_ARG(performance_mode), CONSOLE_ARG_END }
};

extern sl_status_t wifi_set_roam_configuration_command_handler(console_args_t *arguments);
static const char *_wifi_set_roam_configuration_arg_help[] = {
  0,
  0,
  0,
};

static const console_descriptive_command_t _wifi_set_roam_configuration_command = {
  .description   = "Set roaming configuration",
  .argument_help = _wifi_set_roam_configuration_arg_help,
  .handler       = wifi_set_roam_configuration_command_handler,
  .argument_list = { CONSOLE_OPTIONAL_ARG('i', CONSOLE_ENUM_ARG(wifi_interface)),
                     CONSOLE_ARG_INT,
                     CONSOLE_ARG_INT,
                     CONSOLE_ARG_END }
};

extern sl_status_t wifi_set_transmit_rate_command_handler(console_args_t *arguments);
static const char *_wifi_set_transmit_rate_arg_help[] = {
  0,
  0,
  0,
};

static const console_descriptive_command_t _wifi_set_transmit_rate_command = {
  .description   = "Set rate protocol and data rate",
  .argument_help = _wifi_set_transmit_rate_arg_help,
  .handler       = wifi_set_transmit_rate_command_handler,
  .argument_list = { CONSOLE_ENUM_ARG(rate_protocol),
                     CONSOLE_ENUM_ARG(data_rate),
                     CONSOLE_OPTIONAL_ARG('i', CONSOLE_ENUM_ARG(wifi_interface)),
                     CONSOLE_ARG_END }
};

extern sl_status_t wifi_set_tx_power_command_handler(console_args_t *arguments);
static const char *_wifi_set_tx_power_arg_help[] = {
  0,
  0,
};

static const console_descriptive_command_t _wifi_set_tx_power_command = {
  .description   = "Set module TX power",
  .argument_help = _wifi_set_tx_power_arg_help,
  .handler       = wifi_set_tx_power_command_handler,
  .argument_list = { CONSOLE_ARG_UINT, CONSOLE_ARG_UINT, CONSOLE_ARG_END }
};

extern sl_status_t wifi_start_ap_command_handler(console_args_t *arguments);
static const char *_wifi_start_ap_arg_help[] = {
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

static const console_descriptive_command_t _wifi_start_ap_command = {
  .description   = "Start in AP mode. Check sl_wifi_ap_configuration_t for parameters description",
  .argument_help = _wifi_start_ap_arg_help,
  .handler       = wifi_start_ap_command_handler,
  .argument_list = { CONSOLE_OPTIONAL_ARG('a', CONSOLE_ARG_STRING),
                     CONSOLE_OPTIONAL_ARG('b', CONSOLE_ENUM_ARG(wifi_security)),
                     CONSOLE_OPTIONAL_ARG('c', CONSOLE_ENUM_ARG(wifi_encryption)),
                     CONSOLE_OPTIONAL_ARG('d', CONSOLE_ARG_UINT),
                     CONSOLE_OPTIONAL_ARG('e', CONSOLE_ENUM_ARG(wifi_band)),
                     CONSOLE_OPTIONAL_ARG('f', CONSOLE_ENUM_ARG(wifi_bandwidth)),
                     CONSOLE_OPTIONAL_ARG('g', CONSOLE_ENUM_ARG(rate_protocol)),
                     CONSOLE_OPTIONAL_ARG('h', CONSOLE_ENUM_ARG(wifi_ap_flag)),
                     CONSOLE_OPTIONAL_ARG('i', CONSOLE_ARG_UINT),
                     CONSOLE_OPTIONAL_ARG('j', CONSOLE_ARG_UINT),
                     CONSOLE_OPTIONAL_ARG('k', CONSOLE_ARG_UINT),
                     CONSOLE_OPTIONAL_ARG('l', CONSOLE_ARG_UINT),
                     CONSOLE_OPTIONAL_ARG('m', CONSOLE_ARG_UINT),
                     CONSOLE_OPTIONAL_ARG('n', CONSOLE_ARG_UINT),
                     CONSOLE_ARG_END }
};

extern sl_status_t wifi_start_statistic_report_command_handler(console_args_t *arguments);
static const char *_wifi_start_statistic_report_arg_help[] = {
  0,
  0,
};

static const console_descriptive_command_t _wifi_start_statistic_report_command = {
  .description   = "Get wifi statistic report",
  .argument_help = _wifi_start_statistic_report_arg_help,
  .handler       = wifi_start_statistic_report_command_handler,
  .argument_list = { CONSOLE_OPTIONAL_ARG('i', CONSOLE_ENUM_ARG(wifi_interface)),
                     CONSOLE_OPTIONAL_ARG('c', CONSOLE_ARG_UINT),
                     CONSOLE_OPTIONAL_ARG('n', CONSOLE_ARG_UINT),
                     CONSOLE_ARG_END }
};

extern sl_status_t wifi_start_wps(console_args_t *arguments);
static const char *_wifi_start_wps_arg_help[] = {
  0,
  0,
  0,
};

static const console_descriptive_command_t _wifi_start_wps_command = { .description   = "Enable WPS on access point",
                                                                       .argument_help = _wifi_start_wps_arg_help,
                                                                       .handler       = wifi_start_wps,
                                                                       .argument_list = {
                                                                         CONSOLE_ENUM_ARG(wifi_interface),
                                                                         CONSOLE_ENUM_ARG(wps_mode),
                                                                         CONSOLE_OPTIONAL_ARG('p', CONSOLE_ARG_STRING),
                                                                         CONSOLE_ARG_END } };

extern sl_status_t wifi_stop_scan_command_handler(console_args_t *arguments);
static const char *_wifi_stop_scan_arg_help[] = {
  0,
};

static const console_descriptive_command_t _wifi_stop_scan_command = {
  .description   = "Stop scanning",
  .argument_help = _wifi_stop_scan_arg_help,
  .handler       = wifi_stop_scan_command_handler,
  .argument_list = { CONSOLE_OPTIONAL_ARG('i', CONSOLE_ENUM_ARG(wifi_interface)), CONSOLE_ARG_END }
};

extern sl_status_t wifi_stop_statistic_report_command_handler(console_args_t *arguments);
static const char *_wifi_stop_statistic_report_arg_help[] = {
  0,
};

static const console_descriptive_command_t _wifi_stop_statistic_report_command = {
  .description   = "Stop wifi statistic report",
  .argument_help = _wifi_stop_statistic_report_arg_help,
  .handler       = wifi_stop_statistic_report_command_handler,
  .argument_list = { CONSOLE_OPTIONAL_ARG('i', CONSOLE_ENUM_ARG(wifi_interface)), CONSOLE_ARG_END }
};

extern sl_status_t wifi_test_client_configuration_command_handler(console_args_t *arguments);
static const char *_wifi_test_client_configuration_arg_help[] = {
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

static const console_descriptive_command_t _wifi_test_client_configuration_command = {
  .description   = "Start connecting to specified AP",
  .argument_help = _wifi_test_client_configuration_arg_help,
  .handler       = wifi_test_client_configuration_command_handler,
  .argument_list = { CONSOLE_ARG_STRING,
                     CONSOLE_ARG_STRING,
                     CONSOLE_OPTIONAL_ARG('s', CONSOLE_ENUM_ARG(wifi_security)),
                     CONSOLE_OPTIONAL_ARG('u', CONSOLE_ARG_STRING),
                     CONSOLE_OPTIONAL_ARG('e', CONSOLE_ENUM_ARG(wifi_encryption)),
                     CONSOLE_OPTIONAL_ARG('t', CONSOLE_ARG_UINT),
                     CONSOLE_OPTIONAL_ARG('l', CONSOLE_ARG_UINT),
                     CONSOLE_OPTIONAL_ARG('c', CONSOLE_ARG_UINT),
                     CONSOLE_OPTIONAL_ARG('b', CONSOLE_ENUM_ARG(wifi_band)),
                     CONSOLE_OPTIONAL_ARG('w', CONSOLE_ENUM_ARG(wifi_bandwidth)),
                     CONSOLE_ARG_END }
};

extern sl_status_t sl_wifi_transmit_test_start_command_handler(console_args_t *arguments);
static const char *_wifi_transmit_test_start_arg_help[] = {
  0, 0, 0, 0, 0,
};

static const console_descriptive_command_t _wifi_transmit_test_start_command = {
  .description   = "Start the transmit test. This is a blocking API",
  .argument_help = _wifi_transmit_test_start_arg_help,
  .handler       = sl_wifi_transmit_test_start_command_handler,
  .argument_list = { CONSOLE_ARG_UINT,
                     CONSOLE_ARG_UINT,
                     CONSOLE_ARG_UINT,
                     CONSOLE_ARG_UINT,
                     CONSOLE_ARG_UINT,
                     CONSOLE_ARG_END }
};

extern sl_status_t sl_wifi_ax_transmit_test_start_command_handler(console_args_t *arguments);
static const char *_wifi_ax_transmit_test_start_arg_help[] = {
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

static const console_descriptive_command_t _wifi_ax_transmit_test_start_command = {
  .description   = "Start the transmit test for Wi-Fi 6. This is a blocking API",
  .argument_help = _wifi_ax_transmit_test_start_arg_help,
  .handler       = sl_wifi_ax_transmit_test_start_command_handler,
  .argument_list = { CONSOLE_ARG_UINT, CONSOLE_ARG_UINT, CONSOLE_ARG_UINT, CONSOLE_ARG_UINT, CONSOLE_ARG_UINT,
                     CONSOLE_ARG_UINT, CONSOLE_ARG_UINT, CONSOLE_ARG_UINT, CONSOLE_ARG_UINT, CONSOLE_ARG_UINT,
                     CONSOLE_ARG_UINT, CONSOLE_ARG_UINT, CONSOLE_ARG_UINT, CONSOLE_ARG_UINT, CONSOLE_ARG_UINT,
                     CONSOLE_ARG_UINT, CONSOLE_ARG_UINT, CONSOLE_ARG_UINT, CONSOLE_ARG_UINT, CONSOLE_ARG_UINT,
                     CONSOLE_ARG_UINT, CONSOLE_ARG_UINT, CONSOLE_ARG_UINT, CONSOLE_ARG_UINT, CONSOLE_ARG_UINT,
                     CONSOLE_ARG_UINT, CONSOLE_ARG_UINT, CONSOLE_ARG_UINT, CONSOLE_ARG_END }
};

extern sl_status_t sl_wifi_transmit_test_stop_command_handler(console_args_t *arguments);
static const char *_wifi_transmit_test_stop_arg_help[] = {};

static const console_descriptive_command_t _wifi_transmit_test_stop_command = {
  .description   = "Stop the transmit test. This is a blocking API",
  .argument_help = _wifi_transmit_test_stop_arg_help,
  .handler       = sl_wifi_transmit_test_stop_command_handler,
  .argument_list = { CONSOLE_ARG_END }
};

extern sl_status_t sl_wifi_update_gain_table_command_handler(console_args_t *arguments);
static const char *_wifi_update_gain_table_arg_help[] = {
  0,
  0,
};

static const console_descriptive_command_t _wifi_update_gain_table_command = {
  .description   = "Assign the user configurable channel gain values in different regions to the module from user",
  .argument_help = _wifi_update_gain_table_arg_help,
  .handler       = sl_wifi_update_gain_table_command_handler,
  .argument_list = { CONSOLE_ARG_UINT, CONSOLE_ARG_UINT, CONSOLE_ARG_END }
};

extern sl_status_t set_region_configuration_handler(console_args_t *arguments);
static const char *_set_region_configuration_arg_help[] = { 0, 0 };

static const console_descriptive_command_t _set_region_configuration = {
  .description   = "Set region configuration",
  .argument_help = _set_region_configuration_arg_help,
  .handler       = set_region_configuration_handler,
  .argument_list = { CONSOLE_OPTIONAL_ARG('a', CONSOLE_ENUM_ARG(wifi_init_mode)),
                     CONSOLE_ENUM_ARG(wifi_init_region),
                     CONSOLE_ARG_END }
};

extern sl_status_t wifi_radio_deinit_command_handler(console_args_t *arguments);
static const char *wifi_radio_deinit_arg_help[] = {};

static const console_descriptive_command_t _wifi_radio_deinit = { .description   = "Wifi radio deinit",
                                                                  .argument_help = wifi_radio_deinit_arg_help,
                                                                  .handler       = wifi_radio_deinit_command_handler,
                                                                  .argument_list = { CONSOLE_ARG_END } };

//_ble_user_gain_max_power
extern sl_status_t ble_user_gain_table_max_power_handler(console_args_t *arguments);
static const char *_ble_user_gain_table_max_power_arg_help[] = {
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

static const console_descriptive_command_t _ble_user_gain_table_max_power = {
  .description   = "User gain table max power",
  .argument_help = _ble_user_gain_table_max_power_arg_help,
  .handler       = ble_user_gain_table_max_power_handler,
  .argument_list = { CONSOLE_ENUM_ARG(ble_user_gain_table_region), // CONSOLE_OPTIONAL_ARG('a', CONSOLE_ARG_UINT),
                     CONSOLE_ARG_UINT,                             //CONSOLE_OPTIONAL_ARG('d', CONSOLE_ARG_UINT),
                     CONSOLE_ENUM_ARG(ble_user_gain_table_region),
                     CONSOLE_ARG_UINT,
                     CONSOLE_ENUM_ARG(ble_user_gain_table_region),
                     CONSOLE_ARG_UINT,                             // CONSOLE_OPTIONAL_ARG('a', CONSOLE_ARG_UINT),
                     CONSOLE_ENUM_ARG(ble_user_gain_table_region), //CONSOLE_OPTIONAL_ARG('d', CONSOLE_ARG_UINT),
                     CONSOLE_ARG_UINT,
                     CONSOLE_ENUM_ARG(ble_user_gain_table_region),
                     CONSOLE_ARG_UINT,
                     CONSOLE_ARG_END }
};

extern sl_status_t ble_user_gain_table_max_power_offset_handler(console_args_t *arguments);
static const char *_ble_user_gain_table_max_power_offset_arg_help[] = {};

static const console_descriptive_command_t _ble_user_gain_table_max_power_offset = {
  .description   = "User gain table max power offset",
  .argument_help = _ble_user_gain_table_max_power_offset_arg_help,
  .handler       = ble_user_gain_table_max_power_offset_handler,
  .argument_list = { CONSOLE_ARG_END }
};

extern sl_status_t ble_user_gain_table_lp_chain_0dBm_offset_handler(console_args_t *arguments);
static const char *_ble_user_gain_table_lp_chain_0dBm_offset_arg_help[] = {};

static const console_descriptive_command_t _ble_user_gain_table_lp_chain_0dBm_offset = {
  .description   = "User gain table LP chain 0dBm offset",
  .argument_help = _ble_user_gain_table_lp_chain_0dBm_offset_arg_help,
  .handler       = ble_user_gain_table_lp_chain_0dBm_offset_handler,
  .argument_list = { CONSOLE_ARG_END }
};

extern sl_status_t ble_user_gain_table_lp_chain_8dBm_offset_handler(console_args_t *arguments);
static const char *_ble_user_gain_table_lp_chain_8dBm_offset_arg_help[] = {};

static const console_descriptive_command_t _ble_user_gain_table_lp_chain_8dBm_offset = {
  .description   = "User gain table LP chain 8dBm offset",
  .argument_help = _ble_user_gain_table_lp_chain_8dBm_offset_arg_help,
  .handler       = ble_user_gain_table_lp_chain_8dBm_offset_handler,
  .argument_list = { CONSOLE_ARG_END }
};

extern sl_status_t ble_tx_test_mode_handler(console_args_t *arguments);
static const char *_ble_tx_test_mode_arg_help[] = {
  0,
  0,
  0,
  0,
};

static const console_descriptive_command_t _ble_tx_test_mode = { .description   = "BLE TX Testmode",
                                                                 .argument_help = _ble_tx_test_mode_arg_help,
                                                                 .handler       = ble_tx_test_mode_handler,
                                                                 .argument_list = {
                                                                   CONSOLE_ARG_UINT,
                                                                   CONSOLE_ARG_UINT,
                                                                   CONSOLE_OPTIONAL_ARG('a', CONSOLE_ARG_UINT),
                                                                   CONSOLE_OPTIONAL_ARG('b', CONSOLE_ARG_UINT),
                                                                   CONSOLE_ARG_END } };

//_ble_tx_test_mode
extern sl_status_t ble_rx_test_mode_handler(console_args_t *arguments);
static const char *_ble_rx_test_mode_arg_help[] = {
  0,
  0,
};

static const console_descriptive_command_t _ble_rx_test_mode = {
  .description   = "BLE RX Testmode",
  .argument_help = _ble_rx_test_mode_arg_help,
  .handler       = ble_rx_test_mode_handler,
  .argument_list = { CONSOLE_ARG_UINT, CONSOLE_ARG_UINT, CONSOLE_ARG_END }
};

//_ble_end_test_mode
extern sl_status_t ble_end_test_command_handler(console_args_t *arguments);
static const char *_ble_end_test_arg_help[] = {};

static const console_descriptive_command_t _ble_end_test_mode = { .description   = "BLE End test commands",
                                                                  .argument_help = _ble_end_test_arg_help,
                                                                  .handler       = ble_end_test_command_handler,
                                                                  .argument_list = { CONSOLE_ARG_END } };

extern sl_status_t rsi_ble_per_transmit_command_handler(console_args_t *arguments);
static const char *_ble_per_transmit_arg_help[] = {
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

static const console_descriptive_command_t _ble_per_transmit_command = {
  .description   = "Start BLE PER transmission",
  .argument_help = _ble_per_transmit_arg_help,
  .handler       = rsi_ble_per_transmit_command_handler,
  .argument_list = { CONSOLE_ARG_UINT,
                     CONSOLE_ARG_UINT,
                     CONSOLE_ARG_UINT,
                     CONSOLE_ARG_UINT,
                     CONSOLE_ARG_UINT,
                     CONSOLE_ARG_UINT,
                     CONSOLE_OPTIONAL_ARG('a', CONSOLE_ARG_UINT),
                     CONSOLE_OPTIONAL_ARG('d', CONSOLE_ARG_UINT),
                     CONSOLE_OPTIONAL_ARG('c', CONSOLE_ARG_UINT),
                     CONSOLE_OPTIONAL_ARG('n', CONSOLE_ARG_UINT),
                     CONSOLE_OPTIONAL_ARG('p', CONSOLE_ARG_UINT),
                     CONSOLE_OPTIONAL_ARG('s', CONSOLE_ARG_UINT),
                     CONSOLE_ARG_END }
};

extern sl_status_t rsi_ble_per_receive_command_handler(console_args_t *arguments);
static const char *_ble_per_receive_arg_help[] = {
  0, 0, 0, 0, 0,
};

static const console_descriptive_command_t _ble_per_receive_command = { .description   = "Start BLE PER reception",
                                                                        .argument_help = _ble_per_receive_arg_help,
                                                                        .handler = rsi_ble_per_receive_command_handler,
                                                                        .argument_list = {
                                                                          CONSOLE_ARG_UINT,
                                                                          CONSOLE_ARG_UINT,
                                                                          CONSOLE_ARG_UINT,
                                                                          CONSOLE_OPTIONAL_ARG('a', CONSOLE_ARG_UINT),
                                                                          CONSOLE_OPTIONAL_ARG('b', CONSOLE_ARG_UINT),
                                                                          CONSOLE_ARG_END } };

extern sl_status_t rsi_bt_per_stats_command_handler(console_args_t *arguments);
static const char *_bt_per_stats_arg_help[] = {
  0,
};

static const console_descriptive_command_t _bt_per_stats_command = {
  .description   = "Read the BLE PER statistics",
  .argument_help = _bt_per_stats_arg_help,
  .handler       = rsi_bt_per_stats_command_handler,
  .argument_list = { CONSOLE_OPTIONAL_ARG('t', CONSOLE_ARG_UINT), CONSOLE_ARG_END }
};

extern sl_status_t rsi_bt_get_local_name_command_handler(console_args_t *arguments);
static const char *_bt_get_local_name_arg_help[] = {};

static const console_descriptive_command_t _bt_get_local_name_command = { .description   = "Get BLE device name",
                                                                          .argument_help = _bt_get_local_name_arg_help,
                                                                          .handler =
                                                                            rsi_bt_get_local_name_command_handler,
                                                                          .argument_list = { CONSOLE_ARG_END } };

extern sl_status_t rsi_bt_set_local_name_command_handler(console_args_t *arguments);
static const char *_bt_set_local_name_arg_help[] = {};

static const console_descriptive_command_t _bt_set_local_name_command = {
  .description   = "Set BLE device name to a predefined value",
  .argument_help = _bt_set_local_name_arg_help,
  .handler       = rsi_bt_set_local_name_command_handler,
  .argument_list = { CONSOLE_ARG_END }
};

extern sl_status_t rsi_bt_get_local_device_address_command_handler(console_args_t *arguments);
static const char *_bt_get_local_device_address_arg_help[] = {};

static const console_descriptive_command_t _bt_get_local_device_address_command = {
  .description   = "Get BLE device address",
  .argument_help = _bt_get_local_device_address_arg_help,
  .handler       = rsi_bt_get_local_device_address_command_handler,
  .argument_list = { CONSOLE_ARG_END }
};

extern sl_status_t rsi_ble_set_advertise_data_command_handler(console_args_t *arguments);
static const char *_ble_set_advertise_data_arg_help[] = {};

static const console_descriptive_command_t _ble_set_advertise_data_command = {
  .description   = "Set BLE advertising data to device name",
  .argument_help = _ble_set_advertise_data_arg_help,
  .handler       = rsi_ble_set_advertise_data_command_handler,
  .argument_list = { CONSOLE_ARG_END }
};

extern sl_status_t rsi_ble_start_advertising_command_handler(console_args_t *arguments);
static const char *_ble_start_advertising_arg_help[] = {};

static const console_descriptive_command_t _ble_start_advertising_command = {
  .description   = "Start BLE advertising",
  .argument_help = _ble_start_advertising_arg_help,
  .handler       = rsi_ble_start_advertising_command_handler,
  .argument_list = { CONSOLE_ARG_END }
};

extern sl_status_t rsi_ble_stop_advertising_command_handler(console_args_t *arguments);
static const char *_ble_stop_advertising_arg_help[] = {};

static const console_descriptive_command_t _ble_stop_advertising_command = {
  .description   = "Stop BLE advertising",
  .argument_help = _ble_stop_advertising_arg_help,
  .handler       = rsi_ble_stop_advertising_command_handler,
  .argument_list = { CONSOLE_ARG_END }
};
extern sl_status_t wifi_reset_command_handler(console_args_t *arguments);
static const char *wifi_reset_arg_help[] = {};

static const console_descriptive_command_t wifi_reset_command = { .description   = "Reset the device.",
                                                                  .argument_help = wifi_reset_arg_help,
                                                                  .handler       = wifi_reset_command_handler,
                                                                  .argument_list = { CONSOLE_ARG_END } };

extern sl_status_t sl_si91x_calibration_write_command_handler(console_args_t *arguments);
static const char *_si91x_calibration_write_arg_help[] = {
  0, 0, 0, 0, 0, 0,
};

static const console_descriptive_command_t _si91x_calibration_write_command = {
  .description   = "Write Calibration data",
  .argument_help = _si91x_calibration_write_arg_help,
  .handler       = sl_si91x_calibration_write_command_handler,
  .argument_list = { CONSOLE_ARG_UINT,
                     CONSOLE_ARG_INT,
                     CONSOLE_ARG_INT,
                     CONSOLE_ARG_INT,
                     CONSOLE_OPTIONAL_ARG('c', CONSOLE_ARG_INT),
                     CONSOLE_OPTIONAL_ARG('t', CONSOLE_ARG_UINT),
                     CONSOLE_ARG_END }
};

extern sl_status_t sl_si91x_calibration_read_command_handler(console_args_t *arguments);
static const char *_si91x_calibration_read_arg_help[] = {
  0,
};

static const console_descriptive_command_t _si91x_calibration_read_command = {
  .description   = "Read Calibration data",
  .argument_help = _si91x_calibration_read_arg_help,
  .handler       = sl_si91x_calibration_read_command_handler,
  .argument_list = { CONSOLE_OPTIONAL_ARG('t', CONSOLE_ARG_UINT), CONSOLE_ARG_END }
};

extern sl_status_t sl_si91x_frequency_offset_command_handler(console_args_t *arguments);
static const char *_si91x_frequency_offset_arg_help[] = {
  0,
};

static const console_descriptive_command_t _si91x_frequency_offset_command = {
  .description   = "Calibrate frequency offset",
  .argument_help = _si91x_frequency_offset_arg_help,
  .handler       = sl_si91x_frequency_offset_command_handler,
  .argument_list = { CONSOLE_ARG_INT, CONSOLE_ARG_END }
};

const console_database_t console_command_database = { CONSOLE_DATABASE_ENTRIES(
  { "get", &_get_command },
  { "help", &_help_command },
  { "list", &_list_command },
  { "reset", &wifi_reset_command },
  { "set_region_configuration", &_set_region_configuration },
  { "wifi_radio_deinit", &_wifi_radio_deinit },
  { "ble_per_transmit", &_ble_per_transmit_command },
  { "ble_per_receive", &_ble_per_receive_command },
  { "bt_per_stats", &_bt_per_stats_command },
  { "ble_user_gain_max_power", &_ble_user_gain_table_max_power },
  { "ble_user_gain_max_power_offset", &_ble_user_gain_table_max_power_offset },
  { "ble_user_gain_lp_chain_0dBm_offset", &_ble_user_gain_table_lp_chain_0dBm_offset },
  { "ble_user_gain_table_lp_chain_8dBm_offset", &_ble_user_gain_table_lp_chain_8dBm_offset },
  { "ble_tx_test_mode", &_ble_tx_test_mode },
  { "ble_rx_test_mode", &_ble_rx_test_mode },
  { "ble_end_test_mode", &_ble_end_test_mode },
  { "bt_get_local_name", &_bt_get_local_name_command },
  { "bt_get_local_device_address", &_bt_get_local_device_address_command },
  { "bt_set_local_name", &_bt_set_local_name_command },
  { "ble_set_advertise_data", &_ble_set_advertise_data_command },
  { "ble_start_advertising", &_ble_start_advertising_command },
  { "ble_stop_advertising", &_ble_stop_advertising_command },
  { "net_deinit", &_net_deinit_command },
  { "net_down", &_net_down_command },
  { "net_init", &_net_init_command },
  { "net_join_multicast_address", &_net_join_multicast_address_command },
  { "net_leave_multicast_address", &_net_leave_multicast_address_command },
  { "net_up", &_net_up_command },
  { "rtt", &_rtt_command },
  { "set", &_set_command },
  { "set_nvm_profile", &_set_nvm_profile_command },
  { "sl_net_dns_resolve_hostname", &_sl_net_dns_resolve_hostname_command },
  { "sl_net_ping", &_sl_net_ping_command },
  { "sl_si91x_get_ram_log", &_sl_si91x_get_ram_log_command },
  { "start_dhcp", &_start_dhcp_command },
  { "thread", &_thread_command },
  { "wifi_11ax_config", &_wifi_11ax_config_command },
  { "wifi_assert", &_wifi_assert_command },
  { "wifi_config_pll_mode", &_wifi_config_pll_mode_command },
  { "wifi_config_power_chain", &_wifi_config_power_chain_command },
  { "wifi_bsd_get_host_by_name", &_wifi_bsd_get_host_by_name_command },
  { "wifi_bsd_get_peer_name", &_wifi_bsd_get_peer_name_command },
  { "wifi_bsd_get_sock_name", &_wifi_bsd_get_sock_name_command },
  { "wifi_bsd_get_sock_opt", &_wifi_bsd_get_sock_opt_command },
  { "wifi_bsd_set_sock_opt", &_wifi_bsd_set_sock_opt_command },
  { "wifi_bsd_socket_accept", &_wifi_bsd_socket_accept_command },
  { "wifi_bsd_socket_bind", &_wifi_bsd_socket_bind_command },
  { "wifi_bsd_socket_close", &_wifi_bsd_socket_close_command },
  { "wifi_bsd_socket_connect", &_wifi_bsd_socket_connect_command },
  { "wifi_bsd_socket_create", &_wifi_bsd_socket_create_command },
  { "wifi_bsd_socket_listen", &_wifi_bsd_socket_listen_command },
  { "wifi_bsd_socket_receive", &_wifi_bsd_socket_receive_command },
  { "wifi_bsd_socket_receive_from", &_wifi_bsd_socket_receive_from_command },
  { "wifi_bsd_socket_select", &_wifi_bsd_socket_select_command },
  { "wifi_bsd_socket_send", &_wifi_bsd_socket_send_command },
  { "wifi_bsd_socket_send_to", &_wifi_bsd_socket_send_to_command },
  { "wifi_connect", &_wifi_connect_command },
  { "wifi_deinit", &_wifi_deinit_command },
  { "wifi_disable_target_wake_time", &_wifi_disable_target_wake_time_command },
  { "wifi_disconnect", &_wifi_disconnect_command },
  { "wifi_disconnect_ap_client", &_wifi_disconnect_ap_client_command },
  { "wifi_enable_target_wake_time", &_wifi_enable_target_wake_time_command },
  { "wifi_filter_broadcast", &_wifi_filter_broadcast_command },
  { "wifi_generate_wps_pin", &_wifi_generate_wps_pin_command },
  { "wifi_get_advanced_scan_configuration", &_wifi_get_advanced_scan_configuration_command },
  { "wifi_get_ap_client_count", &_wifi_get_ap_client_count_command },
  { "wifi_get_ap_client_list", &_wifi_get_ap_client_list_command },
  { "wifi_get_ap_configuration", &_wifi_get_ap_configuration_command },
  { "wifi_get_channel", &_wifi_get_channel_command },
  { "wifi_get_client_info", &_wifi_get_client_info_command },
  { "wifi_get_default_interface", &_wifi_get_default_interface_command },
  { "wifi_get_fw_version", &_wifi_get_fw_version_command },
  { "wifi_get_mac_address", &_wifi_get_mac_address_command },
  { "wifi_get_pairwise_master_key", &_wifi_get_pairwise_master_key_command },
  { "wifi_get_performance_profile", &_wifi_get_performance_profile_command },
  { "wifi_get_signal_strength", &_wifi_get_signal_strength_command },
  { "wifi_get_statistics", &_wifi_get_statistics_command },
  { "wifi_get_transmit_rate", &_wifi_get_transmit_rate_command },
  { "wifi_get_tx_power", &_wifi_get_tx_power_command },
  { "wifi_init", &_wifi_init_command },
  { "wifi_iot_socket_accept", &_wifi_iot_socket_accept_command },
  { "wifi_iot_socket_bind", &_wifi_iot_socket_bind_command },
  { "wifi_iot_socket_close", &_wifi_iot_socket_close_command },
  { "wifi_iot_socket_connect", &_wifi_iot_socket_connect_command },
  { "wifi_iot_socket_create", &_wifi_iot_socket_create_command },
  { "wifi_iot_socket_get_host_by_name", &_wifi_iot_socket_get_host_by_name_command },
  { "wifi_iot_socket_get_opt", &_wifi_iot_socket_get_opt_command },
  { "wifi_iot_socket_get_peer_name", &_wifi_iot_socket_get_peer_name_command },
  { "wifi_iot_socket_get_sock_name", &_wifi_iot_socket_get_sock_name_command },
  { "wifi_iot_socket_listen", &_wifi_iot_socket_listen_command },
  { "wifi_iot_socket_receive", &_wifi_iot_socket_receive_command },
  { "wifi_iot_socket_receive_from", &_wifi_iot_socket_receive_from_command },
  { "wifi_iot_socket_send", &_wifi_iot_socket_send_command },
  { "wifi_iot_socket_send_to", &_wifi_iot_socket_send_to_command },
  { "wifi_iot_socket_set_opt", &_wifi_iot_socket_set_opt_command },
  { "wifi_is_interface_up", &_wifi_is_interface_up_command },
  { "wifi_load_certificate", &_wifi_load_certificate_command },
  { "wifi_scan", &_wifi_scan_command },
  { "wifi_send_raw_data", &_wifi_send_raw_data_command },
  { "wifi_set_advanced_client_config", &_wifi_set_advanced_client_config_command },
  { "wifi_set_advanced_scan_configuration", &_wifi_set_advanced_scan_configuration_command },
  { "wifi_set_antenna", &_wifi_set_antenna_command },
  { "wifi_set_ap_configuration", &_wifi_set_ap_configuration_command },
  { "wifi_set_channel", &_wifi_set_channel_command },
  { "wifi_set_default_interface", &_wifi_set_default_interface_command },
  { "wifi_set_performance_profile", &_wifi_set_performance_profile_command },
  { "wifi_set_roam_configuration", &_wifi_set_roam_configuration_command },
  { "wifi_set_transmit_rate", &_wifi_set_transmit_rate_command },
  { "wifi_set_tx_power", &_wifi_set_tx_power_command },
  { "wifi_start_ap", &_wifi_start_ap_command },
  { "wifi_start_statistic_report", &_wifi_start_statistic_report_command },
  { "wifi_start_wps", &_wifi_start_wps_command },
  { "wifi_stop_scan", &_wifi_stop_scan_command },
  { "wifi_stop_statistic_report", &_wifi_stop_statistic_report_command },
  { "wifi_test_client_configuration", &_wifi_test_client_configuration_command },
  { "wifi_transmit_test_start", &_wifi_transmit_test_start_command },
  { "wifi_ax_transmit_test_start", &_wifi_ax_transmit_test_start_command },
  { "wifi_transmit_test_stop", &_wifi_transmit_test_stop_command },
  { "wifi_update_gain_table", &_wifi_update_gain_table_command },
  { "si91x_calibration_write", &_si91x_calibration_write_command },
  { "si91x_calibration_read", &_si91x_calibration_read_command },
  { "si91x_frequency_offset", &_si91x_frequency_offset_command }, ) };
