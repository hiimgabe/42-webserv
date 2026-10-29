# Webserv

Webserv is an educational HTTP server written in C++, built with C++98 compiler flags. It implements socket handling, request parsing, routing, and responses directly, with a `poll()` event loop and CGI processes launched through `fork()` and `execve()`.

The included website demonstrates HTTP methods and CGI alongside pages explaining how the server works.

## Build and run

Use a Linux/POSIX environment with Make and a C++ compiler available as `c++`. The code depends on POSIX sockets, `poll()`, pipes, and process APIs; native Windows builds are not provided. On Windows, run the commands in a Linux environment such as WSL.

From the project directory containing the [Makefile](Makefile):

```sh
make
./webserv config/tester.conf
```

If you are in the enclosing `webserv` directory, run `cd webserv-main` first. Keep the server's working directory here: configured paths such as `./www` are resolved relative to it.

Open [http://localhost:8080](http://localhost:8080) for the demo. Stop the server with **Ctrl+C**. A configuration file is required; there is no default configuration selected when the argument is omitted.

The supplied [tester configuration](config/tester.conf) starts two server configurations:

| Server | Ports | Root location methods | Non-CGI POST body limit |
| --- | --- | --- | --- |
| `webserv-1` | `8080`, `5050` | `GET` | 100 bytes |
| `webserv-2` | `8000`, `5000` | `GET`, `POST`, `DELETE` | 1024 bytes |

All four ports must be available. Listeners bind to all IPv4 interfaces (`INADDR_ANY`), even when accessed through `localhost`.

Use `tester.conf` as the starting point. [eg.conf](config/eg.conf) references a missing `web/` directory and places `root` and `index` outside the location blocks used by the implementation.

### Build targets

| Command | Effect |
| --- | --- |
| `make` | Build `webserv`, with object files in `bin/`. |
| `make clean` | Remove `bin/`. |
| `make fclean` | Remove objects and `webserv`; also attempt to remove `upload/upload.txt`. |
| `make re` | Clean and rebuild. |

The compiler flags are `-Wall -Wextra -Werror -std=c++98 -g`. Cleanup targets do not clear the demo's `www/uploads/` directory.

## Try the server

With the server running, use a second terminal:

```sh
# Fetch the demo homepage.
curl -i http://localhost:8080/

# List the assets directory.
curl -i http://localhost:8080/assets/

# Inspect a configured redirect without following it.
curl -i http://localhost:8080/redirect

# Request a missing resource.
curl -i http://localhost:8080/does-not-exist
```

The [browser request tester](http://localhost:8000/testing.html) can send requests to the server. Use URLs on the same origin, such as `http://localhost:8000/`, to avoid browser cross-origin restrictions. Its method menu includes `PUT`, but the server implements handlers for `GET`, `POST`, and `DELETE` only.

For a form POST example:

```sh
curl -i --data 'message=Hello' http://localhost:8000/
```

This appends the encoded body and a newline to `www/uploads/upload.txt`, creating the file if needed. Multipart upload handling is also present; file uploads use the submitted filename and can overwrite an existing file. `DELETE` resolves resources under the selected location's upload directory, rather than deleting arbitrary static resources by their GET URL.

There is no `make test` target or source-based automated test suite. The repository includes prebuilt `ubuntu_tester` and `cgi_tester` executables, but does not include their sources or usage documentation. The requests above provide a small manual smoke check.

## Configuration

Configuration is line-oriented, with `server {` and `location /path {` opening their own lines, one directive per line, and closing braces on separate lines. Use semicolon-terminated directives as in `tester.conf`. Blank lines and lines starting with `#` are ignored.

A minimal configuration for serving the included website is:

```nginx
server {
    listen 8080;
    server_name webserv;
    client_max_body_size 1024;

    location / {
        root ./www;
        index index.html;
        methods GET;
    }
}
```

To use this example, save it as `config/local.conf` and run `./webserv config/local.conf` from the project directory.

| Scope | Directive | Meaning |
| --- | --- | --- |
| Server | `listen 8080 5050;` | One or more numeric listening ports. |
| Server | `server_name webserv;` | Server label used in logging; not hostname-based virtual hosting. |
| Server | `client_max_body_size 1024;` | Integer byte limit checked for non-CGI POST requests. Set it explicitly; size suffixes such as `1M` are not supported. |
| Server | `error_page 400 /error/400.html;` | Custom error file, appended to the selected location's root. |
| Location | `root ./www;` | Filesystem base for this location. |
| Location | `index index.html;` | File selected for a location root or trailing-slash request. |
| Location | `methods GET POST DELETE;` | Allowed methods for this location. |
| Location | `autoindex on;` | Generate a listing when the resolved path is a directory. Defaults to off. |
| Location | `upload_store ./uploads;` | Upload subdirectory; with `root ./www`, the sample resolves to `www/uploads`. |
| Location | `redirect 301 https://example.com;` | Return a redirect status and destination. |
| Location | `cgi_path /usr/bin/python3 /bin/bash;` | Interpreter paths paired by position with CGI extensions. |
| Location | `cgi_extension .py .sh .cgi;` | CGI extension mapping; `.cgi` runs directly. |

Locations use the longest matching literal URL prefix. For ordinary files, the matched prefix is removed before joining the remaining path to `root`: `/assets/css/main.css` under `location /assets` with `root ./www/assets` maps to `www/assets/css/main.css`. Keep a `location /` fallback.

The regex-shaped `location ~ \.bla$` blocks in `tester.conf` are not interpreted as regular expressions by the router.

### CGI

The first server in `tester.conf` configures `/cgi-bin` for `GET` and `POST`, with Python at `/usr/bin/python3` and Bash at `/bin/bash`. Install those interpreters or adjust the paths before using the examples. The statistics script additionally uses Linux command-line utilities, including `top`, `df`, `uptime`, `awk`, `sed`, and `bc`.

```sh
curl -i http://localhost:8080/cgi-bin/test.py
curl -i --data 'msg=Hello' http://localhost:8080/cgi-bin/test.py
```

The Python example renders a form and echoes its `msg` field. The Bash example is available at [http://localhost:8080/cgi-bin/server-stats.sh](http://localhost:8080/cgi-bin/server-stats.sh).

Keep interpreter paths in the same order as their extensions: `.py` maps to the first path, `.sh` to the second. A trailing `.cgi` entry needs no interpreter; its file must be executable and compatible with the host platform. CGI dispatch is tied to URLs beginning with `/cgi-bin`, so merely adding CGI directives to another location does not enable execution there.

## Implementation limits

- Request bodies are framed using `Content-Length`; chunked transfer decoding is not implemented.
- Client connections are closed after a request; persistent connections are not implemented.
- Server selection reads the last four characters of the `Host` header to match a configured port, falling back to the first server. The supplied four-digit ports fit this implementation; general hostname and port routing is not implemented.
- `client_max_body_size` is checked after buffering and only in the non-CGI POST branch. It does not limit CGI bodies.
- JSON and binary POST types share multipart-oriented parsing code. Use the form example above for a simple POST; this is not a general JSON API server.

## Source map

| Path | Responsibility |
| --- | --- |
| [source/main.cpp](source/main.cpp) | Command-line entry point, configuration loading, and signal setup. |
| [source/Config.cpp](source/Config.cpp) | Configuration parsing, socket setup, and the `poll()` loop. |
| [source/BufferRequest.cpp](source/BufferRequest.cpp) | Request buffering and body-length tracking. |
| [source/Request.cpp](source/Request.cpp) | Method handlers, static files, uploads, redirects, and responses. |
| [source/CGIRequest.cpp](source/CGIRequest.cpp) | CGI environment, pipes, child processes, and output handling. |
| [include/](include/) | Class declarations and shared types. |
| [config/](config/) | Configuration examples. |
| [www/](www/) | Demo website, CGI examples, error pages, and upload fixtures. |
