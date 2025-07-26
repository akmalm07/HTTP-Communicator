#pragma once

#include "http_enums.h"

namespace communicator
{
	template<>
	inline uint32_t to_uint32<HTTPParse>(std::string_view method)
	{
		if (method.find("Status-Code"))
			return static_cast<uint32_t>(HTTPParse::StatusCode);
		else if (method.find("Status-Message"))
			return static_cast<uint32_t>(HTTPParse::StatusMessage);
		else if (method.find("Content-Length"))
			return static_cast<uint32_t>(HTTPParse::ContentLength);
		else if (method.find("Content-Type"))
			return static_cast<uint32_t>(HTTPParse::ContentType);
		else if (method.find("Transfer-Encoding"))
			return static_cast<uint32_t>(HTTPParse::TransferEncoding);
		else if (method.find("Connection"))
			return static_cast<uint32_t>(HTTPParse::Connection);
		return 0;
	}

	template<>
	inline uint32_t to_uint32<HTTPMethod>(std::string_view method)
	{
		if (method.find("GET"))
			return static_cast<uint32_t>(HTTPMethod::GET);
		else if (method.find("POST"))
			return static_cast<uint32_t>(HTTPMethod::POST);
		else if (method.find("PUT"))
			return static_cast<uint32_t>(HTTPMethod::PUT);
		else if (method.find("DEL"))
			return static_cast<uint32_t>(HTTPMethod::DEL);
		else if (method.find("PATCH"))
			return static_cast<uint32_t>(HTTPMethod::PATCH);
		else if (method.find("HEAD"))
			return static_cast<uint32_t>(HTTPMethod::HEAD);
		else if (method.find("OPTIONS"))
			return static_cast<uint32_t>(HTTPMethod::OPTIONS);
		return 0;
	}

	template<>
	inline uint32_t to_uint32<HTTPConnection>(std::string_view method)
	{
		if (method.find("keep-alive"))
			return static_cast<uint32_t>(HTTPConnection::Persistent);
		else if (method.find("close"))
			return static_cast<uint32_t>(HTTPConnection::Close);
		else if (method.find("upgrade"))
			return static_cast<uint32_t>(HTTPConnection::Upgrade);
	}

	template<>
	inline uint32_t to_uint32<HTTPContent>(std::string_view content)
	{
		if (content.find("None"))
			return static_cast<uint32_t>(HTTPContent::None);
		else if (content.find("text/plain"))
			return static_cast<uint32_t>(HTTPContent::TextPlain);
		else if (content.find("text/html"))
			return static_cast<uint32_t>(HTTPContent::TextHTML);
		else if (content.find("text/css"))
			return static_cast<uint32_t>(HTTPContent::TextCSS);
		else if (content.find("text/javascript"))
			return static_cast<uint32_t>(HTTPContent::TextJavaScript);
		else if (content.find("application/json"))
			return static_cast<uint32_t>(HTTPContent::ApplicationJSON);
		else if (content.find("application/xml"))
			return static_cast<uint32_t>(HTTPContent::ApplicationXML);
		else if (content.find("application/x-www-form-urlencoded"))
			return static_cast<uint32_t>(HTTPContent::ApplicationFormUrlEncoded);
		else if (content.find("image/png"))
			return static_cast<uint32_t>(HTTPContent::ImagePNG);
		else if (content.find("image/jpeg"))
			return static_cast<uint32_t>(HTTPContent::ImageJPEG);
		else if (content.find("image/gif"))
			return static_cast<uint32_t>(HTTPContent::ImageGIF);
		return 0;
	}

