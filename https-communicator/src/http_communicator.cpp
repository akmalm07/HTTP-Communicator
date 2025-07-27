#include "headers.h"
#include "http_communicator.h"


namespace communicator
{
	HTTPCommunicator::HTTPCommunicator(std::string_view url, const std::unordered_map<std::string, std::string>& headers, size_t requestTimeout)
		: _headers(headers), _requestTimeout(requestTimeout), _requestUrl(std::string(url))
	{
		make_persistent_connection(url);
	}

	HTTPErr HTTPCommunicator::make_persistent_connection(std::string_view url, const std::unordered_map<std::string, std::string>& extraHeaders)
	{
		if (_socket && _socket->is_open())
		{
			std::cout << "Persistent connection already established." << std::endl;
			return HTTPErr::None;
		}

		auto outputResult = decrypt_url_http(url);
		if (!outputResult.has_value())
		{
			std::cerr << "Invalid URL: " << url << std::endl;
			return HTTPErr::InvalidURL;
		}

		auto socketResult = create_and_connect_socket(_persistentIoContext, outputResult->host, outputResult->port, _requestTimeout);
		if (!socketResult.has_value())
		{
			return socketResult.error();
		}

		auto requestResult = this->write_headers(HTTPMethod::GET, HTTPConnection::Close, HTTPContent::None, outputResult->host, outputResult->path, "", extraHeaders);
		if (!requestResult.has_value())
		{
			return requestResult.error();
		}
		
		_socket = std::make_unique<asio::ip::tcp::socket>(std::move(socketResult.value()));

		// Send the HTTP request
		asio::write(*_socket, asio::buffer(requestResult.value()));

		_requestPort = outputResult->port;
		_requestHost = outputResult->host;

		DEBUG_LN
		std::cout << "Persistent connection established to: " << outputResult->host << ":" << outputResult->port << " with Path: " << outputResult->path << std::endl;

		// Read the HTTP response
		auto responseResult = read_http_response(*_socket);
		if (!responseResult.has_value())
		{
			std::cerr << "Failed to read HTTP response: " << static_cast<int>(responseResult.error()) << std::endl;
			return responseResult.error();
		}
		
		
		DEBUG_LN
		std::cout << "HTTP response received: " << responseResult->body << std::endl;
		
		return HTTPErr::None;
	}

	std::expected<std::string, HTTPErr> HTTPCommunicator::get_string(std::string_view url, const std::unordered_map<std::string, std::string>& headers)
	{
		auto outputResult = get(url, headers);
		if (!outputResult.has_value())
		{
			return std::unexpected(outputResult.error());
		}
		if (outputResult->body.empty())
		{
			return std::unexpected(HTTPErr::InvalidData);
		}
		return outputResult->body;
	}

	std::expected<HTTPOutput, HTTPErr> HTTPCommunicator::get(std::string_view url, const std::unordered_map<std::string, std::string>& headers)
	{
		return ::communicator::get(_requestHost, url, _requestPort, HTTPConnection::Persistent, headers, _socket.get());
	}

	std::expected<HTTPOutput, HTTPErr> HTTPCommunicator::post(std::string_view url, HTTPContent content, std::string_view body, const std::unordered_map<std::string, std::string>& headers)
	{
		return ::communicator::post(_requestHost, url, _requestPort, HTTPConnection::Persistent, content, body, headers, _socket.get());
	}

	std::expected<HTTPOutput, HTTPErr> HTTPCommunicator::post(std::string_view url, HTTPContent content, std::vector<uint8_t> body, const std::unordered_map<std::string, std::string>& headers)
	{
		return ::communicator::post(_requestHost, url, _requestPort, HTTPConnection::Persistent, content, body, headers, _socket.get());
	}

