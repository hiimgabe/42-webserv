
#include "../include/Request.hpp"
#include <filesystem>

Request::Request(const std::string& fullRequest, int fd, const std::vector<ServerConfig> &servers)
{
	//std::cout << fullRequest << '\n';
	std::stringstream	ss(fullRequest);
	_fullRequest = fullRequest;
	ss >> _method >> _uri >> _protocol;
	setConfig(servers);
	_location = getLocation(*this, _config.locations);
	setPath();
	cleanPath();
	size_t bodyStart = fullRequest.find("\r\n\r\n");
	if (bodyStart != std::string::npos)
	{
		_requestHeader = fullRequest.substr(0, bodyStart + 4);
		_body = fullRequest.substr(bodyStart + 4);
	} else {
		std::cout << "No body found in request.\n";
	}
	_fd = fd;
	if (DEBUG)
	{
		std::cout	<< "\n==========\nServerConfig:\n"
					<< "\nserver_name : " << _config.server_name << '\n';
        std::cout << "  Error Pages:\n";
        for (std::map<int, std::string>::iterator it = _config.error_page.begin(); it != _config.error_page.end(); ++it)
            std::cout << "    " << it->first << " " << it->second << '\n';
		std::cout	<< "\nclient_max_body_size : " << _config.client_max_body_size << '\n'
					<< "ports : ";
		for (size_t i = 0; i < _config.port.size(); i++)
			std::cout << std::dec << _config.port[i] << ' ';
		std::cout << '\n';
		std::cout << "locations:\n";
		for (std::map<std::string, Location>::iterator it = _config.locations.begin(); it != _config.locations.end(); ++it)
		{
			std::cout << "location : " << it->first
					  << "\nroot  : " << it->second.root
					  << "\nindex : " << it->second.index
					  << "\nupload_dir : " << it->second.upload_dir
					  << "\nmethods : "
					  << "\ncgi_path : ";
			for (std::vector<std::string>::iterator itCGIP = it->second.cgi_path.begin(); itCGIP != it->second.cgi_path.end(); ++itCGIP)
				std::cout << *itCGIP << ' ';
			std::cout << "\ncgi_extension : ";
			for (std::vector<std::string>::iterator itCGIE = it->second.cgi_extension.begin(); itCGIE != it->second.cgi_extension.end(); ++itCGIE)
				std::cout << *itCGIE << ' ';
			std::cout << '\n';
			for (std::vector<std::string>::iterator itM = it->second.methods.begin(); itM != it->second.methods.end(); ++itM)
				std::cout << *itM << ' ';
			std::cout << '\n';
		}
	}
	Log::console(MESSAGE, " <" + _config.server_name + "> " + " [" + _method + "] " + _uri);
  	handleRequest();
}


Request::~Request() {}


void Request::sendFullResponse(int fd, const std::string &response)
{
    size_t  totalSent = 0;
    size_t  responseLength = response.length();

    while (totalSent < responseLength)
    {
        ssize_t sent = send(fd, response.c_str() + totalSent, responseLength - totalSent, 0);
        if (sent < 0)
        {
            Log::console(ERROR, "Failed to send full response.");
            break;
        }
        totalSent += sent;
    }
    if (totalSent < responseLength)
        Log::console(ERROR, "Response not fully send. Sent: " + toString(totalSent) + " of " + toString(responseLength));
}

// utils
static bool	endsWith(std::string path, std::string find)
{
	if (path.size() < find.size())
		return (false);
	return(path.compare(path.size() - find.size(), find.size(), find) == 0);
}

static bool	isCGIRequest(std::string &uri)
{
	return (uri.find("/cgi-bin") == 0);
}

//	part of the string that comes after the ?
//	ex: GET /script.cgi?name=Gabi&age=25 HTTP/1.1
//	

bool	methodIsAllowed(std::string &method, std::vector<std::string> &vec)
{
	for (size_t i = 0; i < vec.size(); i++)
	{
		if (vec[i].find(method) == 0)
			return(true);
	}
	return (false);
}	

