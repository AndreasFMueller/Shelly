/*
 * common.h
 *
 * (c) 2025 Prof Dr Andreas Müller
 */
#include <stdexcept>

namespace shelly {

class shellyexception : public std::runtime_error {
public:
	shellyexception(const std::string& w) : std::runtime_error(w) { }
};

} // namespace shelly