	std::expected<HTTPOutput, HTTPErr> get(std::string_view url, const std::unordered_map<std::string, std::string>& headers, asio::ip::tcp::socket* socket)
	{
		auto outputResult = decrypt_url_http(url);
		if (!outputResult.has_value())
		{
			return std::unexpected(outputResult.error());
		}

		return send_http_request(HTTPMethod::GET, HTTPContent::None, HTTPConnection::Close, outputResult->host, outputResult->path, outputResult->port, "", headers, socket);
	}

	std::expected<HTTPOutput, HTTPErr> get(std::string_view host, std::string_view path, std::string_view port, HTTPConnection connection, const std::unordered_map<std::string, std::string>& headers, asio::ip::tcp::socket* socket)
	{
		return send_http_request(HTTPMethod::GET, HTTPContent::None, HTTPConnection::Close, host, path, port, "", headers, socket);
	}

	std::expected<HTTPOutput, HTTPErr> post(std::string_view url, HTTPContent content, std::string_view body, const std::unordered_map<std::string, std::string>& headers, asio::ip::tcp::socket* socket)
	{
		auto outputResult = decrypt_url_http(url);
		if (!outputResult.has_value())
		{
			return std::unexpected(outputResult.error());
		}

		return send_http_request(HTTPMethod::POST, content, HTTPConnection::Close, outputResult->host, outputResult->path, outputResult->port, body, headers, socket);
	
	}

	std::expected<HTTPOutput, HTTPErr> post(std::string_view host, std::string_view path, std::string_view port, HTTPConnection connection, HTTPContent content, std::string_view body, const std::unordered_map<std::string, std::string>& headers, asio::ip::tcp::socket* socket)
	{
		return send_http_request(HTTPMethod::POST, content, connection, host, path, port, body, headers, socket);
	}

	std::expected<HTTPOutput, HTTPErr> post(std::string_view host, std::string_view path, std::string_view port, HTTPConnection connection, HTTPContent content, std::vector<uint8_t> body, const std::unordered_map<std::string, std::string>& headers, asio::ip::tcp::socket* socket)
	{
		return send_http_request(content, connection, host, path, port, body, headers, socket);
	}

	void HTTPCommunicator::set_headers(const std::unordered_map<std::string, std::string>& headers)
	{
		_headers = headers;
	}

	//void HTTPCommunicator::set_proxy(std::string_view proxyHost, uint16_t proxyPort)
	//{
	//	_proxyHost = std::string(proxyHost);
	//	_proxyPort = proxyPort;
	//	if (_proxyHost.empty() || _proxyPort == 0)
	//	{
	//		std::cerr << "Invalid proxy settings." << std::endl;
	//		return;
	//	}
	//	std::cout << "Proxy set to: " << _proxyHost << ":" << _proxyPort << std::endl;
	//}

	HTTPCommunicator::~HTTPCommunicator()
	{
		if (_socket && _socket->is_open())
		{
			DEBUG_LN
			std::cout << "Closing persistent connection." << std::endl;

			std::string req = "GET / HTTP/1.1\r\nHost: " + _requestHost + "\r\nConnection: close\r\n\r\n";

			send_close_http_request(*_socket, _requestHost, _requestPort);

			try
			{
				_socket->close();
			}
			catch (const std::exception& e)
			{
				std::cerr << "Error closing socket: " << e.what() << std::endl;
			}
		}
	}


	std::expected<HTTPOutput, HTTPErr> send_http_request(HTTPMethod method, HTTPContent content, HTTPConnection connection, std::string_view host, std::string_view path, std::string_view port, std::string_view body, const std::unordered_map<std::string, std::string>& headers, asio::ip::tcp::socket* socket)
	{
		try
		{
			asio::io_context ioContext;

			if (socket && socket->is_open())
				goto GOTO_SKIP_SOCKET_CREATION_HTTP_REQ;

			{
				auto socketResult = create_and_connect_socket(ioContext, host, port, 10);
				if (!socketResult.has_value())
				{
					return std::unexpected(socketResult.error());
				}
				socket = &socketResult.value();
			}


		GOTO_SKIP_SOCKET_CREATION_HTTP_REQ:

			auto requestResult = write_headers(method, content, connection, host, path, body, headers);
			if (!requestResult.has_value())
			{
				return std::unexpected(requestResult.error());
			}
			// Send the HTTP request
			asio::write(*socket, asio::buffer(requestResult.value()));


			DEBUG_LN
			std::cout << "HTTP request sent: " << requestResult.value() << std::endl;

			return read_http_response(*socket);
		}
		catch (const std::exception& e)
		{
			std::cerr << "Error: " << e.what() << std::endl;
			return std::unexpected(HTTPErr::ConnectionFailed);
		}

	}

