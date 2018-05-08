/*
 * rt-thread does not provide a fixed layout for rt_thread, which makes it
 * impossible to determine the appropriate offsets within the structure
 * unaided. The constants defined below
 * provide the neccessary information OpenOCD needs to provide support
 * in the most robust manner possible.
 *
 * This file should be linked along with the project to enable RTOS
 * support for rt-thread.
 */

#include <rtconfig.h>
#include <rtdef.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef OFFSET_OF
#undef OFFSET_OF
#endif

#define OFFSET_OF(type, member) ((rt_size_t)&(((type *)0)->member))

const rt_size_t RT_USED openocd_rt_thread_name_offset = OFFSET_OF(struct rt_thread, name);

const rt_size_t RT_USED openocd_rt_thread_sp_offset = OFFSET_OF(struct rt_thread, sp);

const rt_size_t RT_USED openocd_rt_thread_stat_offset = OFFSET_OF(struct rt_thread, stat);

const rt_size_t RT_USED openocd_rt_thread_current_priority_offset = OFFSET_OF(struct rt_thread, current_priority);

const rt_size_t RT_USED openocd_rt_thread_next_offset = OFFSET_OF(struct rt_thread, list) + OFFSET_OF(rt_list_t, next);

const rt_size_t RT_USED openocd_rt_thread_prev_offset = OFFSET_OF(struct rt_thread, list) + OFFSET_OF(rt_list_t, prev);

const rt_size_t RT_USED openocd_rt_object_prev_offset = OFFSET_OF(struct rt_object_information, object_list) + OFFSET_OF(rt_list_t, prev);

const rt_size_t RT_USED openocd_rt_object_next_offset = OFFSET_OF(struct rt_object_information, object_list) + OFFSET_OF(rt_list_t, next);

#ifdef __cplusplus
}
#endif
