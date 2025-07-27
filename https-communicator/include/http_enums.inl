#pragma once

#include "http_enums.h"

namespace communicator
{
	template<>
	inline uint32_t to_uint32<HTTPParse>(std::string_view method)
	{
		if (method.find("Status-Code") != std::string_view::npos)
			return static_cast<uint32_t>(HTTPParse::StatusCode);
		else if (method.find("Status-Message") != std::string_view::npos)
			return static_cast<uint32_t>(HTTPParse::StatusMessage);
		else if (method.find("Content-Length") != std::string_view::npos)
			return static_cast<uint32_t>(HTTPParse::ContentLength);
		else if (method.find("Content-Type") != std::string_view::npos)
			return static_cast<uint32_t>(HTTPParse::ContentType);
		else if (method.find("Transfer-Encoding") != std::string_view::npos)
			return static_cast<uint32_t>(HTTPParse::TransferEncoding);
		else if (method.find("Connection") != std::string_view::npos)
			return static_cast<uint32_t>(HTTPParse::Connection);
		return 0;
	}

	template<>
	inline uint32_t to_uint32<HTTPMethod>(std::string_view method)
	{
		if (method.find("GET") != std::string_view::npos)
			return static_cast<uint32_t>(HTTPMethod::GET);
		else if (method.find("POST") != std::string_view::npos)
			return static_cast<uint32_t>(HTTPMethod::POST);
		else if (method.find("PUT") != std::string_view::npos)
			return static_cast<uint32_t>(HTTPMethod::PUT);
		else if (method.find("DEL") != std::string_view::npos)
			return static_cast<uint32_t>(HTTPMethod::DEL);
		else if (method.find("PATCH") != std::string_view::npos)
			return static_cast<uint32_t>(HTTPMethod::PATCH);
		else if (method.find("HEAD") != std::string_view::npos)
			return static_cast<uint32_t>(HTTPMethod::HEAD);
		else if (method.find("OPTIONS") != std::string_view::npos)
			return static_cast<uint32_t>(HTTPMethod::OPTIONS);
		return 0;
	}

	template<>
	inline uint32_t to_uint32<HTTPConnection>(std::string_view method)
	{
		if (method.find("keep-alive") != std::string_view::npos)
			return static_cast<uint32_t>(HTTPConnection::Persistent);
		else if (method.find("close") != std::string_view::npos)
			return static_cast<uint32_t>(HTTPConnection::Close);
		else if (method.find("upgrade") != std::string_view::npos)
			return static_cast<uint32_t>(HTTPConnection::Upgrade);
	}

	template<>
	inline uint32_t to_uint32<HTTPContent>(std::string_view content)
	{
		if (content.find("None") != std::string_view::npos)
			return static_cast<uint32_t>(HTTPContent::None);
		else if (content.find("text/plain") != std::string_view::npos)
			return static_cast<uint32_t>(HTTPContent::TextPlain);
		else if (content.find("text/html") != std::string_view::npos)
			return static_cast<uint32_t>(HTTPContent::TextHTML);
		else if (content.find("text/css") != std::string_view::npos)
			return static_cast<uint32_t>(HTTPContent::TextCSS);
		else if (content.find("text/javascript") != std::string_view::npos)
			return static_cast<uint32_t>(HTTPContent::ApplicationJavaScript);
		else if (content.find("application/json") != std::string_view::npos)
			return static_cast<uint32_t>(HTTPContent::ApplicationJSON);
		else if (content.find("application/xml") != std::string_view::npos)
			return static_cast<uint32_t>(HTTPContent::ApplicationXML);
		else if (content.find("application/x-www-form-urlencoded") != std::string_view::npos)
			return static_cast<uint32_t>(HTTPContent::ApplicationFormUrlEncoded);
		else if (content.find("image/png") != std::string_view::npos)
			return static_cast<uint32_t>(HTTPContent::ImagePNG);
		else if (content.find("image/jpeg") != std::string_view::npos)
			return static_cast<uint32_t>(HTTPContent::ImageJPEG);
		else if (content.find("image/gif") != std::string_view::npos)
			return static_cast<uint32_t>(HTTPContent::ImageGIF);
		return 0;
	}

