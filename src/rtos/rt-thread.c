/***************************************************************************
 *   Copyright (C) 2017 by Allwinner, Inc.                                    *
 *   Zhijin Zeng <zhijinzeng@allwinnertech.com>                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <helper/log.h>
#include <helper/time_support.h>
#include <helper/types.h>
#include <rtos/rtos.h>
#include <target/target.h>
#include <target/target_type.h>
#include <rtos/rtos_standard_stackings.h>

#ifndef RT_THREAD_MAX_STRLEN
#define RT_THREAD_MAX_STRLEN 32
#endif

#define MY_RT_THREAD 0

struct rt_thread_params_thread
{
        struct rt_thread_params_thread * next;
	struct rt_thread_params_thread * prev;
	symbol_address_t thread_address;
};
typedef struct rt_thread_params_thread * rt_thread_params_thread_t; 

struct rt_thread_params {
	const char *target_name;
	const unsigned char pointer_width;
	symbol_address_t thread_stack_offset;
	symbol_address_t thread_name_offset;
	symbol_address_t thread_state_offset;
	symbol_address_t thread_priority_offset;
	symbol_address_t thread_prev_offset;
	symbol_address_t thread_next_offset;
	symbol_address_t rt_object_prev_offset;
	symbol_address_t rt_object_next_offset;
	bool thread_offsets_updated;
	size_t threadid_start;
	const struct rtos_register_stacking *stacking_info;
	size_t num_threads;
	rt_thread_params_thread_t threads;
};

static const struct stack_register_offset rtos_rt_thread_arm926_stack_offsets[] = {
#if MY_RT_THREAD  //for my rt-thread code, the first value in stack is fcse
       { 0x08, 32 },	/* r0   */
       { 0x0c, 32 },	/* r1   */
       { 0x10, 32 },	/* r2   */
       { 0x14, 32 },	/* r3   */
       { 0x18, 32 },	/* r4   */
       { 0x1c, 32 },	/* r5   */
       { 0x20, 32 },	/* r6   */
       { 0x24, 32 },	/* r7   */
       { 0x28, 32 },	/* r8   */
       { 0x2c, 32 },	/* r9   */
       { 0x30, 32 },	/* r10  */
       { 0x34, 32 },	/* r11  */
       { 0x38, 32 },	/* r12  */
       { -2,   32 },	/* sp   */
       { 0x3c, 32 },	/* lr   */
       { 0x40, 32 },	/* pc   */
       { 0x04, 32 },	/* SPSR */
#else //for rt-thread mainline
       { 0x04, 32 },	/* r0   */
       { 0x08, 32 },	/* r1   */
       { 0x0c, 32 },	/* r2   */
       { 0x10, 32 },	/* r3   */
       { 0x14, 32 },	/* r4   */
       { 0x18, 32 },	/* r5   */
       { 0x1c, 32 },	/* r6   */
       { 0x20, 32 },	/* r7   */
       { 0x24, 32 },	/* r8   */
       { 0x28, 32 },	/* r9   */
       { 0x2c, 32 },	/* r10  */
       { 0x30, 32 },	/* r11  */
       { 0x34, 32 },	/* r12  */
       { -2,   32 },	/* sp   */
       { 0x38, 32 },	/* lr   */
       { 0x3c, 32 },	/* pc   */
       { 0x00, 32 },	/* SPSR */
#endif
};


const struct rtos_register_stacking rtos_rt_thread_arm926_stacking = {
#if MY_RT_THREAD
       0x44,										/* stack_registers_size */
#else
       0x40,
#endif
       -1,									        /* stack_growth_direction */
       ARRAY_SIZE(rtos_rt_thread_arm926_stack_offsets),	                                /* num_output_registers */
       rtos_generic_stack_align8,				                        /* stack_alignment */
       rtos_rt_thread_arm926_stack_offsets				                /* register_offsets */
};

static const struct rt_thread_params rt_thread_params_list[] = {
	{
		"arm926ejs",							        /* target_name */
		sizeof(uint32_t),					                /* pointer_width */
		0,									/* thread_stack_offset */
		0,									/* thread_name_offset */
		0,									/* thread_state_offset */
		0,									/* thread_priority_offset */
		0,									/* thread_prev_offset */
		0,									/* thread_next_offset */
		0,									/* rt_object_prev_offset */
		0,									/* rt_objcet_next_offset */
		false,								        /* thread_offsets_updated */
		1,									/* threadid_start */
		&rtos_rt_thread_arm926_stacking,	                                /* stacking_info */
		0,									/* num_threads */
		NULL
	},
};

