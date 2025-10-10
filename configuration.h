/*
 * configuration.h
 *
 * (c) 2025 Prof Dr Andreas Müller 
 */
#ifndef _configuration_h
#define _configuration_h

#include <string>
#include <list>
#include <memory>
#include "json.hpp"

namespace shelly {

class configuration {
	nlohmann::json	data;
	static std::list<std::string>	splitpath(const std::string& path);
public:
	configuration(const std::string& filename);
	std::string	stringvalue(const std::string& path) const;
	int	intvalue(const std::string& path) const;
	std::list<std::string>	idlist() const;
	nlohmann::json	device(const std::string& id) const;
	bool	has(const std::string& path) const;
};

typedef std::shared_ptr<configuration>	configuration_ptr;

} // namespace shelly

#endif /* _configuration_h */
