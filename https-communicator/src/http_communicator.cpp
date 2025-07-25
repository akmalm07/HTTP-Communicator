#include "headers.h"
#include "http_communicator.h"


namespace communicator
{
	HTTPCommunicator::HTTPCommunicator(const std::unordered_map<std::string, std::string>& headers, size_t requestTimeout)
		: _headers(headers), _requestTimeout(requestTimeout)
	{

	}

	HTTPErr HTTPCommunicator::make_persistent_connection(std::string_view url, const std::unordered_map<std::string, std::string>& extraHeaders)
	{
		if (_socket && _socket->is_open())
		{
			std::cout << "Persistent connection already established." << std::endl;
			return HTTPErr::None;
		}

		auto output = decrypt_url_http(url);
		if (!output.has_value())
		{
			std::cerr << "Invalid URL: " << url << std::endl;
			return HTTPErr::InvalidURL;
		}

		auto socketResult = create_and_connect_socket(_persistentIoContext, output->host, output->port);
		if (!socketResult.has_value())
		{
			return socketResult.error();
		}

		auto requestResult = write_headers(HTTPMethod::GET, HTTPConnection::Close, HTTPContent::None, output->host, output->path, "", extraHeaders);
		if (!requestResult.has_value())
		{
			return requestResult.error();
		}
		
		_socket = std::make_unique<asio::ip::tcp::socket>(std::move(socketResult.value()));

		// Send the HTTP request
		asio::write(*_socket, asio::buffer(requestResult.value()));

		std::cout << "Persistent connection established to: " << output->host << ":" << output->port << std::endl;
		std::cout << "Path: " << output->path << std::endl;

		return HTTPErr::None;
	}

	std::expected<std::string, HTTPErr> HTTPCommunicator::get(std::string_view url)
	{
		auto output = decrypt_url_http(url);
		if (!output.has_value())
		{
			return std::unexpected(output.error());
		}
		return send_http_request(HTTPMethod::GET, HTTPContent::None, output->host, output->path, output->port, "");
	}

	std::expected<std::string, HTTPErr> HTTPCommunicator::post(std::string_view url, HTTPContent content, std::string_view body, const std::unordered_map<std::string, std::string>& headers)
	{
		auto output = decrypt_url_http(url);
		if (!output.has_value())
		{
			return std::unexpected(output.error());
		}
		return send_http_request(HTTPMethod::POST, content, output->host, output->path, output->port, body);
	
	}

	void HTTPCommunicator::set_headers(const std::unordered_map<std::string, std::string>& headers)
	{
		_headers = headers;
	}

	void HTTPCommunicator::set_proxy(std::string_view proxyHost, uint16_t proxyPort)
	{
		_proxyHost = std::string(proxyHost);
		_proxyPort = proxyPort;
		if (_proxyHost.empty() || _proxyPort == 0)
		{
			std::cerr << "Invalid proxy settings." << std::endl;
			return;
		}
		std::cout << "Proxy set to: " << _proxyHost << ":" << _proxyPort << std::endl;
	}


	std::expected<std::string, HTTPErr> HTTPCommunicator::send_http_request(HTTPMethod method, HTTPContent content, std::string_view host, std::string_view path, std::string_view port, std::string_view body, const std::unordered_map<std::string, std::string>& extraHeaders)
	{
		try
		{
			asio::io_context ioContext;
		
			auto socketResult = create_and_connect_socket(ioContext, host, port);
			if (!socketResult.has_value())
			{
				return std::unexpected(socketResult.error());
			}
			asio::ip::tcp::socket socket = std::move(socketResult.value());

			auto requestResult = write_headers(method, HTTPConnection::Close, content, host, path, body, extraHeaders);
			if (!requestResult.has_value())
			{
				return std::unexpected(requestResult.error());
			}
			// Send the HTTP request
			asio::write(socket, asio::buffer(requestResult.value()));

			std::cout << "HTTP request sent: " << requestResult.value() << std::endl;

			return read_http_response_body(socket);
		}
		catch (const std::exception& e)
		{
			std::cerr << "Error: " << e.what() << std::endl;
			return std::unexpected(HTTPErr::ConnectionFailed);
		}

	}

