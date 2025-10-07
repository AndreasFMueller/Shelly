/*
 * loop.cpp
 *
 * (c) 2025 Prof Dr Andreas Müller
 */
#include "loop.h"
#include "json.hpp"
#include "debug.h"
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
loop::loop() {
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
bool	loop::sendrequest(const std::list<std::string>& idlist) {
	nlohmann::json	j;
	{
		auto	ids = nlohmann::json::array();
		for (auto id : idlist) {
			ids.push_back(id);
		}
		j["ids"] = ids;
	}
	{
		auto	select = nlohmann::json::array();
		select.push_back("status");
		j["select"] = select;
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
		j["pick"] = pick;
	}
	request = j.dump();
	std::cout << request << std::endl;
	response = std::string();

	// build the URL from configuration data (XXX actually do that)
	std::string	url = "https://shelly-209-eu.shelly.cloud";
	std::string	endpoint = "/v2/devices/api/get";
	std::string	key = "MzZlNDQwdWlkF605193D617BE7552074F20C4BAFCBA30B854FB7A3EFFA5FF7036BED3CDFD08640C8AF52D5F0E72A";
	std::string	requesturl = url + endpoint + "?auth_key=" + key;

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
		debug(LOG_ERR, DEBUG_LOG, 0, "curl failed: %s",
			curl_easy_strerror(res));
		return false;
	}
	return true;
}

/**
 * \brief Processing a response from the cloud
 *
 * \param response	the response as a JSON object
 */
bool	loop::process(const nlohmann::json& response) {
	for (auto item : response) {
		std::string	id = item["id"];
		nlohmann::json	status = item["status"];
		double	ts = status["ts"];
		float	temperature = status["temperature:0"]["tC"];
		float	humidity = status["humidity:0"]["rh"];
		float	voltage = status["devicepower:0"]["battery"]["V"];
		float	percent = status["devicepower:0"]["battery"]["percent"];
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"id = %s, temperature = %f, humidty = %f, "
			"voltage = %f, percent = %f, last = %f",
			id.c_str(), temperature, humidity,
			voltage, percent, ts);
	}
	return true;
}

/**
 * \brief run the main event loop
 */
void	loop::run() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "start the event loop");
	while (1) {
		// send a request
		std::list<std::string>	ids;
		ids.push_back("54320457a234");
		ids.push_back("54320457a250");
		sendrequest(ids);

		// process the response
		debug(LOG_DEBUG, DEBUG_LOG, 0, "response: %s",
			response.c_str());
		nlohmann::json	r = nlohmann::json::parse(response);
		process(r);

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