bool	Request::payloadCheck(long unsigned int max_body_size)
{
	return (_body.size() > max_body_size);
}

void	Request::handleRequest()
{
	struct stat	pathInfo;

	std::string autoIndexBool = _location.autoindex == true ? "true" : "false";

	if (!_location.redirect.second.empty())
	{
		setRedirectResponse();
		Log::console(MESSAGE, " <" + _config.server_name + "> " + "Redirecting.");
		sendFullResponse(_fd, _response);
	}
	else if (stat(_path.c_str(), &pathInfo) != 0 && _method != "DELETE" && _method != "POST")
	{
		sendErrorResponse(_fd, NOT_FOUND, _config.error_page);
		Log::console(WARNING,  "Resource doesn't exist.");
	}
	else if (!methodIsAllowed(_method, _location.methods))
	{
		Log::console(WARNING,   "Method isn't allowed.");
		sendErrorResponse(_fd, METH_NOT_ALLOWED, _config.error_page);
	}
	else
	{
		if (isCGIRequest(_uri) && !S_ISDIR(pathInfo.st_mode))
		{
			cgiRequest();
			//if (_location.url != "/cgi-bin")
			//{
			//	Log::console(WARNING, " This location isn't configured to handle CGI.");
			//	sendErrorResponse(_fd, 400, _config.error_page);
			//}
			//else
		}
		else if (_method == "GET")
		{
			if (S_ISDIR(pathInfo.st_mode) && _location.autoindex)
				handleAutoIndexResponse();
			else if (S_ISDIR(pathInfo.st_mode))
				sendErrorResponse(_fd, FORBIDDEN, _config.error_page);
			else
				getRequest();
		}
		else if (_method == "POST")
		{
			if (payloadCheck(_config.client_max_body_size))
			{
				Log::console(WARNING, "Body exceeds limit.");
				sendErrorResponse(_fd, LARGE_PAYLOAD, _config.error_page);
			}
			else
				postRequest();
		}
		else if (_method == "DELETE")
			deleteRequest();
	}
	/*
	if (payloadCheck(_config.client_max_body_size))
	{
		sendErrorResponse(_fd, LARGE_PAYLOAD, _config.error_page);
		return ;
	}
	if (!_location.redirect.second.empty())
	{
		setRedirectResponse();
		sendFullResponse(_fd, _response);
		return ;
	}
	if (!methodIsAllowed(_method, _location.methods, _location.url))
	{

		Log::console(ERROR, "Method " + _method + " isn't allowed in this location.\n");
		sendErrorResponse(_fd, METH_NOT_ALLOWED, _config.error_page);
		return ;
	}
	if (stat(_path.c_str(), &pathInfo) != 0)
	{
		std::cout << "dbg: " << _path << " does not exist\n";
		sendErrorResponse(_fd, NOT_FOUND, _config.error_page);
		//sendFullResponse(_fd, _response);
		return ;
	}
	if (S_ISDIR(pathInfo.st_mode) && _location.autoindex)
		handleAutoIndexResponse();
	else if (S_ISDIR(pathInfo.st_mode))
	{
		sendErrorResponse(_fd, FORBIDDEN, _config.error_page);
		return ;
	}
	else if (isCGIRequest(_uri) && !S_ISDIR(pathInfo.st_mode))
	{
		std::cout << "dbg: CGI\n";
		std::cout << "dbg: CGI FULL REQUEST\n" << _fullRequest << std::endl;
		cgiRequest();
	}
	else if (_method == "GET")
	{
		std::cout << "dbg: GET\n"; 
		getRequest();
	}	
	else if (_method == "POST") 
	{
		std::cout << "dbg: POST\n"; 
		if (_uri == "/data")
			handleFileUpload();
		else
			postRequest();
	}	
	else if (_method == "DELETE")
	{
		std::cout << "dbg: DELETE\n"; 
		deleteRequest();
	}
		*/
}

void	Request::sendErrorResponse(int fd, int code, std::map<int, std::string> errorPages)
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

