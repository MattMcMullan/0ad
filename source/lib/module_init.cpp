/* Copyright (c) 2010 Wildfire Games
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/*
 * helpers for module initialization/shutdown.
 */

#include "precompiled.h"
#include "lib/module_init.h"

#include "lib/sysdep/cpu.h"	// cpu_CAS

// not yet initialized, or already shutdown
static const ModuleInitState UNINITIALIZED = 0;	// value documented in header
// running user callback - concurrent ModuleInit callers must spin
static const ModuleInitState BUSY = INFO::ALREADY_EXISTS;	// never returned
// init succeeded; allow shutdown
static const ModuleInitState INITIALIZED = INFO::SKIPPED;


LibError ModuleInit(volatile ModuleInitState* initState, LibError (*init)())
{
	for(;;)
	{
		if(cpu_CAS(initState, UNINITIALIZED, BUSY))
		{
			LibError ret = init();
			*initState = (ret == INFO::OK)? INITIALIZED : ret;
			cpu_MemoryBarrier();
			return ret;
		}

		const ModuleInitState latchedInitState = *initState;
		if(latchedInitState == UNINITIALIZED || latchedInitState == BUSY)
		{
			cpu_Pause();
			continue;
		}

		debug_assert(latchedInitState == INITIALIZED || latchedInitState < 0);
		return (LibError)latchedInitState;
	}
}


LibError ModuleShutdown(volatile ModuleInitState* initState, void (*shutdown)())
{
	for(;;)
	{
		if(cpu_CAS(initState, INITIALIZED, BUSY))
		{
			shutdown();
			*initState = UNINITIALIZED;
			cpu_MemoryBarrier();
			return INFO::OK;
		}

		const ModuleInitState latchedInitState = *initState;
		if(latchedInitState == INITIALIZED || latchedInitState == BUSY)
		{
			cpu_Pause();
			continue;
		}

		if(latchedInitState == UNINITIALIZED)
			return INFO::SKIPPED;

		debug_assert(latchedInitState < 0);
		return (LibError)latchedInitState;
	}
}
