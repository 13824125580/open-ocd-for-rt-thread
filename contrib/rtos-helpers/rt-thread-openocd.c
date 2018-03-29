/*
 * rt-thread does not provide a fixed layout for rt_thread, which makes it
 * impossible to determine the appropriate offsets within the structure
 * unaided. A priori knowledge of offsets based on os_dbg.c is tied to a
 * specific release and thusly, brittle. The constants defined below
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

#if RT_THREAD_CFG_DBG_EN == 0
#error "OS_CFG_DBG_EN is required to enable RTOS support for OpenOCD"
#endif

#define OFFSET_OF(type, member) ((rt_size_t)&(((type *)0)->member))

#ifdef __GNUC__
#define USED __attribute__((used))
#else
#define USED
#endif

const rt_size_t USED openocd_rt_thread_name_offset = OFFSET_OF(rt_thread, name);

const rt_size_t USED openocd_rt_thread_sp_offset = OFFSET_OF(rt_thread, sp);
const rt_size_t USED openocd_rt_thread_stat_offset = OFFSET_OF(rt_thread, stat);
const rt_size_t USED openocd_rt_thread_current_priority_offset = OFFSET_OF(rt_thread, current_priority);
const rt_size_t USED openocd_rt_thread_next_offset = OFFSET_OF(rt_thread, list) + OFFSET_OF(rt_list_t, next);
const rt_size_t USED openocd_rt_thread_prev_offset = OFFSET_OF(rt_thread, list) + OFFSET_OF(rt_list_t, prev);
const rt_size_t USED openocd_rt_object_prev_offset = OFFSET_OF(rt_object_information, object_list) + OFFSET_OF(rt_list_t, prev);
const rt_size_t USED openocd_rt_object_next_offset = OFFSET_OF(rt_object_information, object_list) + OFFSET_OF(rt_list_t, next);

#ifdef __cplusplus
}
#endif


