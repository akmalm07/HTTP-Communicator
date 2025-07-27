#include "headers.h"
#include "http_communicator.h"


namespace communicator
{


	// --- HTTPCommunicator Implementation ---

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

		auto requestResult = this->write_headers(HTTPMethod::GET, HTTPConnection::Persistent, HTTPContent::None, outputResult->host, outputResult->path, "", extraHeaders);
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
		auto res = get_string();

		if (!res.has_value())
		{
			DEBUG_LN
				std::cerr << "Failed to read HTTP response: " << static_cast<int>(res.error()) << std::endl;
			return res.error();
		}
		DEBUG_LN
			std::cout << "Persistent connection established successfully." << std::endl;
		return HTTPErr::None;
	}

	std::expected<std::vector<uint8_t>, HTTPErr> HTTPCommunicator::get_bytes(std::string_view url, const std::unordered_map<std::string, std::string>& headers)
	{
		auto outputResult = get(url, headers);
		if (!outputResult.has_value())
		{
			return std::unexpected(outputResult.error());
		}

		std::vector<uint8_t> body;
		if (std::holds_alternative<std::vector<uint8_t>>(outputResult->body))
		{
			body = std::get<std::vector<uint8_t>>(outputResult->body);
		}
		else
		{
			DEBUG_LN
				std::cerr << "Expected string body, but got binary data." << std::endl;
			return std::unexpected(HTTPErr::InvalidReadingData);
		}

		if (body.empty())
		{
			return std::unexpected(HTTPErr::InvalidData);
		}

		if (is_str_data(outputResult->contentType))
		{
			return std::unexpected(HTTPErr::InvalidContentType);
		}

		if (is_binary_data(outputResult->contentType))
		{
			run_decrytion(body, outputResult->contentEncoding);
		}

		if (outputResult->connection == HTTPConnection::Close)
		{
			DEBUG_LN
				std::cout << "Connection closed after request." << std::endl;

			HTTPErr err = attempt_to_close_socket(*_socket);
			if (err != HTTPErr::None)
				return std::unexpected(err);
		}

		if (outputResult->statusCode != 200)
		{
			return std::unexpected(HTTPErr::ResponseError);
		}

		return body;
	}


	void HTTPCommunicator::set_headers(const std::unordered_map<std::string, std::string>& headers)
	{
		_headers = headers;
	}


	std::expected<std::string, HTTPErr> HTTPCommunicator::get_string(std::string_view url, const std::unordered_map<std::string, std::string>& headers)
	{
		auto outputResult = get(url, headers);
		if (!outputResult.has_value())
		{
			return std::unexpected(outputResult.error());
		}

		std::string body;
		if (std::holds_alternative<std::string>(outputResult->body))
		{
			body = std::get<std::string>(outputResult->body);
		}
		else
		{
			DEBUG_LN
				std::cerr << "Expected string body, but got binary data." << std::endl;
			return std::unexpected(HTTPErr::InvalidReadingData);
		}

		if (body.empty())
		{
			return std::unexpected(HTTPErr::InvalidData);
		}

		if (is_str_data(outputResult->contentType))
		{
			run_decrytion(body, outputResult->contentEncoding);
		}

		if (is_binary_data(outputResult->contentType))
		{
			return std::unexpected(HTTPErr::InvalidContentType);
		}

		if (outputResult->connection == HTTPConnection::Close)
		{
			DEBUG_LN
				std::cout << "Connection closed after request." << std::endl;

			HTTPErr err = attempt_to_close_socket(*_socket);
			if (err != HTTPErr::None)
				return std::unexpected(err);
		}
		
		if (outputResult->statusCode != 200)
		{
			return std::unexpected(HTTPErr::ResponseError);
		}

		return body;
	}

	std::expected<HTTPOutput, HTTPErr> HTTPCommunicator::get_http_output(std::string_view url, const std::unordered_map<std::string, std::string>& headers)
	{
		return get(url, headers);
	}

	HTTPErr HTTPCommunicator::post_bytes(std::string_view url, HTTPContent content, const std::vector<uint8_t>& body, const std::unordered_map<std::string, std::string>& headers)
	{
		auto outputResult = post(url, content, body, headers);
		if (!outputResult.has_value())
		{
			return outputResult.error();
		}

		std::vector<uint8_t> trueBody;
		if (std::holds_alternative<std::vector<uint8_t>>(outputResult->body))
		{
			trueBody = std::get<std::vector<uint8_t>>(outputResult->body);
		}
		else
		{
			DEBUG_LN
				std::cerr << "Expected string body, but got binary data." << std::endl;
			return HTTPErr::InvalidReadingData;
		}

		if (trueBody.empty())
		{
			return HTTPErr::InvalidData;
		}

		if (is_str_data(outputResult->contentType))
		{
			return HTTPErr::InvalidContentType;
		}

		if (is_binary_data(outputResult->contentType))
		{
			run_decrytion(trueBody, outputResult->contentEncoding);
		}

		if (outputResult->connection == HTTPConnection::Close)
		{
			DEBUG_LN
				std::cout << "Connection closed after request." << std::endl;

			HTTPErr err = attempt_to_close_socket(*_socket);
			if (err != HTTPErr::None)
				return err;
		}

		if (outputResult->statusCode != 200)
		{
			return HTTPErr::ResponseError;
		}

		return HTTPErr::None;
	}
	HTTPErr HTTPCommunicator::post_string(std::string_view url, HTTPContent content, std::string_view body, const std::unordered_map<std::string, std::string>& headers)
	{
		auto outputResult = post(url, content, body, headers);
		if (!outputResult.has_value())
		{
			return outputResult.error();
		}

		std::string trueBody;
		if (std::holds_alternative<std::string>(outputResult->body))
		{
			trueBody = std::get<std::string>(outputResult->body);
		}
		else
		{
			DEBUG_LN
				std::cerr << "Expected string body, but got binary data." << std::endl;
			return HTTPErr::InvalidReadingData;
		}

		if (trueBody.empty())
		{
			return HTTPErr::InvalidData;
		}

		if (is_str_data(outputResult->contentType))
		{
			run_decrytion(trueBody, outputResult->contentEncoding);
		}

		if (is_binary_data(outputResult->contentType))
		{
			return HTTPErr::InvalidContentType;
		}

		if (outputResult->connection == HTTPConnection::Close)
		{
			DEBUG_LN
				std::cout << "Connection closed after request." << std::endl;

			HTTPErr err = attempt_to_close_socket(*_socket);
			if (err != HTTPErr::None)
				return err;
		}

		if (outputResult->statusCode != 200)
		{
			return HTTPErr::ResponseError;
		}

		return HTTPErr::None;
	}

	std::expected<HTTPOutput, HTTPErr> HTTPCommunicator::post_http_output(std::string_view url, HTTPContent content, std::string_view body, const std::unordered_map<std::string, std::string>& headers)
	{
		return post(url, content, body, headers);
	}

	std::expected<HTTPOutput, HTTPErr> HTTPCommunicator::get(std::string_view url, const std::unordered_map<std::string, std::string>& headers)
	{
		HTTPErr err = check_before_sending_request(HTTPMethod::GET);
		if (err != HTTPErr::None)
		{
			return std::unexpected(err);
		}

		DEBUG_LN
			std::cout << "Using persistent connection for GET request." << std::endl;
		return ::communicator::get(_requestHost, url, _requestPort, headers, HTTPConnection::Persistent, _socket.get());

	}

	std::expected<HTTPOutput, HTTPErr> HTTPCommunicator::post(
		std::string_view url,
		HTTPContent content,
		std::string_view body,
		const std::unordered_map<std::string, std::string>& headers)
	{
		HTTPErr err = check_before_sending_request(HTTPMethod::POST);
		if (err != HTTPErr::None)
		{
			DEBUG_LN
				std::cerr << "Failed to check before sending POST request: " << static_cast<int>(err) << std::endl;
			return std::unexpected(err);
		}

		DEBUG_LN
			std::cout << "Using persistent connection for POST request." << std::endl;
		return ::communicator::post(_requestHost, url, _requestPort, content, body, headers, HTTPConnection::Persistent, _socket.get());
	}

	std::expected<HTTPOutput, HTTPErr> HTTPCommunicator::post(
		std::string_view url,
		HTTPContent content,
		const std::vector<uint8_t>& body,
		const std::unordered_map<std::string,
		std::string>& headers)
	{
		HTTPErr err = check_before_sending_request(HTTPMethod::POST);
			if (err != HTTPErr::None)
			{
				DEBUG_LN
					std::cerr << "Failed to check before sending POST request: " << static_cast<int>(err) << std::endl;
				return std::unexpected(err);
			}
		DEBUG_LN
			std::cout << "Using persistent connection for POST request." << std::endl;
		return ::communicator::post(_requestHost, url, _requestPort, content, body, headers, HTTPConnection::Persistent, _socket.get());
	}

	HTTPErr HTTPCommunicator::check_before_sending_request(HTTPMethod method)
	{
		if (!(_socket && _socket->is_open()))
		{
			DEBUG_LN
				std::cout << "Creating new connection for " << to_string(method) << " request." << std::endl;
			HTTPErr err = make_persistent_connection(_requestUrl);
			if (err != HTTPErr::None)
			{
				DEBUG_LN
					std::cerr << "Failed to make persistent connection: " << static_cast<int>(err) << std::endl;
				return err;
			}
		}

		return HTTPErr::None;
	}

	HTTPErr HTTPCommunicator::attempt_to_close_socket(asio::ip::tcp::socket& socket)
	{
		if (_socket && _socket->is_open())
		{
	
			asio::error_code ec;
			_socket->close(ec);
			if (ec)
			{
				DEBUG_LN
					std::cerr << "Error closing socket: " << ec.message() << std::endl;
				return HTTPErr::FailedToCloseSocket;
			}
		}

		return HTTPErr::None;
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


		return ::communicator::write_str_request(method, contentType, connection, host, path, body, headers);
	}


	HTTPCommunicator::~HTTPCommunicator()
	{
		if (_socket && _socket->is_open())
		{
			DEBUG_LN
				std::cout << "Closing persistent connection." << std::endl;

			std::string req = "GET / HTTP/1.1\r\nHost: " + _requestHost + "\r\nConnection: close\r\n\r\n";

			send_close_http_request(*_socket, _requestHost, _requestPort);

			attempt_to_close_socket(*_socket);
		}
	}


	
	// --- GET Methods ---

	std::expected<HTTPOutput, HTTPErr> get(
		std::string_view url,
		const std::unordered_map<std::string, std::string>& headers,
		HTTPConnection connection,
		asio::ip::tcp::socket* socket) 
	{
		auto outputResult = decrypt_url_http(url);
		if (!outputResult.has_value())
		{
			return std::unexpected(outputResult.error());
		}

		return send_http_request(HTTPMethod::GET, HTTPContent::None, outputResult->host, outputResult->path, outputResult->port, "", headers, connection, socket);
	}

	std::expected<HTTPOutput, HTTPErr> get(std::string_view host, std::string_view path, std::string_view port, const std::unordered_map<std::string, std::string>& headers, HTTPConnection connection, asio::ip::tcp::socket* socket)
	{
		return send_http_request(HTTPMethod::GET, HTTPContent::None, host, path, port, "", headers, connection, socket);
	}



	// --- POST Methods ---

	std::expected<HTTPOutput, HTTPErr> post(
		std::string_view url,
		HTTPContent content,
		std::string_view body,
		const std::unordered_map<std::string, std::string>& headers,
		HTTPConnection connection,
		asio::ip::tcp::socket* socket)
	{
		auto outputResult = decrypt_url_http(url);
		if (!outputResult.has_value())
		{
			return std::unexpected(outputResult.error());
		}

		return send_http_request(HTTPMethod::POST, content, outputResult->host, outputResult->path, outputResult->port, body, headers, connection, socket);
	}

	std::expected<HTTPOutput, HTTPErr> post(std::string_view host,
		std::string_view path,
		std::string_view port,
		HTTPContent content,
		std::string_view body,
		const std::unordered_map<std::string, std::string>& headers,
		HTTPConnection connection,
		asio::ip::tcp::socket* socket)
	{
		return send_http_request(HTTPMethod::POST, content, host, path, port, body, headers, connection, socket);
	}

	std::expected<HTTPOutput, HTTPErr> post(
		std::string_view host, 
		std::string_view path,
		std::string_view port, HTTPConnection connection, 
		HTTPContent content, const std::vector<uint8_t>& body, 
		const std::unordered_map<std::string, std::string>& headers, 
		asio::ip::tcp::socket* socket)
	{
		return send_http_request(content, host, path, port, body, headers, connection, socket);
	}


	// --- HTTP Request Sending Methods ---

	std::expected<HTTPOutput, HTTPErr> send_http_request(
		HTTPMethod method,
		HTTPContent content,
		std::string_view host,
		std::string_view path,
		std::string_view port,
		std::string_view body,
		const std::unordered_map<std::string, std::string>& extraHeaders,
		HTTPConnection connection,
		asio::ip::tcp::socket* socket)
	{
		try
		{
			auto requestResult = write_str_request(method, content, connection, host, path, body, extraHeaders);
			if (!requestResult.has_value())
			{
				return std::unexpected(requestResult.error());
			}
			if (socket && socket->is_open())
			{
				// Send the HTTP request
				asio::error_code ec;
				asio::write(*socket, asio::buffer(requestResult.value()), ec);

				if (ec)
				{
					DEBUG_LN
						std::cerr << "Error sending HTTP request: " << ec.message() << std::endl;
					return std::unexpected(HTTPErr::ConnectionFailed);
				}

				DEBUG_LN
					std::cout << "HTTP request sent: " << requestResult.value() << std::endl;
				return read_http_response(*socket);
			}
			else
			{
				asio::io_context ioContext;
				auto socketResult = create_and_connect_socket(ioContext, host, port, 10);
				if (!socketResult.has_value())
				{
					return std::unexpected(socketResult.error());
				}

				asio::write(socketResult.value(), asio::buffer(requestResult.value()));


				DEBUG_LN
					std::cout << "Raw HTTP request sent: " << requestResult.value() << std::endl;

				return read_http_response(socketResult.value());
			}
		}
		catch (const std::exception& e)
		{
			std::cerr << "Error: " << e.what() << std::endl;
			return std::unexpected(HTTPErr::ConnectionFailed);
		}

	}

	std::expected<HTTPOutput, HTTPErr> send_http_request(
		HTTPContent content, 
		std::string_view host,
		std::string_view path,
		std::string_view port, 
		const std::vector<uint8_t>& body, 
		const std::unordered_map<std::string, std::string>& extraHeaders,
		HTTPConnection connection, 
		asio::ip::tcp::socket* socket)
	{
		try
		{
			auto requestResult = write_str_request(HTTPMethod::POST, content, connection, host, path, "", extraHeaders);
			if (!requestResult.has_value())
			{
				return std::unexpected(requestResult.error());
			}
			if (socket && socket->is_open())
			{
				// Send the HTTP request
				asio::write(*socket, asio::buffer(requestResult.value()));
				asio::write(*socket, asio::buffer(body));
				DEBUG_LN
					std::cout << "HTTP request sent: " << requestResult.value() << std::endl;
				return read_http_response(*socket);
			}
			else
			{
				asio::io_context ioContext;
				auto socketResult = create_and_connect_socket(ioContext, host, port, 10);
				if (!socketResult.has_value())
				{
					return std::unexpected(socketResult.error());
				}

				asio::write(socketResult.value(), asio::buffer(requestResult.value()));
				asio::write(socketResult.value(), asio::buffer(body));


				DEBUG_LN
					std::cout << "Raw HTTP request sent: " << requestResult.value() << std::endl;

				return read_http_response(socketResult.value());
			}
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
			{
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
			else
			{
				auto socketResult = create_and_connect_socket(ioContext, host, port, 10);
				if (!socketResult.has_value())
				{
					return std::unexpected(socketResult.error());
				}
			
				asio::write(socketResult.value(), asio::buffer(request));

				DEBUG_LN
					std::cout << "Raw HTTP request sent: " << request << std::endl;

				return read_http_response(socketResult.value());
			}
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


	// --- Request Writing Methods ---

	std::expected<std::string, HTTPErr> write_str_request(HTTPMethod method, HTTPContent contentType, HTTPConnection connection, std::string_view host, std::string_view path, std::string_view body, const std::unordered_map<std::string, std::string>& headers)
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

		if (!body.empty())
		requestStream << body;

		return requestStream.str();
	}


	// --- HTTP Response Reading Method ---

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


		std::istream responseStream(&responseBuffer);

	/*	DEBUG
		(
			std::cout << "HTTP Response Stream: " << std::endl;
		std::string line;
		while (std::getline(responseStream, line) && !line.empty())
		{
			std::cout << line << std::endl;
		}
		);*/

		std::string httpVersion;
		unsigned int statusCode = 0;
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

		
		std::variant<std::string, std::vector<uint8_t>> body;

		if (transferEncoding == HTTPTransferEncoding::Chunked && contentLength == 0 && is_str_data(contentType))
		{
			std::ostringstream bodyStream;
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
			DEBUG_LN
				std::cout << "HTTP Response Body:\n" << bodyStream.str() << std::endl;
			body = bodyStream.str();
		}
		else if (contentLength > 0 && (contentType == HTTPContent::None || is_str_data(contentType)))
		{
			std::ostringstream bodyStream;
			std::string bodyContent;
			if (responseBuffer.size() > 0)
				bodyStream << &responseBuffer;

			size_t remaining = contentLength - bodyStream.str().size();

			if (remaining > 0)
				asio::read(socket, responseBuffer, asio::transfer_exactly(remaining));

			bodyStream << &responseBuffer;

			DEBUG_LN
				std::cout << "HTTP Response Body:\n" << bodyStream.str() << std::endl;
			body = bodyStream.str();

		}
		else if (is_binary_data(contentType) && responseBuffer.size() > 0)
		{
			
			std::vector<uint8_t> binaryBody(
				asio::buffers_begin(responseBuffer.data()),
				asio::buffers_end(responseBuffer.data())
			);

			DEBUG_LN
				std::cout << "HTTP Response Body (Binary Data): Size = " << binaryBody.size() << " bytes." << std::endl;

			body = std::move(binaryBody);	
		}


		return HTTPOutput{
			body,
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

	void run_decrytion(std::string& body, HTTPContentEncoding algorithm)
	{
		switch (algorithm)
		{
		case HTTPContentEncoding::None:
		case HTTPContentEncoding::Identity:
			return;
			break;

		case HTTPContentEncoding::Gzip:
			// Implement Gzip decompression -- In Process
			break;
		case HTTPContentEncoding::Deflate:
			// Implement Deflate decompression -- In Process
			break;
		case HTTPContentEncoding::Zstd:
			// Implement Zstd decompression -- In Process
			break;
		case HTTPContentEncoding::Brotli:
			// Implement Brotli decompression -- In Process
			break;
		default:
			std::cerr << "Unsupported content encoding: " << static_cast<int>(algorithm) << std::endl;
			break;
		}
	}

	void run_decrytion(std::vector<uint8_t>& body, HTTPContentEncoding algorithm)
	{
		switch (algorithm)
		{
		case HTTPContentEncoding::None:
		case HTTPContentEncoding::Identity:
			return;
			break;
		case HTTPContentEncoding::Gzip:
			// Implement Gzip decompression -- In Process
			break;
		case HTTPContentEncoding::Deflate:
			// Implement Deflate decompression -- In Process
			break;
		case HTTPContentEncoding::Zstd:
			// Implement Zstd decompression -- In Process
			break;
		case HTTPContentEncoding::Brotli:
			// Implement Brotli decompression -- In Process
			break;
		default:
			std::cerr << "Unsupported content encoding: " << static_cast<int>(algorithm) << std::endl;
			break;
		}
	}



	// --- Socket Creation and Connection Methods ---

	std::expected<asio::ip::tcp::socket, HTTPErr> create_and_connect_socket(
		asio::io_context& ioContext,
		std::string_view host,
		std::string_view port,
		size_t requestTimeout)		
		// Remember to move the result into the socket variable
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



	// --- Validation Methods ---

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

		return HTTPErr::None;
	}

	std::string istream_to_string(std::istream& stream)
	{
		std::streampos currentPos = stream.tellg();
		if (currentPos == -1)
			return "";  

		stream.seekg(0, std::ios::end);
		std::streampos endPos = stream.tellg();

		if (endPos == -1)
			return ""; 

		std::streamsize size = endPos - currentPos;

		std::string content;
		if (size > 0)
		{
			content.resize(size);
			stream.seekg(currentPos);
			stream.read(&content[0], size);
			stream.clear();
			stream.seekg(currentPos);
		}

		return content;
	}

	bool is_str_data(HTTPContent content)
	{
		return content == HTTPContent::TextPlain ||
			content == HTTPContent::TextHTML ||
			content == HTTPContent::ApplicationJSON ||
			content == HTTPContent::ApplicationXML ||
			content == HTTPContent::ApplicationFormUrlEncoded ||
			content == HTTPContent::ApplicationJavaScript ||
			content == HTTPContent::TextCSS;
	}

	bool is_binary_data(HTTPContent content)
	{
		return content == HTTPContent::ImageJPEG ||
			content == HTTPContent::ImagePNG ||
			content == HTTPContent::ImageGIF ||
			content == HTTPContent::ApplicationOctetStream;
	}


	std::expected<HTTPOutput, HTTPErr> post(
		std::string_view host,
		std::string_view path,
		std::string_view port,
		HTTPContent content,
		const std::vector<uint8_t>& body,
		const std::unordered_map<std::string, std::string>& headers,
		HTTPConnection connection,
		asio::ip::tcp::socket* socket)
	{
		return send_http_request(content, host, path, port, body, headers, connection, socket);
	}

	
	std::expected<HTTPOutput, HTTPErr> post(
		std::string_view url,
		HTTPContent content,
		const std::vector<uint8_t>& body,
		const std::unordered_map<std::string, std::string>& headers,
		HTTPConnection connection,
		asio::ip::tcp::socket* socket)
	{
		auto outputResult = decrypt_url_http(url);
		if (!outputResult.has_value())
		{
			return std::unexpected(outputResult.error());
		}
		return send_http_request(content, outputResult->host, outputResult->path, outputResult->port, body, headers, connection, socket);
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


	// -- Helper Methods --

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
}