#pragma once

#include <asio.hpp>
#include <expected>
#include <vector>
#include <variant>

#include "http_enums.h"

namespace communicator
{

	struct URLDescriptorOutput // URLDescriptorOutput
	{
		std::string host;
		std::string path;
		std::string port;
	};

	struct HTTPOutput
	{
		std::variant<std::string, std::vector<uint8_t>> body;
		HTTPContent contentType;
		HTTPConnection connection;
		HTTPTransferEncoding transferEncoding;
		HTTPContentEncoding contentEncoding;
		HTTPLanguage language;
		size_t contentLength;
		unsigned int statusCode;
		std::string statusMessage;
	};

	class HTTPCommunicator
	{
	public:

		HTTPCommunicator(std::string_view url, const std::unordered_map<std::string, std::string>& headers, size_t requestTimeout);

		virtual HTTPErr make_persistent_connection(std::string_view url, const std::unordered_map<std::string, std::string>& extraHeaders = {});

		virtual std::expected<std::vector<uint8_t>, HTTPErr> get_bytes(std::string_view url = "/", const std::unordered_map<std::string, std::string>& headers = {});

		virtual std::expected<std::string, HTTPErr> get_string(std::string_view url = "/", const std::unordered_map<std::string, std::string>& headers = {});

		virtual std::expected<HTTPOutput, HTTPErr> get_http_output(std::string_view url = "/", const std::unordered_map<std::string, std::string>& headers = {});


		virtual HTTPErr post_bytes(
			std::string_view url,
			HTTPContent content,
			const std::vector<uint8_t>& body,
			const std::unordered_map<std::string, std::string>& headers = {});

		virtual HTTPErr post_string(
			std::string_view url,
			HTTPContent content,
			std::string_view body,
			const std::unordered_map<std::string, std::string>& headers = {});

		virtual std::expected<HTTPOutput, HTTPErr> post_http_output(
			std::string_view url,
			HTTPContent content,
			std::string_view body,
			const std::unordered_map<std::string, std::string>& headers = {}
		);

		virtual void set_headers(const std::unordered_map<std::string, std::string>& headers);
		//virtual void set_proxy(std::string_view proxyHost, uint16_t proxyPort);

		virtual ~HTTPCommunicator();

	private:

		std::unordered_map<std::string, std::string> _headers;
		
		std::string _requestUrl;

		std::string _requestHost;
		std::string _requestPort; 

		size_t _requestTimeout = 30;
		int _maxRedirects = 5;
		bool _followRedirects = true;

		asio::io_context _persistentIoContext;

		std::unique_ptr<asio::ip::tcp::socket> _socket;

		std::string _proxyHost = "";
		uint16_t _proxyPort = 0;

	private:

		std::expected<HTTPOutput, HTTPErr> send_raw_http_request(std::string_view request, std::string_view path = "/");

		virtual std::expected<HTTPOutput, HTTPErr> get(std::string_view url, const std::unordered_map<std::string, std::string>& headers = {});


		virtual std::expected<HTTPOutput, HTTPErr> post(
			std::string_view url, 
			HTTPContent content, 
			std::string_view body, 
			const std::unordered_map<std::string, std::string>& headers = {});
		

		virtual std::expected<HTTPOutput, HTTPErr> post(
			std::string_view url, 
			HTTPContent content, 
			const std::vector<uint8_t>& body,
			const std::unordered_map<std::string, 
			std::string>& headers = {});

		HTTPErr check_before_sending_request(HTTPMethod method);


		std::expected<std::string, HTTPErr> write_headers(
			HTTPMethod method,
			HTTPConnection connenction,  
			HTTPContent contentType,
			std::string_view host,
			std::string_view path,
			std::string_view body = "",
			const std::unordered_map<std::string, std::string>& extraHeaders = {});

		
		HTTPErr attempt_to_close_socket(asio::ip::tcp::socket& socket);
	};


	// --- GET Methods ---

	std::expected<HTTPOutput, HTTPErr> get(
		std::string_view url, 
		const std::unordered_map<std::string, std::string>& headers = {},
		HTTPConnection connection = HTTPConnection::Close,
		asio::ip::tcp::socket* socket = nullptr);
	

	std::expected<HTTPOutput, HTTPErr> get(
		std::string_view host,
		std::string_view path, std::string_view port, 
		const std::unordered_map<std::string, std::string>& headers = {},
		HTTPConnection connection = HTTPConnection::Close, 
		asio::ip::tcp::socket* socket = nullptr);
	

	// --- POST Methods ---