static const char * const rt_thread_symbol_list[] = {
	"rt_current_thread",
	"rt_object_container",

	/* also see: contrib/rtos-helpers/rt-thread-openocd.c */
	"openocd_rt_thread_name_offset",
	"openocd_rt_thread_sp_offset",
	"openocd_rt_thread_stat_offset",
	"openocd_rt_thread_current_priority_offset",
	"openocd_rt_thread_prev_offset",
	"openocd_rt_thread_next_offset",
	"openocd_rt_object_prev_offset",
	"openocd_rt_object_next_offset",
	NULL
};

enum rt_thread_symbol_values {
	rt_thread_VAL_rt_current_thread,
	rt_thread_VAL_rt_object_container,

	/* also see: contrib/rtos-helpers/rt-thread-openocd.c */
	rt_thread_VAL_rt_thread_name_offset,
	rt_thread_VAL_rt_thread_sp_offset,
	rt_thread_VAL_rt_thread_stat_offset,
	rt_thread_VAL_rt_thread_current_priority_offset,
	rt_thread_VAL_rt_thread_prev_offset,
	rt_thread_VAL_rt_thread_next_offset,
	rt_thread_VAL_rt_object_prev_offset,
	rt_thread_VAL_rt_object_next_offset,
};

static const char * const rt_thread_thread_state_list[] = {
	"Init",
	"Ready",
	"Suspended",
	"Running",
	"Close",
};


static void free_params_thread(struct rtos * rtos)
{
	struct rt_thread_params *params = rtos->rtos_specific_params;
	size_t thread_index;
        
	rt_thread_params_thread_t params_thread_t = params->threads;

	for (thread_index = 0; thread_index < params->num_threads; thread_index++)
	{
		if (params_thread_t != NULL)
		{
	                rt_thread_params_thread_t params_thread_t_next = params_thread_t->next;
			free(params_thread_t);
			params_thread_t = params_thread_t_next;
		}
	}
	params->threads = NULL;
}


static int rt_thread_find_or_create_thread(struct rtos *rtos, symbol_address_t thread_address,
		threadid_t *threadid)
{
	struct rt_thread_params *params = rtos->rtos_specific_params;
	size_t thread_index;

        rt_thread_params_thread_t params_thread_t = params->threads;
	
	for (thread_index = 0; thread_index < params->num_threads; thread_index++)
	{
		if (params_thread_t == NULL)
			break;
		if (params_thread_t->thread_address == thread_address)
			goto found;
		else{
			params_thread_t = params_thread_t->next;
		}
	}

	params_thread_t = malloc(sizeof(struct rt_thread_params_thread));
        if (params_thread_t == NULL) {
		LOG_ERROR("rt-thread: out of memory");
		return ERROR_FAIL;
	}
	memset(params_thread_t, 0x00, sizeof(struct rt_thread_params_thread));
	params_thread_t->thread_address = thread_address;
	
	params_thread_t->next = params->threads;
	params_thread_t->prev = params_thread_t;

	if (params->threads != NULL)
		params->threads->prev = params_thread_t;
	
	params->threads = params_thread_t;

	params->num_threads++;
found:
	*threadid = thread_index + params->threadid_start;
	return ERROR_OK;
}

static int rt_thread_find_thread_address(struct rtos *rtos, threadid_t threadid,
		symbol_address_t *thread_address)
{
	struct rt_thread_params *params = rtos->rtos_specific_params;
	size_t thread_index;
	size_t num;

	thread_index = threadid - params->threadid_start;
	thread_index = params->num_threads - thread_index;

	if (thread_index >= params->num_threads) {
		LOG_ERROR("rt-thread: failed to find thread address");
		return ERROR_FAIL;
	}
        
        rt_thread_params_thread_t params_thread_t = params->threads;
	for (num = 0; num < thread_index; num++)
	{
		if (params_thread_t != NULL)
		{
                        *thread_address = params_thread_t->thread_address;
			params_thread_t = params_thread_t->next;
		}
	}

	return ERROR_OK;
}