void	Request::handleFileUpload(std::string fileName)
{
	size_t filenamePos = _body.find("filename=\"");
	if (filenamePos == std::string::npos) {
		Log::console(ERROR, "Filename not found in request body.");
		return;
	}
	//Getting the filename so we can store it with the correct name
	size_t filenameStart = filenamePos + 10;
	size_t filenameEnd = _body.find("\"", filenameStart);
	_filename = _body.substr(filenameStart, filenameEnd - filenameStart);	
	//now the content type
	size_t contentTypePos = _body.find("Content-Type: ");
	if (contentTypePos == std::string::npos) {
		Log::console(ERROR, "Content-Type not found in request body.");
		return;
	}
	size_t contentTypeStart = contentTypePos + 15;
	size_t contentTypeEnd = _body.find("\r\n", contentTypeStart);
	_contentType = _body.substr(contentTypeStart, contentTypeEnd - contentTypeStart);
	size_t contentStart = _body.find("\r\n\r\n", filenameEnd) + 4;
	size_t contentEnd = _body.find("\r\n");
	_content = _body.substr(contentStart, contentEnd - contentStart);
	//now we save the file
	std::string	workingDirectory = getWorkingDir();
	std::string	dataPath = workingDirectory + _location.upload_dir;
	if (mkdir(dataPath.c_str(), 0777) == -1 && errno != EEXIST) {
		Log::console(ERROR, "Failed to create directory.");
		return;
	}
	std::string	path = _location.root + _location.upload_dir;
	std::ofstream file((path + '/' + _filename).c_str(), std::ios::binary);
	if (!file) {
		Log::console(ERROR, "Failed to open file.");
		return;
	}
	file.write(_content.c_str(), _content.length());
	file.close();
	std::string	header;
	std::string	body = "<h1>" + fileName + " Posted" + "</h1>";
	header = "HTTP/1.1 200 OK\r\n";
	header += "Content-type: text/html\r\n";
	header += "Content-Length: " + toString(body.length()) + "\r\n\r\n";
	std::string	response = header + body;
	sendFullResponse(_fd, response);
}

std::string	Request::getBoundary()
{
	std::string	boundaryKey = "boundary=";
	size_t	boundaryPos = _requestHeader.find(boundaryKey);

	if (boundaryPos == std::string::npos)
		return ("");
	boundaryPos += boundaryKey.size();
	size_t	boundaryEnd = _requestHeader.find("\r\n", boundaryPos);
	std::string	boundary = "--" + _requestHeader.substr(boundaryPos, boundaryEnd - boundaryPos);
	return (boundary);
}

std::string	Request::getFileContentBoundary(std::string &boundary)
{
	size_t	startBody = _body.find(boundary);
	if (startBody == std::string::npos)
		return ("");
	startBody += boundary.size();
	size_t	headerEnd = _body.find("\r\n\r\n", startBody);
	if (headerEnd == std::string::npos)
		return("");
	headerEnd += 4;
	size_t	endBody = _body.find(boundary, headerEnd);
	if (endBody == std::string::npos)
		return ("");
	std::string	fileContent = _body.substr(headerEnd, endBody - headerEnd - 2);
	return (fileContent);
}

std::string	Request::getFileName()
{
	size_t	filenamePos = _body.find("filename=\"");
	if (filenamePos == std::string::npos)
		return ("");
	filenamePos += 10;
	size_t	filenameEnd = _body.find("\"", filenamePos);
	std::string fileName = _body.substr(filenamePos, filenameEnd - filenamePos);
	return (fileName);
}

void	Request::makeDirectory(std::string &directory)
{
	std::string	wd = getWorkingDir();
	std::string	root;
	if (_location.root[0] == '.')
		root = _location.root.substr(1);
	else
		root = _location.root;
	std::string	path = wd + root + directory;
	struct stat	info;

	if (stat(path.c_str(), &info) != 0)
	{
		if (mkdir(path.c_str(), 0777))
			Log::console(ERROR,  " <" + _config.server_name + "> " + "coundl't create " + directory + " directory for uploading.");
	}
}

