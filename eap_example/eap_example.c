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


static struct instance_data bob_peer;
static struct instance_data eve_peer;
static struct instance_data eve_server;
static struct instance_data alice_server;


int eap_example_peer_init(struct instance_data *self);
void eap_example_peer_deinit(struct instance_data *self);
int eap_example_peer_step(struct instance_data *self);


int eap_example_server_init(struct instance_data *self);
void eap_example_server_deinit(struct instance_data *self);
int eap_example_server_step(struct instance_data *self);


void eap_example_peer_tx(struct instance_data *self,
		const u8 *data, size_t data_len) {
	/* eve_peer tx, alice_server rx */
	if (self->name == EVE_PEER) {
		eap_example_server_rx(&alice_server, data, data_len);
	}
	/* bob_peer tx, eve_server rx */
	else if (self->name == BOB_PEER) {
		eap_example_server_rx(&eve_server, data, data_len);
	}
}


void eap_example_server_tx(struct instance_data *self,
		const u8 *data, size_t data_len) {
	/* alice_server tx, eve_peer rx */
	if (self->name == ALICE_SERVER) {
		eap_example_peer_rx(&eve_peer, data, data_len);
	}
	/* eve_server rx, bob_peer tx */
	else if (self->name == EVE_SERVER) {
		eap_example_peer_rx(&bob_peer, data, data_len);
	}
}

enum instance_name eap_example_get_instance_name(void *sm) {
	if (bob_peer.eap_sm == sm) {
		return BOB_PEER;
	} else if (eve_peer.eap_sm == sm) {
		return EVE_PEER;
	} else if (eve_server.eap_sm == sm) {
		return EVE_SERVER;
	} else if (alice_server.eap_sm == sm) {
		return ALICE_SERVER;
	}
}


int main(int argc, char *argv[])
{
	/* Specify names for instances */
	bob_peer.name = BOB_PEER;
	eve_peer.name = EVE_PEER;
	eve_server.name = EVE_SERVER;
	alice_server.name = ALICE_SERVER;

	int res_a_s, res_b_p, res_e_s, res_e_p;

	wpa_debug_level = 0;

	if (eap_example_peer_init(&bob_peer) < 0 ||
	    eap_example_server_init(&alice_server) < 0 ||
	    eap_example_peer_init(&eve_peer) < 0 ||
	    eap_example_server_init(&eve_server) < 0)
		return -1;

	do {
		printf("---[ alice_server ]--------------------------------\n");
		res_a_s = eap_example_server_step(&alice_server);
		printf("---[ eve_peer ]----------------------------------\n");
		res_e_p = eap_example_peer_step(&eve_peer);
		printf("---[ eve_server ]--------------------------------\n");
		res_e_s = eap_example_server_step(&eve_server);
		printf("---[ bob_peer ]----------------------------------\n");
		res_b_p = eap_example_peer_step(&bob_peer);
	} while (res_a_s || res_b_p || res_e_p || res_e_s);

	eap_example_peer_deinit(&bob_peer);
	eap_example_server_deinit(&alice_server);
	eap_example_peer_deinit(&eve_peer);
	eap_example_server_deinit(&eve_server);

	return 0;
}