	std::expected<HTTPOutput, HTTPErr> send_http_request(HTTPContent content, HTTPConnection connection, std::string_view host, std::string_view path, std::string_view port, std::vector<uint8_t> body, const std::unordered_map<std::string, std::string>& extraHeaders, asio::ip::tcp::socket* socket)
	{
		try
		{
			asio::io_context ioContext;
			if (socket && socket->is_open())
				goto GOTO_SKIP_SOCKET_CREATION_HTTP_REQ_POST_BYTES;
			{
				auto socketResult = create_and_connect_socket(ioContext, host, port, 10);
				if (!socketResult.has_value())
				{
					return std::unexpected(socketResult.error());
				}
				socket = &socketResult.value();
			}

		GOTO_SKIP_SOCKET_CREATION_HTTP_REQ_POST_BYTES:

			auto requestResult = write_headers(HTTPMethod::POST, content, connection, host, path, std::string_view(reinterpret_cast<const char*>(body.data()), body.size()), extraHeaders);
			if (!requestResult.has_value())
			{
				return std::unexpected(requestResult.error());
			}
			// Send the HTTP request
			asio::write(*socket, asio::buffer(requestResult.value()));
			asio::write(*socket, asio::buffer(body)); 
			DEBUG_LN
				std::cout << "HTTP request sent: " << requestResult.value() << std::endl;
			return read_http_response(*socket);
		}
		catch (const std::exception& e)
		{
			std::cerr << "Error: " << e.what() << std::endl;
			return std::unexpected(HTTPErr::ConnectionFailed);
		}
	}

	std::expected<HTTPOutput, HTTPErr> send_raw_http_request(std::string_view host, std::string_view path, std::string_view port, std::string_view request, asio::ip::tcp::socket* socket)
	{
		try
		{
			asio::io_context ioContext;
			if (socket && socket->is_open())
				goto GOTO_SKIP_SOCKET_CREATION_RAW_HTTP_REQ;


			{
				auto socketResult = create_and_connect_socket(ioContext, host, port, 10);
				if (!socketResult.has_value())
				{
					return std::unexpected(socketResult.error());
				}
				socket = &socketResult.value();
			}


		GOTO_SKIP_SOCKET_CREATION_RAW_HTTP_REQ:

			auto result = is_valid_http_request(request);
			if (result != HTTPErr::None)
			{
				return std::unexpected(result);
			}

			asio::write(*socket, asio::buffer(request));
			
			DEBUG_LN
			std::cout << "Raw HTTP request sent: " << request << std::endl;
			
			return read_http_response(*socket);
		}
		catch (const std::exception& e)
		{
			std::cerr << "Error: " << e.what() << std::endl;
			return std::unexpected(HTTPErr::ConnectionFailed);
		}
	}

	void send_close_http_request(asio::ip::tcp::socket& socket, std::string_view host, std::string_view port, std::string_view path)
	{
		if (!socket.is_open())
		{
			DEBUG_LN
			std::cerr << "Socket is not open or valid." << std::endl;
			return;
		}
		std::string request = "GET " + std::string(path) + " HTTP/1.1\r\n"
			"Host: " + std::string(host) + "\r\n"
			"Connection: close\r\n"
			"\r\n";
		try
		{
			asio::write(socket, asio::buffer(request));
			DEBUG_LN
				std::cout << "Close HTTP request sent." << std::endl;
		}
		catch (const std::exception& e)
		{
			DEBUG_LN
			std::cerr << "Error sending close request: " << e.what() << std::endl;
		}
	}


