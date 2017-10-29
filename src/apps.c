
#include <string.h>

#include "os.h"
#include "apps.h"

extern char *strtok_r(char *str, const char *delim, char **saveptr);

static int echo(int argc, char *argv[]) {
	for (int i = 1; i < argc; ++i) {
		os_sys_write(argv[i]);
		os_sys_write(i == argc - 1 ? "\n" : " ");
	}
	return 0;
}

static int sleep(int argc, char *argv[]) {
	return 1;
}

static int uptime(int argc, char *argv[]) {
	return 1;
}

static const struct {
	const char *name;
	int(*fn)(int, char *[]);
} app_list[] = {
	{ "echo", echo },
	{ "sleep", sleep },
	{ "uptime", uptime },
};

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(*a))

static void do_task(void *input_arg) {
	struct args *args = (struct args*) input_arg;
	char *command = args->argv;
	char *saveptr;
	char *arg = strtok_r(command, " ", &saveptr);
	char *argv[256];
	int argc = 0;
	while (arg) {
		argv[argc++] = arg;
		arg = strtok_r(NULL, " ", &saveptr);
	}

	for (int i = 0; i < ARRAY_SIZE(app_list); ++i) {
		if (!strcmp(argv[0], app_list[i].name)) {
			args->res = app_list[i].fn(argc, argv);
			return;
		}
	}

	char msg[256] = "No such function: ";
	strcat(msg, argv[0]);
	strcat(msg, "\n");
	os_sys_write(msg);
	return;
}

void shell(void *args) {
	while (1) {
		os_sys_write("> ");
		char buffer[256];
		int bytes = os_sys_read(buffer, sizeof(buffer));
		if (!bytes) {
			break;
		}

		if (bytes < sizeof(buffer)) {
			buffer[bytes] = '\0';
		}

		struct args args;
		char *saveptr;
		const char *comsep = "\n;";
		char *cmd = strtok_r(buffer, comsep, &saveptr);
		while (cmd) {
			args.argv = cmd;
			int id = os_clone(do_task, (void*) &args);
			os_waitpid(id);
			cmd = strtok_r(NULL, comsep, &saveptr);
		}
	}

	os_sys_write("\n");
	os_halt(0);
}
