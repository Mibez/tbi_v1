/**
* @file     channel.c
* @brief    Socket based channel interface for sending telemetry
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>

#include "channel.h"
#include "protocol.h"
#include "utils.h"

#define TBI_DEFAULT_SERVER_ADDRESS "127.0.0.1"
#define TBI_CHANNEL_MTU 1500U
#define TBI_DEFAULT_PORT 8000U
#define TBI_MAX_CLIENTS 1U

/** @brief Connect to server (blocking)
 * 
 * @param[in]  tbi     TBI context
 * 
 * @return 0 on success, or a negative error value
 */
int tbi_client_channel_open(tbi_ctx_t* tbi)
{
    struct sockaddr_in address;
    int ret, len;

    /* Allocate new channel context */
	tbi->channel = (tbi_channel_t*)malloc(sizeof(tbi_channel_t));
    if(!tbi->channel)
        goto exit;

    /* Allocate buffer for stored data */
    tbi->channel->buf = (uint8_t*)malloc(TBI_CHANNEL_MTU * sizeof(uint8_t));
    if(!tbi->channel->buf)
        goto exit_channel_allocated;

    /* Create socket */
    tbi->channel->server = false;
    tbi->channel->connected = false;
    if((tbi->channel->conn_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        goto exit_buf_allocated;

    /* Set up server address */
    memset(&address, '0', sizeof(address));
    address.sin_family = AF_INET;
    address.sin_port = htons(TBI_DEFAULT_PORT);

    if(inet_pton(AF_INET, TBI_DEFAULT_SERVER_ADDRESS, &address.sin_addr) <= 0) {
        printf("Invalid server address!\n");
        goto exit_buf_allocated;
    }

    /* Connect to server */
    if((ret = connect(tbi->channel->conn_fd, (struct sockaddr*)&address, sizeof(address))) < 0) {
        perror("Unable to connect to server");
        goto exit_buf_allocated;
    }

    /* Create timestamp that will be shared with server. All future telemetry
        msgs should have timestamps relative to this */
    tbi->channel->start_ts = get_current_time_ms();
    tbi->channel->connected = true;

    /* Form client handshake message */
    len = tbi_protocol_client_handshake(tbi->channel->buf, tbi->msgspec_version, 
        msgspec_checksum(tbi), tbi->channel->start_ts);
    if(len <= 0)
        goto exit_socket_opened;

    /* Send handshake */
    if((ret = write(tbi->channel->conn_fd, tbi->channel->buf, len)) < len) {
        if(ret < 0)
            perror("Error writing to socket");
        goto exit_socket_opened;
    }

    /* Verify server handshake */
    len = read(tbi->channel->conn_fd, tbi->channel->buf, TBI_CHANNEL_MTU);
    if(len < 0) {
        perror("Error reading from socket");
        goto exit_socket_opened;
    }

    if(tbi_protocol_client_verify_handshake_ack(tbi->channel->buf, len) != 0) {
        printf("Invalid handshake from server of length %d bytes!\n", len);
        goto exit_socket_opened;
    }

    return 0;

exit_socket_opened:
    close(tbi->channel->conn_fd);
exit_buf_allocated:
    free(tbi->channel->buf);
exit_channel_allocated:
    free(tbi->channel);
exit:    
    tbi->channel = NULL;
    return -1;
}

/** @brief Send RTM message to server
 * 
 * @param[in]  tbi     TBI context
 * @param[in]  flags   Message flags
 * @param[in]  msgtype Message type
 * @param[in]  buf     Buffer containing the message
 * @param[in]  buf_len Buffer length
 * 
 * @return 0 on success, or a negative error value
 */
int tbi_client_channel_send_rtm(tbi_ctx_t* tbi, uint8_t flags, uint8_t msgtype, uint8_t* buf, int buf_len)
{
    int ret;

    if(!tbi || !tbi->channel || !tbi->channel->connected)
        return -1;

    /* Set flags to first byte (RTM/DCB) */
    if((ret = tbi_set_client_flags(buf, TBI_FLAGS_RTM)) != 0)
        return -1;

    /* Debug */
    printf("Channel sending RTM: ");
    for(int i = 0; i < buf_len; i++) { printf("0x%X ", buf[i]); }
    printf("\n");

    /* Send it */
    if((ret = write(tbi->channel->conn_fd, buf, buf_len)) < buf_len) {
        if(ret < 0)
            perror("Error writing to socket");
        return -1;
    }
    printf("...Sent!\n");

    return 0;
}

int tbi_client_channel_send_dcb(tbi_ctx_t* tbi)
{
    printf("DUMMY channel send DCB!\n");
    return 0;
}

/** @brief Close connection, free resources */
void tbi_client_channel_close(tbi_ctx_t* tbi)
{
    if(tbi->channel) {
        /* Close connection */
        if(tbi->channel->connected)
            close(tbi->channel->conn_fd);

        /* Free memory */
        if(tbi->channel->buf)
            free(tbi->channel->buf);
        free(tbi->channel);
        tbi->channel = NULL;
    }
}

/** @brief Open socket and wait for a client to connect (blocking)
 * 
 * @param[in]  tbi     TBI context
 * 
 * @return 0 on success, or a negative error value
 */
int tbi_server_channel_open(tbi_ctx_t* tbi)
{
    struct sockaddr_in address;
    int ret, len;

    /* Allocate new channel context */
	tbi->channel = (tbi_channel_t*)malloc(sizeof(tbi_channel_t));
    if(!tbi->channel)
        goto exit;

    /* Allocate buffer for stored data */
    tbi->channel->buf = (uint8_t*)malloc(TBI_CHANNEL_MTU * sizeof(uint8_t));
    if(!tbi->channel->buf)
        goto exit_channel_allocated;

    /* Set metadata and create a listening socket */
    tbi->channel->server = true;
    tbi->channel->connected = false;
    tbi->channel->listen_fd = socket(AF_INET, SOCK_STREAM, 0);

    if(tbi->channel->listen_fd < 0)
        goto exit_buf_allocated;

	memset(&address, '0', sizeof(address));
	memset(tbi->channel->buf, 0, sizeof(TBI_CHANNEL_MTU * sizeof(uint8_t)));

	address.sin_family = AF_INET;
	address.sin_addr.s_addr = htonl(INADDR_ANY);
	address.sin_port = htons(TBI_DEFAULT_PORT);

    if((ret = bind(tbi->channel->listen_fd, (struct sockaddr*)&address, sizeof(address))) != 0) {
        perror("Error binding server socket");
        goto exit_buf_allocated;
    }

    if((ret = listen(tbi->channel->listen_fd, TBI_MAX_CLIENTS)) != 0) {
        perror("Error in socket listen()");
        goto exit_buf_allocated;
    }

    tbi->channel->conn_fd = accept(tbi->channel->listen_fd, (struct sockaddr*)NULL, NULL);
    if(tbi->channel->conn_fd < 0) {
        perror("Error in server accept");
        goto exit_listen_socket_opened;
    }

    printf("Client connected!\n");
    tbi->channel->connected = true;

    /* Receive client handshake */
    len = read(tbi->channel->conn_fd, tbi->channel->buf, TBI_CHANNEL_MTU);
    if(len < 0) {
        perror("Error reading from socket");
        goto exit_client_connected;
    }
    
    /* Verify client handshake, and form server handshake */
    len = tbi_protocol_server_handshake(
        tbi->channel->buf, len,
        tbi->msgspec_version,
        msgspec_checksum(tbi),
        &tbi->channel->start_ts
    );
    if(len <= 0) {
        printf("Invalid client handshake!\n");
        goto exit_client_connected;
    }

   /* Send handshake */
    if((ret = write(tbi->channel->conn_fd, tbi->channel->buf, len)) < len) {
        if(ret < 0)
            perror("Error writing to socket");
        goto exit_client_connected;
    }

    return 0;

exit_client_connected:
    close(tbi->channel->conn_fd);
exit_listen_socket_opened:
    close(tbi->channel->listen_fd);
exit_buf_allocated:
    free(tbi->channel->buf);
exit_channel_allocated:
    free(tbi->channel);
exit:    
    tbi->channel = NULL;
    return -1;
}

/** @brief Receive a message from client (blocking)
 * 
 * @param[in]  tbi     TBI context
 * 
 * @return Number of bytes received
 */
int tbi_server_channel_recv(tbi_ctx_t* tbi)
{
    int len;

    /* Receive message from client */
    printf("Server receiving...\n");
    len = read(tbi->channel->conn_fd, tbi->channel->buf, TBI_CHANNEL_MTU);
    if(len < 0) {
        perror("Error reading from socket");
    }
    
    /* Debug */
    printf("Received %d bytes: ", len);
    for(int i = 0; i < len; i++) { printf("0x%X ", tbi->channel->buf[i]);}
    printf("\n");

    return len;
}

/** @brief Close connection, free up resources */
void tbi_server_channel_close(tbi_ctx_t* tbi)
{
    if(tbi->channel) {
        /* Close connection */
        if(tbi->channel->connected) {
            close(tbi->channel->conn_fd);
            close(tbi->channel->listen_fd);
        }

        /* Free memory */
        if(tbi->channel->buf)
            free(tbi->channel->buf);
        free(tbi->channel);
        tbi->channel = NULL;
    }
}