static int rt_thread_find_last_thread_address(struct rtos *rtos, symbol_address_t *thread_address, int * thread_count)
{
	struct rt_thread_params *params = rtos->rtos_specific_params;
	int retval;
	size_t num;

	symbol_address_t object_list_address = 0;
	symbol_address_t thread_list_address = 0;
	symbol_address_t thread_list_head_address = 0;
	
	/*get the object_list of rt_object_container */
	retval = target_read_memory(rtos->target,
			rtos->symbols[rt_thread_VAL_rt_object_container].address + params->rt_object_next_offset,
			params->pointer_width,
			1,
			(void *)&object_list_address);

	if (retval != ERROR_OK) {
		LOG_ERROR("rt-thread: failed to read object list address");
		return retval;
	}
	
	thread_list_address = object_list_address - params->thread_next_offset;
	num = 0;
	thread_list_head_address = thread_list_address;
	*thread_address = thread_list_address;

	/* advance to end of thread list */
	do{
	         retval = target_read_memory(rtos->target,
				thread_list_address + params->thread_next_offset,
				params->pointer_width,
				1,
				(void *)&thread_list_address);
		if (retval != ERROR_OK) {
			LOG_ERROR("rt-thread: failed to read next thread address");
			return retval;
		}
		if ((rtos->symbols[rt_thread_VAL_rt_object_container].address + params->rt_object_next_offset) == thread_list_address){
			num++;
			break;
		}
                thread_list_address -= params->thread_next_offset;
		*thread_address = thread_list_address;
		num++;
        } while (thread_list_address != 0 && thread_list_address != thread_list_head_address);
        
	*thread_count = num;
	return ERROR_OK;
}

static int rt_thread_update_thread_offsets(struct rtos *rtos)
{
	struct rt_thread_params *params = rtos->rtos_specific_params;

	if (params->thread_offsets_updated)
		return ERROR_OK;

	const struct thread_offset_map {
		enum rt_thread_symbol_values symbol_value;
		symbol_address_t *thread_offset;
	} thread_offset_maps[] = {
		{
			rt_thread_VAL_rt_thread_sp_offset,
			&params->thread_stack_offset,
		},
		{
			rt_thread_VAL_rt_thread_name_offset,
			&params->thread_name_offset,
		},
		{
			rt_thread_VAL_rt_thread_stat_offset,
			&params->thread_state_offset,
		},
		{
			rt_thread_VAL_rt_thread_current_priority_offset,
			&params->thread_priority_offset,
		},
		{
			rt_thread_VAL_rt_thread_prev_offset,
			&params->thread_prev_offset,
		},
		{
			rt_thread_VAL_rt_thread_next_offset,
			&params->thread_next_offset,
		},
		{
			rt_thread_VAL_rt_object_next_offset,
			&params->rt_object_next_offset,
		},
		{
			rt_thread_VAL_rt_object_prev_offset,
			&params->rt_object_prev_offset,
		},
	};

	for (size_t i = 0; i < ARRAY_SIZE(thread_offset_maps); i++) {
		const struct thread_offset_map *thread_offset_map = &thread_offset_maps[i];

		int retval = target_read_memory(rtos->target,
				rtos->symbols[thread_offset_map->symbol_value].address,
				params->pointer_width,
				1,
				(void *)thread_offset_map->thread_offset);
		if (retval != ERROR_OK) {
			LOG_ERROR("rt-thread: failed to read thread offset");
			return retval;
		}
	}
	
	params->thread_offsets_updated = true;
	return ERROR_OK;
}

static bool rt_thread_detect_rtos(struct target *target)
{
	return target->rtos->symbols != NULL &&
			target->rtos->symbols[rt_thread_VAL_rt_current_thread].address != 0;
}

static int rt_thread_reset_handler(struct target *target, enum target_reset_mode reset_mode, void *priv)
{
	struct rt_thread_params *params = target->rtos->rtos_specific_params;
        size_t thread_index;
        
	rt_thread_params_thread_t params_thread_t = params->threads;

	params->thread_offsets_updated = false;
	
	for (thread_index = 0; thread_index < params->num_threads; thread_index++)
	{
		if (params_thread_t != NULL)
		{
	                rt_thread_params_thread_t params_thread_t_next = params_thread_t->next;
			free(params_thread_t);
			params_thread_t = params_thread_t_next;
		}
	}
	params->threads = NULL;
	params->num_threads = 0;

	return ERROR_OK;
}