void	Request::sendPostResponse(std::string resource)
{
	std::string	decodedResource;
	std::string	content;

	for (size_t i = 0; i < resource.length(); i++)
	{
		if (resource[i] == '+')
			decodedResource += ' ';
		else if (resource[i] == '%' && i + 2 < resource.length())
		{
			std::string	hexValue = resource.substr(i + 1, 2);
			char	decodedChar = static_cast<char>(std::strtol(hexValue.c_str(), NULL, 16));
			decodedResource += decodedChar;
			i+= 2;
		}
		else
			decodedResource += resource[i];
	}
	while (decodedResource.find("=") != std::string::npos)
	{
		decodedResource = decodedResource.substr(decodedResource.find("=") + 1);
		content += "<p>" + decodedResource.substr(0, decodedResource.find("&")) + "</p>";
		decodedResource = decodedResource.substr(decodedResource.find("&") + 2);
	}
	std::string	page = "<!DOCTYPE html><html><head><title>Passed: </title></head><body><h1>" + content + "</h1><br><br>";
	std::string response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: " + toString(page.length()) + "\r\n\r\n" + page;
	sendFullResponse(_fd, response);
}

std::string	Request::getContentType()
{
	std::string	contentTypeKey = "Content-Type: ";
	size_t	contentTypePos = _requestHeader.find(contentTypeKey);
	if (contentTypePos == std::string::npos)
		return ("");
	contentTypePos += contentTypeKey.size();
	size_t contentTypeEndPos;
	if (_requestHeader.find("\r\n", contentTypePos) < _requestHeader.find(";", contentTypePos))
		contentTypeEndPos = _requestHeader.find("\r\n", contentTypePos);
	else
		contentTypeEndPos = _requestHeader.find(";", contentTypePos);
	std::string	contentType = _requestHeader.substr(contentTypePos, contentTypeEndPos - contentTypePos);
	return (contentType);
}

//std::string	Request::getFileContentOctetStream()
//{
//	
//}

void	Request::postRequest()
{
	std::string	uploadStore = _location.upload_dir;
	std::string	contentType = getContentType();
	std::string	content;

	if (uploadStore.empty() || uploadStore == "./" || uploadStore == "/")
		uploadStore = "uploads";
	if (contentType.empty())
	{
		sendErrorResponse(_fd, BAD_REQUEST, _config.error_page);
		return (Log::console(WARNING,   "Couldn't get content type."));
	}
	std::string	boundary = getBoundary();
	if (boundary.empty() && contentType == "multipart/form-data")
	{
		sendErrorResponse(_fd, BAD_REQUEST, _config.error_page);
		return (Log::console(WARNING,   "Couldn't get content boundaries."));
	}
	if (contentType == "application/octet/stream" && boundary.empty())
		content = _body;
	if (contentType == "multipart/form-data" ||
		contentType == "application/json" ||
		contentType == "application/octet-stream")
		content = getFileContentBoundary(boundary);
	//else if (contentType == "application/octet-stream")
	//	content = getFileContentOctetStream();
	else
		content = _body;
	std::string	fileName = getFileName();
	if (content.empty() || (fileName.empty() && contentType == "multipart/form-data"))
	{
		sendErrorResponse(_fd, BAD_REQUEST, _config.error_page);
		return (Log::console(WARNING,   "couldn't process file."));
	}
	makeDirectory(uploadStore);
	if (contentType == "multipart/form-data" ||
		contentType == "application/json" ||
		contentType == "application/octet-stream")
	{
		/*
		std::string	directory = uploadStore + '/' + fileName;
		std::ofstream	ofs(directory.c_str(), std::ios_base::out | std::ios_base::trunc);//WHAT
		if (!ofs)
			return (Log::console(WARNING, "Couldn't write to file."));
		ofs << content;
		if (!ofs)
			return (Log::console(WARNING, "Failed to write content to file: " + fileName));
		ofs.close();*/
		handleFileUpload(fileName);
	}
	else
	{
		_path = _location.root + _location.upload_dir + "/upload.txt";
		if (_location.upload_dir.empty())
		{
			Log::console(WARNING, "Can't post, upload_dir is empty in this location.\n");
		}
		else
		{
			std::ifstream check(_path.c_str());
			bool fileExists = check.is_open();
			check.close();
			std::ofstream file(_path.c_str(), std::ios::app);
			if (!file.is_open())
			{
				Log::console(ERROR, "Failed to open file for writing.");
				sendErrorResponse(_fd, INTERNAL_SERVER, _config.error_page);
				//setErrorResponse(500);
			}
			else {
				file << _body << '\n';
				file.close();
				_contentLength = _body.length();
				setContentType();
				if (fileExists) {
					setHeader(200); }
				else {
					setHeader(201); }
				sendPostResponse(content);
				sendFullResponse(_fd, _header);
			}
		}
	}	
}

