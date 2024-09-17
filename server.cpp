#include "server.h"

#include <iostream>
#include <thread>
#include <vector>
#include <memory>
#include <string>

boost::asio::io_context 	ServerData::io_context;
std::mutex 					ServerData::client_count_mutex;
int 						ServerData::port				= 12345;
int 						ServerData::client_count 		= 0;

// boost::asio::ip::address::from_string("192.168.0.103")
std::vector<std::shared_ptr<tcp::socket>> clients; // Контейнер для клиентов

void run_server()
{
	try 
	{
		tcp::acceptor 	acceptor(ServerData::io_context, tcp::endpoint(tcp::v4(), ServerData::port));

		std::cout << "Сервер запущен на порту 12345" << std::endl;


		while (true) 
		{
            // Ожидание подключения клиента
            auto socket = std::make_shared<tcp::socket>(ServerData::io_context);
            acceptor.accept(*socket);

            std::cout << "Клиент подключился." << std::endl;
            ServerData::client_count++;
            std::cout << "Количесиво подключенных к серверу клиентов: " << ServerData::client_count << std::endl;

            // Добавляем новый сокет в контейнер
            clients.push_back(socket);

            // Запускаем обработку клиента в отдельном потоке
            std::thread(handle_client, socket).detach();
        }
	}
	catch (std::exception& e) 
    {
        std::cerr << "Ошибка: " << e.what() << std::endl;
    }
}

void handle_client(std::shared_ptr<tcp::socket> socket) 
{
    try 
    {
        while (true) 
        {
            char data[1024];
            boost::system::error_code error;

            // Читаем данные от клиента
            size_t length = socket->read_some(boost::asio::buffer(data), error);

            if (error == boost::asio::error::eof) 
            {
                std::cout << "Клиент отключился." << std::endl;
                ServerData::client_count--;
            	std::cout << "Количесиво подключенных к серверу клиентов: " << ServerData::client_count << std::endl;
                break; // Клиент отключился
            } 
            else if (error) 
            {
            	ServerData::client_count--;
            	std::cout << "Количесиво подключенных к серверу клиентов: " << ServerData::client_count << std::endl;
                throw boost::system::system_error(error);
            }

            // Отправляем сообщение всем клиентам
            for (const auto& client : clients) 
            {
                if (client != socket)
                { 
                	// Не отправляем обратно тому, от кого пришло сообщение
                    boost::asio::write(*client, boost::asio::buffer(data, length));
                }
            }

            std::cout << "Получено сообщение от клиента: " << std::string(data, length) << std::endl;
        }

    } catch (std::exception& e) {
        std::cerr << "Ошибка: " << e.what() << std::endl;
        // Удаляем клиентский сокет из списка при ошибке
        clients.erase(std::remove(clients.begin(), clients.end(), socket), clients.end());
    }
}

void listen(tcp::acceptor& acceptor)
{
	while (true)      
	{
		tcp::socket socket(ServerData::io_context);
		acceptor.accept(socket);
		std::cout << "Клиент подключен" << std::endl;


        std::cout << read_message_from_client(socket) << std::endl;
	}
}

void send_message_to_client(std::string message, tcp::socket& socket)
{
    boost::asio::write(socket, boost::asio::buffer(message));
}

std::string read_message_from_client(tcp::socket& socket)
{
    char buffer[1024];
    size_t len = socket.read_some(boost::asio::buffer(buffer));
    std::string message_from_client(buffer, len);
    
    return message_from_client;
}

int main()
{
	run_server();

	return 0;
}