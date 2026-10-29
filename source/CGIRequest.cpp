#include "../include/CGIRequest.hpp"

CGIRequest::CGIRequest(Request &request, const std::string &requestStr, int client_fd)
{
	_method = request.getMethod();
	_uri = request.getUri();
	_protocol = request.getProtocol();
	_header = getHeader(requestStr);
	_body = request.getBody();
	_client_fd = client_fd;
	_config = request.getConfig();
	_location = getLocation(request, _config.locations);
	getDocumentRoot();
	getDecodedUri();
	_extension = getExtension(_decoded_uri);
	_scriptName = getScriptName();
	_serverPort = getServerPort(requestStr);
	getScriptPath();
	getInterpreter();
	getHeadersMap();
	getQueryString();
	getPathInfo(); // updates _pathInfo and _pathTranslated
	initCGIenv();
	initCGIExecEnv();
	if (DEBUG)
	{
		std::cout << "==== CGIRequest ====\nvariables:\n"
				  << "_method		: " << _method
				  << "\n_uri		: " << _uri
				  << "\n_protocol	: " << _protocol
				  << "\n_header		: " << _header
				  << "\n_body		: " << _body
				  << "\n_client_fd	: " << _client_fd
				  << "\n_config.server_name	: " << _config.server_name
				  << "\n_location.url	: " << _location.url
				  << "\n_document_root	: " << _document_root
				  << "\n_decoded_uri	: " << _decoded_uri
				  << "\n_extension	: " << _extension // might not be valid
				  << "\n_scriptName	: " << _scriptName
				  << "\n_scriptPath	: " << _scriptPath
				  << "\n_interpreter	: " << _interpreter
				  << "\n_queryString	: " << _queryString
				  << "\n_pathInfo	: " << _pathInfo
				  << "\n_pathTranslated	: " << _pathTranslated
				  << "\n_serverPort	: " << _serverPort
				  << "\n";
	}
}

CGIRequest::~CGIRequest()
{
	for (size_t	i = 0; _envExec[i] != NULL; i++	)
		if (_envExec[i])
			free(_envExec[i]);
	delete[] _envExec;
	_env.clear();
	_requestHeaders.clear();
	for (size_t i = 0; _argv[i] != NULL; i++)
		if (_argv[i])
			free(_argv[i]);
}

void	CGIRequest::initCGIExecEnv()
{
	std::map<std::string, std::string>::iterator	it = _env.begin();
	std::map<std::string, std::string>::iterator	ite = _env.end();
	size_t	i = 0;

	_envExec = new char *[_env.size() + 1];
	if (!_envExec)
		Log::console(ERROR, "Error allocating memory for env variables.");
	for(; it != ite; ++it, i++)
	{
		std::string var = it->first + "=" + it->second;
		_envExec[i] = strdup(var.c_str());
	}
	_envExec[i] = NULL;
}

std::string	CGIRequest::getServerPort(const std::string &fullRequest)
{
	std::string	toFind = "Host: ";
	std::string	port;
	size_t	hostPos = fullRequest.find(toFind);
	if (hostPos != std::string::npos)
	{
		size_t	endPos = fullRequest.find("\r\n", hostPos);
		size_t	lastColonPos = fullRequest.rfind(":", endPos);
		port = fullRequest.substr(lastColonPos + 1, endPos - lastColonPos);
	}
	return (port);
}

std::string	CGIRequest::getContentType()
{
	std::string	type = "";
	std::string	cntType = "Content-Type: ";
	size_t	cntTypePos = _header.find(cntType);
	
	if (cntTypePos != std::string::npos)
		return ("");
	cntTypePos += cntType.size();
	size_t	cntTypeEnd = _header.find("\r\n", cntTypePos);
	if (cntTypeEnd > cntTypePos)
		type = _header.substr(cntTypePos, cntTypeEnd - cntTypePos);
	return (type);
}

