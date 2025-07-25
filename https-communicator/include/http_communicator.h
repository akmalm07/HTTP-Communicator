#pragma once

#include <asio.hpp>
#include <expected>


namespace communicator
{
	enum class HTTPMethod : uint32_t
	{
		GET,
		POST,
		PUT,
		DEL,
		PATCH,
		HEAD,
		OPTIONS
	};

	enum class HTTPConnection : uint32_t
	{
		Persistent, 
		Close,      
		Upgrade     
	};

	enum class HTTPContent : uint32_t
	{
		None,
		TextPlain,
		TextHTML,
		TextCSS,
		TextJavaScript,
		ApplicationJSON,
		ApplicationXML,
		ApplicationFormUrlEncoded,
		ImagePNG,
		ImageJPEG,
		ImageGIF,
	//	ApplicationOctetStream,
	//	ApplicationJavaScript
	};

	enum class HTTPErr : uint32_t
	{
		None = 0,
		DNSResolutionFailed,
		ConnectionFailed,
		RequestTimeout,
		ResponseError,
		InvalidURL,
		RedirectLimitExceeded,
		InvalidContentSize,
		UnsupportedTransferEncoding,
		NoBodyForMethod,

		//Temporary errors
		ChunkedEncodingNotSupported

	};

	enum class ParseHTTP : uint32_t
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

	struct HTTPOutput
	{
		std::string body;
		HTTPContent contentType;
		HTTPConnection connection;
		size_t contentLength;
		unsigned int statusCode;
		std::string statusMessage;
	};

	class HTTPCommunicator
	{
	public:

		HTTPCommunicator(const std::unordered_map<std::string, std::string>& headers, size_t requestTimeout);

		virtual HTTPErr make_persistent_connection(std::string_view url, const std::unordered_map<std::string, std::string>& extraHeaders = {});

		virtual std::expected<std::string, HTTPErr> get(std::string_view url);
		virtual std::expected<std::string, HTTPErr> post(std::string_view url, HTTPContent content, std::string_view body, const std::unordered_map<std::string, std::string>& headers = {});

		virtual void set_headers(const std::unordered_map<std::string, std::string>& headers);
		virtual void set_proxy(std::string_view proxyHost, uint16_t proxyPort);

	private:

		std::unordered_map<std::string, std::string> _headers;
		
		size_t _requestTimeout = 30;
		int _maxRedirects = 5;
		bool _followRedirects = true;

		asio::io_context _persistentIoContext;

		std::unique_ptr<asio::ip::tcp::socket> _socket = nullptr;



		std::string _proxyHost = "";
		uint16_t _proxyPort = 0;

	private:


		std::expected<std::string, HTTPErr> send_http_request(
			HTTPMethod method, 
			HTTPContent content,
			std::string_view host,
			std::string_view path,
			std::string_view port,
			std::string_view body = "",
			const std::unordered_map<std::string, std::string>& extraHeaders = {});

		std::expected<std::string, HTTPErr> write_headers(
			HTTPMethod method,
			HTTPConnection connenction,
			HTTPContent contentType,
			std::string_view host,
			std::string_view path,
			std::string_view body = "",
			const std::unordered_map<std::string, std::string>& extraHeaders = {});

		std::expected<HTTPOutput, HTTPErr> read_http_response(asio::ip::tcp::socket& socket, HTTPMethod method);

		std::expected<asio::ip::tcp::socket, HTTPErr> create_and_connect_socket(asio::io_context& ioContext, std::string_view host, std::string_view port);

	};

	//std::string parse_http(const std::string& response, ParseHTTP type);


	std::string to_string(HTTPMethod method);
	
	std::string to_string(HTTPConnection method);
	
	std::string to_string(HTTPContent content);

	template<>
	uint32_t to_uint32(std::string_view method);


	

	std::expected<URLDecriptorOutput, HTTPErr> decrypt_url_http(std::string_view url);

}