	std::expected<std::string, HTTPErr> write_headers(HTTPMethod method, HTTPContent contentType, HTTPConnection connection, std::string_view host, std::string_view path, std::string_view body, const std::unordered_map<std::string, std::string>& headers)
	{
		std::ostringstream requestStream;
		requestStream << to_string(method) << " " << path << " HTTP/1.1\r\n";
		requestStream << "Host: " << host << "\r\n";

		for (const auto& header : headers)
		{
			requestStream << header.first << ": " << header.second << "\r\n";
		}

		requestStream << "Connection: " << to_string(connection) << "\r\n";

		if (!body.empty())
		{
			requestStream << "Content-Length: " << body.size() << "\r\n";

			requestStream << "Content-Type: " << to_string(contentType) << "\r\n";
		}
		else if (method == HTTPMethod::POST || method == HTTPMethod::PUT)
		{
			return std::unexpected(HTTPErr::NoBodyForMethod);
		}

		requestStream << "\r\n";

		requestStream << body;

		return requestStream.str();
	}


	std::expected<HTTPOutput, HTTPErr> HTTPCommunicator::send_raw_http_request(std::string_view request, std::string_view path)
	{
		return ::communicator::send_raw_http_request(_requestHost, path, _requestPort, request, _socket.get());
	}

	std::expected<std::string, HTTPErr> HTTPCommunicator::write_headers(HTTPMethod method, HTTPConnection connection, HTTPContent contentType, std::string_view host, std::string_view path, std::string_view body, const std::unordered_map<std::string, std::string>& extraHeaders)
	{
		auto headers = extraHeaders;
		
		headers.insert(_headers.begin(), _headers.end());
		
		for (const auto& extraHeader : headers)
		{
			auto it = _headers.find(extraHeader.first);
			if (it != _headers.end())
			{
				headers.erase(it);
			}
		}


		return ::communicator::write_headers(method, contentType, connection, host, path, body, headers);
	}

