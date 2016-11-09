#ifndef _EAP_EXAMPLE_PEER_H_
#define _EAP_EXAMPLE_PEER_H_

#include "includes.h"

#include "common.h"
#include "wpabuf.h"

#include "eap_example.h"

void eap_example_peer_rx(struct instance_data *self,
		const u8 *data, size_t data_len);

#endif // _EAP_EXAMPLE_PEER_H_
