#include "headers.h"
#include "http_communicator.h"


namespace communicator
{
	HTTPCommunicator::HTTPCommunicator(const std::unordered_map<std::string, std::string>& headers, size_t requestTimeout)
		: _headers(headers), _requestTimeout(requestTimeout)
	{

	}

	std::expected<std::string, HTTPErr> HTTPCommunicator::get(std::string_view url)
	{
		auto output = decrypt_url_http(url);
		if (!output.has_value())
		{
			return std::unexpected(output.error());
		}
		return send_http_request(HTTPMethod::GET, output->host, output->path, output->port, "");
	}

	std::expected<std::string, HTTPErr> HTTPCommunicator::post(std::string_view url, std::string_view body, const std::unordered_map<std::string, std::string>& headers)
	{
		auto output = decrypt_url_http(url);
		if (!output.has_value())
		{
			return std::unexpected(output.error());
		}
		return send_http_request(HTTPMethod::GET, output->host, output->path, output->port, "");
	
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


	std::expected<std::string, HTTPErr> HTTPCommunicator::send_http_request(HTTPMethod method, std::string_view host, std::string_view path, std::string_view port, std::string_view body, const std::unordered_map<std::string, std::string>& extraHeaders)
	{
		try
		{
			asio::io_context ioContext;


			asio::ip::tcp::resolver resolver(ioContext);
			
			asio::error_code ec;
			asio::ip::tcp::resolver::results_type endpoint = resolver.resolve(host, port, ec);

			asio::steady_timer timer(ioContext);

			bool connectStatus = false;
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

			requestStream << "Content-Length: " << body.size() << "\r\n";
			requestStream << "Connection: close\r\n\r\n";
			requestStream << body;
			
			// Send the HTTP request
			asio::write(socket, asio::buffer(requestStream.str()));

			return read_http_response_body(socket);
		}
		catch (const std::exception& e)
		{
			std::cerr << "Error: " << e.what() << std::endl;
			return std::unexpected(HTTPErr::ConnectionFailed);
		}

	}

	std::expected<std::string, HTTPErr> HTTPCommunicator::read_http_response_body(asio::ip::tcp::socket& socket)
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

		// Read headers
		std::string header;
		size_t contentLength = 0;
		while (std::getline(responseStream, header) && header != "\r")
		{
			if (header.starts_with("Content-Length:"))
			{
				try {
					contentLength = std::stoul(header.substr(15));
				}
				catch (...) {
					return std::unexpected(HTTPErr::InvalidContentSize);
				}
			}
		}

		if (contentLength == 0)
			return std::unexpected(HTTPErr::InvalidContentSize);

		// Read remaining body (some may have been read with headers)
		std::string bodyContent;
		std::ostringstream bodyStream;
		if (responseBuffer.size() > 0)
			bodyStream << &responseBuffer;

		size_t remaining = contentLength - bodyStream.str().size();

		if (remaining > 0)
			asio::read(socket, responseBuffer, asio::transfer_exactly(remaining));

		bodyStream << &responseBuffer;

		return bodyStream.str();
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