#pragma once

#include <string>
#include <cstdint>

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
		None,
		Persistent,
		Close,
		Upgrade
	};

	enum class HTTPTransferEncoding : uint32_t
	{
		None,
		Chunked,
		Identity,

		//Rarely used:
			//Gzip,
			//Deflate,
	};

	enum class HTTPContentEncoding : uint32_t
	{
		None,
		Identity,
		Gzip,
		Deflate,
		Zstd,
		Brotli
	};

	enum class HTTPContent : uint32_t
	{
		None,
		TextPlain,
		TextHTML,
		TextCSS,
		ApplicationJavaScript,
		ApplicationJSON,
		ApplicationXML,
		ApplicationFormUrlEncoded,
		ImagePNG,
		ImageJPEG,
		ImageGIF,

		//Unsupported Yet:
		ApplicationOctetStream,
	};


	enum class HTTPLanguage : uint32_t
	{
		None,
		EnglishUS,
		EnglishUK,
		English,
		French,
		German,
		Spanish,
		Russian,
		Japanese,
		Korean,
		Chinese,
		Italian,
		Dutch,
	};


	enum class HTTPErr : uint32_t
	{
		None = 0,
		DNSResolutionFailed,
		ConnectionFailed,
		RequestTimeout,
		InvalidData,
		InvalidReadingData,
		FailedToMakePersistentConnection,
		EmptyRequest,
		InvalidMethod,
		FailedToCloseSocket,
		InvalidContentType,
		MissingCRLF,
		InvalidHTTPVersion,
		MissingHeaderTerminator,
		ResponseError,
		InvalidURL,
		RedirectLimitExceeded,
		InvalidContentSize,
		NoBodyForMethod,
		HTTPVersionUndefined,
		UnsupportedEncoding,
		//Temporary errors
		ChunkedEncodingNotSupported,
		UnsupportedTransferEncoding,
		UnsupportedContentEncoding,

	};

	enum class HTTPParse : uint32_t
	{
		StatusCode,
		StatusMessage,
		ContentLength,
		ContentType,
		TransferEncoding,
		Connection,
	};

	template <typename T>
	concept IsValidProgramEnum =
		std::same_as<T, HTTPParse> ||
		std::same_as<T, HTTPMethod> ||
		std::same_as<T, HTTPConnection> ||
		std::same_as<T, HTTPContent> ||
		std::same_as<T, HTTPErr> ||
		std::same_as<T, HTTPContentEncoding> ||
		std::same_as<T, HTTPTransferEncoding> ||
		std::same_as<T, HTTPLanguage>;


	std::string to_string(HTTPMethod method);

	std::string to_string(HTTPConnection method);

	std::string to_string(HTTPContent content);

	std::string to_string(HTTPErr err);

	template<IsValidProgramEnum T>
	uint32_t to_uint32(std::string_view method);

}

#include "http_enums.inl"