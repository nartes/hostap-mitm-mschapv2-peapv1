/*
 * Example application showing how EAP peer and server code from
 * wpa_supplicant/hostapd can be used as a library. This example program
 * initializes both an EAP server and an EAP peer entities and then runs
 * through an EAP-PEAP/MSCHAPv2 authentication.
 * Copyright (c) 2007, Jouni Malinen <j@w1.fi>
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 */

#include "eap_example.h"
#include "eap_example_peer.h"
#include "eap_example_server.h"

#include "includes.h"

#include "common.h"
#include "wpabuf.h"


static struct instance_data peer;
static struct instance_data server;


int eap_example_peer_init(struct instance_data *self);
void eap_example_peer_deinit(struct instance_data *self);
int eap_example_peer_step(struct instance_data *self);


int eap_example_server_init(struct instance_data *self);
void eap_example_server_deinit(struct instance_data *self);
int eap_example_server_step(struct instance_data *self);


void eap_example_peer_tx(const u8 *data, size_t data_len) {
	eap_example_server_rx(&server, data, data_len);
}


void eap_example_server_tx(const u8 *data, size_t data_len) {
	eap_example_peer_rx(&peer, data, data_len);
}


int main(int argc, char *argv[])
{
	int res_s, res_p;

	wpa_debug_level = 0;

	if (eap_example_peer_init(&peer) < 0 ||
	    eap_example_server_init(&server) < 0)
		return -1;

	do {
		printf("---[ server ]--------------------------------\n");
		res_s = eap_example_server_step(&server);
		printf("---[ peer ]----------------------------------\n");
		res_p = eap_example_peer_step(&peer);
	} while (res_s || res_p);

	eap_example_peer_deinit(&peer);
	eap_example_server_deinit(&server);

	return 0;
}