	std::expected<std::string, HTTPErr> HTTPCommunicator::write_headers(HTTPMethod method, HTTPConnection connection, HTTPContent contentType, std::string_view host, std::string_view path, std::string_view body, const std::unordered_map<std::string, std::string>& extraHeaders)
	{
		std::ostringstream requestStream;
		requestStream << to_string(method) << " " << path << " HTTP/1.1\r\n";
		requestStream << "Host: " << host << "\r\n";
		for (const auto& header : _headers)
		{
			requestStream << header.first << ": " << header.second << "\r\n";
		}

		for (const auto& header : extraHeaders)
		{
			requestStream << header.first << ": " << header.second << "\r\n";
		}

		requestStream << "Connection: " << to_string(connection) << "\r\n\r\n";

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

	std::expected<HTTPOutput, HTTPErr> HTTPCommunicator::read_http_response(asio::ip::tcp::socket& socket, HTTPMethod method)
	{
		// Read the HTTP response
		asio::streambuf responseBuffer;
		asio::read_until(socket, responseBuffer, "\r\n\r\n");

		// Parse status line
		std::istream responseStream(&responseBuffer);
		std::string httpVersion;
		unsigned int statusCode;
		std::string statusMessage;

		responseStream >> httpVersion >> statusCode;
		std::getline(responseStream, statusMessage);

		if (statusCode != 200)
			return std::unexpected(HTTPErr::ResponseError);

		if (method == HTTPMethod::HEAD)
		{
			return std::unexpected(HTTPErr::NoBodyForMethod);
		}

		// Read headers
		std::string header;
		size_t contentLength = 0;

		HTTPContent contentType = HTTPContent::None;

		HTTPConnection connection = HTTPConnection::Close;

		while (std::getline(responseStream, header) && header != "\r")
		{

			if (header.starts_with("Transfer-Encoding:"))
			{
				if (header.substr(19) == "chunked")
				{
					return std::unexpected(HTTPErr::ChunkedEncodingNotSupported);
				}
				else if (header.substr(19) == "identity")
				{
					contentType = HTTPContent::TextPlain; 
				}
				else
				{
					return std::unexpected(HTTPErr::UnsupportedTransferEncoding);
				}
			}

			if (header.starts_with("Content-Type:"))
			{
				if (header.substr(14).find("text/html") != std::string::npos)
				{
					contentType = HTTPContent::TextHTML;
				}
				else if (header.substr(14).find("text/css") != std::string::npos)
				{
					contentType = HTTPContent::TextCSS; 
				}
				else if (header.substr(14).find("application/json") != std::string::npos)
				{
					contentType = HTTPContent::ApplicationJSON;
				}
				else if (header.substr(14).find("application/xml") != std::string::npos)
				{
					contentType = HTTPContent::ApplicationXML;
				}
				else if (header.substr(14).find("application/x-www-form-urlencoded") != std::string::npos)
				{
					contentType = HTTPContent::ApplicationFormUrlEncoded;
				}
				else if (header.substr(14).find("image/png") != std::string::npos)
				{
					contentType = HTTPContent::ImagePNG;
				}
				else if (header.substr(14).find("image/jpeg") != std::string::npos)
				{
					contentType = HTTPContent::ImageJPEG;
				}
				else if (header.substr(14).find("image/gif") != std::string::npos)
				{
					contentType = HTTPContent::ImageGIF;
				}
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

			if (header.starts_with("Connection:"))
			{
				if (header.substr(12) == "keep-alive")
				{
					connection = HTTPConnection::Persistent;
				}
				else if (header.substr(12) == "upgrade")
				{
					connection = HTTPConnection::Upgrade;
				}
			}

			if (header.empty())
				break; 
		}

		if (contentLength == 0)
			return std::unexpected(HTTPErr::InvalidContentSize);

		std::string bodyContent;
		std::ostringstream bodyStream;
		if (responseBuffer.size() > 0)
			bodyStream << &responseBuffer;

		size_t remaining = contentLength - bodyStream.str().size();

		if (remaining > 0)
			asio::read(socket, responseBuffer, asio::transfer_exactly(remaining));

		bodyStream << &responseBuffer;

		return HTTPOutput{
			bodyStream.str(),
			contentType,
			connection,
			contentLength,
			statusCode,
			statusMessage
		};
	}

	std::expected<asio::ip::tcp::socket, HTTPErr> HTTPCommunicator::create_and_connect_socket(asio::io_context& ioContext, std::string_view host, std::string_view port)
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
				if (ec)
				{
					std::cerr << "Connection failed: " << ec.message() << std::endl;
					return;
				}
				connectStatus = true;
				timer.cancel();
			}
		);

