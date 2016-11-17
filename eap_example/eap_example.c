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


void eap_example_save_last_data(struct instance_data *self,
		const u8 *data, size_t data_len) {
	self->last_data = wpabuf_alloc_copy(data, data_len);
}


void eap_example_peer_tx(struct instance_data *self,
		const u8 *data, size_t data_len) {
	/* eve_peer tx, alice_server rx */
	if (self->name == EVE_PEER) {
		eap_example_save_last_data(&alice_server, data, data_len);
		eap_example_server_rx(&alice_server, data, data_len);
	}
	/* bob_peer tx, eve_server rx */
	else if (self->name == BOB_PEER) {
		eap_example_save_last_data(&eve_server, data, data_len);
		eap_example_server_rx(&eve_server, data, data_len);
	}
}


void eap_example_server_tx(struct instance_data *self,
		const u8 *data, size_t data_len) {
	/* alice_server tx, eve_peer rx */
	if (self->name == ALICE_SERVER) {
		eap_example_save_last_data(&eve_peer, data, data_len);
		eap_example_peer_rx(&eve_peer, data, data_len);
	}
	/* eve_server rx, bob_peer tx */
	else if (self->name == EVE_SERVER) {
		eap_example_save_last_data(&bob_peer, data, data_len);
		eap_example_peer_rx(&bob_peer, data, data_len);
	}
}


void eap_example_mitm_server_tx(struct instance_data *self) {
	wpabuf_free(eve_peer.mitm_data);
	eve_peer.mitm_data = self->mitm_data;
	self->mitm_data = 0;
}


void eap_example_mitm_peer_tx(struct instance_data *self) {
	wpabuf_free(eve_server.mitm_data);
	eve_server.mitm_data = self->mitm_data;
	self->mitm_data = 0;
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

	return UNKNOWN;
}


struct instance_data * eap_example_get_instance_data(void * sm) {
	struct instance_data * self = NULL;
	if (bob_peer.eap_sm == sm) {
		self = &bob_peer;
	} else if (eve_peer.eap_sm == sm) {
		self = &eve_peer;
	} else if (eve_server.eap_sm == sm) {
		self = &eve_server;
	} else if (alice_server.eap_sm == sm) {
		self = &alice_server;
	}

	return self;
}


void eap_example_mitm_retransmit(void * sm) {
	struct instance_data * self = eap_example_get_instance_data(sm);
	self->mitm_retransmit = 1;
}


int eap_example_get_mitm_retransmit(void * sm) {
	struct instance_data * self = eap_example_get_instance_data(sm);

	return self->mitm_retransmit;
}


void eap_example_mitm_retransmit_cb(struct instance_data * self) {
	if (self->mitm_retransmit) {
		if (self->name == ALICE_SERVER || self->name == EVE_SERVER) {
			/* Prod EAP Server to reset pending_wait from
			 * the outside */
			eap_example_server_pending_cb(self);
			eap_example_server_rx(self,
					      self->last_data->buf,
					      self->last_data->size);
		}
		else if (self->name == BOB_PEER || self->name == EVE_PEER)
		{
			/* Eap Peer can reset pending_request by himself */
			eap_example_peer_rx(self,
					    self->last_data->buf,
					    self->last_data->size);
		}
		self->mitm_retransmit = 0;
	}
}


void eap_example_init(struct instance_data *self, enum instance_name name) {
	self->name = name;
	self->mitm_retransmit = 0;
	if (name == EVE_PEER || name == EVE_SERVER) {
		self->mitm_data = 0;
		self->mitm_protocol_state = 0x1;
		wpa_printf(MSG_DEBUG, "MITM: Init protocol for instance %s",
				name == EVE_PEER ? "Eve Peer" : "Eve Server");
	}
}


int eap_example_step(struct instance_data * self) {
	switch (self->name) {
	case ALICE_SERVER:
		printf("---[ alice_server ]--------------------------------\n");
		break;
	case EVE_PEER:
		printf("---[ eve_peer ]----------------------------------\n");
		break;
	case EVE_SERVER:
		printf("---[ eve_server ]--------------------------------\n");
		break;
	case BOB_PEER:
		printf("---[ bob_peer ]----------------------------------\n");
		break;
	default:
		break;
	}

	eap_example_mitm_retransmit_cb(self);

	switch (self->name) {
	case ALICE_SERVER:
	case EVE_SERVER:
		return eap_example_server_step(self);
	case EVE_PEER:
	case BOB_PEER:
		return eap_example_peer_step(self);
	default:
		return -1;
	}
}


int main(int argc, char *argv[])
{
	int res_a_s, res_b_p, res_e_s, res_e_p;
	wpa_debug_level = 0;

	eap_example_init(&bob_peer, BOB_PEER);
	eap_example_init(&eve_peer, EVE_PEER);
	eap_example_init(&eve_server, EVE_SERVER);
	eap_example_init(&alice_server, ALICE_SERVER);

	if (eap_example_peer_init(&bob_peer) < 0 ||
	    eap_example_server_init(&alice_server) < 0 ||
	    eap_example_peer_init(&eve_peer) < 0 ||
	    eap_example_server_init(&eve_server) < 0)
		return -1;

	int wait_count = 100;

	do {
		res_a_s = eap_example_step(&alice_server);
		res_e_p = eap_example_step(&eve_peer);
		res_e_s = eap_example_step(&eve_server);
		res_b_p = eap_example_step(&bob_peer);
	} while (res_a_s || res_b_p || res_e_p || res_e_s || --wait_count > 0);

	eap_example_peer_deinit(&bob_peer);
	eap_example_server_deinit(&alice_server);
	eap_example_peer_deinit(&eve_peer);
	eap_example_server_deinit(&eve_server);

	return 0;
}
