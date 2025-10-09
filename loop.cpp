/*
 * loop.cpp
 *
 * (c) 2025 Prof Dr Andreas Müller
 */
#include "loop.h"
#include "json.hpp"
#include "debug.h"
#include "database.h"
#include "format.h"
#include <chrono>
#include <thread>
#include <iostream>
#include <sstream>
#include <curl/curl.h>
#include <cstring>
#include <list>

namespace shelly {

/**
 * \brief construct the loop object
 */
loop::loop(configuration_ptr config) : _config(config),
	// this initialization makes sure the json strings are parseable
	request("{}"), response("{}") {
}

/**
 * \brief destroy the loop object
 */
loop::~loop() {
}

/**
 * \brief read data from the request string
 *
 * \param data		the data buffer to write to
 * \param size		the size of a data item
 * \param nitems	the number of items
 */
size_t	loop::read_callback(char *data, size_t size, size_t nitems) {
	// check for empty request
	if (request.size() == 0) {
		return 0;
	}

	// find out what to send
	std::string	senddata;
	size_t	l = size * nitems;
	if (l < request.size()) {
		senddata = request.substr(0, l);
		request = request.substr(l);
	} else {
		senddata = request;
		request = std::string();
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "next batch to send: '%s'",
		senddata.c_str());

	// send the data
	l = senddata.size();
	strncpy(data, senddata.c_str(), l);
	return l;
}

/**
 * \brief Callback to write received data
 *
 * \param data		the buffer to read the data from
 * \param size		the size of an item
 * \param nitems	the number of items available in the buffer
 */
size_t	loop::write_callback(char *data, size_t size, size_t nitems) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "write %ld bytes from %p", size * nitems,
		data);
	std::string	newdata(data, size * nitems);
	response = response.append(newdata);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "response now '%s'", response.c_str());
	return size * nitems;
}

/**
 * \brief Callback to write data from the response
 *
 * \param data		data buffer containing the data
 * \param size		item size
 * \param nmemb		number of items received
 * \param userdata	pointer to the loop object
 * \return		data size written to the data buffer
 */
static size_t	write_callback(void *data, size_t size, size_t nmemb,
		void *userdata) {
	loop	*l = (loop *)userdata;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "add to %p, size = %ld, nmemb = %ld",
		data, size, nmemb);
	return l->write_callback((char *)data, size, nmemb);
}

/**
 * \brief callback to request data to send to the cloud 
 * 
 * \param data		data buffer to write the request data to
 * \param size		size of an item
 * \param nitems	number of items requested
 * \param userdata	pointer to the loop object
 * \return		data size read from the data buffer
 */
static size_t	read_callback(void *data, size_t size, size_t nitems,
		void *userdata) {
	loop	*l = (loop *)userdata;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "requesting at most %d bytes",
		size * nitems);
	return l->read_callback((char *)data, size, nitems);
}

/**
 * \brief send a request to the cloud
 *
 * \param ids		list of device ids to query
 */
void	loop::sendrequest(const std::list<std::string>& idlist) {
	// create the request JSON
	nlohmann::json	requestjson;
	{
		auto	ids = nlohmann::json::array();
		for (auto id : idlist) {
			ids.push_back(id);
		}
		requestjson["ids"] = ids;
	}
	{
		auto	select = nlohmann::json::array();
		select.push_back("status");
		requestjson["select"] = select;
	}
	{
		auto	pick = nlohmann::json();
		auto	status = nlohmann::json::array();
		status.push_back("ts");
		status.push_back("temperature:0");
		status.push_back("humidity:0");
		status.push_back("devicepower:0");
		status.push_back("sys");
		pick["status"] = status;
		auto	settings = nlohmann::json::array();
		pick["settings"] = settings;
		requestjson["pick"] = pick;
	}
	request = requestjson.dump();

	// build the URL from configuration data
	std::string	url = _config->stringvalue("cloud.url");
	std::string	endpoint = _config->stringvalue("cloud.endpoint");
	std::string	key = _config->stringvalue("cloud.key");
	std::string	requesturl = url + endpoint + "?auth_key=" + key;

	// empty the response string
	response = std::string();

	// send the cloud request
	CURL	*curl;
	curl = curl_easy_init();
	curl_easy_setopt(curl, CURLOPT_URL, requesturl.c_str());
	curl_easy_setopt(curl, CURLOPT_POST, 1);
	struct curl_slist	*list = NULL;
	list = curl_slist_append(list,
		"Content-Type: application/json");
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);
	curl_easy_setopt(curl, CURLOPT_READFUNCTION,
		shelly::read_callback);
	curl_easy_setopt(curl, CURLOPT_READDATA, (void*)this);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION,
		shelly::write_callback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)this);
	curl_easy_setopt(curl, CURLOPT_USERAGENT, "shellyd-agent");
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10);

	// perform the request
	CURLcode	res = curl_easy_perform(curl);
	if (res != CURLE_OK) {
		std::string	e = stringprintf("curl failed: %s",
			curl_easy_strerror(res));
		throw std::runtime_error(e);
	}
}

