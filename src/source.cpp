#undef UNICODE

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <stdexcept>

#include "socket/socket.h"
#include "console/console.h"

using namespace Lunaris;

constexpr auto tcp_port = 40302;
constexpr size_t tcp_reasonable_size_max = 128; // /path/to/something = 123456.789
constexpr size_t tcp_number_max_len = 8; // 12345678

/*struct header_data {
	uint8_t file_id;
	uint16_t data_coming_size;
	// .. write on file "${file_id}.raw" whatever comes next (max: data_coming_size)
};*/

void mode_client();
void mode_host();


void send_number_through(Lunaris::TCP_client& cli, const int num)
{
	char buf[tcp_number_max_len + 1];
	snprintf(buf, sizeof(buf), "%i", num);
	cout << console::color::DARK_GRAY << buf;
	cli.send(buf, tcp_number_max_len);
}

int get_number_from(Lunaris::TCP_client& cli)
{
	auto dat = cli.recv(tcp_number_max_len);
	dat.push_back('\0');
	cout << console::color::DARK_GRAY << dat.data();
	return static_cast<int>(std::strtol(dat.data(), nullptr, 10));
}

//template<typename T>
//T get_n_cast_any(Lunaris::TCP_client& cli)
//{
//	auto dat = cli.recv(sizeof(T));
//	if (dat.size() == sizeof(T)) return *((T*)dat.data());
//	throw std::runtime_error("RECV did not receive enough data.");
//	return {};
//}


int main(int argc, char* argv[])
{
	bool is_host = false; // 0 == undef, 1 == host, 2 == client

	if (argc > 1) {
		switch(argv[1][0]) {
		case 's':
		case 'S':
			is_host = true;
			break;
		case 'c':
		case 'C':
			is_host = false;
			break;
		default:
			cout << console::color::GREEN << "If you want to call by parameter, use 'c' for CLIENT, 's' for SERVER.";
			return 0;
		}
	}
	else {
		cout << console::color::YELLOW << "Starting as client. If you want to start as server, consider calling 'app S' or 'app s'.";		
	}

	if (is_host) mode_host();
	else mode_client();

	return 1; // if returned, bad!
}

void mode_client()
{
	std::string ip;

	cout << console::color::GOLD << "IP address?";
	std::getline(std::cin, ip);
	while(ip.size() && ip.back() == '\n') ip.pop_back();


	TCP_client cli;
	if (!cli.setup(socket_config().set_port(tcp_port).set_ip_address(ip).set_family(socket_config::e_family::IPV6))) {
		cout << console::color::RED << "Could not connect to host. Try again later";
		return;
	}

	while(1) {
		int fid;

		cout << console::color::GOLD << "File id? [0..255]";

		std::cin >> fid;

		fid %= 256;

		std::string dat;
		cout << console::color::GOLD << "Write what you want to send: (up to 127 characters please.)";

		while(dat.size() == 0) {

			std::getline(std::cin, dat);
			while(dat.size() && dat.back() == '\n') dat.pop_back();

			dat = dat.substr(0, 127);

			if (dat.size() == 0) cout << console::color::YELLOW << "You must type something valid. Type something and hit enter";
		}

		dat.insert(dat.end(), '\0');

		send_number_through(cli, fid);
		send_number_through(cli, dat.length());

		cli.send(dat.data(), dat.size());
	}
}

void mode_host()
{
	cout << console::color::LIGHT_PURPLE << "Starting as server...";
	TCP_host host;
	const auto config = socket_config()
		.set_family(socket_config::e_family::IPV6)
		.set_port(tcp_port);

	if (!host.setup(config)) {
		cout << console::color::RED << "Could not launch server.";
		return;
	}

	cout << console::color::LIGHT_PURPLE << "Server ready.";

	while(1) {
		TCP_client cli = host.listen();

		if (cli.valid()) {
			const auto info = cli.info();
			cout << console::color::LIGHT_PURPLE << "Handling current user:";
			cout << console::color::LIGHT_PURPLE << "- IP: " << info.ip_address;
			cout << console::color::LIGHT_PURPLE << "- Port: " << info.port;
			cout << console::color::LIGHT_PURPLE << "- Family: " << (info.family == socket_config::e_family::IPV6 ? "IPV6" : "IPV4");
		}

		try {
			while(cli.valid()) {
				const int fid = get_number_from(cli);
				const int coming_size = get_number_from(cli);

				if (coming_size == 0) {
					cout << console::color::BLUE << "# DISCONNECTED!";
					break;
				}

				const std::string path = std::to_string(fid) + ".raw";				

				cout << console::color::BLUE << "# Got data: path=" << path << " datalen=" << coming_size;

				if (coming_size > tcp_reasonable_size_max) throw std::runtime_error("data_coming_size was too big!");

				auto dat = cli.recv(static_cast<size_t>(coming_size));

				if (dat.size() != coming_size) throw std::runtime_error("failed to receive data fully.");

				std::fstream fp{path.c_str(), std::ios::out | std::ios::app};

				fp.write(dat.data(), dat.size());
				fp.write("\n", 1);
				fp.close();

				cout << console::color::BLUE << "# Wrote: " << dat.data();
			}
		}
		catch(const std::exception& e)
		{
			cout << console::color::RED << "Connection got exception: " << e.what() << ". Disconnected.";
		}
		catch(...)
		{
			cout << console::color::RED << "Connection got unknown exception. Disconnected.";
		}
	}
}
