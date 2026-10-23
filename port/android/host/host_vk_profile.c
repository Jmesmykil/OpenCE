/*
HOST_VK_PROFILE.C

The GPU's time by part of the frame, under the Vulkan renderer, while a profile is recorded (debug.profile_run:
host_main.c names the folder, host_vk_profile_folder). The GPU writes a timestamp (vkCmdWriteTimestamp) where each
frame's command buffer begins, where each part of the frame the game names begins (VK_COMMAND_MARKER: "sky",
"objects", "hud"... from halo_render_marker, as the Switch's renderer has them: port/switch/host/host_dk.c's
debug.gpu_profile) and where the command buffer ends: the time from one to the next is the part's. A frame slot's
timestamps are read once its fence has passed (host_vk_profile_frame_begin, as the slot is recorded again). Every
PROFILE_FRAMES frames the averages go into gpu.txt in the profile's folder, the parts that took longest first, with
the GPU's frame and the time from one present to the next (the CPU's and GPU's together, as the player sees it).

A timestamp inside a rendering is the time the GPU reached it, which on a tiler (the phones' GPUs draw a target in
tiles, after sorting its triangles into them) is when the tile pass reached it: a part's time is good for comparing
parts and runs, not as the cost of one draw. The parts of the Android port's own (the copies of the screen, the
present) are counted in the part they fall in. */

#include "host.h"
#include "host_vk.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define B host_vkb

#define PROFILE_MARKS 256
#define PROFILE_NAMES 64
#define PROFILE_FRAMES 300
/* the part names of the frame's start (before the game names one) */
#define PROFILE_START 0

char host_vk_profile_folder[512];

static struct
{
	int on;
	VkQueryPool pool;
	double ns_per_tick;
	uint64_t valid_mask;
	/* each slot's marks: the part each begins, and how many */
	uint8_t pass[HOST_VK_FRAMES][PROFILE_MARKS];
	uint32_t count[HOST_VK_FRAMES];
	char names[PROFILE_NAMES][VK_MARKER_NAME_SIZE];
	uint32_t name_count;
	/* the window's sums: each part's, the GPU's frames, the presents' */
	uint64_t ns[PROFILE_NAMES];
	uint64_t gpu_total_ns, gpu_longest_ns;
	uint32_t gpu_frames, gpu_slow;
	uint64_t present_last_ns, present_total_ns, present_longest_ns;
	uint32_t presents, presents_slow;
	uint32_t window, lost;
	FILE *file;
} profile;

static uint64_t now_ns(void)
{
	struct timespec now;

	clock_gettime(CLOCK_MONOTONIC, &now);
	return (uint64_t)now.tv_sec * 1000000000ull + (uint64_t)now.tv_nsec;
}

void host_vk_profile_initialize(void)
{
	VkQueueFamilyProperties families[16];
	uint32_t family_count = 16;
	VkQueryPoolCreateInfo info = { VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO };
	uint32_t bits;
	char path[600];

	if (!host_vk_profile_folder[0] || profile.on)
		return;
	host_vk.vkGetPhysicalDeviceQueueFamilyProperties(host_vk.physical, &family_count, NULL);
	if (family_count > 16)
		family_count = 16;
	host_vk.vkGetPhysicalDeviceQueueFamilyProperties(host_vk.physical, &family_count, families);
	bits = host_vk.queue_family < family_count ? families[host_vk.queue_family].timestampValidBits : 0;
	if (!bits || !host_vk.properties.limits.timestampComputeAndGraphics)
	{
		host_logf(HOST_LOG_WARN, "vk: the GPU's queue keeps no timestamps; gpu.txt has no GPU times");
		return;
	}
	info.queryType = VK_QUERY_TYPE_TIMESTAMP;
	info.queryCount = HOST_VK_FRAMES * PROFILE_MARKS;
	if (!HOST_VK_CHECK(vkCreateQueryPool(B.device, &info, NULL, &profile.pool)))
		return;
	snprintf(path, sizeof(path), "%s/gpu.txt", host_vk_profile_folder);
	profile.file = fopen(path, "w");
	if (!profile.file)
	{
		host_logf(HOST_LOG_WARN, "vk: cannot write %s; no GPU profile", path);
		vkDestroyQueryPool(B.device, profile.pool, NULL);
		profile.pool = VK_NULL_HANDLE;
		return;
	}
	profile.ns_per_tick = (double)host_vk.properties.limits.timestampPeriod;
	profile.valid_mask = bits >= 64 ? ~0ull : (1ull << bits) - 1;
	memcpy(profile.names[PROFILE_START], "(before the game's first part)", sizeof("(before the game's first part)"));
	profile.name_count = 1;
	profile.on = 1;
	fprintf(profile.file, "GPU %s, %u timestamp bits, %.3f ns a tick; every %u frames, the GPU's time by part of the "
		"frame (host_vk_profile.c)\n\n", host_vk.properties.deviceName, (unsigned)bits, profile.ns_per_tick,
		(unsigned)PROFILE_FRAMES);
	fflush(profile.file);
	host_logf(HOST_LOG_INFO, "vk: the GPU's time by part of the frame goes to %s", path);
}

static void mark(VkCommandBuffer command, unsigned slot, uint32_t pass)
{
	uint32_t index = profile.count[slot];

	if (index >= PROFILE_MARKS)
		return;
	vkCmdWriteTimestamp(command, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, profile.pool, slot * PROFILE_MARKS + index);
	profile.pass[slot][index] = (uint8_t)pass;
	profile.count[slot] = index + 1;
}

