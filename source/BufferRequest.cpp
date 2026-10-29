#include "../include/BufferRequest.hpp"

BufferRequest::BufferRequest(): _isOver(false), _currLength(0), _contentLength(0)
{
	std::string	contentLenStr = "Content-Length: ";

	size_t	contentLenPos = _buffer.find(contentLenStr);
	if (contentLenPos != std::string::npos)
	{
		std::string::iterator	it = _buffer.begin() + contentLenPos + contentLenStr.length();
		std::string	len;

		while (it != _buffer.end() && std::isdigit(*it))
		{
			len += *it;
			it++;
		}
		_contentLength = std::atoi(len.c_str());
	}
	else
		_contentLength = 0;
}

BufferRequest::~BufferRequest(void) {}

void	BufferRequest::appendBuffer(const char *buffer, size_t size)
{
	_buffer.append(buffer, size);
	if (!_contentLength)
		setContentLength();
	finishRequest();
}

std::string	&BufferRequest::getBuffer(void) { return(_buffer); }

size_t	BufferRequest::getContentLength(void) const { return(_contentLength); }

size_t	BufferRequest::getCurrentLength(void) const { return(_currLength); }

void	BufferRequest::setContentLength(void)
{
	std::string	contentLenStr = "Content-Length: ";

	size_t	contentLenPos = _buffer.find(contentLenStr);
	if (contentLenPos != std::string::npos)
	{
		std::string::iterator	it = _buffer.begin() + contentLenPos + contentLenStr.length();
		std::string	len;

		while (it != _buffer.end() && std::isdigit(*it))
		{
			len += *it;
			it++;
		}
		_contentLength = std::atoi(len.c_str());
	}
	else
		_contentLength = 0;
}

void	BufferRequest::finishRequest(void)
{
	if (_contentLength)
	{
		size_t requestEnd = _buffer.find("\r\n\r\n");
		if (requestEnd == std::string::npos && _contentLength != 0)
			std::cerr << "Error couldn't find body?\n";
		std::string::iterator	it = _buffer.begin() + requestEnd + 4;
		_currLength = std::distance(it, _buffer.end());
		if (_contentLength == _currLength)
			_isOver = true;
	}
	else
		_isOver = true;
}

bool	BufferRequest::requestOver(void) const { return(_isOver); }