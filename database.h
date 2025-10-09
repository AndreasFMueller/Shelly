/*
 * database.h
 *
 * (c) 2025 Prof Dr Andreas Müller
 */
#ifndef _database_h
#define _database_h

#include <mysql.h>
#include "configuration.h"

namespace shelly {

class database {
	configuration_ptr	_config;
	MYSQL	*mysql;
	int	temperature_id;
	int	humidity_id;
	int	capacity_id;
	int	battery_id;
	int	sensorid(const std::string& station, const std::string& sensor);
	int	fieldid(const std::string& fieldname);
public:
	database(configuration_ptr config);
	~database();
	void	add(const std::string& station, const std::string& sensor,
			time_t timekey,
			float temperature, float humidity, float voltage,
			float capacity);
};

} // namespace shelly

#endif /* _database_h */
