
#ifndef	REQUEST_HPP
#define	REQUEST_HPP

#define NOT_FOUND 404
#define FORBIDDEN 403
#define BAD_REQUEST 400
#define UNAUTHORIZED 401
#define CONFLICT 409
#define LARGE_PAYLOAD 413
#define INTERNAL_SERVER 500
#define TIMEOUT 504
#define METH_NOT_ALLOWED 405

#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <map>
#include <cstdio>
#include <sys/stat.h>
#include <dirent.h>
#include "BufferRequest.hpp"
#include "Log.hpp"
#include "Config.hpp"
#include "CGIRequest.hpp"

#define DEBUG 0

class Request
{
	public:
		Request(const std::string& fullRequest, int fd, const std::vector<ServerConfig> &servers);
		void	parseHeaders(std::stringstream &ss);
		void	handleRequest();
		void	handleAutoIndexResponse();
		void	setErrorResponse(int error);
		void	getRequest();
		void	postRequest();
		void	handleFileUpload(std::string fileName);
		void	deleteRequest();
		void	setContentType(std::string &path);
		void	cgiRequest();
		void	setPath();
		void	setContentType();
		void	setHeader(int code);
		void	cleanPath();
		void	setConfig(const std::vector<ServerConfig> &servers);
		void	sendFullResponse(int fd, const std::string &response);
		void	sendErrorResponse(int fd, int code, std::map<int, std::string> errorPages);
		void	setRedirectResponse();
		void	makeDirectory(std::string &directory);
		void	sendPostResponse(std::string resource);
		bool	payloadCheck(long unsigned int max_body_size);
		bool	specialPathCase();
		std::string	getBoundary();
		std::string	getFileContentBoundary(std::string &boundary);
		std::string	getFileName();
		std::string	getContentType();
		std::string	getDeletePath();
		std::string	joinPath(const std::string& a, const std::string& b);
		const std::string	&getMethod() const;
		const std::string	&getUri() const;
		const std::string	&getProtocol() const;
		const std::string	&getHeader() const;
		const std::string	&getBody() const;
		ServerConfig	getConfig() const;
		~Request();

	private:
		int			_fd;
		int			_contentLength;
		std::string	_requestHeader;
		std::string _fullRequest;
		std::string	_response;
		std::string	_method;
		std::string	_uri;
		std::string _body;
		std::string	_protocol;
		std::string	_contentType;
		std::string	_header;
		std::string	_content;
		std::string	_queryString;
		std::string	_path;
		ServerConfig	_config;
		Location	_location;
		std::string _filename;
};

#endif