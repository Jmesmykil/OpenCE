/*
OVERLAY_COMMANDS.C

Commands from outside the game, one at a time, through a file: an overlay
(the Switch's Ultrahand, say) writes a line to overlay/command.txt in the
data folder, and once a second the game looks for it, takes it (the file is
deleted), and runs it. The line is a console command, run as typed at the
console (hs_compile_and_evaluate: cheats, map_name, game_revert...), or
"config <setting> <value>" (or "config <setting> = <value>", the line as
config.toml has it), which sets and saves a setting as the menus do
(config_set_text). overlay/result.txt gets the command and what came of it,
for looking at when one does not work. The whole protocol is the overlay's
spec, halo-overlay-command-spec.md; one command in the file at a time, and
its contents are trusted.

The Switch's only (its Ultrahand overlays are the reason; the other builds
have an empty overlay_commands_update), and only with the overlay installed:
its package's folder, /switch/.packages/halo/overlay, there at the start
(hardran3's OpenCE-Ultrahand). Then the game says so at the main menu, and
empties the package's overlay/game and overlay/flags folders (making them if
they are not there), which are the game's and the overlay's for this run;
and looks for the command once a second while game.overlay_commands is on.
Without that folder none of this is done, not even the look.
*/

#include "cseries.h"
#include "cseries/cseries_windows.h"
#include "game/game.h"
#include "hs/hs.h"
#include "interface/terminal.h"
#include "../src/posix.h"

#ifdef HALO_SWITCH

#include <stdio.h>
#include <string.h>

/* the platform layer's (port/linux/src) */
extern const char *platform_data_root(void);
extern FILE *platform_fopen_host(char const *path, char const *mode);
extern int platform_remove_host(char const *path);
extern int platform_remove_directory_host(char const *path);
extern int config_boolean(const char *name);
extern int config_set_text(const char *name, const char *value, char *error, size_t error_size);
extern void platform_log(const char *format, ...);

enum
{
	OVERLAY_COMMAND_SIZE = 1024,
	OVERLAY_LOOK_MILLISECONDS = 1000,
};

/* the Ultrahand package's folder: the overlay installed */
#define OVERLAY_PACKAGE_FOLDER "sdmc:/switch/.packages/halo/overlay"

static boolean overlay_is_folder(
	char const *path)
{
	struct posix_file_information information;

	return posix_stat(path, &information) == 0 && (information.flags & _posix_file_is_directory) != 0;
}

/* everything in a folder deleted, the folders in it with what they hold; the folder kept */
static void overlay_folder_empty(
	char const *path,
	int depth)
{
	void *directory;
	char name[256];

	if (depth > 8 || !(directory = posix_directory_open(path)))
		return;
	while (posix_directory_next(directory, name, sizeof(name)))
	{
		char entry[1024];

		snprintf(entry, sizeof(entry), "%s/%s", path, name);
		if (overlay_is_folder(entry))
		{
			overlay_folder_empty(entry, depth + 1);
			if (platform_remove_directory_host(entry) != 0)
				platform_log("overlay: could not delete the folder %s", entry);
		}
		else if (platform_remove_host(entry) != 0)
		{
			platform_log("overlay: could not delete %s", entry);
		}
	}
	posix_directory_close(directory);
}

/* one of the package's folders for this run: emptied, or made */
static void overlay_folder_prepare(
	char const *name)
{
	char path[1024];

	snprintf(path, sizeof(path), "%s/%s", OVERLAY_PACKAGE_FOLDER, name);
	if (overlay_is_folder(path))
		overlay_folder_empty(path, 0);
	else
		posix_make_directory(path);
}

/* (once, at the first look) whether the overlay is installed, and if it is, its folders made ready */
static boolean overlay_present(
	void)
{
	static int present = -1;

	if (present < 0)
	{
		present = overlay_is_folder(OVERLAY_PACKAGE_FOLDER);
		if (present)
		{
			overlay_folder_prepare("game");
			overlay_folder_prepare("flags");
			platform_log("overlay: the Ultrahand package's folder is there (%s): its commands are taken",
				OVERLAY_PACKAGE_FOLDER);
		}
	}
	return present > 0;
}

