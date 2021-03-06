#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>

static char buf[1024*8];

static int no_proxy = 0;

static void parse_arg(int argc, char **argv)
{
	int c;
	int digit_optind = 0;

	while (1) {
		int this_option_optind = optind ? optind : 1;
		int option_index = 0;
		static struct option long_options[] = {
			{"no_proxy",  0,  0,  'n' },
			{0,           0,  0,  0   }
		};

		c = getopt_long_only(argc, argv, "", long_options, &option_index);
		if (c == -1)
			break;

		switch (c) {
			case 'n':
				no_proxy = 1;
				break;
			case '?':
				break;
			default:
				;
		}
	}
}

int main(int argc, char **argv)
{
	int s=-1, ret;
	struct addrinfo hints, *res;
	char *proxy;
	char *host, *port;

	parse_arg(argc, argv);

	proxy = getenv("http_proxy");

	if (!no_proxy && proxy) {
		if (strncmp(proxy, "http://", 7) != 0) {
			perror("proxy invalid");
			return -1;
		}
		host = proxy + 7;
		proxy = strchr(host, ':');
		if (!proxy) {
			perror("proxy invalid");
			return -1;
		}
		*proxy ='\0';
		port = proxy + 1;
		strtol(port, &proxy, 10);
		if (proxy) {
			*proxy ='\0';
		}
		printf("HTTP_PROXY = http://%s:%s\n\n", host, port);
	} else {
		host = "panasonic.com";
		port = "80";
	}

	printf("GET http://panasonic.com\n\n");

	memset(&hints, 0, sizeof(hints));
	hints.ai_socktype = SOCK_STREAM;

	if (getaddrinfo(host, port, &hints, &res) != 0) {
		perror("getaddrinfo error");
		return -1;
	}

	s = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if (s <0) {
		perror("socket error");
		return -1;
	}

	if (connect(s, (struct sockaddr *)res->ai_addr, res->ai_addrlen) != 0) {
		perror("connect error");
		close(s);
		return -1;
	}

	ret = sprintf(buf,
			"GET http://panasonic.com/ HTTP/1.1\r\n" \
			"User-Agent: Wget/1.15 (linux-gnu)\r\n" \
			"Accept: */*\r\n" \
			"Host: panasonic.com\r\n" \
			"Connection: Close\r\n" \
			"Proxy-Connection: Keep-Alive\r\n" \
			"\r\n"
			);

	printf("request\n%s\n\n", buf);

	if (send(s, buf, ret, 0) != ret) {
		perror("send error");
		close(s);
		return -1;
	}

	if (recv(s, buf, sizeof(buf), 0) <= 0) {
		perror("recv error");
		close(s);
		return -1;
	}

	printf("response\n%s\n", buf);

	close(s);

	return 0;
}
