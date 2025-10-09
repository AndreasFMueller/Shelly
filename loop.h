/*
 * loop.h
 *
 * (c) 2025 Prof Dr Andreas Müller
 */
#ifndef _loop_h
#define _loop_h

#include <json.hpp>
#include <list>
#include <string>
#include <chrono>
#include "configuration.h"

namespace shelly {

class loop {
	configuration_ptr	_config;
	std::string	request;
	std::string	response;
public:
	loop(configuration_ptr config);
	~loop();
	void	run();
	size_t	read_callback(char *data, size_t size, size_t nitems);
	size_t	write_callback(char *data, size_t size, size_t nitems);
	void	sendrequest(const std::list<std::string>& ids);
	void	process(const nlohmann::json& response);
	std::chrono::seconds	timekey() const;
};

} // namespace shelly

#endif /* _loop_h */