void	CGIRequest::initCGIenv()
{
	if (_method == "POST")
	{
		_env["CONTENT_LENGTH"] = toString(_body.size());
		_env["CONTENT_TYPE"] = getContentType();
	}
	_env["GATEWAY_INTERFACE"] = "CGI/1.1";
	_env["PATH_INFO"] = _pathInfo;
	_env["PATH_TRANSLATED"] = _pathTranslated;
	_env["QUERY_STRING"] = _queryString;
	_env["REQUEST_METHOD"] = _method;
	_env["REQUEST_URI"] = _uri;
	_env["SCRIPT_NAME"] = _scriptName;
	_env["SCRIPT_FILENAME"] = _scriptPath;
	_env["SERVER_NAME"] = _config.server_name;
	_env["SERVER_PORT"] = _serverPort;
	_env["SERVER_PROTOCOL"] = _protocol;
	_env["SERVER_SOFTWARE"] = "webserv-42";
	_env["REDIRECT_STATUS"] = "200";
	_env["DOCUMENT_ROOT"] = getRootPath();
	if (_requestHeaders.find("cookie") != _requestHeaders.end())
		_env["HTTP_COOKIE"] = _requestHeaders["cookie"];
	std::map<std::string, std::string>::iterator	it = _requestHeaders.begin();
	std::map<std::string, std::string>::iterator	ite = _requestHeaders.end();
	for (; it != ite; ++it)
	{
		std::string	varName = it->first;
		std::transform(varName.begin(), varName.end(), varName.begin(), ::toupper);
		varName = "HTTP_" + varName;
		_env[varName] = it->second;
	}
	_argv[0] = strdup(_interpreter.c_str());
	_argv[1] = strdup(_scriptPath.c_str());
	_argv[2] = NULL;
}

std::string	CGIRequest::getRootPath()
{
	std::string	workDir = getWorkingDir();
	std::string	root = _location.root;
	if (root[0] == '.')
		root.erase(0, 1);
	if (root[root.size() - 1] == ';')
		root = root.substr(0, root.size() - 1);
	return (workDir + root);
}

void	CGIRequest::getPathInfo()
{
	std::string	uriNoQueryStr = _decoded_uri.substr(0, _decoded_uri.find('?'));
	std::string	pathInfo = uriNoQueryStr.substr(uriNoQueryStr.find(_scriptName) + _scriptName.size());
	_pathInfo = pathInfo.empty() ? "/" : pathInfo;
	_pathTranslated = getRootPath() + _pathInfo;
}

void	CGIRequest::getQueryString()
{
	size_t posQM = _uri.find_first_of('?');
	if (posQM != std::string::npos)
		_queryString = _uri.substr(posQM + 1);
}

void	CGIRequest::getHeadersMap()
{
	std::stringstream	strStream(_header);
	std::string	line;

	while (std::getline(strStream, line))
	{
		if (line.empty())
			continue ;
		size_t	colonPos = line.find_first_of(':');
		if (colonPos != std::string::npos)
		{
			std::string key = line.substr(0, colonPos);
			std::string value = line.substr(colonPos + 1);
			key.erase(key.find_last_not_of(" \t\r\n") + 1);
			value.erase(0, value.find_first_not_of(" \t\r\n"));
			_requestHeaders[key] = value;
		}
	}
}

std::string	CGIRequest::getHeader(const std::string &fullRequest)
{
	std::string	header;
	size_t	pos = fullRequest.find_first_of("\r\n");
	header = fullRequest.substr(pos + 2, fullRequest.size());
	return (header);
}

void	CGIRequest::getInterpreter()
{
	if (std::abs((int)(_location.cgi_extension.size() - _location.cgi_path.size())) > 1)
	{
		Log::console(ERROR, "CGI extension and CGI path size difference is greater than 1.");
		_interpreter = _scriptPath;
		return ;
	}
	if (_extension == ".cgi")
	{
		_interpreter = _scriptPath;
		return ;
	}
	for (size_t i = 0; i < _location.cgi_extension.size(); i++)
	{
		if (_location.cgi_extension[i] == _extension)
		{
			_interpreter = _location.cgi_path[i];
			return ;
		}
	}
	_interpreter = _scriptPath;
}

