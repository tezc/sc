/*
 * MIT License
 *
 * Copyright (c) 2021 Ozan Tezcan
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE 700
#endif

#include "sc_mmap.h"

#include <errno.h>
#include <string.h>
#include <sys/stat.h>

#if defined(_WIN32)
#include <io.h>
#include <stdint.h>

#pragma warning(disable : 4996)

static void sc_mmap_errstr(struct sc_mmap *m)
{
	int rc;
	DWORD err = GetLastError();
	LPSTR errstr = 0;

	rc = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER |
				    FORMAT_MESSAGE_FROM_SYSTEM,
			    NULL, err, 0, (LPSTR) &errstr, 0, NULL);
	if (rc != 0) {
		strncpy(m->err, errstr, sizeof(m->err) - 1);
		LocalFree(errstr);
	}
}

int sc_mmap_init(struct sc_mmap *m, const char *name, int file_flags, int prot,
		 int map_flags, size_t offset, size_t len)
{
	const int mode = prot & PROT_WRITE ? _S_IREAD | _S_IWRITE : _S_IREAD;
	struct _stat64 st;
	int fd, rc, saved_err = 0;
	void *p = NULL;

	*m = (struct sc_mmap){
		.ptr = NULL,
		.fd = -1,
		.len = 0,
	};

	fd = _open(name, file_flags, mode);
	if (fd == -1) {
		goto error;
	}

	rc = _stat64(name, &st);
	if (rc != 0) {
		goto cleanup_fd;
	}

	len = (len == 0) ? (size_t) st.st_size - offset : len;

	HANDLE fm, h = INVALID_HANDLE_VALUE;
	const size_t max_size = offset + len;

	const DWORD offset_low = (offset & 0xFFFFFFFFL);
	const DWORD offset_high = ((uint64_t) offset >> 32) & 0xFFFFFFFFL;
	const DWORD size_low = (max_size & 0xFFFFFFFFL);
	const DWORD size_high = ((uint64_t) max_size >> 32) & 0xFFFFFFFFL;
	const DWORD protect = (prot & PROT_WRITE) ? PAGE_READWRITE :
							  PAGE_READONLY;

	if ((map_flags & MAP_ANONYMOUS) == 0) {
		h = (HANDLE) _get_osfhandle(fd);
		if (h == INVALID_HANDLE_VALUE) {
			goto cleanup_fd;
		}
	}

	fm = CreateFileMapping(h, NULL, protect, size_high, size_low, NULL);
	if (fm == NULL) {
		goto cleanup_fd;
	}

	p = MapViewOfFile(fm, prot, offset_high, offset_low, len);
	CloseHandle(fm);

	if (p == NULL) {
		goto cleanup_fd;
	}

	m->fd = fd;
	m->ptr = p;
	m->len = len;

	return 0;

cleanup_fd:
	saved_err = GetLastError();
	_close(fd);
	SetLastError(saved_err);
error:
	sc_mmap_errstr(m);

	return -1;
}

int sc_mmap_msync(struct sc_mmap *m, size_t offset, size_t len)
{
	BOOL b;
	char *p = (char *) m->ptr + offset;

	b = FlushViewOfFile(p, len);
	if (b == 0) {
		sc_mmap_errstr(m);
		return -1;
	}

	return 0;
}

int sc_mmap_mlock(struct sc_mmap *m, size_t offset, size_t len)
{
	BOOL b;
	char *p = (char *) m->ptr + offset;

	b = VirtualLock((LPVOID) p, len);
	if (b == 0) {
		sc_mmap_errstr(m);
		return -1;
	}

	return 0;
}

int sc_mmap_munlock(struct sc_mmap *m, size_t offset, size_t len)
{
	BOOL b;
	char *p = (char *) m->ptr + offset;

	b = VirtualUnlock((LPVOID) p, len);
	if (b == 0) {
		sc_mmap_errstr(m);
		return -1;
	}

	return 0;
}

int sc_mmap_term(struct sc_mmap *m)
{
	BOOL b;
	int rc = 0;

	if (m->fd == -1) {
		return 0;
	}

	_close(m->fd);

	b = UnmapViewOfFile(m->ptr);
	if (b == 0) {
		sc_mmap_errstr(m);
		rc = -1;
	}

	m->fd = -1;
	m->ptr = NULL;
	m->len = 0;

	return rc;
}

#else

#include <unistd.h>

int sc_mmap_init(struct sc_mmap *m, const char *name, int file_flags, int prot,
		 int map_flags, size_t offset, size_t len)
{
	const int mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;

	int fd, rc, saved_errno;
	void *p = NULL;
	struct stat st;

	*m = (struct sc_mmap){
		.ptr = NULL,
		.fd = -1,
		.len = 0,
	};

	fd = open(name, file_flags, mode);
	if (fd == -1) {
		goto error;
	}

	rc = stat(name, &st);
	if (rc != 0) {
		goto cleanup_fd;
	}

	len = (len == 0) ? st.st_size - offset : len;

	if (prot & PROT_WRITE) {
#if defined(__APPLE__)
		int block = st.st_blksize;
		size_t pos = (st.st_size / block) * block + block - 1;

		for (; pos < len + block - 1; pos += block) {
			if (pos >= len) {
				pos = len - 1;
			}

			ssize_t seek = lseek(fd, pos, SEEK_SET);
			if (seek == -1) {
				goto cleanup_fd;
			}

			ssize_t written = write(fd, "", 1);
			if (written != 1) {
				goto cleanup_fd;
			}
		}
#else
		do {
			rc = posix_fallocate(fd, offset, len);
		} while (rc == EINTR);

		if (rc != 0) {
			errno = rc;
			goto cleanup_fd;
		}
#endif
	}

	p = mmap(NULL, len, prot, map_flags, fd, offset);
	if (p == MAP_FAILED) {
		goto cleanup_fd;
	}

	m->fd = fd;
	m->ptr = p;
	m->len = len;

	return 0;

cleanup_fd:
	saved_errno = errno;
	close(fd);
	errno = saved_errno;
error:
	strncpy(m->err, strerror(errno), sizeof(m->err) - 1);

	return -1;
}

int sc_mmap_term(struct sc_mmap *m)
{
	int rc;

	if (m->fd == -1) {
		return 0;
	}

	close(m->fd);

	rc = munmap(m->ptr, m->len);
	if (rc != 0) {
		strncpy(m->err, strerror(errno), sizeof(m->err) - 1);
	}

	m->fd = -1;
	m->ptr = NULL;
	m->len = 0;

	return rc;
}

int sc_mmap_msync(struct sc_mmap *m, size_t offset, size_t len)
{
	int rc;
	char *p = (char *) m->ptr + offset;

	rc = msync(p, len, MS_SYNC);
	if (rc != 0) {
		strncpy(m->err, strerror(errno), sizeof(m->err) - 1);
	}

	return rc;
}

int sc_mmap_mlock(struct sc_mmap *m, size_t offset, size_t len)
{
	int rc;
	char *p = (char *) m->ptr + offset;

	rc = mlock(p, len);
	if (rc != 0) {
		strncpy(m->err, strerror(errno), sizeof(m->err) - 1);
	}

	return rc;
}

int sc_mmap_munlock(struct sc_mmap *m, size_t offset, size_t len)
{
	int rc;
	char *p = (char *) m->ptr + offset;

	rc = munlock(p, len);
	if (rc != 0) {
		strncpy(m->err, strerror(errno), sizeof(m->err) - 1);
	}

	return rc;
}

const char *sc_mmap_err(struct sc_mmap *m)
{
	return m->err;
}

#endif