		timer.expires_after(std::chrono::seconds(_requestTimeout));

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



	std::string parse_http(const std::string& response, ParseHTTP type)
	{
		std::istringstream stream(response);
		std::string line;

		while (std::getline(stream, line))
		{
			if (line.empty())
				break;

			switch (type)
			{
			case communicator::ParseHTTP::ContentLength:
				if (line.starts_with("Content-Length:"))
					return line.substr(16);
				break;
			case communicator::ParseHTTP::ContentType:
				if (line.starts_with("Content-Type:"))
					return line.substr(14);
				break;
			case communicator::ParseHTTP::TransferEncoding:
				if (line.starts_with("Transfer-Encoding:"))
					return line.substr(19);
				break;
			case communicator::ParseHTTP::Connection:
				if (line.starts_with("Connection:"))
					return line.substr(12);
				break;
			case communicator::ParseHTTP::StatusMessage:
				if (line.starts_with("HTTP/"))
				{
					auto pos = line.find(' ');
					if (pos != std::string::npos)
						return line.substr(pos + 1);
				}
				break;
			case communicator::ParseHTTP::StatusCode:
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


	std::string to_string(HTTPMethod method)
	{
		switch (method)
		{
		case HTTPMethod::GET: return "GET";
		case HTTPMethod::POST: return "POST";
		case HTTPMethod::PUT: return "PUT";
		case HTTPMethod::DEL: return "DELETE";
		case HTTPMethod::PATCH: return "PATCH";
		case HTTPMethod::HEAD: return "HEAD";
		case HTTPMethod::OPTIONS: return "OPTIONS";
		default: return "";
		}
	}

	std::string to_string(HTTPConnection method)
	{
		switch (method)
		{
		case HTTPConnection::Persistent: return "keep-alive";
		case HTTPConnection::Close: return "close";
		case HTTPConnection::Upgrade: return "upgrade";
		default: return "";
		}
	}

	std::string to_string(HTTPContent content)
	{
		switch (content)
		{
		case HTTPContent::None: return "text/plain";
		case HTTPContent::TextPlain: return "text/plain";
		case HTTPContent::TextHTML: return "text/html";
		case HTTPContent::ApplicationJSON: return "application/json";
		case HTTPContent::ApplicationXML: return "application/xml";
		case HTTPContent::ApplicationFormUrlEncoded: return "application/x-www-form-urlencoded";
		case HTTPContent::ImagePNG: return "image/png";
		case HTTPContent::ImageJPEG: return "image/jpeg";
		case HTTPContent::ImageGIF: return "image/gif";
		default: return "";
		}
	}

	std::expected<URLDecriptorOutput, HTTPErr> decrypt_url_http(std::string_view url)
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
		return URLDecriptorOutput{ hostStr, path, port };
	}
}