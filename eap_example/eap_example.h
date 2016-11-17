#ifndef _EAP_EXAMPLE_H_
#define _EAP_EXAMPLE_H_

#include "includes.h"

#include "common.h"
#include "wpabuf.h"

struct instance_data {
	void *eap_ctx;
	void *eap_cb;
	void *eap_conf;
	void *eap_sm;
	enum instance_name {ALICE_SERVER, BOB_PEER, EVE_SERVER, EVE_PEER,
			    UNKNOWN} name;
	struct wpabuf *last_data;
	int mitm_retransmit;
};


void eap_example_peer_tx(struct instance_data *self,
		const u8 *data, size_t data_len);
void eap_example_server_tx(struct instance_data *self,
		const u8 *data, size_t data_len);
enum instance_name eap_example_get_instance_name(void *sm);
void forge_mschapv2_challenge_from_alice_server_to_bob_peer (
		const u8 *data, size_t data_len);
void eap_example_mitm_retransmit(void *sm);
int  eap_example_get_mitm_retransmit(void *sm);

#endif // _EAP_EXAMPLE_H_
