
#ifndef	CGIREQUEST_HPP
#define CGIREQUEST_HPP

#include <iostream>
#include <map>
#include <algorithm>
#include <sys/wait.h>
#include <sys/time.h>
#include <ctime>
#include <cstdlib>
#include "Request.hpp"

class Request;

class CGIRequest
{
	public:
		CGIRequest(Request &request, const std::string &requestStr, int client_fd);
		~CGIRequest();
		void	getDecodedUri();
		void	getDocumentRoot();
		unsigned int	hexConvert(const std::string &token);
		std::string	getScriptName();
		void	getScriptPath();
		void	getInterpreter();
		std::string	getHeader(const std::string &fullRequest);
		void	getHeadersMap();
		void	getQueryString();
		void	getPathInfo();
		std::string	getRootPath();
		std::string	getContentType();
		std::string	getServerPort(const std::string &fullRequest);
		void	initCGIenv();
		void	initCGIExecEnv();
		void	handleRequest();
		void	sendErrorResponse(int fd, int code, std::map<int, std::string> errorPages);

	private:
		// from request
		std::string	_method; // from request
		std::string	_uri; // from request
		std::string	_decoded_uri;
		std::string	_protocol; // from request
		std::string _header; // from request?
		std::string	_body; // 
		int _client_fd; // from constructor
		std::string	_interpreter; // argv[0]
		std::string	_scriptPath; // argv[1]
		std::string	_extension;
		char		*_argv[3]; // _interpreter, _scriptPath, NULL
		std::map<std::string, std::string>	_env; // all the environment variables
		std::map<std::string, std::string>	_requestHeaders;
		char	**_envExec; // environment variables for execv()
		std::string	_pathInfo; // from request == URI
		std::string	_pathTranslated; // _pathInfo but full path
		std::string	_queryString; // from request
		std::string	_scriptName; // script name, from request
		std::string	_document_root;
		std::string	_serverPort;
		ServerConfig	_config;
		Location	_location;
		int	_pipeChild[2];
		int	_pipeParent[2];

};

#endif