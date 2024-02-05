typedef enum _requests
{
	invoke_base = 0x119, 
	invoke_read = 0x129,
	invoke_write = 0x139,
	invoke_guarded_region = 0x132,
	invoke_pattern = 0x179,
	invoke_swap = 0x172,
	invoke_success = 0x91a,
	invoke_remove_node = 0x136,
	invoke_free = 0x133,
	invoke_unique = 0x92b,
	invoke_allocate,
	invoke_unload,
	invoke_query_memory,
}requests, *prequests;

typedef struct _read_invoke {
	uint32_t pid;
	uintptr_t address;
	void* buffer;
	size_t size;
} read_invoke, * pread_invoke;

typedef struct _write_invoke {
	uint32_t pid;
	uintptr_t address;
	void* buffer;
	size_t size;
} write_invoke, * pwrite_invoke;

typedef struct _base_invoke {
	uint32_t pid;
	uintptr_t handle;
	const char* name;
	size_t size;
} base_invoke, * pbase_invoke;

typedef struct _guarded_region_invoke {
	uint32_t pid;
	uintptr_t buffer;
	size_t size;
} guarded_region_invoke, * pguarded_region_invoke;

typedef struct _swap_invoke
{
	uint32_t pid;
	uintptr_t dst;
	uintptr_t src;
	uintptr_t old;
}swap_invoke, * pswap_invoke;

typedef struct _pattern_invoke
{
	int pid;
	uintptr_t base;
	char signature[260];
	uintptr_t address;
}pattern_invoke, * ppattern_invoke;

typedef struct _invoke_data
{
	uint32_t unique;
	requests code;
	void* data;
}invoke_data, * pinvoke_data;

typedef struct _allocate_invoke
{
	int pid;
	uintptr_t base;
	size_t size;
}allocate_invoke, * pallocate_invoke;

typedef struct _remove_node_invoke
{
	int pid;
	uintptr_t address;
}remove_node_invoke, * premove_node_invoke;

typedef struct _free_invoke
{
	int pid;
	uintptr_t address;
}free_invoke, * pfree_invoke;

typedef struct _query_memory_invoke
{
	int pid;
	uintptr_t address;
	uintptr_t page_base;
	uint32_t page_prot;
	size_t page_size;
}query_memory_invoke, * pquery_memory_invoke;

typedef struct _unload_invoke
{
	bool unloaded;
}unload_invoke, * punload_invoke;