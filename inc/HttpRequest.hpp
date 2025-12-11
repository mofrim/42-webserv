/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zrz <zrz@student.42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/08 15:46:18 by zrz               #+#    #+#             */
/*   Updated: 2025/12/11 20:43:26 by zrz              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <string>
#include <map>
#include <vector>

// Defining allowed HTTP methods and parsing states for clarity
enum e_method { M_GET, M_POST, M_DELETE, M_UNKNOWN };
enum e_state { S_REQUEST_LINE, S_HEADERS, S_BODY, S_COMPLETE, S_ERROR };

class HttpRequest { 
	// the methods still need to be finished to be defined but for now i think this structure should be enough to start
	private:
		// Core Data Members
		e_method                     _method;
		std::string                  _uri;
		std::string                  _query_string;
		std::string                  _version; // Should be "HTTP/1.1"
		std::map<std::string, std::string> _headers;
		std::vector<char>            _body;

		// State & Control
		e_state                      _state;
		int                          _error_code;
		size_t                       _content_length;
		// Buffer for partially read data, like gnl (important for non-blocking I/O)
		std::string                  _raw_buffer; 

		// Internal Parsing Logic
		void _parseRequestLine();
		void _parseHeaders();
		void _parseBody();
		
		// Utility
		void _setError(int code);

	public:
		HttpRequest();
		~HttpRequest();

		// Will be the main public interface for feeding data to the parser
		void parse(const std::string& data); 

		// Getters
		e_method            getMethod() const { return _method; }
		const std::string&  getUri() const { return _uri; }
		const std::string&  getQueryString() const { return _query_string; }
		std::string         getHeader(const std::string& key) const; // Case-insensitive lookup
		const std::vector<char>& getBody() const { return _body; }
		
		// State Checkers
		bool isComplete() const { return _state == S_COMPLETE; }
		bool hasError() const { return _state == S_ERROR; }
		int  getErrorCode() const { return _error_code; }
		
		// Exception class for parsing failures /* TODO */
		class HttpParseException : public std::exception { /* ... */ };
};

/*

I don't know where to put this yet but I got most if not all the Status codes
and put them here for future use, for when we do http protocol parser

enum StatusCode {
	// Information responses
	Continue_100 = 100,
	SwitchingProtocol_101 = 101,
	Processing_102 = 102,
	EarlyHints_103 = 103,

	// Successful responses
	OK_200 = 200,
	Created_201 = 201,
	Accepted_202 = 202,
	NonAuthoritativeInformation_203 = 203,
	NoContent_204 = 204,
	ResetContent_205 = 205,
	PartialContent_206 = 206,
	MultiStatus_207 = 207,
	AlreadyReported_208 = 208,
	IMUsed_226 = 226,

	// Redirection messages
	MultipleChoices_300 = 300,
	MovedPermanently_301 = 301,
	Found_302 = 302,
	SeeOther_303 = 303,
	NotModified_304 = 304,
	UseProxy_305 = 305,
	unused_306 = 306,
	TemporaryRedirect_307 = 307,
	PermanentRedirect_308 = 308,

	// Client error responses
	BadRequest_400 = 400,
	Unauthorized_401 = 401,
	PaymentRequired_402 = 402,
	Forbidden_403 = 403,
	NotFound_404 = 404,
	MethodNotAllowed_405 = 405,
	NotAcceptable_406 = 406,
	ProxyAuthenticationRequired_407 = 407,
	RequestTimeout_408 = 408,
	Conflict_409 = 409,
	Gone_410 = 410,
	LengthRequired_411 = 411,
	PreconditionFailed_412 = 412,
	PayloadTooLarge_413 = 413,
	UriTooLong_414 = 414,
	UnsupportedMediaType_415 = 415,
	RangeNotSatisfiable_416 = 416,
	ExpectationFailed_417 = 417,
	ImATeapot_418 = 418,
	MisdirectedRequest_421 = 421,
	UnprocessableContent_422 = 422,
	Locked_423 = 423,
	FailedDependency_424 = 424,
	TooEarly_425 = 425,
	UpgradeRequired_426 = 426,
	PreconditionRequired_428 = 428,
	TooManyRequests_429 = 429,
	RequestHeaderFieldsTooLarge_431 = 431,
	UnavailableForLegalReasons_451 = 451,

	// Server error responses
	InternalServerError_500 = 500,
	NotImplemented_501 = 501,
	BadGateway_502 = 502,
	ServiceUnavailable_503 = 503,
	GatewayTimeout_504 = 504,
	HttpVersionNotSupported_505 = 505,
	VariantAlsoNegotiates_506 = 506,
	InsufficientStorage_507 = 507,
	LoopDetected_508 = 508,
	NotExtended_510 = 510,
	NetworkAuthenticationRequired_511 = 511,
};

*/