	std::expected<HTTPOutput, HTTPErr> read_http_response(asio::ip::tcp::socket& socket)
	{
		// Read the HTTP response
		asio::streambuf responseBuffer;
		asio::error_code ec;
		std::size_t bytes = asio::read_until(socket, responseBuffer, "\r\n\r\n", ec);

		if (ec == asio::error::eof)
		{
			// EOF reached, but we might have data
			if (bytes > 0)
			{
				std::istream responseStream(&responseBuffer);
				std::string response(bytes, '\0');
				responseStream.read(&response[0], bytes);
				DEBUG_LN
				std::cout << "Partial response before EOF:\n" << response << std::endl;
			}
			else
			{
				DEBUG_LN
				std::cerr << "Connection closed with no data.\n";
				return std::unexpected(HTTPErr::InvalidData);
			}
		}
		else if (ec)
		{
			std::cerr << "Read error: " << ec.message() << std::endl;
		}


		// Parse status line
		std::istream responseStream(&responseBuffer);

		DEBUG_LN
		std::cout << "Response received:\n " << std::string(std::istreambuf_iterator<char>(responseStream), {}) << std::endl;

		std::string httpVersion;
		unsigned int statusCode;
		std::string statusMessage;

		responseStream >> httpVersion >> statusCode;
		std::getline(responseStream, statusMessage);

		if (statusCode != 200)
			return std::unexpected(HTTPErr::ResponseError);
		if (httpVersion != "HTTP/1.1" && httpVersion != "HTTP/2.0")
			return std::unexpected(HTTPErr::HTTPVersionUndefined);
		// Read headers
		std::string header;

		size_t contentLength = 0;
		HTTPContent contentType = HTTPContent::None;
		HTTPTransferEncoding transferEncoding = HTTPTransferEncoding::None;
		HTTPContentEncoding contentEncoding = HTTPContentEncoding::None;
		HTTPConnection connection = HTTPConnection::Close;
		HTTPLanguage language = HTTPLanguage::None;

		while (std::getline(responseStream, header) && header != "\r")
		{

			if (header.starts_with("Transfer-Encoding:"))
			{
				transferEncoding = static_cast<HTTPTransferEncoding>(to_uint32<HTTPTransferEncoding>(header.substr(19)));
			}

			if (header.starts_with("Content-Encoding:"))
			{
				contentEncoding = static_cast<HTTPContentEncoding>(to_uint32<HTTPContentEncoding>(header.substr(18)));
			}

			if (header.starts_with("Content-Type:"))
			{				
				contentType = static_cast<HTTPContent>(to_uint32<HTTPContent>(header.substr(14)));
			}

			if (header.starts_with("Content-Length:"))
			{
				try
				{
					contentLength = std::stoul(header.substr(15));
				}
				catch (...) 
				{
					return std::unexpected(HTTPErr::InvalidContentSize);
				}
			}

			if (header.starts_with("Content-Language:"))
			{
				language = static_cast<HTTPLanguage>(to_uint32<HTTPLanguage>(header.substr(17)));
			}

			if (header.starts_with("Connection:"))
			{
				connection = static_cast<HTTPConnection>(to_uint32<HTTPConnection>((header.substr(12))));
			}

			if (header.empty())
				break; 
		}

		
		std::ostringstream bodyStream;

		if (transferEncoding == HTTPTransferEncoding::Chunked && contentLength == 0)
		{
			while (true)
			{
				std::string chunkSizeStr;
				std::getline(responseStream, chunkSizeStr);

				if (chunkSizeStr.empty())
					continue;

				size_t chunkSize = std::stoul(chunkSizeStr, nullptr, 16);

				if (chunkSize == 0)
					break;

				asio::read(socket, responseBuffer, asio::transfer_exactly(chunkSize));
				bodyStream << &responseBuffer; 

				contentLength += chunkSize;

				std::string crlf;
				std::getline(responseStream, crlf);
			}
		}
		else
		{
			std::string bodyContent;
			if (responseBuffer.size() > 0)
				bodyStream << &responseBuffer;

			size_t remaining = contentLength - bodyStream.str().size();

			if (remaining > 0)
				asio::read(socket, responseBuffer, asio::transfer_exactly(remaining));

			bodyStream << &responseBuffer;

		}

		return HTTPOutput{
			bodyStream.str(),
			contentType,
			connection,
			transferEncoding,
			contentEncoding,
			language,
			contentLength,
			statusCode,
			statusMessage
		};
	}

	std::expected<asio::ip::tcp::socket, HTTPErr> create_and_connect_socket(asio::io_context& ioContext, std::string_view host, std::string_view port, size_t requestTimeout)
	{
		asio::steady_timer timer(ioContext);
		bool connectStatus = false;
		asio::ip::tcp::resolver resolver(ioContext);

		asio::error_code ec;
		asio::ip::tcp::resolver::results_type endpoint = resolver.resolve(host, port, ec);

		if (ec)
		{
			std::cerr << "DNS resolution failed: " << ec.message() << std::endl;
			return std::unexpected(HTTPErr::DNSResolutionFailed);
		}

		asio::ip::tcp::socket socket(ioContext);

		asio::async_connect(socket, endpoint,
			[&connectStatus, &timer](const asio::error_code& ec, const asio::ip::tcp::endpoint& endpoint)
			{
				if (!ec)
				{
					connectStatus = true;
					timer.cancel();
				}
			}
		);

		timer.expires_after(std::chrono::seconds(requestTimeout));

		timer.async_wait([&connectStatus, &socket](const asio::error_code& ec)
			{
				if (!connectStatus)
				{
					std::cerr << "Connection timed out." << std::endl;
					socket.close();
				}
			});

		ioContext.run();

		if (!connectStatus)
		{
			return std::unexpected(HTTPErr::ConnectionFailed);
		}

		return std::move(socket);
	}