	template<>
	inline uint32_t to_uint32<HTTPErr>(std::string_view err)
	{
		if (err.find("None"))
			return static_cast<uint32_t>(HTTPErr::None);
		else if (err.find("DNS Resolution Failed"))
			return static_cast<uint32_t>(HTTPErr::DNSResolutionFailed);
		else if (err.find("Connection Failed"))
			return static_cast<uint32_t>(HTTPErr::ConnectionFailed);
		else if (err.find("Request Timeout"))
			return static_cast<uint32_t>(HTTPErr::RequestTimeout);
		else if (err.find("Response Error"))
			return static_cast<uint32_t>(HTTPErr::ResponseError);
		else if (err.find("Invalid URL"))
			return static_cast<uint32_t>(HTTPErr::InvalidURL);
		else if (err.find("Redirect Limit Exceeded"))
			return static_cast<uint32_t>(HTTPErr::RedirectLimitExceeded);
		else if (err.find("Invalid Content Size"))
			return static_cast<uint32_t>(HTTPErr::InvalidContentSize);
		else if (err.find("Unsupported Transfer Encoding"))
			return static_cast<uint32_t>(HTTPErr::UnsupportedTransferEncoding);
		else if (err.find("NoBody For Method"))
			return static_cast<uint32_t>(HTTPErr::NoBodyForMethod);
		else if (err.find("Chunked Encoding Not Supported"))
			return static_cast<uint32_t>(HTTPErr::ChunkedEncodingNotSupported);
		return 0;
	}

	template<>
	inline uint32_t to_uint32<HTTPTransferEncoding>(std::string_view compression)
	{
		if (compression == "None")
			return static_cast<uint32_t>(HTTPTransferEncoding::None);
		else if (compression.find("chunked"))
			return static_cast<uint32_t>(HTTPTransferEncoding::Chunked);
		else if (compression.find("identity"))
			return static_cast<uint32_t>(HTTPTransferEncoding::Identity);
		return 0;
	}


	template<>
	inline uint32_t to_uint32<HTTPContentEncoding>(std::string_view encoding)
	{
		if (encoding.find("None"))
			return static_cast<uint32_t>(HTTPContentEncoding::None);
		else if (encoding.find("chunked"))
			return static_cast<uint32_t>(HTTPContentEncoding::Chunked);
		else if (encoding.find("identity"))
			return static_cast<uint32_t>(HTTPContentEncoding::Identity);
		else if (encoding.find("gzip"))
			return static_cast<uint32_t>(HTTPContentEncoding::Gzip);
		else if (encoding.find("deflate"))
			return static_cast<uint32_t>(HTTPContentEncoding::Deflate);
		else if (encoding.find("zstd"))
			return static_cast<uint32_t>(HTTPContentEncoding::Zstd);
		else if (encoding.find("brotli"))
			return static_cast<uint32_t>(HTTPContentEncoding::Brotli);
		return 0;
	}

	template<>
	inline uint32_t to_uint32<HTTPLanguage>(std::string_view language)
	{
		if (language.find("None"))
			return static_cast<uint32_t>(HTTPLanguage::None);
		else if (language.find("en-US"))
			return static_cast<uint32_t>(HTTPLanguage::EnglishUS);
		else if (language.find("en-UK"))
			return static_cast<uint32_t>(HTTPLanguage::EnglishUK);
		else if (language.find("en"))
			return static_cast<uint32_t>(HTTPLanguage::English);
		else if (language.find("fr"))
			return static_cast<uint32_t>(HTTPLanguage::French);
		else if (language.find("de"))
			return static_cast<uint32_t>(HTTPLanguage::German);
		else if (language.find("es"))
			return static_cast<uint32_t>(HTTPLanguage::Spanish);
		else if (language.find("ru"))
			return static_cast<uint32_t>(HTTPLanguage::Russian);
		else if (language.find("ja"))
			return static_cast<uint32_t>(HTTPLanguage::Japanese);
		else if (language.find("ko"))
			return static_cast<uint32_t>(HTTPLanguage::Korean);
		else if (language.find("zh"))
			return static_cast<uint32_t>(HTTPLanguage::Chinese);
		else if (language.find("it"))
			return static_cast<uint32_t>(HTTPLanguage::Italian);
		else if (language.find("nl"))
			return static_cast<uint32_t>(HTTPLanguage::Dutch);
		return 0;
	}
}