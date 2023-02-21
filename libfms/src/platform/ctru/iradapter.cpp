// SPDX-License-Identifier: GPL-3.0-or-later
#include "../iradapter.h"

#include <3ds.h>

#include <core/types.h>

namespace fms {
namespace platform {


/**
 * \internal
 * \brief Gets the event which is called when data is received over IR.
 * \param[out] handle A pointer to the handle of the event.
 * \returns A result describing if this call was successful.
 * \movetolibctru
 */
Result getRecvFinishedEvent(Handle* handle) {
	Result ret = 0;

	u32 *cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0xE,0,0); // 0xE0000

	if(R_FAILED(ret = svcSendSyncRequest(iruGetServHandle())))return ret;
	ret = (Result)cmdbuf[1];
	*handle = (Handle) cmdbuf[3];

	return ret;
}


IRAdapter::IRAdapter(const IRAdapterDescriptor & /*adapter*/)
	: d_ptr(new IRAdapterPrivate()) {
	Result ret = iruInit(d_ptr->buffer->data(), d_ptr->buffer->size());
	if (R_FAILED(ret)) {
		// TODO: proper error handling
		throw std::exception();
	}

	ret = getRecvFinishedEvent(&d_ptr->receiveEvent);
	if (R_FAILED(ret)) {
		throw std::exception();
	}
}

IRAdapter::IRAdapter(IRAdapter &&other)
	: d_ptr(std::move(other.d_ptr)) {}

IRAdapter::~IRAdapter() {
	IRU_Shutdown();
}

} // NS platform
} // NS fms