static int rt_thread_create(struct target *target)
{
	struct rt_thread_params *params;

	for (size_t i = 0; i < ARRAY_SIZE(rt_thread_params_list); i++)
		if (strcmp(rt_thread_params_list[i].target_name, target->type->name) == 0) {
			params = malloc(sizeof(*params));
			if (params == NULL) {
				LOG_ERROR("rt-thread: out of memory");
				return ERROR_FAIL;
			}

			memcpy(params, &rt_thread_params_list[i], sizeof(rt_thread_params_list[i]));
			target->rtos->rtos_specific_params = (void *)params;

			target_register_reset_callback(rt_thread_reset_handler, NULL);

			return ERROR_OK;
		}

	LOG_ERROR("rt-thread: target not supported: %s", target->type->name);
	return ERROR_FAIL;
}

static int rt_thread_update_threads(struct rtos *rtos)
{
	struct rt_thread_params *params = rtos->rtos_specific_params;
	int retval;

	/* free previous thread details */
	rtos_free_threadlist(rtos);
	
	/* update thread offsets */
	retval = rt_thread_update_thread_offsets(rtos);
	if (retval != ERROR_OK) {
		LOG_ERROR("rt-thread: failed to update thread offsets");
		return retval;
	}

	/* read current thread address */
	symbol_address_t current_thread_address = 0;

	retval = target_read_memory(rtos->target,
			rtos->symbols[rt_thread_VAL_rt_current_thread].address,
			params->pointer_width,
			1,
			(void *)&current_thread_address);
	if (retval != ERROR_OK) {
		LOG_ERROR("rt-thread: failed to read current thread address");
		return retval;
	}

	symbol_address_t thread_object_container = 0;
	retval = target_read_memory(rtos->target,
			rtos->symbols[rt_thread_VAL_rt_object_container].address,
			params->pointer_width,
			1,
			(void *)&thread_object_container);
	if (retval != ERROR_OK) {
		LOG_ERROR("rt-thread: failed to read thread count");
		return retval;
	}

	symbol_address_t thread_address = 0;

	retval = rt_thread_find_last_thread_address(rtos, &thread_address, &rtos->thread_count);
	if (retval != ERROR_OK) {
		LOG_ERROR("rt-thread: failed to find last thread address");
		return retval;
	}

	rtos->thread_details = calloc(rtos->thread_count, sizeof(struct thread_detail));
	if (rtos->thread_details == NULL) {
		LOG_ERROR("rt-thread: out of memory");
		return ERROR_FAIL;
	}

	int i;
	int current_index = 0;

	free_params_thread(rtos);
	for (i = 0; i < rtos->thread_count; i++) {
		struct thread_detail *thread_detail = &rtos->thread_details[i];
		char thread_str_buffer[RT_THREAD_MAX_STRLEN + 1];
		memset(thread_str_buffer, 0x00, sizeof(thread_str_buffer));

		/* find or create new threadid */
		retval = rt_thread_find_or_create_thread(rtos, thread_address, &thread_detail->threadid);
		if (retval != ERROR_OK) {
			LOG_ERROR("rt-thread: failed to find or create thread");
			return retval;
		}

		if (thread_address == current_thread_address){
			rtos->current_thread = thread_detail->threadid;
                        current_index = i;
		}

		thread_detail->exists = true;
		symbol_address_t thread_name_address = thread_address + params->thread_name_offset;
		retval = target_read_buffer(rtos->target,
				thread_name_address,
				sizeof(thread_str_buffer),
				(void *)thread_str_buffer);
		if (retval != ERROR_OK) {
			LOG_ERROR("rt-thread: failed to read thread name");
			return retval;
		}

		thread_str_buffer[sizeof(thread_str_buffer) - 1] = '\0';
		thread_detail->thread_name_str = strdup(thread_str_buffer);

		/* read thread extra info */
		uint8_t thread_state;
		uint8_t thread_priority;

		retval = target_read_u8(rtos->target,
				thread_address + params->thread_state_offset,
				&thread_state);
		if (retval != ERROR_OK) {
			LOG_ERROR("rt-thread: failed to read thread state");
			return retval;
		}

		retval = target_read_u8(rtos->target,
				thread_address + params->thread_priority_offset,
				&thread_priority);
		if (retval != ERROR_OK) {
			LOG_ERROR("rt-thread: failed to read thread priority");
			return retval;
		}

		const char *thread_state_str;

		if (thread_state < ARRAY_SIZE(rt_thread_thread_state_list))
			thread_state_str = rt_thread_thread_state_list[thread_state];
		else
			thread_state_str = "Unknown";

		snprintf(thread_str_buffer, sizeof(thread_str_buffer), "State: %s, Priority: %d",
				thread_state_str, thread_priority);
		thread_detail->extra_info_str = strdup(thread_str_buffer);

		/* read previous thread address */
		retval = target_read_memory(rtos->target,
				thread_address + params->thread_prev_offset,
				params->pointer_width,
				1,
				(void *)&thread_address);
		thread_address -= params->thread_next_offset;
		if (retval != ERROR_OK) {
			LOG_ERROR("rt-thread: failed to read previous thread address");
			return retval;
		}
	}
        
	struct thread_detail temp;
	memcpy(&temp, &rtos->thread_details[0],sizeof(struct thread_detail));
	memcpy(&rtos->thread_details[0], &rtos->thread_details[current_index], sizeof(struct thread_detail));
	memcpy(&rtos->thread_details[current_index], &temp, sizeof(struct thread_detail));

	rtos->thread_details[current_index].threadid = rtos->thread_details[0].threadid;
	rtos->thread_details[0].threadid = temp.threadid;
	
	//rtos->current_thread = 0 + params->threadid_start; 
	rtos->current_thread = rtos->thread_details[0].threadid;

   
        struct rt_thread_params_thread * params_thread_first = NULL;
        struct rt_thread_params_thread * params_thread_current = NULL;
        struct rt_thread_params_thread * params_thread_next = params->threads;

        size_t index;
        for(index = 0; index < params->num_threads; index++)
        {
                 if (params_thread_next == NULL)
                         break;

                 if (index == params->num_threads - 1)
                 {
                         params_thread_first = params_thread_next;
                 }
        
                 if (index == params->num_threads - current_index - 1)
                 {
                         params_thread_current = params_thread_next;
                 }
                 params_thread_next = params_thread_next->next;
         }

         if( params_thread_first != NULL && params_thread_current != NULL && params_thread_first->thread_address != current_thread_address)
         {
                 struct rt_thread_params_thread params_temp;
                 memcpy(&params_temp, params_thread_first, sizeof(struct rt_thread_params_thread));
                 params_thread_first->thread_address = params_thread_current->thread_address;    
                 params_thread_current->thread_address = params_temp.thread_address;
         }

	return ERROR_OK;
}

