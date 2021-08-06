/*
  Copyright (c) 2021 Ryan Powell.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the GNU Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include <string.h>
#include <stdlib.h>
#include <sys/reent.h>
#include <malloc.h>
#include "freertos/FreeRTOS.h"
#include "nrf.h"

extern void *pvPortRealloc(void *mem, size_t newsize);
extern void *pvPortCalloc(size_t count, size_t size);

void* malloc(size_t size)
{
    return pvPortMalloc(size);
}

void* calloc(size_t nmemb, size_t size)
{
    return pvPortCalloc(nmemb, size);
}

void* realloc(void* ptr, size_t size)
{
    return pvPortRealloc(ptr, size);
}

void free(void *ptr)
{
    vPortFree(ptr);
}

void* _malloc_r(struct _reent *r, size_t size)
{
    (void)r;
    return pvPortMalloc(size);
}

void _free_r(struct _reent *r, void* ptr)
{
    (void)r;
    vPortFree(ptr);
}


void* _realloc_r(struct _reent *r, void* ptr, size_t size)
{
    (void)r;
    return pvPortRealloc( ptr, size );
}

void* _calloc_r(struct _reent *r, size_t nmemb, size_t size)
{
    (void)r;
    return pvPortCalloc(nmemb, size);
}

/* The following functions are implemented by newlib's heap allocator,
   but aren't available in the heap component.
   Define them as non-functional stubs here, so that the application
   can not cause the newlib heap implementation to be linked in
 */

int malloc_trim(size_t pad)
{
    return 0; // indicates failure
}

size_t malloc_usable_size(void* p)
{
    return 0;
}

void malloc_stats(void)
{
}

int mallopt(int parameter_number, int parameter_value)
{
    return 0; // indicates failure
}

struct mallinfo mallinfo(void)
{
    struct mallinfo dummy = {0};
    return dummy;
}

void* valloc(size_t n) __attribute__((alias("malloc")));
void* pvalloc(size_t n) __attribute__((alias("malloc")));
void cfree(void* p) __attribute__((alias("free")));
