#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <boost/program_options/positional_options.hpp>
#include <fstream>

#include "PongServer.hpp"

namespace po = boost::program_options;
namespace bfs = boost::filesystem;

#define PONG_SERVER_PORT 34333

int main (int argc, char const* argv[]) {
	std::cout << "FullCircle Pong Server" << std::endl;

	// default config file path
	boost::filesystem::path config_file;
	char* homedir = getenv("HOME");
	if (homedir != NULL) {
		config_file = config_file / homedir / ".fullcirclerc";
	} else {
		config_file = config_file / "etc" / "fullcirclerc";
	}


	try {
		po::options_description generic("Generic options (config file and command line)");
		generic.add_options()
			("port,p", po::value<std::string>(), "port of the fullcircle server")
			("server,s", po::value<std::string>(), "fullcircle server")
			("listenport,l", po::value<std::string>(), "port to which players connect")
			;
		std::ostringstream coss;
		coss << "configuration file (" << config_file << " by default).";
		po::options_description cmd("Command line options");
		cmd.add_options()
			("config", po::value<std::string>(), coss.str().c_str())
			("help,?", "produce help message")
			("version,v", "print version and exit")
			;
		std::ostringstream oss;
		oss << "Usage: " << argv[0] << " -s <IP> -p <PORT> ...";
		po::options_description cmdline_options(oss.str());
		cmdline_options.add(generic).add(cmd);

		po::variables_map vm;
		po::store(po::parse_command_line(argc, argv, cmdline_options), vm);
		po::notify(vm);

		// Load additional config file settings.
		if (vm.count("config")) {
			boost::filesystem::path temp(vm["config"].as<std::string>());
			if ( boost::filesystem::exists(temp) )
				config_file = vm["config"].as<std::string>();
			else {
				std::cerr << "Configuration file " << vm["config"].as<std::string>() << " not found!" << std::endl;
				return 1;
			}
		}

		po::options_description config_file_options;
		config_file_options.add(generic);
		boost::filesystem::path config(config_file);
		if ( boost::filesystem::exists(config) ) {
			po::store(po::parse_config_file<char>(config_file.c_str(), config_file_options, true), vm);
		}
		po::notify(vm);

		// Begin processing of commandline parameters.
		std::string server;
		uint16_t port;
		uint16_t listenport;

		if (vm.count("help")) {
			std::cout << cmdline_options << std::endl;
			return 1;
		}

		fullcircle::VersionInfo::Ptr version(new fullcircle::VersionInfo());
		if (vm.count("version")) {
			std::cout << "fullcircle version " << version->getVersion() << std::endl;
			return 0;
		}

		if (vm.count("server") != 1 ) {
			std::cerr << "You must specify a server (-s <HOST/IP>)." << std::endl;
			return 1;
		} else {
			server=vm["server"].as<std::string>();
		}

		if (vm.count("port") != 1 ) {
			std::cerr << "You must specify a port. " << std::endl;
			return 1;
		} else {
			std::istringstream converter(vm["port"].as<std::string>());
			if ( !( converter >> port)) {
				std::cerr << "Cannot convert port to an integer. " << std::endl;
				return 1;
			}
		}

		if (vm.count("listenport") != 1 ) {
			std::cerr << "Warning: using fallback port " << PONG_SERVER_PORT << std::endl;
			listenport = PONG_SERVER_PORT;
		} else {
			std::istringstream converter(vm["listenport"].as<std::string>());
			if ( !( converter >> listenport)) {
				std::cerr << "Cannot convert listenport to an integer" << std::endl;
				return 1;
			}
		}

		boost::asio::io_service my_io_service;
		boost::asio::ip::tcp::resolver resolver(my_io_service);
		std::ostringstream oss2;
		oss2 << port;
		boost::asio::ip::tcp::resolver::query query(
				server, oss2.str());
		boost::asio::ip::tcp::resolver::iterator iterator = 
			resolver.resolve(query);

		std::cout << "Initializing Server..." << std::endl;
		PongServer ps(8, 8, 10);
		std::cout << "Waiting for players" << std::endl;
		ps.waitForPlayers(listenport);
		std::cout << "Starting the game" << std::endl;
		ps.startGame(&iterator);

	} catch (fullcircle::GenericException& ex) {
		std::cout << "Caught exception: " << ex.what() << std::endl;
		exit(1);
	} catch(std::exception& e) {
		std::cerr << "error: " << e.what() << std::endl;
		return 1;
	} catch(...) {
		std::cerr << "Exception of unknown type!" << std::endl;
		return 1;
	}

	return 0;
}