/**
 * \brief Processing a response from the cloud
 *
 * \param response	the response as a JSON object
 */
void	loop::process(const nlohmann::json& response) {
	// to process the item, we need a database
	database	db(_config);

	for (auto item : response) {
		std::string	id = item["id"];
		debug(LOG_DEBUG, DEBUG_LOG, 0, "processing id %s", id.c_str());
		// get the station and the sensor from the id
		nlohmann::json	device = _config->device(id);
		std::string	station = device["station"];
		std::string	sensor = device["sensor"];
		debug(LOG_DEBUG, DEBUG_LOG, 0, "processing for %s/%s",
			station.c_str(), sensor.c_str());

		// get the data to add
		nlohmann::json	status = item["status"];
		debug(LOG_DEBUG, DEBUG_LOG, 0, "status: %s", status.dump(4).c_str());
		double	ts = 0;
		try {
			ts = status["ts"];
		} catch (const std::exception& x) {
			debug(LOG_INFO, DEBUG_LOG, 0, "no timestamp for id %s",
				id.c_str());
		}
		float	temperature = status["temperature:0"]["tC"];
		float	humidity = status["humidity:0"]["rh"];
		float	voltage = status["devicepower:0"]["battery"]["V"];
		float	percent = status["devicepower:0"]["battery"]["percent"];
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"id = %s, temperature = %f, humidty = %f, "
			"voltage = %f, percent = %f, last = %f",
			id.c_str(), temperature, humidity,
			voltage, percent, ts);

		// get the time key to add
		try {
			time_t	t = timekey().count();	
			db.add(station, sensor, t,
				temperature, humidity, voltage, percent);
		} catch (const std::exception& x) {
			debug(LOG_ERR, DEBUG_LOG, 0, "adding to %s/%s "
				"(temperatur=%.1f,wHumidity=%.0f, battery=%.2f,"
				" capacity=%.0f) failed: %s",
				station.c_str(), sensor.c_str(),
				temperature, humidity, voltage, percent,
				x.what());
		}
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "all ids processed");
}

/**
 * \brief get the time rounded down
 */
std::chrono::seconds	loop::timekey() const {
	std::chrono::system_clock::time_point	start
		= std::chrono::system_clock::now();

	std::chrono::seconds	d
		= std::chrono::duration_cast<std::chrono::seconds>(
		std::chrono::duration_cast<std::chrono::minutes>(
		start.time_since_epoch()));
	return d;
}

/**
 * \brief run the main event loop
 */
void	loop::run() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "start the event loop");
	while (1) {
		// send a request
		try {
			std::list<std::string>	ids = _config->idlist();
			sendrequest(ids);
		} catch (const std::exception& x) {
			debug(LOG_ERR, DEBUG_LOG, 0, "cannot retrieve data: %s",
				x.what());
			goto next;
		}

		// process the response
		debug(LOG_DEBUG, DEBUG_LOG, 0, "response: %s",
			response.c_str());
		try {
			nlohmann::json	r = nlohmann::json::parse(response);
			process(r);
		} catch (const std::exception& x) {
			debug(LOG_ERR, DEBUG_LOG, 0, "cannot process data: %s",
				x.what());
		}

	next:
		// compute how much time we have to wait for the
		// next run
		std::chrono::system_clock::time_point	start
			= std::chrono::system_clock::now();

		std::chrono::seconds	d
			= std::chrono::duration_cast<std::chrono::seconds>(
			std::chrono::duration_cast<std::chrono::minutes>(
			start.time_since_epoch()));

		start = std::chrono::time_point<std::chrono::system_clock,
			std::chrono::seconds>(d);
		auto end = start + std::chrono::seconds(60);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "next point in time: %ld",
			std::chrono::system_clock::to_time_t(end));

		// wait
		std::this_thread::sleep_until(end);
	}
}

} // namespace shelly 
