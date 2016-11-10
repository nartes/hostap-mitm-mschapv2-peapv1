#ifndef _EAP_EXAMPLE_H_
#define _EAP_EXAMPLE_H_

#include "includes.h"

#include "common.h"
#include "wpabuf.h"

struct instance_data {
	void *eap_ctx;
	void *eap_cb;
	void *eap_conf;
};


void eap_example_peer_tx(struct instance_data *self,
		const u8 *data, size_t data_len);
void eap_example_server_tx(struct instance_data *self,
		const u8 *data, size_t data_len);

#endif // _EAP_EXAMPLE_H_
