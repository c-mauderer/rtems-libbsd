/**
 * @file
 *
 * @brief It mounts a network file system (nfs).
 */

/*
 * Copyright (c) 2021 Chris Johns <chrisj@rtems.org>  All rights reserved.
 * Copyright (c) 2016 embedded brains GmbH.  All rights reserved.
 *
 *  embedded brains GmbH
 *  Dornierstr. 4
 *  82178 Puchheim
 *  Germany
 *  <rtems@embedded-brains.de>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <assert.h>
#include <sys/stat.h>
#include <unistd.h>

#include <rtems.h>
#include <rtems/libio.h>

#include <rtems/console.h>
#include <rtems/shell.h>
#include <rtems/telnetd.h>

#include <librtemsNfs.h>

#include <rtems/bsd/test/network-config.h>

#define TEST_NAME "LIBBSD NFS 1"
#define TEST_STATE_USER_INPUT 1

static const char *test_top = "test-nfs01";

#define rtems_test_errno_assert(__exp) \
  do { \
    if (!(__exp)) { \
      printf( "%s: %d errno:%d:%s %s\n", __FILE__, __LINE__, errno, strerror(errno), #__exp ); \
      assert(1 == 0); \
    } \
  } while (0)

typedef struct test_dir_entry
{
	struct test_dir_entry *next;
	char name[4];
} test_dir_entry;

typedef struct test_dir
{
	struct test_dir *parent;
	struct test_dir_entry *dirs;
	struct test_dir_entry *indir;
	const char *name;
	int depth;
	int num;
} test_dir;

/*
 * Non-recursive directory tree walk
 */
typedef enum {
	walk_tree_dir_start,
	walk_tree_dir_end,
	walk_tree_dir_entry
} walk_tree_dir;

typedef bool (*walk_tree_callout)(walk_tree_dir state,
				  test_dir *dir,
				  struct dirent *entry,
				  struct stat* stat,
				  void *data);

static char
test_stat_label(struct stat *s)
{
	if (S_ISBLK(s->st_mode)) {
		return 'b';
	}
	if (S_ISCHR(s->st_mode)) {
		return 'c';
	}
	if (S_ISDIR(s->st_mode)) {
		return 'd';
	}
	if (S_ISFIFO(s->st_mode)) {
		return 'F';
	}
	if (S_ISLNK(s->st_mode)) {
		return 'l';
	}
	if (S_ISREG(s->st_mode)) {
		return 'f';
	}
	if (S_ISSOCK(s->st_mode)) {
		return 's';
	}
	return 'X';
}

static void
test_walk_tree(const char *start, walk_tree_callout callout, void *data)
{
	test_dir top = {
		.parent = NULL,
		.dirs = NULL,
		.indir = NULL,
		.name = start,
		.depth = 0,
		.num = 0
	};
	test_dir *dir = &top;
	bool active = true;

	rtems_test_errno_assert(chdir(start) == 0);

	active = callout(walk_tree_dir_start, dir, NULL, NULL, data);

	while (dir != NULL && active) {
		test_dir *tmp_dir;
		if (active && dir->dirs == NULL && dir->indir == NULL) {
			struct DIR *ddir;
			rtems_test_errno_assert((ddir = opendir(".")) != NULL);
			while (active) {
				struct dirent *dp;
				struct stat s;
				char t;
				bool current_or_parent;
				++dir->num;
				dp = readdir(ddir);
				if (dp == NULL) {
					rtems_test_errno_assert(closedir(ddir) == 0);
					break;
				}
				rtems_test_errno_assert(stat(dp->d_name, &s) == 0);
				if ((dp->d_namlen == 1 && dp->d_name[0] == '.') ||
				    (dp->d_namlen == 2 && dp->d_name[0] == '.' && dp->d_name[1] == '.')) {
					current_or_parent = true;
				} else {
					current_or_parent = false;
				}
				t = test_stat_label(&s);
				active = callout(walk_tree_dir_entry, dir, dp, &s, data);
				if (active && !current_or_parent && t == 'd') {
					test_dir_entry *dent;
					assert((dent = malloc(sizeof(test_dir_entry) + dp->d_namlen)) != NULL);
					dent->next = dir->dirs;
					dir->dirs = dent;
					strlcpy(&dent->name[0], dp->d_name, dp->d_namlen + sizeof(dent->name));
				}
			}
		}
		if (dir->dirs != NULL) {
			free(dir->indir);
			dir->indir = dir->dirs;
			dir->dirs = dir->indir->next;
			if (active && dir->indir != NULL) {
				assert((tmp_dir = malloc(sizeof(test_dir))) != NULL);
				tmp_dir->parent = dir;
				tmp_dir->dirs = NULL;
				tmp_dir->indir = NULL;
				tmp_dir->name = dir->indir->name;
				tmp_dir->depth = dir->depth + 1;
				tmp_dir->num = 0;
				dir = tmp_dir;
				active = callout(walk_tree_dir_start, dir, NULL, NULL, data);
				if (active) {
					rtems_test_errno_assert(chdir(dir->name) == 0);
				}
			}
		} else  {
			rtems_test_errno_assert(chdir("..") == 0);
			if (active) {
				active = callout(walk_tree_dir_end, dir, NULL, NULL, data);
			}
			free(dir->indir);
			tmp_dir = dir;
			dir = dir->parent;
			if (tmp_dir != &top) {
				free(tmp_dir);
			}
		}
	}
}

typedef struct test_printer_data {
	char path[MAXPATHLEN];
	int count;
} test_printer_data;

