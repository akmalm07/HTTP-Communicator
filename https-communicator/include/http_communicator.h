#pragma once

#include <asio.hpp>
#include <expected>


namespace communicator
{
	enum class HTTPMethod
	{
		GET,
		POST,
		PUT,
		DEL,
		PATCH,
		HEAD,
		OPTIONS
	};

	enum class HTTPErr
	{
		None = 0,
		DNSResolutionFailed,
		ConnectionFailed,
		RequestTimeout,
		ResponseError,
		InvalidURL,
		RedirectLimitExceeded,
		InvalidContentSize,

	};

	enum class ParseHTTP
	{
		StatusCode,
		StatusMessage,
		ContentLength,
		ContentType,
		TransferEncoding,
		Connection,
	};

	struct URLDecriptorOutput
	{
		std::string host;
		std::string path;
		std::string port;
	};


	class HTTPCommunicator
	{
	public:

		HTTPCommunicator(const std::unordered_map<std::string, std::string>& headers, size_t requestTimeout);

		virtual std::expected<std::string, HTTPErr> get(std::string_view url);
		virtual std::expected<std::string, HTTPErr> post(std::string_view url, std::string_view body, const std::unordered_map<std::string, std::string>& headers = {});

		virtual void set_headers(const std::unordered_map<std::string, std::string>& headers);
		virtual void set_proxy(std::string_view proxyHost, uint16_t proxyPort);

	private:

		std::unordered_map<std::string, std::string> _headers;
		
		size_t _requestTimeout = 30;
		int _maxRedirects = 5;
		bool _followRedirects = true;

		std::string _proxyHost = "";
		uint16_t _proxyPort = 0;

	private:


		std::expected<std::string, HTTPErr> send_http_request(
			HTTPMethod method,
			std::string_view host,
			std::string_view path,
			std::string_view port,
			std::string_view body = "",
			const std::unordered_map<std::string, std::string>& extra_headers = {});

		std::expected<std::string, HTTPErr> read_http_response_body(asio::ip::tcp::socket& socket);

	};

	//std::string parse_http(const std::string& response, ParseHTTP type);


	std::string to_string(HTTPMethod method);
	

	std::expected<URLDecriptorOutput, HTTPErr> decrypt_url_http(std::string_view url);

}