	HTTPErr is_valid_http_request(std::string_view request)
	{
		if (request.empty())
		{
			std::cerr << "HTTP request is empty." << std::endl;
			return HTTPErr::EmptyRequest;
		}

		// Validate method
		constexpr std::string_view methods[] = { "GET ", "POST ", "PUT ", "DELETE ", "PATCH ", "HEAD ", "OPTIONS " };
		bool validMethod = false;
		for (auto& method : methods)
		{
			if (request.starts_with(method))
			{
				validMethod = true;
				break;
			}
		}
		if (!validMethod)
		{
			return HTTPErr::InvalidMethod;
		}

		auto firstLineEnd = request.find("\r\n");
		if (firstLineEnd == std::string_view::npos)
		{
			return HTTPErr::MissingCRLF;
		}

		if (request.find("\r\n\r\n") == std::string_view::npos)
		{
			return HTTPErr::MissingHeaderTerminator;
		}

		auto request_line = request.substr(0, firstLineEnd);
		if (request_line.find("HTTP/1.1") == std::string_view::npos &&
			request_line.find("HTTP/1.0") == std::string_view::npos)
		{
			return HTTPErr::InvalidHTTPVersion;
		}

		// Passed all checks
		return HTTPErr::None;
	}




	std::string parse_http(const std::string& response, HTTPParse type)
	{
		std::istringstream stream(response);
		std::string line;

		while (std::getline(stream, line))
		{
			if (line.empty())
				break;

			switch (type)
			{
			case communicator::HTTPParse::ContentLength:
				if (line.starts_with("Content-Length:"))
					return line.substr(16);
				break;
			case communicator::HTTPParse::ContentType:
				if (line.starts_with("Content-Type:"))
					return line.substr(14);
				break;
			case communicator::HTTPParse::TransferEncoding:
				if (line.starts_with("Transfer-Encoding:"))
					return line.substr(19);
				break;
			case communicator::HTTPParse::Connection:
				if (line.starts_with("Connection:"))
					return line.substr(12);
				break;
			case communicator::HTTPParse::StatusMessage:
				if (line.starts_with("HTTP/"))
				{
					auto pos = line.find(' ');
					if (pos != std::string::npos)
						return line.substr(pos + 1);
				}
				break;
			case communicator::HTTPParse::StatusCode:
				if (line.starts_with("HTTP/"))
				{
					auto pos = line.find(' ');
					if (pos != std::string::npos)
					{
						auto endPos = line.find(' ', pos + 1);
						return line.substr(pos + 1, endPos - pos - 1);
					}
				}
				break;
			}
		}
		return {};
	}

	std::expected<URLDescriptorOutput, HTTPErr> decrypt_url_http(std::string_view url)
	{
		std::string_view host;
		std::string path, port = "80";
		size_t pos = url.find("http://");

		if (pos == std::string::npos)
		{
			return std::unexpected(HTTPErr::InvalidURL);
		}

		pos += 7;

		size_t slashPos = url.find('/', pos);
		if (slashPos != std::string::npos)
		{
			host = url.substr(pos, slashPos - pos);
			path = url.substr(slashPos);
		}
		else
		{
			host = url.substr(pos);
			path = "/";
		}

		if (host.empty())
		{
			return std::unexpected(HTTPErr::InvalidURL);
		}

		std::string hostStr(host);

		size_t colonPos = host.find(':');

		if (colonPos != std::string::npos)
		{
			hostStr = host.substr(0, colonPos);
			port = host.substr(colonPos + 1);
		}
		return URLDescriptorOutput{ hostStr, path, port };
	}
}