#include "token.h"
#include "value.h"
#include "compile.h"
#include "codeblock.h"
#include "environment.h"
#include "globals.h"
#include <errno.h>

static void usage(const char *program_name) {
	die("usage: %s (-e 'expression' | -f filename)", program_name);
}

static char *read_file(const char *filename) {
	FILE *file = fopen(filename, "r");

	if (file == NULL)
		die("unable to read file '%s': %s", filename, strerror(errno));

	size_t length = 0;
	size_t capacity = 2048;
	char *contents = xmalloc(capacity);

	while (!feof(file)) {
		size_t amntread = fread(&contents[length], 1, capacity - length, file);

		if (amntread == 0) {
			if (!feof(file))
				die("unable to read file '%s': %s'", filename, strerror(errno));
			break;
		}

		length += amntread;

		if (length == capacity) {
			capacity *= 2;
			contents = xrealloc(contents, capacity);
		}
	}

	if (fclose(file) == EOF)
		perror("couldn't close input file");

	contents = xrealloc(contents, length + 1);
	contents[length] = '\0';
	return contents;
}

int main(int argc, char **argv) {
	init_environment();
	init_global_variables();
	init_builtin_functions();

	if (argc != 3 || argv[1][0] != '-' || argv[1][2] != '\0')
		usage(argv[0]);

	switch (argv[1][1]) {
	case 'e': compile("-e", argv[2]); break;
	case 'f': compile(argv[2], read_file(argv[2])); break;
	default: usage(argv[0]);
	}

	int main_index = lookup_global_variable("main");
	if (main_index == -1)
		die("you must define a `main` function");

	value ret = call_value(fetch_global_variable(main_index), 0, NULL);

	free_environment();
	free_global_variables();

	if (is_number(ret))
		return as_number(ret);
}
