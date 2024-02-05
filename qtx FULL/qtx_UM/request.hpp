#pragma once
#define STATUS_SUCCESS 1
#define STATUS_UNSUCCESSFUL 0

#define IOCTL_INIT				CTL_CODE(FILE_DEVICE_UNKNOWN, 0x710, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)
#define IOCTL_GET_BASE			CTL_CODE(FILE_DEVICE_UNKNOWN, 0x720, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)
#define IOCTL_READ				CTL_CODE(FILE_DEVICE_UNKNOWN, 0x730, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)
#define IOCTL_WRITE				CTL_CODE(FILE_DEVICE_UNKNOWN, 0x740, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)
#define IOCTL_RMOVE_NODE		CTL_CODE(FILE_DEVICE_UNKNOWN, 0x750, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)
#define IOCTL_FREE		        CTL_CODE(FILE_DEVICE_UNKNOWN, 0x760, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)
#define IOCTL_PATTERN		    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x770, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)
#define IOCTL_ALLOCATE	        CTL_CODE(FILE_DEVICE_UNKNOWN, 0x780, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)
#define IOCTL_SWAP	            CTL_CODE(FILE_DEVICE_UNKNOWN, 0x790, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)
#define IOCTL_QUERY_MEMORY      CTL_CODE(FILE_DEVICE_UNKNOWN, 0x800, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)
#define IOCTL_GET_MODULE      CTL_CODE(FILE_DEVICE_UNKNOWN, 0x810, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)

typedef enum Request {
	Key = 0x1B4B60D0,
	rReqInitDriver = 1853172337578914,
	rReqGetWindowStyles = 913752309856562,
	rReqSetWindowStyles = 10171373554236980,
	rReqGetWindowRect = 2105953210941948,
	rReqSetWindowRect = 1653172103578569,
	rReqReadMemory = 719430956129135,
	rReqWriteMemory = 1205175564109394,
	rReqQueryMemory = 310750134750157091,
	rReqMoveMouse = 1356903150978512300,
	rReqQuerySystemInfo = 13578095491295217,
	rReqGrantHandleAccess = 13067092468515423,
	rReqHideTopWindow = 103571056378015708,
	rReqBatchReadMemory = 130958064170955397
} Requests;

typedef struct _init_invoke {
	IN ULONG PID;
	IN INT Key;
} init_invoke, * pinit_invoke;

typedef struct _base_invoke {
	IN ULONG PID;
	OPTIONAL OUT ULONGLONG BASE_ADDRESS;
} base_invoke, * pbase_invoke;

typedef struct _module_invoke {
	IN ULONG PID;
	OPTIONAL OUT uintptr_t handle;
	IN const char* name;
	IN size_t size;
} module_invoke, * pmodule_invoke;

typedef struct _read_invoke {
	IN ULONG PID;
	IN ULONG NumBytesToRead;
	IN PVOID Buffer;
	IN PVOID Address;
	OPTIONAL OUT PNTSTATUS Status;
	OPTIONAL OUT PSIZE_T BytesRead;
} read_invoke, * pread_invoke;

typedef struct _write_invoke {
	IN SHORT PID;
	IN ULONG NumBytesToWrite;
	IN PVOID Buffer;
	IN PVOID Address;
	OPTIONAL OUT PNTSTATUS Status;
	OPTIONAL OUT PSIZE_T BytesWritten;
} write_invoke, * pwrite_invoke;

typedef struct _swap_invoke
{
	IN ULONG PID;
	IN uintptr_t dst;
	IN uintptr_t src;
	IN uintptr_t old;
}swap_invoke, * pswap_invoke;

typedef struct _pattern_invoke
{
	IN ULONG PID;
	IN uintptr_t base;
	IN char signature [ 260 ];
	OPTIONAL OUT uintptr_t address;
}pattern_invoke, * ppattern_invoke;

typedef struct _allocate_invoke
{
	IN ULONG PID;
	OPTIONAL OUT uintptr_t base;
	IN size_t size;
}allocate_invoke, * pallocate_invoke;

typedef struct _remove_node_invoke
{
	IN ULONG PID;
	IN uintptr_t address;
}remove_node_invoke, * premove_node_invoke;

typedef struct _free_invoke
{
	IN ULONG PID;
	IN uintptr_t address;
}free_invoke, * pfree_invoke;

typedef struct _query_memory_invoke
{
	IN ULONG PID;
	IN uintptr_t address;
	OPTIONAL OUT uintptr_t page_base;
	IN UINT32 page_prot;
	IN size_t page_size;
}query_memory_invoke, * pquery_memory_invoke;

typedef struct _IO_STATUS_BLOCK {
	union {
		NTSTATUS Status;
		PVOID    Pointer;
	};
	ULONG_PTR Information;
} IO_STATUS_BLOCK, * PIO_STATUS_BLOCK;

typedef
VOID
( NTAPI* PIO_APC_ROUTINE ) (
	IN PVOID ApcContext,
	IN PIO_STATUS_BLOCK IoStatusBlock,
	IN ULONG Reserved
	);

extern "C" __int64 DirectIO(
	HANDLE FileHandle,
	HANDLE Event,
	PIO_APC_ROUTINE
	ApcRoutine,
	PVOID ApcContext,
	PIO_STATUS_BLOCK IoStatusBlock,
	ULONG IoControlCode,
	PVOID InputBuffer,
	ULONG InputBufferLength,
	PVOID OutputBuffer,
	ULONG OutputBufferLength );

namespace qtx
{
	class communication_ctx
	{
	public:

		communication_ctx( ULONG PID );
		UINT64 get_base_address( );
		void rkm( void* address, void* buffer, const std::size_t size );
		void wkm( void* address, void* buffer, const std::size_t size );

		void remove_vad( const std::uintptr_t address );
		void free_k( const std::uintptr_t address );

		bool allocate_vad( std::uintptr_t& base, const std::size_t size );
		bool query_kmemory( const uintptr_t addr, uintptr_t* page_base, uint32_t* page_prot, ULONG* page_size );

		std::uintptr_t swap_kpointer( std::uintptr_t src, std::uintptr_t dst );
		std::uintptr_t find_signature( const std::uintptr_t base, const std::string signature );
		std::uintptr_t get_kmodule( const char* module_name );

		template<class T>
		__forceinline auto rpm( std::uintptr_t Address ) -> T
		{
			T buffer {};

			IO_STATUS_BLOCK block;
			read_invoke args;

			args.Address = reinterpret_cast< PVOID >(Address);
			args.Buffer = &buffer;
			args.BytesRead = nullptr;
			args.NumBytesToRead = sizeof( T );
			args.PID = this->target_pid;
			args.Status = nullptr;

			DirectIO(
				this->driver_handle,
				nullptr,
				nullptr,
				nullptr,
				&block,
				IOCTL_READ,
				&args,
				sizeof( args ),
				&args,
				sizeof( args ) );

			return buffer;
		}

		template<typename T>
		__forceinline auto wpm( T data, UINT64 Address ) -> T
		{
			IO_STATUS_BLOCK block;
			write_invoke args;

			args.Address = reinterpret_cast< PVOID >(Address);
			args.Buffer = &data;
			args.BytesWritten = nullptr;
			args.NumBytesToWrite = sizeof( T );
			args.PID = this->target_pid;
			args.Status = nullptr;
			DirectIO(
				this->driver_handle,
				nullptr,
				nullptr,
				nullptr,
				&block,
				IOCTL_WRITE,
				&args,
				sizeof( args ),
				nullptr,
				0 );
		}

	private:

		ULONG target_pid;
		HANDLE driver_handle;
		UINT64 base_address;
	};
};