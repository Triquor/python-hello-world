#pragma once
#include "../includes.h"

namespace qtx
{
	communication_ctx::communication_ctx( ULONG PID )
	{
		this->target_pid = PID;
		this->driver_handle = QTX_IMPORT( CreateFileA ).safe_cached( )(_( "\\\\.\\{82cd7f0e-2028-4956-b0b4-39c76fdaef1d}" ), GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
		if ( driver_handle != INVALID_HANDLE_VALUE )
		{
			std::printf( " [log] -> handle created to i/o.\n" );

			IO_STATUS_BLOCK block;
			init_invoke args;
			SecureZeroMemory( &args, sizeof( args ) );

			args.PID = PID;
			args.Key = Request::Key;

			if ( DirectIO(
				this->driver_handle,
				nullptr,
				nullptr,
				nullptr,
				&block,
				IOCTL_INIT,
				&args,
				sizeof( args ),
				&args,
				sizeof( args ) ) == 0L ) {

				const auto base_address = this->get_base_address( );
				if ( !base_address ) {
					return;
				}
			}
		}
		else
		{
			std::printf( " [log] -> failed to create a handle to i/o.\n" );
		}
		return;
	}

	auto communication_ctx::get_base_address( ) -> UINT64
	{
		IO_STATUS_BLOCK block;
		base_invoke args;

		args.PID = this->target_pid;

		DirectIO(
			this->driver_handle,
			nullptr,
			nullptr,
			nullptr,
			&block,
			IOCTL_GET_BASE,
			&args,
			sizeof( args ),
			&args,
			sizeof( args ) );
		return args.BASE_ADDRESS;
	}

	auto communication_ctx::rkm( void* address, void* buffer, const std::size_t size ) -> void
	{
		IO_STATUS_BLOCK block;
		read_invoke args;

		args.Address = address;
		args.Buffer = &buffer;
		args.BytesRead = nullptr;
		args.NumBytesToRead = size;
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
			nullptr,
			0 );
	}

	auto communication_ctx::wkm( void* address, void* buffer, const std::size_t size ) -> void
	{
		IO_STATUS_BLOCK block;
		write_invoke args;

		args.Address = address;
		args.Buffer = &buffer;
		args.BytesWritten = nullptr;
		args.NumBytesToWrite = size;
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

	auto communication_ctx::free_k( const std::uintptr_t address ) -> void
	{
		IO_STATUS_BLOCK block;
		free_invoke args;

		args.PID = this->target_pid;
		args.address = address;

		DirectIO(
			this->driver_handle,
			nullptr,
			nullptr,
			nullptr,
			&block,
			IOCTL_FREE,
			&args,
			sizeof( args ),
			nullptr,
			0 );
	}

	auto communication_ctx::allocate_vad( std::uintptr_t& base, const std::size_t size ) -> bool
	{
		IO_STATUS_BLOCK block;
		allocate_invoke args;

		args.PID = this->target_pid;
		args.base = base;
		args.size = size;

		DirectIO(
			this->driver_handle,
			nullptr,
			nullptr,
			nullptr,
			&block,
			IOCTL_ALLOCATE,
			&args,
			sizeof( args ),
			&args,
			sizeof( args ) );

		base = args.base;
		return args.base != 0;
	}

	auto communication_ctx::swap_kpointer( std::uintptr_t src, std::uintptr_t dst ) -> std::uintptr_t
	{
		IO_STATUS_BLOCK block;
		swap_invoke args;

		args.PID = this->target_pid;
		args.src = src;
		args.dst = dst;
		args.old = 0;

		DirectIO(
			this->driver_handle,
			nullptr,
			nullptr,
			nullptr,
			&block,
			IOCTL_SWAP,
			&args,
			sizeof( args ),
			&args,
			sizeof( args ) );
		return args.old;
	}

	auto communication_ctx::query_kmemory( const std::uintptr_t addr, std::uintptr_t* page_base, std::uint32_t* page_prot, ULONG* page_size ) -> bool
	{
		IO_STATUS_BLOCK block;
		query_memory_invoke   args;

		args.PID = this->target_pid;
		args.address = addr;

		auto result = DirectIO(
			this->driver_handle,
			nullptr,
			nullptr,
			nullptr,
			&block,
			IOCTL_SWAP,
			&args,
			sizeof( args ),
			&args,
			sizeof( args ) );

		if ( page_base ) *page_base = args.page_base;
		if ( page_prot ) *page_prot = args.page_prot;
		if ( page_size ) *page_size = args.page_size;

		return result;
	}

	auto communication_ctx::remove_vad( const std::uintptr_t address ) -> void
	{
		IO_STATUS_BLOCK block;
		remove_node_invoke   args;

		args.PID = this->target_pid;
		args.address = address;

		DirectIO(
			this->driver_handle,
			nullptr,
			nullptr,
			nullptr,
			&block,
			IOCTL_RMOVE_NODE,
			&args,
			sizeof( args ),
			nullptr,
			0 );
	}

	auto communication_ctx::get_kmodule( const char* module_name ) -> std::uintptr_t
	{
		IO_STATUS_BLOCK block;
		module_invoke args;

		args.PID = this->target_pid;
		args.handle = 0;
		args.name = module_name;

		DirectIO(
			this->driver_handle,
			nullptr,
			nullptr,
			nullptr,
			&block,
			IOCTL_GET_MODULE,
			&args,
			sizeof( args ),
			&args,
			sizeof( args ) );
		return args.handle;
	}

	auto communication_ctx::find_signature( const std::uintptr_t base, const std::string signature ) -> std::uintptr_t
	{
		IO_STATUS_BLOCK block;
		pattern_invoke args;

		args.PID = this->target_pid;
		args.base = base;
		args.address = 0;

		// :skull:
		//memset( args.signature, 0, sizeof( char ) * 260 );
		//strcpy( args.signature, signature.c_str( ) );

		DirectIO(
			this->driver_handle,
			nullptr,
			nullptr,
			nullptr,
			&block,
			IOCTL_PATTERN,
			&args,
			sizeof( args ),
			&args,
			sizeof( args ) );
		return args.address;
	}
};