	template<>
	inline uint32_t to_uint32<HTTPErr>(std::string_view err)
	{
		if (err.find("None") != std::string_view::npos)
			return static_cast<uint32_t>(HTTPErr::None);
		else if (err.find("DNS Resolution Failed") != std::string_view::npos)
			return static_cast<uint32_t>(HTTPErr::DNSResolutionFailed);
		else if (err.find("Connection Failed") != std::string_view::npos)
			return static_cast<uint32_t>(HTTPErr::ConnectionFailed);
		else if (err.find("Request Timeout") != std::string_view::npos)
			return static_cast<uint32_t>(HTTPErr::RequestTimeout);
		else if (err.find("Response Error") != std::string_view::npos)
			return static_cast<uint32_t>(HTTPErr::ResponseError);
		else if (err.find("Invalid URL") != std::string_view::npos)
			return static_cast<uint32_t>(HTTPErr::InvalidURL);
		else if (err.find("Redirect Limit Exceeded") != std::string_view::npos)
			return static_cast<uint32_t>(HTTPErr::RedirectLimitExceeded);
		else if (err.find("Invalid Content Size") != std::string_view::npos)
			return static_cast<uint32_t>(HTTPErr::InvalidContentSize);
		else if (err.find("Unsupported Transfer Encoding") != std::string_view::npos)
			return static_cast<uint32_t>(HTTPErr::UnsupportedTransferEncoding);
		else if (err.find("NoBody For Method") != std::string_view::npos)
			return static_cast<uint32_t>(HTTPErr::NoBodyForMethod);
		else if (err.find("Chunked Encoding Not Supported") != std::string_view::npos)
			return static_cast<uint32_t>(HTTPErr::ChunkedEncodingNotSupported);
		return 0;
	}

	template<>
	inline uint32_t to_uint32<HTTPTransferEncoding>(std::string_view compression)
	{
		if (compression == "None")
			return static_cast<uint32_t>(HTTPTransferEncoding::None);
		else if (compression.find("chunked") != std::string_view::npos)
			return static_cast<uint32_t>(HTTPTransferEncoding::Chunked);
		else if (compression.find("identity") != std::string_view::npos)
			return static_cast<uint32_t>(HTTPTransferEncoding::Identity);
		return 0;
	}


	template<>
	inline uint32_t to_uint32<HTTPContentEncoding>(std::string_view encoding)
	{
		if (encoding.find("None") != std::string_view::npos)
			return static_cast<uint32_t>(HTTPContentEncoding::None);
		else if (encoding.find("identity") != std::string_view::npos)
			return static_cast<uint32_t>(HTTPContentEncoding::Identity);
		else if (encoding.find("gzip") != std::string_view::npos)
			return static_cast<uint32_t>(HTTPContentEncoding::Gzip);
		else if (encoding.find("deflate") != std::string_view::npos)
			return static_cast<uint32_t>(HTTPContentEncoding::Deflate);
		else if (encoding.find("zstd") != std::string_view::npos)
			return static_cast<uint32_t>(HTTPContentEncoding::Zstd);
		else if (encoding.find("brotli") != std::string_view::npos)
			return static_cast<uint32_t>(HTTPContentEncoding::Brotli);
		return 0;
	}

	template<>
	inline uint32_t to_uint32<HTTPLanguage>(std::string_view language)
	{
		if (language.find("None") != std::string_view::npos)
			return static_cast<uint32_t>(HTTPLanguage::None);
		else if (language.find("en-US") != std::string_view::npos)
			return static_cast<uint32_t>(HTTPLanguage::EnglishUS);
		else if (language.find("en-UK") != std::string_view::npos)
			return static_cast<uint32_t>(HTTPLanguage::EnglishUK);
		else if (language.find("en") != std::string_view::npos)
			return static_cast<uint32_t>(HTTPLanguage::English);
		else if (language.find("fr") != std::string_view::npos)
			return static_cast<uint32_t>(HTTPLanguage::French);
		else if (language.find("de") != std::string_view::npos)
			return static_cast<uint32_t>(HTTPLanguage::German);
		else if (language.find("es") != std::string_view::npos)
			return static_cast<uint32_t>(HTTPLanguage::Spanish);
		else if (language.find("ru") != std::string_view::npos)
			return static_cast<uint32_t>(HTTPLanguage::Russian);
		else if (language.find("ja") != std::string_view::npos)
			return static_cast<uint32_t>(HTTPLanguage::Japanese);
		else if (language.find("ko") != std::string_view::npos)
			return static_cast<uint32_t>(HTTPLanguage::Korean);
		else if (language.find("zh") != std::string_view::npos)
			return static_cast<uint32_t>(HTTPLanguage::Chinese);
		else if (language.find("it") != std::string_view::npos)
			return static_cast<uint32_t>(HTTPLanguage::Italian);
		else if (language.find("nl") != std::string_view::npos)
			return static_cast<uint32_t>(HTTPLanguage::Dutch);
		return 0;
	}
}