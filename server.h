#ifndef SERVER_H
#define SERVER_H

#include <boost/asio.hpp>

using boost::asio::ip::tcp;

struct ServerData
{
	static boost::asio::io_context 	io_context;
	static std::mutex 				client_count_mutex;
	static int 						port;
	static int 						client_count;
};

void 								run_server();
void 								listen(tcp::acceptor& acceptor);
void 								send_message_to_client(std::string message, tcp::socket& socket);
void 								handle_client(std::shared_ptr<tcp::socket> socket);
std::string 						read_message_from_client(tcp::socket& socket);

#endif // SERVER_H