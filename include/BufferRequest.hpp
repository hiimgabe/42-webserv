
#ifndef	BUFFERREQUEST_HPP
#define	BUFFERREQUEST_HPP

#include <iostream>
#include <cstdlib>

class   BufferRequest
{
    public:
		BufferRequest(void);
		~BufferRequest(void);
		
		void	appendBuffer(const char *buffer, size_t size);

		std::string	&getBuffer(void);
		size_t	getContentLength(void) const;
		size_t	getCurrentLength(void) const;

		void	setContentLength(void);
		void	finishRequest(void);

		bool	requestOver(void) const;
    
	private:
		std::string	_buffer;
		bool	_isOver;
		size_t	_currLength;
		size_t	_contentLength;
};

#endif