static int part_compare(const void *first, const void *second)
{
	uint64_t a = profile.ns[*(const uint32_t *)first], b = profile.ns[*(const uint32_t *)second];

	return a < b ? 1 : a > b ? -1 : 0;
}

/* the window's averages into gpu.txt */
static void window_write(void)
{
	uint32_t order[PROFILE_NAMES], index;
	double frames = profile.gpu_frames ? (double)profile.gpu_frames : 1.0;

	profile.window++;
	fprintf(profile.file, "window %u: %u frames timed; the GPU %.2f ms a frame on average, %.2f the longest, %u over "
		"33 ms", (unsigned)profile.window, (unsigned)profile.gpu_frames, (double)profile.gpu_total_ns / frames / 1e6,
		(double)profile.gpu_longest_ns / 1e6, (unsigned)profile.gpu_slow);
	if (profile.presents)
	{
		fprintf(profile.file, "; presents %.2f ms apart on average (%.1f a second), %.2f the longest, %u over 33 ms",
			(double)profile.present_total_ns / profile.presents / 1e6,
			profile.present_total_ns ? 1e9 * profile.presents / (double)profile.present_total_ns : 0.0,
			(double)profile.present_longest_ns / 1e6, (unsigned)profile.presents_slow);
	}
	if (profile.lost)
		fprintf(profile.file, "; %u marks not read", (unsigned)profile.lost);
	fprintf(profile.file, "\n");
	for (index = 0; index < profile.name_count; index++)
		order[index] = index;
	qsort(order, profile.name_count, sizeof(order[0]), part_compare);
	for (index = 0; index < profile.name_count && profile.ns[order[index]]; index++)
	{
		fprintf(profile.file, "  %8.3f ms  %5.1f%%  %s\n", (double)profile.ns[order[index]] / frames / 1e6,
			profile.gpu_total_ns ? 100.0 * (double)profile.ns[order[index]] / (double)profile.gpu_total_ns : 0.0,
			profile.names[order[index]]);
	}
	fprintf(profile.file, "\n");
	fflush(profile.file);
	memset(profile.ns, 0, sizeof(profile.ns));
	profile.gpu_total_ns = profile.gpu_longest_ns = 0;
	profile.gpu_frames = profile.gpu_slow = 0;
	profile.present_total_ns = profile.present_longest_ns = 0;
	profile.presents = profile.presents_slow = 0;
	profile.lost = 0;
}

void host_vk_profile_frame_begin(unsigned slot, VkCommandBuffer command)
{
	uint32_t count;

	if (!profile.on)
		return;
	count = profile.count[slot];
	/* (the slot's last frame, its fence passed: read, then its queries reset for this one) */
	if (count >= 2)
	{
		uint64_t ticks[PROFILE_MARKS];

		if (vkGetQueryPoolResults(B.device, profile.pool, slot * PROFILE_MARKS, count, sizeof(ticks), ticks,
			sizeof(ticks[0]), VK_QUERY_RESULT_64_BIT) == VK_SUCCESS)
		{
			uint64_t frame_ns = 0;
			uint32_t index;

			for (index = 0; index + 1 < count; index++)
			{
				uint64_t start = ticks[index] & profile.valid_mask, end = ticks[index + 1] & profile.valid_mask;
				uint64_t ns = (uint64_t)((double)((end - start) & profile.valid_mask) * profile.ns_per_tick);

				profile.ns[profile.pass[slot][index]] += ns;
				frame_ns += ns;
			}
			profile.gpu_total_ns += frame_ns;
			if (frame_ns > profile.gpu_longest_ns)
				profile.gpu_longest_ns = frame_ns;
			if (frame_ns > 33333333ull)
				profile.gpu_slow++;
			profile.gpu_frames++;
		}
		else
			profile.lost += count;
		if (profile.gpu_frames >= PROFILE_FRAMES)
			window_write();
	}
	profile.count[slot] = 0;
	vkCmdResetQueryPool(command, profile.pool, slot * PROFILE_MARKS, PROFILE_MARKS);
	mark(command, slot, PROFILE_START);
}

void host_vk_profile_mark(const struct vk_command_marker *command)
{
	char name[VK_MARKER_NAME_SIZE];
	uint32_t index;

	if (!profile.on)
		return;
	memcpy(name, command->name, sizeof(name));
	name[sizeof(name) - 1] = 0;
	for (index = 0; index < profile.name_count && strcmp(profile.names[index], name); index++)
		;
	if (index == profile.name_count)
	{
		if (profile.name_count == PROFILE_NAMES)
			return;
		memcpy(profile.names[profile.name_count++], name, sizeof(name));
	}
	mark(host_vk_frame_command(), B.frame, index);
}

void host_vk_profile_frame_end(unsigned slot, VkCommandBuffer command, int presented)
{
	uint64_t now;

	if (!profile.on)
		return;
	/* (the last mark ends the last part: its own time is none) */
	mark(command, slot, PROFILE_START);
	if (!presented)
		return;
	now = now_ns();
	if (profile.present_last_ns)
	{
		uint64_t apart = now - profile.present_last_ns;

		profile.present_total_ns += apart;
		if (apart > profile.present_longest_ns)
			profile.present_longest_ns = apart;
		if (apart > 33333333ull)
			profile.presents_slow++;
		profile.presents++;
#ifndef HALO_PS5
		/* (the CPU profile's frames, for its slow frames' samples: host_debug.c) */
		host_debug_profile_frame(apart);
#endif
	}
	profile.present_last_ns = now;
}
