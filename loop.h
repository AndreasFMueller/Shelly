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

namespace shelly {

class loop {
	std::string	request;
	std::string	response;
public:
	loop();
	~loop();
	void	run();
	size_t	read_callback(char *data, size_t size, size_t nitems);
	size_t	write_callback(char *data, size_t size, size_t nitems);
	bool	sendrequest(const std::list<std::string>& ids);
	bool	process(const nlohmann::json& response);
};

} // namespace shelly

#endif /* _loop_h */
