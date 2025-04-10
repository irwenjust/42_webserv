#pragma once

#include "cgi.hpp"
#include "request.hpp"
#include "location.hpp"
#include "ClientConnection.hpp"

class Location;

std::unordered_map<int, std::string> code_map = {
	{ 100, "Continue" },
	{ 101, "Switching Protocols" },
	{ 102, "Processing" },

	{ 200, "OK" },
	{ 201, "Created" },
	{ 202, "Accepted" },
	{ 203, "Non-authoritative Information" },
	{ 204, "No Content" },
	{ 205, "Reset Content" },
	{ 206, "Partial Content" },
	{ 207, "Multi-Status" },
	{ 208, "Already Reported" },
	{ 226, "IM Used" },

	{ 300, "Multiple Choices" },
	{ 301, "Moved Permanently" },
	{ 302, "Found" },
	{ 303, "See Other" },
	{ 304, "Not Modified" },
	{ 305, "Use Proxy" },
	{ 307, "Temporary Redirect" },
	{ 308, "Permanent Redirect" },

	{ 400, "Bad Request" },
	{ 401, "Unauthorized" },
	{ 402, "Payment Required" },
	{ 403, "Forbidden" },
	{ 404, "Not Found" },
	{ 405, "Method Not Allowed" },
	{ 406, "Not Acceptable" },
	{ 407, "Proxy Authentication Required" },
	{ 408, "Request Timeout" },
	{ 409, "Conflict" },
	{ 410, "Gone" },
	{ 411, "Length Required" },
	{ 412, "Precondition Failed" },
	{ 413, "Payload Too Large" },
	{ 414, "Request-URI Too Long" },
	{ 415, "Unsupported Media Type" },
	{ 416, "Requested Range Not Satisfiable" },
	{ 417, "Expectation Failed" },
	{ 418, "I’m a teapot" },
	{ 421, "Misdirected Request" },
	{ 422, "Unprocessable Entity" },
	{ 423, "Locked" },
	{ 424, "Failed Dependency" },
	{ 426, "Upgrade Required" },
	{ 428, "Precondition Required" },
	{ 429, "Too Many Requests" },
	{ 431, "Request Header Fields Too Large" },
	{ 444, "Connection Closed Without Response" },
	{ 451, "Unavailable For Legal Reasons" },
	{ 499, "Client Closed Request" },

	{ 500, "Internal Server Error" },
	{ 501, "Not Implemented" },
	{ 502, "Bad Gateway" },
	{ 503, "Service Unavailable" },
	{ 504, "Gateway Timeout" },
	{ 505, "HTTP Version Not Supported" },
	{ 506, "Variant Also Negotiates" },
	{ 507, "Insufficient Storage" },
	{ 508, "Loop Detected" },
	{ 510, "Not Extended" },
	{ 511, "Network Authentication Required" },
	{ 599, "Network Connect Timeout Error" }
};

std::unordered_map<std::string, std::string> mime_map = {
	{ "", "text/plain" },
	{ ".au", "audio/basic" },
	{ ".avi", "video/x-msvideo" },
	{ ".css", "text/css" },
	{ ".csv", "text/csv" },
	{ ".gif", "image/gif" },
	{ ".gz", "application/x-gzip" },
	{ ".htm", "text/html" },
	{ ".html", "text/html" },
	{ ".ico", "image/x-icon" },
	{ ".jpeg", "image/jpeg" },
	{ ".jpg", "image/jpeg" },
	{ ".json", "application/json" },
	{ ".mp3", "audio/mpeg" },
	{ ".mp4", "video/mp4" },
	{ ".mpeg", "video/mpeg" },
	{ ".mpg", "video/mpeg" },
	{ ".pdf", "application/pdf" },
	{ ".png", "image/png" },
	{ ".rtf", "application/rtf" },
	{ ".svg", "image/svg+xml" },
	{ ".tar", "application/x-tar" },
	{ ".txt", "text/plain" },
	{ ".word", "application/msword" },
	{ ".xhtml", "application/xhtml+xml" },
	{ ".xml", "text/xml" },
	{ "default", "text/plain" }
};

class Response {
    private:
	std::weak_ptr<ClientConnection> _client;
	std::shared_ptr<Request> _request;
	std::shared_ptr<Location> _location;

	std::map<std::string, std::string> _additional_headers;

	int _status_code;
	std::string _setCookie;

	int has_errors(void);
	void create_response(int status);

	void fix_uri(void);

	int handle_post(void);
	int handle_delete(void);
	int handle_get(void);

	std::shared_ptr<Location> find_location(void);
	void set_error_page(int code);
	void generate_error_page(int code);

	bool directory_index(std::string path);
	std::string get_content_type(std::string uri);

	bool is_cgi(std::string uri);
	void do_cgi(void);

	// void _handleCookies();
	void _validateCookie();
	void _createCookie();
	void _removeInvalidCookie(std::string cookie_path);

	bool init_cgi(std::shared_ptr<ClientConnection> client);

    public:
	std::ostringstream _body;
	std::ostringstream buffer;

	Response(std::shared_ptr<ClientConnection> client, std::shared_ptr<Request> req);
	void finish_response(void);
	void finish_cgi(std::shared_ptr<Request> req_cgi);
	void set_error(int code);
	~Response();
};