void	CGIRequest::getScriptPath()
{
	_scriptPath = _document_root + _decoded_uri;
	std::string	workDir = getWorkingDir();

	_scriptPath = _scriptPath.substr(0, _scriptPath.find('?'));
	_scriptPath = workDir + _scriptPath;
	if (!_scriptPath.empty() && _scriptPath[_scriptPath.size() - 1] == '/')
		_scriptPath = _scriptPath.substr(0, _scriptPath[_scriptPath.size() - 1]);
	size_t	scriptNamePos = _scriptPath.rfind(_scriptName);
	if (scriptNamePos != std::string::npos)
		_scriptPath = _scriptPath.substr(0, scriptNamePos + _scriptName.size());
}

void	CGIRequest::getDocumentRoot()
{
	_document_root = _location.root.substr(0, _location.root.find(_location.url));
	if (!_document_root.empty() && _document_root[0] == '.')
		_document_root.erase(0, 1);
	if (!_document_root.empty() && _document_root[_document_root.size() - 1] == ';')
		_document_root = _document_root.substr(0, _document_root.size() - 1);
}

void	CGIRequest::getDecodedUri()
{
	_decoded_uri = _uri;
	size_t	percent = _decoded_uri.find("%");
	while (percent != std::string::npos)
	{
		if (_decoded_uri.length() < percent + 2)
			break;
		char	decodedChar = hexConvert(_decoded_uri.substr(percent + 1, 2));
		_decoded_uri.replace(percent, 3, toString(decodedChar));
		percent = _decoded_uri.find("%");
	}
}

unsigned int	CGIRequest::hexConvert(const std::string &token)
{
	unsigned int	res;
	std::stringstream	ss;
	ss << token;
	ss >> std::hex >> res;
	return (res);
}

std::string	CGIRequest::getScriptName()
{
	std::string	uri = _decoded_uri;
	size_t	queryPos = uri.find('?');

	if (queryPos != std::string::npos)
		uri = uri.substr(0, queryPos);
	size_t	fragmentPos = uri.find("#");
	if (fragmentPos != std::string::npos)
		uri = uri.substr(0, fragmentPos);
	size_t	extensionPos = uri.rfind(_extension);
	if (extensionPos != std::string::npos)
	{
		if (extensionPos + _extension.length() == uri.length() || uri[extensionPos + _extension.length()] == '/')
		{
			size_t	lastSlash = uri.rfind('/', extensionPos);
			if (lastSlash != std::string::npos)
				return (uri.substr(0, extensionPos + _extension.length()));
		}
	}
	return(uri.substr(uri.rfind('/') + 1));
}

