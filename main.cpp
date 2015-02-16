#include <string.h>
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
		//char tmp = str[i];
		str[i] ^= str[j];
		str[j] ^= str[i];
		str[i] ^= str[j];
		//str[j] = tmp;
	}
}

int main()
{
	//std::cout << "Hello World!\n";
	//std::cout << "new line!" << std::endl;

	void *ptr = nullptr;
	int x = sizeof(ptr);

	mtest t;
	t("ptr size: %u", x);

	char buf[] = "test string revers for gcc this member";

	t(buf);
	reverse_string(buf);
	t(buf);
	reverse_word(buf);
	t(buf);

	return 0;
}

