//
// chat_client.cpp
// ~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2017 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "test_asio.h"
#include <cstdlib>
#include <deque>
#include <iostream>
#include <thread>
#include "asio.hpp"


using asio::ip::tcp;

typedef std::deque<chat_message> chat_message_queue;

class chat_client
{
public:
	chat_client(asio::io_context& io_context,
		const tcp::resolver::results_type& endpoints)
		: io_context_(io_context),
		socket_(io_context)
	{
		do_connect(endpoints);
	}

	void write(const chat_message& msg)
	{
		asio::post(io_context_,
			[this, msg]()
		{
			bool write_in_progress = !write_msgs_.empty();
			write_msgs_.push_back(msg);
			if (!write_in_progress)
			{
				do_write();
			}
		});
	}


	void write(std::string msg)
	{

		asio::async_write(socket_,
			asio::buffer(msg.c_str(),
			msg.length()),
			[this](std::error_code ec, std::size_t /*length*/)
		{
			
		});
	}


	void close()
	{
		asio::post(io_context_, [this]() { socket_.close(); });
	}

private:
	void do_connect(const tcp::resolver::results_type& endpoints)
	{
		asio::async_connect(socket_, endpoints,
			[this](std::error_code ec, tcp::endpoint)
		{
			if (!ec)
			{
				do_read_header();
			}
		});
	}

	void do_read_header()
	{
		asio::async_read(socket_,
			asio::buffer(read_msg_.data(), chat_message::header_length),
			[this](std::error_code ec, std::size_t /*length*/)
		{
			if (!ec && read_msg_.decode_header())
			{
				do_read_body();
			}
			else
			{
				socket_.close();
			}
		});
	}

	void do_read_body()
	{
		asio::async_read(socket_,
			asio::buffer(read_msg_.body(), read_msg_.body_length()),
			[this](std::error_code ec, std::size_t /*length*/)
		{
			if (!ec)
			{
				std::cout.write(read_msg_.body(), read_msg_.body_length());
				std::cout << "\n";
				do_read_header();
			}
			else
			{
				socket_.close();
			}
		});
	}

	void do_write()
	{
		asio::async_write(socket_,
			asio::buffer(write_msgs_.front().data(),
			write_msgs_.front().length()),
			[this](std::error_code ec, std::size_t /*length*/)
		{
			if (!ec)
			{
				write_msgs_.pop_front();
				if (!write_msgs_.empty())
				{
					do_write();
				}
			}
			else
			{
				socket_.close();
			}
		});
	}

private:
	asio::io_context& io_context_;
	tcp::socket socket_;
	chat_message read_msg_;
	chat_message_queue write_msgs_;
};

// int main(int argc, char* argv[])
// {
// 	try
// 	{
	

// 		asio::io_context io_context;

// 		tcp::resolver resolver(io_context);
// 		auto endpoints = resolver.resolve("127.0.0.1","4711");
// 		chat_client c(io_context, endpoints);
// 		std::thread t([&io_context](){ io_context.run(); });

		
// 		std::string x;
// 		std::string s;
// 		s=s+R"(Content-Length: 310)" + "\r\n\r\n" + R"({"command":"initialize", "arguments" : {"clientID":"vscode", "adapterID" : "mock", "pathFormat" : "path", "linesStartAt1" : true, "columnsStartAt1" : true, "supportsVariableType" : true, "supportsVariablePaging" : true, "supportsRunInTerminalRequest" : true, "locale" : "zh-cn"}, "type" : "request", "seq" : 1})";

// 		std::string s1("hello");
// 		int index = 0;
// 		while (std::cin >> x)
// 		{
// 			std::cout << x << std::endl;
// 			if (index == 0){
// 				c.write(s);
// 			}
// 			else{
// 				c.write(s);
// 			}
// 			index++;
			
// 		}
		
		
// 		c.close();
// 		t.join();
// 	}
// 	catch (std::exception& e)
// 	{
// 		std::cerr << "Exception: " << e.what() << "\n";
// 	}

// 	return 0;
// }
