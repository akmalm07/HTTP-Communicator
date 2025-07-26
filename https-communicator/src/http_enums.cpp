#include "headers.h"
#include "http_enums.h"

namespace communicator
{

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

	std::string to_string(HTTPErr err)
	{
		switch (err)
		{
		case HTTPErr::None: return "No Error";
		case HTTPErr::DNSResolutionFailed: return "DNS Resolution Failed";
		case HTTPErr::ConnectionFailed: return "Connection Failed";
		case HTTPErr::RequestTimeout: return "Request Timeout";
		case HTTPErr::ResponseError: return "Response Error";
		case HTTPErr::InvalidURL: return "Invalid URL";
		case HTTPErr::RedirectLimitExceeded: return "Redirect Limit Exceeded";
		case HTTPErr::InvalidContentSize: return "Invalid Content Size";
		case HTTPErr::UnsupportedTransferEncoding: return "Unsupported Transfer Encoding";
		case HTTPErr::NoBodyForMethod: return "No Body For Method";
		case HTTPErr::ChunkedEncodingNotSupported: return "Chunked Encoding Not Supported";
		default: return "Unknown Error";
		}
	}
}