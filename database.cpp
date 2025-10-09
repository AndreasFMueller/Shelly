/*
 * database.cpp
 *
 * (c) 2025 Prof Dr Andreas Müller
 */
#include "database.h"
#include "format.h"
#include "debug.h"
#include "common.h"

namespace shelly {

/**
 * \brief Retrieving the sensor id
 *
 * \param station	the station name
 * \param sensor	the sensor name
 */
int	database::sensorid(const std::string& station,
		const std::string& sensor) {
	int	rc = -1;
	MYSQL_BIND	bind[2];
	MYSQL_BIND	result[1];
	int	resultid = -1;
	std::string	error;

	debug(LOG_DEBUG, DEBUG_LOG, 0, "retrieving sensor id for %s/%s",
		station.c_str(), sensor.c_str());

	MYSQL_STMT	*stmt = mysql_stmt_init(mysql);
	if (NULL == stmt) {
		error = stringprintf("cannot create statement: %s",
			mysql_error(mysql));
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", error.c_str());
		throw shellyexception(error);
	}

	// parse/prepare the query
	std::string	query(
		"select b.id "
		"from station a, sensor b "
		"where a.id = b.stationid"
		"  and a.name = ? "
		"  and b.name = ? ");
	debug(LOG_DEBUG, DEBUG_LOG, 0, "query to retrieve station id: '%s'",
		query.c_str());

	if (mysql_stmt_prepare(stmt, query.c_str(), query.size())) {
		error = stringprintf( "cannot preparte query "
			"'%s': %s", query.c_str(), mysql_stmt_error(stmt));;
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", error.c_str());
		goto cleanup;
	}
	
	// bind the parameters
	memset(bind, 0, sizeof(bind));

	bind[0].buffer_type = MYSQL_TYPE_STRING;
	bind[0].buffer = (void *)station.c_str();
	bind[0].buffer_length = station.size();
	bind[0].is_null = 0;
	bind[0].length = 0;

	bind[1].buffer_type = MYSQL_TYPE_STRING;
	bind[1].buffer = (void *)sensor.c_str();
	bind[1].buffer_length = sensor.size();
	bind[1].is_null = 0;
	bind[1].length = 0;

	if (mysql_stmt_bind_param(stmt, bind)) {
		error = stringprintf( "cannot bind: %s",
			mysql_stmt_error(stmt));
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", error.c_str());
		goto cleanup;
	}

	// execute the query
	if (mysql_stmt_execute(stmt)) {
		error = stringprintf( "search query failed: %s",
			mysql_stmt_error(stmt));
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", error.c_str());
		goto cleanup;
	}

	// bind the result variable
	memset(result, 0, sizeof(result));
	result[0].buffer_type = MYSQL_TYPE_LONG;
	result[0].buffer = &resultid;
	result[0].is_null = 0;
	result[0].length = 0;
	if (mysql_stmt_bind_result(stmt, result)) {
		error = stringprintf( "bind result failed: %s",
			mysql_stmt_error(stmt));
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", error.c_str());
		goto cleanup;
	}
	
	// store the result
	if (mysql_stmt_store_result(stmt)) {
		error = std::string("cannot store the result: %s",
			mysql_stmt_error(stmt));
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", error.c_str());
		goto cleanup;
	}

	// make sure we have exactly one returned row
	if (1 != mysql_stmt_num_rows(stmt)) {
		error = std::string("query did not return a row");
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", error.c_str());
		goto cleanup;
	}

	// retrieve that row
	if (mysql_stmt_fetch(stmt)) {
		error = stringprintf("could not fetch a row: %s",
			mysql_stmt_error(stmt));
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", error.c_str());
		goto cleanup;
	}

	// retrieve the result id from the row, use it as return value
	rc = resultid;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "sensor id is %d", resultid);

cleanup:
	mysql_stmt_close(stmt);
	if (rc < 0) {
		throw shellyexception(error);
	}
	return rc;
}

/**
 * \brief Get the id of a field
 *
 * \param fieldname	the name of the field
 */
