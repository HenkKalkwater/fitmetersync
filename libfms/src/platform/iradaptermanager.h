// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef FMS_PLATFORM_IRADAPTERMANAGER_H
#define FMS_PLATFORM_IRADAPTERMANAGER_H

#include <memory>
#include <optional>
#include <string>
#include <vector>

#ifdef __3DS__
#include <platform/ctru/iradaptermanager.h>
#endif

#ifdef __linux__
#include <platform/linux/iradaptermanager.h>
#endif

namespace fms {
namespace platform {

class IRAdapter;
class IRAdapterManager;

/**
 * \brief Describes an IR adapter.
 *
 */
class IRAdapterDescriptor final {
	friend class IRAdapter;
	friend class IRAdapterManager;
public:
	virtual ~IRAdapterDescriptor();
	/**
	 * Human readable name of this adapter;
	 */
	std::string name() const;
private:
	IRAdapterDescriptor(IRAdapterHandle &&handle);
	IRAdapterHandle m_handle;
};

class IRAdapterManagerPrivate;

/**
 * \brief A singleton that lists IR adapters on the system.
 *
 * Get an instance of this class by IRAdapterManager::getInstance().
 * With the returned list of \link IRAdapterDescriptor IRAdapterDescriptors \endlink, an
 */
class IRAdapterManager final {
	friend class std::optional<IRAdapterManager>;
public:
	IRAdapterManager(IRAdapterManager &&other);
	virtual ~IRAdapterManager();
	/**
	 * Gets the singleton instance.
	 */
	static IRAdapterManager &getInstance();

	/**
	 * \brief Lists all IR adapters available on this system.
	 */
	std::vector<IRAdapterDescriptor> list();

private:
	IRAdapterManager();
	static IRAdapterManager *s_instance;
	std::unique_ptr<IRAdapterManagerPrivate> d_ptr;
};

}
}

#endif //FMS_PLATFORM_IRADAPTERMANAGER_H
