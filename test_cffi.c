#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int add(int a, int b)
{
	printf("\"add\" called, result = %d\n", a + b);
	return a + b;
}

char *connect_str(const char *lhs, const char *rhs)
{
	printf("\"connect_str\" called, lhs = \"%s\", rhs = \"%s\"\n", lhs, rhs);
	char *ptr = malloc((strlen(lhs) + strlen(rhs) + 1)*sizeof(char));
	strcpy(ptr, lhs);
	strcat(ptr, rhs);
	return ptr;
}

void free_str(char *ptr)
{
	printf("\"free_str\" called, ptr = %zu\n", ptr);
	free(ptr);
}

void print(const char* str)
{
	printf("\"print\" called, str = \"%s\"\n", str);
}