int	database::fieldid(const std::string& fieldname) {
	int	rc = -1;
	MYSQL_BIND	bind[1];
	MYSQL_BIND	result[1];
	int	resultid = -1;
	std::string	error;

	MYSQL_STMT	*stmt = mysql_stmt_init(mysql);
	if (NULL == stmt) {
		error = stringprintf( "cannot create statement: %s",
			mysql_error(mysql));
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", error.c_str());
		throw shellyexception(error);
	}

	// parse/prepare the query
	std::string	query("select a.id from mfield a where a.name = ?");
	debug(LOG_DEBUG, DEBUG_LOG, 0, "query to retrieve field name: '%s'",
		query.c_str());

	if (mysql_stmt_prepare(stmt, query.c_str(), query.size())) {
		error = stringprintf("cannot preparte query '%s': %s",
			query.c_str(), mysql_stmt_error(stmt));
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", error.c_str());
		goto cleanup;
	}
	
	// bind the parameter
	memset(bind, 0, sizeof(bind));

	bind[0].buffer_type = MYSQL_TYPE_STRING;
	bind[0].buffer = (void *)fieldname.c_str();
	bind[0].buffer_length = fieldname.size();
	bind[0].is_null = 0;
	bind[0].length = 0;

	if (mysql_stmt_bind_param(stmt, bind)) {
		error = stringprintf("cannot bind: %s",
			mysql_stmt_error(stmt));
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", error.c_str());
		goto cleanup;
	}

	// execute the query
	if (mysql_stmt_execute(stmt)) {
		error = stringprintf("search query failed: %s",
			mysql_stmt_error(stmt));
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", error.c_str());
		goto cleanup;
	}

	// store the result
	if (mysql_stmt_store_result(stmt)) {
		error = stringprintf("cannot store the result: %s",
			mysql_stmt_error(stmt));
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", error.c_str());
		goto cleanup;
	}

	// count the number of rows
	if (1 != mysql_stmt_num_rows(stmt)) {
		error = std::string("query did not return a row");
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", error.c_str());
		goto cleanup;
	}

	// bind the result
	memset(result, 0, sizeof(result));
	result[0].buffer_type = MYSQL_TYPE_LONG;
	result[0].buffer = &resultid;
	result[0].is_null = 0;
	result[0].length = 0;
	if (mysql_stmt_bind_result(stmt, result)) {
		error = stringprintf("cannot bind the result: %s",
			mysql_stmt_error(stmt));
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", error.c_str());
		goto cleanup;
	}

	// fetch the row
	if (mysql_stmt_fetch(stmt)) {
		error = stringprintf("cannot fetch the row: %s",
			mysql_stmt_error(stmt));
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", error.c_str());
		goto cleanup;
	}

	rc = resultid;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "field id is %d", resultid);

cleanup:
	mysql_stmt_close(stmt);
	if (rc < 0) {
		throw shellyexception(error);
	}
	return rc;
}

/**
 * \brief construct a database connection
 *
 * \param config	the configuration to use
 */
database::database(configuration_ptr config) : _config(config), mysql(NULL) {
	// initialize mysql
	mysql = mysql_init(mysql);
	if (NULL == mysql) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot create mysql");
	}

	// connect to the database
	std::string	hostname = _config->stringvalue("database.hostname");
	std::string	username = _config->stringvalue("database.username");
	std::string	password = _config->stringvalue("database.password");
	std::string	dbname = _config->stringvalue("database.dbname");
	int	port = _config->intvalue("database.port");
	if (NULL == mysql_real_connect(mysql, hostname.c_str(),
		username.c_str(), password.c_str(), dbname.c_str(),
		port, NULL, 0)) {
		debug(LOG_ERR, DEBUG_LOG, 0,
			"cannot connect to the database: %s",
			mysql_error(mysql));
	}

	// read the field ids
	temperature_id = fieldid("temperature");
	humidity_id = fieldid("humidity");
	capacity_id = fieldid("capacity");
	battery_id = fieldid("battery");
	debug(LOG_DEBUG, DEBUG_LOG, 0, "database connection establisched, "
		"temperature_id = %d, humidity_id = %d, capacity_id = %d, "
		"battery_id  = %d",
		temperature_id, humidity_id, capacity_id, battery_id);
}

/**
 * \brief Close the database connection
 */
database::~database() {
	mysql_close(mysql);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "database connection closed");
}

/**
 * \brief Add data for a given sensor
 *
 * \param station		station name
 * \param sensor		sensor name
 * \param temperature		temperature in degrees Celsius
 * \param humidity		relative humidity in percent
 * \param battery		battery voltage
 * \param capacity		battery capacity in percent
 */