static int rt_thread_get_thread_reg_list(struct rtos *rtos, threadid_t threadid, char **hex_reg_list)
{
	struct rt_thread_params *params = rtos->rtos_specific_params;
	int retval;

	/* find thread address for threadid */
	symbol_address_t thread_address = 0;

	retval = rt_thread_find_thread_address(rtos, threadid, &thread_address);
	if (retval != ERROR_OK) {
		LOG_ERROR("rt-thread: failed to find thread address");
		return retval;
	}

	/* read thread stack address */
	symbol_address_t stack_address = 0;

	retval = target_read_memory(rtos->target,
			thread_address + params->thread_stack_offset,
			params->pointer_width,
			1,
			(void *)&stack_address);
	if (retval != ERROR_OK) {
		LOG_ERROR("rt-thread: failed to read stack address");
		return retval;
	}

	return rtos_generic_stack_read(rtos->target,
			params->stacking_info,
			stack_address,
			hex_reg_list);
}

static int rt_thread_get_symbol_list_to_lookup(symbol_table_elem_t *symbol_list[])
{
	*symbol_list = calloc(ARRAY_SIZE(rt_thread_symbol_list), sizeof(symbol_table_elem_t));
	if (*symbol_list == NULL) {
		LOG_ERROR("rt-thread: out of memory");
		return ERROR_FAIL;
	}

	for (size_t i = 0; i < ARRAY_SIZE(rt_thread_symbol_list); i++)
		(*symbol_list)[i].symbol_name = rt_thread_symbol_list[i];

	return ERROR_OK;
}

const struct rtos_type rt_thread_rtos = {
	.name = "rt-thread",
	.detect_rtos = rt_thread_detect_rtos,
	.create = rt_thread_create,
	.update_threads = rt_thread_update_threads,
	.get_thread_reg_list = rt_thread_get_thread_reg_list,
	.get_symbol_list_to_lookup = rt_thread_get_symbol_list_to_lookup,
};