static bool
test_walk_tree_printer(walk_tree_dir state,
		       test_dir *dir,
		       struct dirent *entry,
		       struct stat* stat,
		       void *data)
{
	test_printer_data *pd = (test_printer_data*) data;
	int len;
	switch (state) {
	case walk_tree_dir_start:
		strlcat(pd->path, dir->name, MAXPATHLEN);
		strlcat(pd->path, "/", MAXPATHLEN);
		break;
	case walk_tree_dir_entry:
		++pd->count;
		printf("%8d %3d %6d %c 0%o %10lld %s%s\n",
		       pd->count, dir->depth, dir->num, test_stat_label(stat),
		       stat->st_mode & 0777, stat->st_size,
		       pd->path, &entry->d_name[0]);
		break;
	case walk_tree_dir_end:
		len = strlen(pd->path) - 1;
		while (len > 0) {
			len--;
			if (pd->path[len] == '/') {
				if (len < 1) {
					break;
				}
				if (pd->path[len - 1] != '\\') {
					break;
				}
				len -= 2;
			}
		}
		pd->path[len + 1] = '\0';
		break;
	default:
		break;
	}
	return true;
}

static bool
test_walk_tree_unlink(walk_tree_dir state,
		      test_dir *dir,
		      struct dirent *entry,
		      struct stat* stat,
		      void *data)
{
	if (state == walk_tree_dir_entry) {
		char type = test_stat_label(stat);
		if (type != 'd') {
			printf("unlink: %s\n", entry->d_name);
			rtems_test_errno_assert(unlink(entry->d_name) == 0);
		}
	} else if (state == walk_tree_dir_end) {
		 printf("rmdir: %s\n", dir->name);
		 rtems_test_errno_assert(unlink(dir->name) == 0);
	}
	return true;
}

static void
test_setup(const char *base)
{
	printf("test: nfs: setup\n");
	printf("test: nfs: chdir: %s\n", base);
	rtems_test_errno_assert(chdir(base) == 0);
	printf("test: nfs: mkdir: %s\n", test_top);
	rtems_test_errno_assert(mkdir(test_top, 0777) == 0);
	printf("test: nfs: chdir: %s\n", test_top);
	rtems_test_errno_assert(chdir(test_top) == 0);
}

static void
test_cleanup(const char *base)
{
	printf("test: nfs: cleanup\n");
	printf("test: nfs: chdir: %s\n", base);
	rtems_test_errno_assert(chdir(base) == 0);
	test_walk_tree(test_top, test_walk_tree_unlink, NULL);
}

static void
test_path_eval(const char *base, int depth)
{
	char path[MAXPATHLEN];
	int l;

	printf("test path eval\n");

	test_setup(base);

	for (l = 1; l <= depth; ++l) {
		snprintf(path, sizeof(path), "%d", l);
		printf("test: nfs: mkdir: %s\n", path);
		rtems_test_errno_assert(mkdir(path, 0777) == 0);
		printf("test: nfs: chdir: %s\n", path);
		rtems_test_errno_assert(chdir(path) == 0);
		printf("test: nfs: getcwd: %s\n", path);
		assert(getcwd(path, sizeof(path)) != NULL);
		printf("test: nfs: getcwd: %s\n", path);
	}

	test_cleanup(base);
}

static void
test_nfs(const char *base)
{
	test_printer_data pd;
	test_path_eval(base, 5);
	memset(&pd, 0, sizeof(pd));
	test_walk_tree(base, test_walk_tree_printer, &pd);
}

static void
telnet_shell(char *name, void *arg)
{
	rtems_shell_env_t env;

	rtems_shell_dup_current_env(&env);

	env.devname = name;
	env.taskname = "TLNT";
	env.login_check = NULL;
	env.forever = false;

	rtems_shell_main_loop(&env);
}

rtems_telnetd_config_table rtems_telnetd_config = {
	.command = telnet_shell,
	.arg = NULL,
	.priority = 0,
	.stack_size = 0,
	.login_check = NULL,
	.keep_stdio = false
};

static void
test_main(void)
{
	const char remote_target[] = NET_CFG_NFS_MOUNT_PATH;
	const char *options = NET_CFG_NFS_MOUNT_OPTIONS;
	const char *mount_options = NULL;
	const char* mount_point = "/nfs";
	int retries = 0;
	int rv;

	assert(rtems_telnetd_initialize() == RTEMS_SUCCESSFUL);

	if (strlen(options) != 0) {
		mount_options = options;
	}

	printf("mount: %s -> %s options:%s\n",
	       remote_target, mount_point, mount_options);

	do {
		sleep(1);
		rv = mount_and_make_target_path(&remote_target[0], mount_point,
		    RTEMS_FILESYSTEM_TYPE_NFS, RTEMS_FILESYSTEM_READ_WRITE,
		    mount_options);
		if (rv < 0) {
			printf("mount: %d: %s\n", errno, strerror(errno));
		}
	} while (rv != 0 && retries++ < 5);

	if (rv != 0) {
		printf("error: NFS mount failed\n");
		exit(rv);
	}

	test_nfs(mount_point);

	rtems_task_delete(RTEMS_SELF);
	assert(0);
}

#define CONFIGURE_SHELL_COMMANDS_ALL
#define DEFAULT_NETWORK_SHELL

#define CONFIGURE_FILESYSTEM_NFS

#define CONFIGURE_MAXIMUM_DRIVERS 32

#include <rtems/bsd/test/default-network-init.h>
