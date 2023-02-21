// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef FMS_PLATFORM_IRADAPTER_H
#define FMS_PLATFORM_IRADAPTER_H

#include <memory>
#include <stdint.h>
#include <vector>

#include <core/types.h>

namespace fms {
namespace platform {

class IRAdapterDescriptor;

struct IRAdapterPrivate;

/**
 * \brief Interface to an IR adapter
 *
 * Implementations of this class should ensure that the
 * data being transfered will be sent over IrDA SIR.
 */
class IRAdapter final {
public:
	IRAdapter(const IRAdapterDescriptor &descriptor);
	IRAdapter(IRAdapter &&other);
	virtual ~IRAdapter();
	/**
	 * \brief Reads data from the IRAdapter into the outbuffer.
	 * \param[in] outBufStart Start iterator of the output buffer
	 * \param[in] outBufEnd End iterator of the output buffer
	 * \param[out] readLength The amount of bytes read
	 * \return The last written position of the iterator
	 */
	template<class It>
	It read(It outBufStart, It outBufEnd, std::size_t &readLength);

	/**
	 * \brief Sends data from to the IRAdapter
	 * \param[in] inBufStart Start iterator of the input buffer
	 * \param[in] inBufEnd End iterator of the input buffer
	 */
	template<class It>
	void send(It inBufStart, It inBufEnd);
private:
	std::unique_ptr<IRAdapterPrivate> d_ptr;
};

} // NS platform
} // NS fms

#ifdef __3DS__
#include "ctru/iradapter.inl"
#endif

#ifdef __linux__
#include "linux/iradapter.inl"
#endif

#endif // FMS_PLATFORM_IRADAPTER_H
