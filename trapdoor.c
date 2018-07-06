/*
 * Fun with valgrind trapdoor mechanism
 * Jul 3, 2018
 * root@davejingtian.org
 * https://davejingtian.org
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <valgrind/valgrind.h>

/* From the above header */
#define valgrind_printf_code		0x1403
#define valgrind_printf_fmt_str		"daveti: trapdoor, magic [%d]\n"
#define valgrind_trapdoor_code		\
	"rol $0x3, %%rdi\n\t"		\
	"rol $0xd, %%rdi\n\t"		\
	"rol $0x3d, %%rdi\n\t"		\
	"rol $0x33, %%rdi\n\t"


static unsigned long valgrind_printf_manual(char *fmt, ...)
{
	unsigned long args[6] = {0};
	unsigned long ret = 0;
	va_list vargs;

	/* Follow valgrind ABI */
	va_start(vargs, fmt);
	args[0] = (unsigned long)valgrind_printf_code;
	args[1] = (unsigned long)fmt;
	args[2] = (unsigned long)&vargs;

	/* rdx = client_req(rax); */
	asm volatile ("mov $0x0, %%rdx\n\t"	\
			valgrind_trapdoor_code	\
			"xchg %%rbx, %%rbx\n\t"	\
			: "=d"(ret)		\
			: "a"(&args[0])		\
			: "cc", "memory");
	va_end(vargs);
	return ret;
}

int main(void)
{
	int ret = 0;
	int magic = 777;

	/* Normal valgrind trapdoor */
	ret = VALGRIND_PRINTF(valgrind_printf_fmt_str, magic);
	printf("daveti: ret [%d]\n", ret);

	/* Homemade valgrind trapdoor */
	ret = valgrind_printf_manual(valgrind_printf_fmt_str, magic);
	printf("daveti: ret [%d]\n", ret);

	/* Check ret */
	if (ret == 0)
		printf("normal workflow\n");
	else
		printf("screw valgrind\n");

	return 0;
}