std::string	Request::getDeletePath()
{
	std::string	path;

	size_t	location = _uri.find(_location.url);
	if (location == std::string::npos)
		return ("");
	std::string	resource = _uri.substr(location + _location.url.size());
	if (resource[0] == '/')
		path = _location.upload_dir + resource;
	else
		path =  _location.upload_dir + '/' + resource;
	return (path);
}

void 	Request::deleteRequest()
{
	std::string	deletePath = _location.root + getDeletePath();
	std::ifstream check(deletePath.c_str());
	bool fileExists = check.is_open();
	check.close();
	if (!fileExists) {
		Log::console(WARNING,   "Failed to find file for deletion.");
		sendErrorResponse(_fd, NOT_FOUND, _config.error_page);
		//setErrorResponse(404);
		return;
	}
	std::ofstream file(deletePath.c_str());
	if (!file.is_open()) {
		Log::console(ERROR,  " <" + _config.server_name + "> " + "Failed to open file for deletion.");
		sendErrorResponse(_fd, NOT_FOUND, _config.error_page);
		//setErrorResponse(500);
		return;
	} else {
		_contentLength = 0;
		file.close();
		if (std::remove(deletePath.c_str())) {
			Log::console(ERROR,  " <" + _config.server_name + "> " + "Failed to delete file.");
			//setErrorResponse(204); --do the 204 No content one.
			return;
		}
		setHeader(200);
		sendFullResponse(_fd, _header);
	}
	//check if the path exists 
	//check if you can delete it
}

void	Request::cgiRequest()
{
	CGIRequest	cgiRequest(*this, _fullRequest, _fd);

	cgiRequest.handleRequest();
}

void	Request::getRequest()
// ToDo
// throw error when it is a directory and cant access?
{
	std::ifstream	file(_path.c_str());
	std::ostringstream	body;
	
	if (!file)
	{
		sendErrorResponse(_fd, NOT_FOUND, _config.error_page);
		//setErrorResponse(404);
		return ;
	}
	else
	{
		setContentType();
		//if (_contentType.empty())
		//{
		//	setErrorResponse();
		//}
		body << file.rdbuf();
		_content = body.str();
		_contentLength = _content.length();
		setHeader(200);
		_response += _header + _content;
	}
	sendFullResponse(_fd, _response);
}

void	Request::cleanPath()
{
	size_t	qMark = _path.find('?');

	if (qMark != std::string::npos)
	{
		_queryString = _path.substr(qMark + 1);
		_path = _path.substr(0, qMark);
	}
}

std::string	Request::joinPath(const std::string& a, const std::string& b)
{
    if (a.empty()) return b;
    if (b.empty()) return a;
    if (a[a.size() - 1] == '/' && b[0] == '/')
 	   return a + b.substr(1);
    if (a[a.size() - 1] != '/' && b[0] != '/')
 	   return a + "/" + b;
    return a + b;
}

