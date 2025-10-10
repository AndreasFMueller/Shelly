/*
 * shellyd.cpp -- daemon to load shelly cloud data to meteo database
 *
 * (c) 2025 Prof Dr Andreas Müller
 */
#include <stdexcept>
#include <cstdio>
#include <iostream>
#include <getopt.h>
#include <sys/stat.h>
#include <unistd.h>
#include "debug.h"
#include "loop.h"
#include "common.h"
#include "configuration.h"

namespace shelly {

/**
 * \brief display a short usage message
 *
 * \param progname	the programm name to use
 */
void	usage(char *progname) {
	std::cout << progname << " [ options ]" << std::endl;
	std::cout << std::endl;
	std::cout << "program to read data from the shelly cloud" << std::endl;
	std::cout << std::endl;
	std::cout << "options:" << std::endl;
	std::cout << " -h,-?,--help        display this help message and exit"
		<< std::endl;
	std::cout << " -d,--debug          enable debug messages" << std::endl;
	std::cout << " -c,--config=<c>     read configuration from file <c>"
		<< std::endl;
	std::cout << " -f,--foreground     run in the foreground" << std::endl;
	std::cout << " -n,--dryrun         don't update the database"
		<< std::endl;
	std::cout << " -s,--syslog         send log messages to syslog"
		<< std::endl;
}

static struct option	longopts[] = {
{ "config",		required_argument,	NULL,		'c' },
{ "debug",		no_argument,		NULL,		'd' },
{ "syslog",		no_argument,		NULL,		's' },
{ "help",		no_argument,		NULL,		'h' },
{ "dryrun",		no_argument,		NULL,		'n' },
{ "foreground",		no_argument,		NULL,		'f' },
{ NULL,			0,			NULL,		 0  }
};

/**
 * \brief The shellyd main function
 *
 * \param argc		the number of command line parameters
 * \param argv		array of command line parameters
 */
int	main(int argc, char *const argv[]) {
	bool	foreground = false;
	std::string	configfilename(SHELLYCONFFILE);
	configuration_ptr	config;

	int	c;
	int	longindex;
	while (EOF != (c = getopt_long(argc, argv, "c:d?hfsn", longopts,
		&longindex)))
		switch (c) {
		case 'c':
			configfilename = std::string(optarg);
			debug(LOG_DEBUG, DEBUG_LOG, 0, "using config file '%s'",
				configfilename.c_str());
			break;
		case 'd':
			debuglevel = LOG_DEBUG;
			break;
		case 'h':
		case '?':
			usage(argv[0]);
			return EXIT_SUCCESS;
		case 'f':
			foreground = true;
			break;
		case 's':
			debug_syslog(LOG_LOCAL0);
			break;
		case 'n':
			dryrun = true;
			break;
		}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "command line parsed");

	// parse the configuration file
	config = configuration_ptr(new configuration(configfilename));

	// daemonize unless prevented by the --foreground option
	if (foreground) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "stay in foreground");
	} else {
		pid_t	pid = fork();
		if (pid < 0) {
			debug(LOG_ERR, DEBUG_LOG, 0, "cannot fork: %s",
				strerror(errno));
			return EXIT_FAILURE;
		}
		if (pid > 0) {
			// parent exits
			return EXIT_SUCCESS;
		}
		setsid();
		chdir("/");
		umask(0);
	}

	// TEST access database data
	std::string	hostname = config->stringvalue("database.hostname");
	debug(LOG_DEBUG, DEBUG_LOG, 0, "database hostname: %s",
		hostname.c_str());

	// start the main loop
	loop	l(config);
	l.run();
	
	return EXIT_SUCCESS;
}

} // namespace shelly

int	main(int argc, char *const argv[]) {
	try {
		return shelly::main(argc, argv);
	} catch (const std::exception& x) {
		std::cerr << "terminated by exception: " << x.what();
		std::cerr << std::endl;
	} catch (...) {
		std::cerr << "terminated by unknown exception" << std::endl;
	}
}
