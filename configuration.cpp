/*
 * configurtion.cpp
 *
 * (c) 2025 Prof Dr Andreas Müller
 */
#include "configuration.h"
#include "debug.h"
#include "common.h"
#include <iostream>
#include <fstream>

namespace shelly {

/**
 * \brief read configuration from a file
 *
 * \param filename	name of the configuration file
 */
configuration::configuration(const std::string& filename) {
	std::ifstream	ifs(filename);
	data = nlohmann::json::parse(ifs);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "configuration data: %s",
		data.dump(4).c_str());
}

/**
 * \brief split the path
 *
 * \param path		the path to split at periods
 */
std::list<std::string>	configuration::splitpath(const std::string& path) {
	std::string	p = path;
	std::list<std::string>	result;
	size_t	i;
	while (std::string::npos != (i = p.find_first_of("."))) {
		std::string	c = p.substr(0, i);
		result.push_back(c);
		p = p.substr(i + 1);
	}
	result.push_back(p);
	return result;
}

/**
 * \brief get a configuration falue
 *
 * \param path		json path to the configuration data
 */
std::string	configuration::stringvalue(const std::string& path) const {
	std::list<std::string>	pc = splitpath(path);
	if (pc.size() == 1) {
		std::string	result = data[pc.front()];
		return result;
	}
	nlohmann::json	d = data[pc.front()];
	pc.pop_front();
	while (pc.size() > 1) {
		std::string	c = pc.front();
		d = d[c];
		pc.pop_front();
	}
	std::string	result = d[pc.front()];
	return result;
}

/**
 * \brief retrieve an integer value from the configuration
 *
 * \param path		json path to the value
 */
int	configuration::intvalue(const std::string& path) const {
	std::list<std::string>	pc = splitpath(path);
	nlohmann::json	d = data;
	while (pc.size() > 1) {
		std::string	c = pc.front();
		d = d[c];
		pc.pop_front();
	}
	int	result = d[pc.front()];
	return result;
}

/**
 * \brief Retrieve list of device ids from 
 */
std::list<std::string>	configuration::idlist() const {
	std::list<std::string>	result;
	nlohmann::json	devices = data["devices"];
	for (auto device : devices) {
		result.push_back(device["id"]);
	}
	return result;
}

/**
 * \brief Retrieve a json structure for a device by id
 *
 * \param id	the id of the device
 */
nlohmann::json	configuration::device(const std::string& id) const {
	nlohmann::json	devices = data["devices"];
	for (auto device : devices) {
		if (device["id"] == id)
			return device;
	}
	throw shellyexception("device not found");
}

/**
 * \brief Find out whether a path is present
 *
 * \param path		the path to check
 */
bool	configuration::has(const std::string& path) const {
	std::list<std::string>	pc = splitpath(path);
	nlohmann::json	d = data;
	while (pc.size() > 0) {
		std::string	key = pc.front();
		if (!d.contains(key))
			return false;
		d = d[key];
		pc.pop_front();
	}
	return true;
}

} // namespace shelly