void	CGIRequest::handleRequest()
{
	int	status;
	pid_t	pid;
	clock_t	start = getTime();

	if (open(_scriptPath.c_str(), F_OK) == -1)
		return ; // ERROR SCRIPT, send error response
	if (pipe(_pipeChild) == -1 || pipe(_pipeParent) == -1)
		return ; // ERROR PIPE
	if ((pid = fork()) < 0)
		return ; // ERROR FORK
	if (pid == 0)// child process
	{
		// redirect STDIN and STDOUT to corresponding pipes
		close(_pipeParent[1]);
		dup2(_pipeParent[0], STDIN_FILENO);
		close(_pipeParent[0]);
		close(_pipeChild[0]);
		dup2(_pipeChild[1], STDOUT_FILENO);
		close(_pipeChild[1]);
		if (execve(_argv[0], _argv, _envExec) == -1)
			Log::console(ERROR, "Couldn't run cgi script : " + static_cast<std::string>(_argv[0]) + '\n');
	}
	else // parent process
	{
		bool	timeOutFlag = false;
		close(_pipeChild[1]);
		//	send _body to parent read end of pipe
		//	for further handling of the request
		//	by the cgi script
        if (_method == "POST")
        {
            int bytesWritten = 0;
            int bytesRemaining = _body.length();
            while (bytesRemaining > 0)
            {
                int bytesToWrite = std::min(BUFSIZ, bytesRemaining);
                if (write(_pipeParent[1], _body.c_str() + bytesWritten, bytesToWrite) == -1)
                {
					close(_pipeParent[1]);
					std::string	response;
					std::string	header;
					std::string	content = "Internal Server Error\n";
					header += "HTTP/1.1 500 Internal Server Error\r\n";
					header += "Content-Type: text/html\r\n";
					header += "Content-Length: " + toString(content.length()) + " \r\n\r\n";
					//header += "Connection: close\r\n\r\n";
					response += header + content;
					send(_client_fd, response.c_str(), response.length(), 0);
                }
                bytesWritten += bytesToWrite;
                bytesRemaining -= bytesToWrite;
            }
        }
		close(_pipeParent[1]);
		while (true)
		{
			clock_t	currTime = getTime();
			double elapsed	= currTime - start;
			if (elapsed > 5.0)
			{
				timeOutFlag = true;
				break ;
			}
			pid_t	res = waitpid(pid, &status, WNOHANG); // monitor child process while not blocking execution of parent process
			if (res == 0) // child process still running
			{
				usleep(1000);
				continue;
			}
			else if (res == pid) // if res == pid, waitpid returned child pid which means child was terminated
			{
				if (WIFEXITED(status) && WEXITSTATUS(status) == 0) // child terminated normally, process with response
				{
					char	bufferChildMsg[BUFSIZ];
					std::string	response = "HTTP/1.1 200 OK\r\n";
					ssize_t	msgBytes;
					while ((msgBytes = read(_pipeChild[0], bufferChildMsg, BUFSIZ)) > 0)
						response += std::string(bufferChildMsg, msgBytes);
					send(_client_fd, response.c_str(), response.length(), 0);
				}
				else
				{
					std::string	response;
					std::string	header;
					std::string	content = "Internal Server Error\n";
					header += "HTTP/1.1 500 Internal Server Error\r\n";
					header += "Content-Type: text/html\r\n";
					header += "Content-Length: " + toString(content.length()) + " \r\n";
					header += "Connection: clone\r\n\r\n";
					response += header + content;
					Log::console(WARNING,  "Couldn't run script.");
					send(_client_fd, response.c_str(), response.length(), 0);
				}
				close(_client_fd);
				break;
			}
		}
		if (timeOutFlag)
		{
			kill(pid, SIGKILL);
			sendErrorResponse(_client_fd, TIMEOUT, _config.error_page);
		}
		close(_pipeChild[0]);
	}
}

void	CGIRequest::sendErrorResponse(int fd, int code, std::map<int, std::string> errorPages)
{
	if (errorPages.find(code) != errorPages.end())
	{
		std::string	path = _location.root + errorPages[code];
		std::ifstream	file(path.c_str());
		if (file.is_open())
		{
			std::stringstream	buffer;
			buffer << file.rdbuf();
			std::string	content = buffer.str();
			std::string	header =
			"HTTP/1.1 " + toString(code) + " OK\r\n" +
			"Content-Type: text/html; charset=UTF-8\r\n" +
			"Content-Length: " + toString(content.length()) + "\r\n" +
			"Connection: close\r\n\r\n";
			send(fd, header.c_str(), header.length(), 0);
			send(fd, content.c_str(), content.length(), 0);
			file.close();
		}
	}
	else
	{
		std::pair<std::string, std::string>	reasonAndMessage;
		int	status_code;
		reasonAndMessage = setReasonAndMessage(code);
		
		if (code == 500)
			status_code = 500;
		else if (code == 504)
			status_code = 504;
		else
			status_code = code;
		std::string	response =
		"<html><head><title>"
		+ toString(status_code) + " " + reasonAndMessage.first +
		"</title></head>" +
		"<body><h1>" + reasonAndMessage.first + "</h1><br><br><p>" +
		reasonAndMessage.second +
		"</body></html>";
		std::string	header = 
		"HTTP/1.1 " + toString(status_code) + " " + reasonAndMessage.first + "\r\n"
		"Content-Type: text/html; charset=UTF-8\r\n" +
		"Content-Length: " + toString(response.length()) + "\r\n" +
		"Connection: close\r\n\r\n";
		send(fd, header.c_str(), header.length(), 0);
		send(fd, response.c_str(), response.length(), 0);
	}
}