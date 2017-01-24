#ifndef CONSTANTS_H_
#define CONSTANTS_H_

#pragma once

// MSG_NOSIGNAL does not exists on OS X
#if defined(__APPLE__) || defined(__MACH__)
# ifndef MSG_NOSIGNAL
#   define MSG_NOSIGNAL SO_NOSIGPIPE
# endif
#endif

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/event.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>

#include <errno.h>
#include <string.h>
#include <err.h>

#include <iostream>
#include <algorithm>
#include <map>
#include <vector>
#include <sstream>
#include <memory>

#define MESSAGE_PART_LEN 1024
#define MAX_EVENTS 32

#endif /* CONSTANTS_H_ */