void	database::add(const std::string& station, const std::string& sensor,
		time_t timekey,
		float temperature, float humidity, float battery,
		float capacity) {
	std::string	error;
	MYSQL_STMT	*stmt;

	int	sid = sensorid(station, sensor);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "found sensor id %s/%s -> %d",
		station.c_str(), sensor.c_str(), sid);

	// prepare the statement
	if (NULL == (stmt = mysql_stmt_init(mysql))) {
		error = stringprintf("cannot create a statement: %s",
			mysql_error(mysql));
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", error.c_str());
		throw shellyexception(error);
	}
	std::string	query(	"insert into "
				"sdata(timekey, sensorid, fieldid, value) "
				"values (?, ?, ?, ?)");
	if (mysql_stmt_prepare(stmt, query.c_str(), query.size())) {
		error = stringprintf("cannot parse '%s': %s",
			query.c_str(), mysql_stmt_error(stmt));
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", error.c_str());
		goto cleanup;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "insert query prepared: %s",
		query.c_str());

	// setup binding
	MYSQL_BIND	bind[4];
	memset(bind, 0, sizeof(bind));

	bind[0].buffer_type = MYSQL_TYPE_LONG;
	bind[0].buffer = &timekey;
	bind[0].is_null = 0;
	bind[0].length = 0;

	bind[1].buffer_type = MYSQL_TYPE_LONG;
	bind[1].buffer = &sid;
	bind[1].is_null = 0;
	bind[1].length = 0;

	bind[2].buffer_type = MYSQL_TYPE_LONG;
	bind[2].is_null = 0;
	bind[2].length = 0;

	bind[3].buffer_type = MYSQL_TYPE_FLOAT;
	bind[3].is_null = 0;
	bind[3].length = 0;

	// bind timekey, sid, temperature_id, temperature
	bind[2].buffer = &temperature_id;
	bind[3].buffer = &temperature;
	if (mysql_stmt_bind_param(stmt, bind)) {
		error = stringprintf("cannot bind for temperature: %s",
			mysql_stmt_error(stmt));
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", error.c_str());
		goto cleanup;
	}

	// exectute the query
	if (mysql_stmt_execute(stmt)) {
		error = stringprintf("cannot add temperature: %s",
			mysql_stmt_error(stmt));
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", error.c_str());
		goto cleanup;
	}

	// bind timekey, sid, humidity_id, humidity
	bind[2].buffer = &humidity_id;
	bind[3].buffer = &humidity;
	if (mysql_stmt_bind_param(stmt, bind)) {
		error = stringprintf("cannot bind for humidity: %s",
			mysql_stmt_error(stmt));
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", error.c_str());
		goto cleanup;
	}
	if (mysql_stmt_execute(stmt)) {
		error = stringprintf("cannot add temperature: %s",
			mysql_stmt_error(stmt));
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", error.c_str());
		goto cleanup;
	}

	// bind timekey, sid, battery_id, battery
	bind[2].buffer = &battery_id;
	bind[3].buffer = &battery;
	if (mysql_stmt_bind_param(stmt, bind)) {
		error = stringprintf("cannot bind for battery: %s",
			mysql_stmt_error(stmt));
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", error.c_str());
		goto cleanup;
	}
	if (mysql_stmt_execute(stmt)) {
		error = stringprintf("cannot add battery: %s",
			mysql_stmt_error(stmt));
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", error.c_str());
		goto cleanup;
	}

	// bind timekey, sid, capacity_id, capacity
	bind[2].buffer = &capacity_id;
	bind[3].buffer = &capacity;
	if (mysql_stmt_bind_param(stmt, bind)) {
		error = stringprintf("cannot bind for capacity: %s",
			mysql_stmt_error(stmt));
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", error.c_str());
		goto cleanup;
	}
	if (mysql_stmt_execute(stmt)) {
		error = stringprintf("cannot add capacity: %s",
			mysql_stmt_error(stmt));
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", error.c_str());
		goto cleanup;
	}

	// close the statement
cleanup:
	mysql_stmt_close(stmt);
	if (error.size() > 0) {
		throw shellyexception(error);
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "statement closed");
}

} // namespace shelly
