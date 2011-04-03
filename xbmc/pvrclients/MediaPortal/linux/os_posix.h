#pragma once
/*
 *      Copyright (C) 2005-2010 Team XBMC
 *      http://www.xbmc.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef PVRCLIENT_MEDIAPORTAL_OS_POSIX_H
#define PVRCLIENT_MEDIAPORTAL_OS_POSIX_H

#define _FILE_OFFSET_BITS 64
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <sys/timeb.h>
#include <sys/wait.h>
#include <sys/signal.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <pthread.h>
#include <semaphore.h>
#include <limits.h>

typedef int bool_t;
typedef unsigned char byte;
//
// Success codes
#define S_OK                             0L
#define S_FALSE                          1L
//
// Error codes
#define ERROR_FILENAME_EXCED_RANGE       206L
#define E_OUTOFMEMORY                    0x8007000EL
#define E_FAIL                           0x80004005L
#define INVALID_HANDLE_VALUE             -1

// Socket related:
typedef int SOCKET;
#define SD_BOTH SHUT_RDWR
#define sock_getlasterror errno
#define sock_getlasterror_socktimeout (errno == EAGAIN)
#define LIBTYPE

#define THREAD_FUNC_PREFIX void *

typedef pthread_mutex_t criticalsection_t;
//typedef sem_t wait_event_t;
#define wait_event_t sem_t
typedef void* LPSECURITY_ATTRIBUTES; 

#define PATH_SEPARATOR_CHAR '/'
#ifdef PATH_MAX
#define MAX_PATH PATH_MAX
#else
#define MAX_PATH 260
#endif

//From winbase.h
#define FILE_BEGIN           0
#define FILE_CURRENT         1
#define FILE_END             2

//From WinError.h
#define SUCCEEDED(hr) (((long)(hr)) >= 0)
#define FAILED(hr) (((long)(hr)) < 0)
#define NOERROR              0
#define ERROR_INVALID_NAME   123L

static inline long min(int a, long b)
{
  if((long) a < b)
    return a;
  else
    return b;
}

static inline uint64_t getcurrenttime(void)
{
	struct timeval t;
	gettimeofday(&t, NULL);
	return ((uint64_t)t.tv_sec * 1000) + (t.tv_usec / 1000);
}

static inline int setsocktimeout(int s, int level, int optname, uint64_t timeout)
{
	struct timeval t;
	t.tv_sec = timeout / 1000;
	t.tv_usec = (timeout % 1000) * 1000;
	return setsockopt(s, level, optname, (char *)&t, sizeof(t));
}

static inline void Sleep(unsigned long dwMilliseconds)
{
  usleep(dwMilliseconds*1000);
}

#endif
