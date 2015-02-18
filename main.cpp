#include <string.h>
#include <unistd.h>
#include <sys/fcntl.h>
#include <sys/uio.h>
#include "mtest.h"


void reverse_word(char *str)
{
	char *start = str;
	char *ptr = start;

	while (true)
	{
		while (*ptr != ' ' && *ptr != '\0') ptr++;
		size_t len = ptr - start;
		size_t len2 = len / 2;

		for (size_t i = 0, j = len - 1; i < len2; i++, j--)
		{
			start[i] ^= start[j];
			start[j] ^= start[i];
			start[i] ^= start[j];
		}

		if (*ptr == '\0')
			break;
		start = ++ptr;
	}
}


void reverse_string(char *str)
{
	size_t len = strlen(str);
	size_t len2 = len / 2;


	for (size_t i = 0, j = len-1; i < len2; i++, j--)
	{
		str[i] ^= str[j];
		str[j] ^= str[i];
		str[i] ^= str[j];
	}
}

int main()
{
	void *mem = nullptr;
	int x = sizeof(mem);

	mtest t;
	t("ptr size: %u", x);

	char buf[] = "test string revers for gcc this member";

	t(buf);
	reverse_string(buf);
	t(buf);
	reverse_word(buf);
	t(buf);


	// ==============
	const char *file = "/home/fibo/Projects/test_git/mtest.cpp";
	int fd = open(file, O_RDONLY);

	if (fd == -1)
		return EXIT_FAILURE;

	char foo[64];
	char bar[128];
	iovec iov[2];

	iov[0].iov_base = foo;
	iov[0].iov_len = sizeof(foo);
	iov[1].iov_base = bar;
	iov[1].iov_len = sizeof(bar);

	/*ssize_t ret = */readv(fd, iov, 2);

	foo[sizeof(foo)-1] = '\0';
	t(foo);
	t("===============\n");
	bar[sizeof(bar)-1] = '\0';
	t(bar);

	close(fd);

	return 0;


//	const char *file = "/home/fibo/Projects/test_git/mtest.cpp";
//	int fd = open(file, O_RDONLY);

//	if (fd == -1)
//		return EXIT_FAILURE;

//	off_t size = lseek(fd, 0, SEEK_END);
//	t("file: %s\nsize: %jd\n", file, size);
//	lseek(fd, 0, SEEK_SET);

//	char *buffer = new char [size];
//	char *ptr = buffer;

//	ssize_t ret;
//	while (size != 0 && (ret = read(fd, ptr, size)) != 0)
//	{
//		if (ret == -1)
//		{
//			if (errno == EINTR)
//				continue;
//			perror("read");
//			break;
//		}

//		size -= ret;
//		ptr += ret;
//	}

//	t(buffer);

//	delete [] buffer;

//	close(fd);

//	return 0;
}

