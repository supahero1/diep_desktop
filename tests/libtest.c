/*
 *   Copyright 2025 Franciszek Balcerak
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#include <DiepDesktop/shared/debug.h>

#include <gelf.h>
#include <fcntl.h>
#include <stdio.h>
#include <libelf.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>


static int tty_fd = 1;


static void
test_say_common(
	const char* format,
	bool important,
	va_list args
	)
{
	char buffer[4096];
	sprintf(buffer, important ?
		"\033[1m\033[35m[TEST]\033[39m > %s <\033[0m\n" :
		"\033[1m\033[35m[TEST]\033[39m\033[0m %s\n", format);

	va_list copy;
	va_copy(copy, args);
	vdprintf(tty_fd, buffer, copy);
	va_end(copy);

	sprintf(buffer, important ? "[TEST] > %s <\n" : "[TEST] %s\n", format);
	vfprintf(stderr, buffer, args);
}


static void
__attribute__((format(printf, 1, 2)))
test_say(
	const char* format,
	...
	)
{
	va_list args;
	va_start(args, format);
	test_say_common(format, false, args);
	va_end(args);
}


static void
__attribute__((format(printf, 1, 2)))
test_say_important(
	const char* format,
	...
	)
{
	va_list args;
	va_start(args, format);
	test_say_common(format, true, args);
	va_end(args);
}


int
main(
	void
	)
{
	tty_fd = open("/dev/tty", O_WRONLY);
	assert_neq(tty_fd, -1);

	int fd = open("/proc/self/exe", O_RDONLY);
	assert_neq(fd, -1);

	assert_neq(elf_version(EV_CURRENT), EV_NONE);

	Elf* elf = elf_begin(fd, ELF_C_READ, NULL);
	assert_not_null(elf);

	GElf_Ehdr ehdr;
	assert_not_null(gelf_getehdr(elf, &ehdr));

	Elf_Scn* scn = NULL;
	GElf_Shdr shdr;

	int tests = 0;
	int passed = 0;

	while((scn = elf_nextscn(elf, scn)) != NULL)
	{
		if(gelf_getshdr(scn, &shdr) == NULL) continue;

		if(shdr.sh_type == SHT_SYMTAB)
		{
			Elf_Data* data = elf_getdata(scn, NULL);
			assert_not_null(data);

			int num_symbols = shdr.sh_size / shdr.sh_entsize;
			for(int i = 0; i < num_symbols; ++i)
			{
				GElf_Sym symbol;
				if(gelf_getsym(data, i, &symbol) == NULL) continue;

				const char* sym_name = elf_strptr(elf, shdr.sh_link, symbol.st_name);
				char method[5];
				char name[256];
				int len = sscanf(sym_name, "test_should_%4[a-zA-Z0-9_]__%255[a-zA-Z0-9_]", method, name);
				if(len != 2) continue;

				if(
					strcmp(method, "fail") &&
					strcmp(method, "pass")
					)
				{
					test_say("Invalid test method '%s'", method);
					continue;
				}

				++tests;
				bool should_pass = !strcmp(method, "pass");

				void (*test_func)() = (void*) symbol.st_value;
				assert_not_null(test_func);

				test_say("Running test '%s', expecting %s", name, should_pass ? "success" : "failure");

				int pid = fork();
				if(pid == 0)
				{
					test_func();
					exit(0);
				}
				else
				{
					int status;
					int wpid = waitpid(pid, &status, 0);
					assert_eq(wpid, pid);

					bool success = WIFEXITED(status) && WEXITSTATUS(status) == 0 && should_pass;
					success |= WIFSIGNALED(status) && WTERMSIG(status) == SIGABRT && !should_pass;

					if(success)
					{
						test_say("Test '%s' passed", name);
						++passed;
					}
					else
					{
						test_say_important("\033[31mTest '%s' FAILED !!!\033[39m", name);
					}
				}
			}
		}
	}

	elf_end(elf);
	close(fd);

	test_say_important("Ran %d tests, %d passed, %d failed", tests, passed, tests - passed);

	close(tty_fd);

	return tests == passed ? 0 : 1;
}