void	Request::setPath()
{
	//std::cout << YELLOW << "SETTING PATH\n"
	//			<< "_uri:  " << _uri
	//			<< "\n_location.url: " << _location.url 
	//			<< "\n_location.root: " << _location.root
	//			<< "\n_location.index: " << _location.index 
	//			<< "\n_location.root: " << _location.root
	//			<< RESET << '\n';
	std::string relative_uri = _uri.substr(_location.url.length());
	if (_uri.find("/cgi-bin") == 0)
		_path = joinPath(_location.root, _uri);
	else if (_uri == _location.url || _uri == _location.url + "/")
	{
		_path = joinPath(_location.root, _location.index);
		//std::cout << YELLOW << "_path = _location.root + \"/\" + _location.index;\n" << _path <<  RESET <<'\n'; 
	}
	else if (_uri[_uri.size() - 1] == '/') {
		_path = joinPath(joinPath(_location.root, relative_uri), _location.index);
	}
	else
	{
		_path = joinPath(_location.root, relative_uri);
		//std::cout << YELLOW << "_path = _location.root + relative_uri;\n" << _path <<  RESET <<'\n'; 
	}
}

void	Request::setContentType()
{
	if (endsWith(_path, ".html") || endsWith(_path, ".htm"))
		_contentType = "text/html";
	else if (endsWith(_path, ".txt"))
		_contentType = "text/plain";
	else if (endsWith(_path, ".css"))
		_contentType = "text/css";
	else if (endsWith(_path, ".js"))
		_contentType = "application/javascript";
	else if (endsWith(_path, ".json"))
		_contentType = "application/json";
	else if (endsWith(_path, ".gif"))
		_contentType = "image/gif";
	else if (endsWith(_path, ".png"))
		_contentType = "image/png";
	else if (endsWith(_path, ".jpg") || endsWith(_path, ".jpeg"))
		_contentType = "image/jpeg";
	else if (endsWith(_path, ".ico"))
	    _contentType= "image/x-icon";
    else
	{
		_contentType = "";
        //Log::console(ERROR, "Unknown type: " + _path + '\n'); // added to see what the tester might send through here
	}
}


void	Request::setErrorResponse(int error)
{
	std::string	path = _location.root + _config.error_page[error];// path to desired error in the future
	std::string	errorStr;
	std::ostringstream	body;
	std::ifstream	errorFile(path.c_str());
	
	switch (error)
	{
		case 400:
			errorStr = "Bad Request";
			break;
		case 401:
			errorStr = "Unauthorized";
			break;
		case 403:
			errorStr = "Forbidden";
			break;
		case 404:
			errorStr = "Not Found";
			break;
		case 405:
			errorStr = "Method Not Allowed";
			break;
		case 409:
			errorStr = "Conflict";
			break;
		case 504:
			errorStr = "Gateway Timeout";
			break;
		default:
			errorStr = "Internal Server Error";
			error = 500;
			break;
	}

	if (errorFile)
	{
		body << errorFile.rdbuf();
		_content = body.str();
		_header += "HTTP/1.1 " + toString(error) + " " + errorStr + "\r\n";
       	_header += "Content-Type: text/html\r\n";
       	_header += "Content-Length: " + toString(_content.length()) + "\r\n";
		_header += "Connection: close\r\n\r\n";
		_response += _header + _content;
	}
	else
	{
		std::cerr << path << " could not found or could not open.\n";
		_response += "HTTP/1.1 500 Internal Server Error\r\n";
           _response += "Connection: close\r\n\r\n";
	}
}


void	Request::setHeader(int code)
{
	if (code == 200) {
		_header += "HTTP/1.1 200 OK\r\n";
	}
	else if (code == 201) {
		_header += "HTTP/1.1 201 Created\r\n";
	}
	_header += "Content-Type: " + _contentType + "\r\n";
	_header += "Content-Length: " + toString(_contentLength) + "\r\n";
	_header += "\r\n";
}