	std::expected<HTTPOutput, HTTPErr> post(
		std::string_view url,
		HTTPContent content,
		std::string_view body, 
		const std::unordered_map<std::string, std::string>& headers = {},
		HTTPConnection connection = HTTPConnection::Close,
		asio::ip::tcp::socket* socket = nullptr);

	
	std::expected<HTTPOutput, HTTPErr> post(
		std::string_view host,
		std::string_view path, 
		std::string_view port,
		HTTPContent content, 
		std::string_view body,
		const std::unordered_map<std::string, std::string>& headers = {},
		HTTPConnection connection = HTTPConnection::Close,
		asio::ip::tcp::socket* socket = nullptr);

		// -- POST Methods with Byte Data --

	std::expected<HTTPOutput, HTTPErr> post(
		std::string_view host,
		std::string_view path,
		std::string_view port,
		HTTPContent content,
		const std::vector<uint8_t>& body,
		const std::unordered_map<std::string, std::string>& headers = {},
		HTTPConnection connection = HTTPConnection::Close,
		asio::ip::tcp::socket* socket = nullptr);


	std::expected<HTTPOutput, HTTPErr> post(
		std::string_view url,
		HTTPContent content,
		const std::vector<uint8_t>& body,
		const std::unordered_map<std::string, std::string>& headers = {},
		HTTPConnection connection = HTTPConnection::Close,
		asio::ip::tcp::socket* socket = nullptr);


	// --- Decryption ---

	std::expected<URLDescriptorOutput, HTTPErr> decrypt_url_http(std::string_view url);

	void run_decrytion(std::string& body, HTTPContentEncoding algorithm);

	void run_decrytion(std::vector<uint8_t>& body, HTTPContentEncoding algorithm);


	// --- Encryption ---

	template <typename T>
	std::vector<uint8_t> byteinize(const T& data);


	// --- HTTP Request Sending Methods ---
	
	std::expected<HTTPOutput, HTTPErr> send_http_request(
		HTTPMethod method,
		HTTPContent content,
		std::string_view host,
		std::string_view path,
		std::string_view port,
		std::string_view body = "",
		const std::unordered_map<std::string, std::string>& extraHeaders = {},
		HTTPConnection connection = HTTPConnection::Close,
		asio::ip::tcp::socket* socket = nullptr);


	std::expected<HTTPOutput, HTTPErr> send_http_request( // For sending POST requests with byte data
		HTTPContent content,
		std::string_view host,
		std::string_view path,
		std::string_view port,
		const std::vector<uint8_t>& body,
		const std::unordered_map<std::string, std::string>& extraHeaders = {},
		HTTPConnection connection = HTTPConnection::Close,
		asio::ip::tcp::socket* socket = nullptr);


	std::expected<HTTPOutput, HTTPErr> send_raw_http_request(
		std::string_view host, 
		std::string_view path, 
		std::string_view port, 
		std::string_view request, 
		asio::ip::tcp::socket* socket = nullptr);

	void send_close_http_request(asio::ip::tcp::socket& socket,std::string_view host, std::string_view port, std::string_view path = "/");


	// --- Request Writing Methods ---

	std::expected<HTTPOutput, HTTPErr> read_http_response(asio::ip::tcp::socket& socket);


	// --- Helper Methods ---

	std::expected<std::string, HTTPErr> write_str_request(
		HTTPMethod method,
		HTTPContent contentType,
		HTTPConnection connenction,
		std::string_view host,
		std::string_view path,
		std::string_view body = "",
		const std::unordered_map<std::string, std::string>& extraHeaders = {});

	std::expected<asio::ip::tcp::socket, HTTPErr> create_and_connect_socket(
		// Remember to move the result into the socket variable
		asio::io_context& ioContext,
		std::string_view host,
		std::string_view port, 
		size_t requestTimeout);

	HTTPErr is_valid_http_request(std::string_view request);

	std::string istream_to_string(std::istream& stream);

	bool is_str_data(HTTPContent content);

	bool is_binary_data(HTTPContent content);
	//std::string parse_http(const std::string& response, HTTPParse type);

}



template<typename T>
std::vector<uint8_t> communicator::byteinize(const T& data)
// Converts a data structure to a vector of bytes
{
	static_assert(std::is_trivially_copyable_v<T>, "T must be trivially copyable to safely byteinize");

	if (sizeof(T) == 0)
		return std::vector<uint8_t>();

	const uint8_t* begin = reinterpret_cast<const uint8_t*>(&data);
	const uint8_t* end = begin + sizeof(T);

	return std::vector<uint8_t>(begin, end);
}