/* the result, for debugging: the command, then ok or why not */
static void overlay_result_write(
	char const *command,
	boolean succeeded,
	char const *message)
{
	char path[1024];
	FILE *file;

	snprintf(path, sizeof(path), "%s/overlay/result.txt", platform_data_root());
	file = platform_fopen_host(path, "wb");
	if (!file)
		return;
	fprintf(file, "%s\n%s%s%s\n", command, succeeded ? "ok" : "error", message && *message ? ": " : "",
		message ? message : "");
	fclose(file);
}

/* "config <setting> <value>" or "config <setting> = <value>" */
static void overlay_config(
	char const *command,
	char *arguments)
{
	char error[256] = "";
	char *name, *value, *end;

	while (*arguments == ' ' || *arguments == '\t')
		arguments++;
	name = arguments;
	end = name + strcspn(name, " \t=");
	value = end;
	while (*value == ' ' || *value == '\t' || *value == '=')
		value++;
	*end = 0;
	if (!*name || !*value)
	{
		overlay_result_write(command, FALSE, "config takes a setting and a value: config display.vsync true");
		return;
	}
	if (config_set_text(name, value, error, sizeof(error)))
	{
		platform_log("overlay: %s set to %s", name, value);
		overlay_result_write(command, TRUE, NULL);
	}
	else
	{
		platform_log("overlay: %s", error);
		overlay_result_write(command, FALSE, error);
	}
}

/* (main_loop's, every frame: a look at the file once a second) */
void overlay_commands_update(
	void)
{
	static unsigned long looked_at;
	static boolean folder_made;
	unsigned long now = system_milliseconds();
	char path[1024];
	char command[OVERLAY_COMMAND_SIZE + 1];
	size_t length;
	FILE *file;

	if (!overlay_present())
		return;
	/* (said once the main menu's map is loaded, where the game's messages are shown) */
	{
		static boolean said;

		if (!said && game_map_loaded())
		{
			said = TRUE;
			terminal_printf(global_real_argb_green, "Ultrahand Overlay present and enabled");
		}
	}
	if (now - looked_at < OVERLAY_LOOK_MILLISECONDS)
		return;
	looked_at = now;
	if (!config_boolean("game.overlay_commands"))
		return;
	if (!folder_made)
	{
		snprintf(path, sizeof(path), "%s/overlay", platform_data_root());
		posix_make_directory(path);
		folder_made = TRUE;
	}
	snprintf(path, sizeof(path), "%s/overlay/command.txt", platform_data_root());
	/* (looked for first by the host, which says nothing of a file that is not there: the guest's open of a missing
	file is logged, once a second) */
	{
		struct posix_file_information information;

		if (posix_stat(path, &information) != 0)
			return;
	}
	file = platform_fopen_host(path, "rb");
	if (!file)
		return;
	length = fread(command, 1, OVERLAY_COMMAND_SIZE, file);
	fclose(file);
	platform_remove_host(path);
	command[length] = 0;
	/* (the first line, without its end or the spaces around it) */
	command[strcspn(command, "\r\n")] = 0;
	while (length = strlen(command), length && (command[length - 1] == ' ' || command[length - 1] == '\t'))
		command[length - 1] = 0;
	{
		char *start = command;

		while (*start == ' ' || *start == '\t')
			start++;
		if (start != command)
			memmove(command, start, strlen(start) + 1);
	}
	if (!*command)
		return;
	if (!strncmp(command, "config ", 7) || !strncmp(command, "config\t", 7))
	{
		char copy[OVERLAY_COMMAND_SIZE + 1];

		snprintf(copy, sizeof(copy), "%s", command);
		overlay_config(command, copy + 7);
		return;
	}
	platform_log("overlay: %s", command);
	if (hs_compile_and_evaluate(command))
		overlay_result_write(command, TRUE, NULL);
	else
		overlay_result_write(command, FALSE, "the console did not run it (debug.txt says why)");
}

#else

void overlay_commands_update(
	void)
{
	return;
}

#endif