void	Request::setConfig(const std::vector<ServerConfig> &servers)
{
	std::vector<ServerConfig>	svConf = servers;
	size_t	hostPos = _fullRequest.find("Host:");
	std::string	hostStr, host;
	
	if (hostPos != std::string::npos)
	{
		size_t lineEnd = _fullRequest.find("\r\n", hostPos);
		hostStr = _fullRequest.substr(hostPos, lineEnd - hostPos);
		host = hostStr.substr(hostStr.size() - 4, hostStr.size());
	}
	for (size_t i = 0; i < svConf.size(); i++)
	{
		std::vector<int>	currPorts = svConf[i].port;
		for (size_t j = 0; j < currPorts.size(); j++)
		{
			if (toString(currPorts[j]) == host)
			{
				_config = svConf[i];
			}
		}
	}
	if (_config.server_name.empty())
	{
		if (!svConf[0].server_name.empty())
			_config = svConf[0];
	}
}

void	Request::setRedirectResponse()
{
	_response = "HTTP/1.1 " + toString(_location.redirect.first) + " Redirect\r\n";
	_response += "Location: " + _location.redirect.second + "\r\n\r\n";
}

/************************************************************************/
/*                                                                      */
/*                               autoindex                              */
/*                                                                      */
/************************************************************************/

void	Request::handleAutoIndexResponse()
{
	DIR	*dir = opendir(_path.c_str()); // returns a pointer to the directory stream
	if (!dir)
	{
		Log::console(ERROR,  " <" + _config.server_name + "> " + "Failed to open dir: " + _path + " for autoindexing.\n");
		return;
	}
	std::stringstream	body;
	body << "<!DOCTYPE html><html><head><title>Home</title><style>";
	body << "body{font-family: 'Helvetica neue', Arial, sans-serif;background: linear-gradient(0deg, rgba(176, 72, 169, 1) 35%, rgba(71, 139, 222, 1) 100%);background-attachment: fixed;background-size: cover;color: #333;}";
	body << "#container{background-color: #fff;margin: 5% auto;min-height: 50%;width: 80%;padding: 2%;border-radius: 15px;box-shadow: 0 4px 10px rgba(0, 0, 0, 0.1);}";
	body << "#header{margin: 5% auto 0 auto;text-align: center;}#webserv-logo{max-width: 400px;}";
	body << "button{padding: 10px 20px;margin-top: 15px;font-size: 16px;background-color: rgba(71, 139, 222, 1);color: white;border: none;border-radius: 5px;cursor: pointer;transition: background-color 0.2s ease;}button:hover{background-color: rgba(176, 72, 169, 1);}";
	body <<	"</style></head><body><div id=\"header\"><a href=\"/\"><img id=\"webserv-logo\" src=\"/assets/img/webserv-logo.png\" alt=\"webserv-logo\"></a></div>";
	body <<	"<div id=\"container\"><ul>";
	
			
	struct dirent	*entry;
	while ((entry = readdir(dir)))// returns a pointer to a dirent struct representing the next directory stream
	{
		std::string	currName = entry->d_name;
		std::string	hrefLink = _uri + "/" + currName;
		if (currName == "." || currName == "..")
			continue;
		struct stat	currStat;
		if (stat(_path.c_str(), &currStat) == 0 && S_ISDIR(currStat.st_mode))
			currName = "/" + currName;
		body << "<li><a href=\""<< hrefLink << "\">" << currName << "</a></li>";
	}
	closedir(dir);
	
	body << "</ul></div></body></html>";

	setContentType();
	_content = body.str();
	_contentLength = _content.length();
	setHeader(200);
	_response = _header + _content;
	sendFullResponse(_fd, _response);
}

/************************************************************************/
/*                                                                      */
/*                               autoindex                              */
/*                                  end                                 */
/************************************************************************/

const std::string	&Request::getMethod() const { return(_method); }

const std::string	&Request::getUri() const { return(_uri); }

const std::string	&Request::getProtocol() const { return(_protocol); }

const std::string	&Request::getHeader() const { return(_header); }

const std::string	&Request::getBody() const { return(_body); }

ServerConfig	Request::getConfig() const { return(_config); }