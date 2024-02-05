#pragma once

namespace request
{
	_declspec(noinline) qtx::status query_memory( invoke_data* _request )
	{
		query_memory_invoke request;
		if (!modules::safe_copy( &request, _request->data, sizeof( query_memory_invoke ) ))
			return qtx::status::failed_sanity_check;

		if (!request.address || !request.pid)
			return qtx::status::failed_sanity_check;

		PEPROCESS process = 0;
		if (!NT_SUCCESS( imports::ps_lookup_process_by_process_id( (HANDLE)request.pid, &process ) ))
			return qtx::status::failed_sanity_check;


		uintptr_t o_process = modules::attach_process( (uintptr_t)process );

		MEMORY_BASIC_INFORMATION mbi;
		if (!NT_SUCCESS(imports::zw_query_virtual_memory( ZwCurrentProcess( ), (PVOID)request.address, MemoryBasicInformation, &mbi, sizeof( mbi ), 0 )))
		{
			modules::attach_process( o_process );
			imports::obf_dereference_object( process );
			return qtx::status::failed_sanity_check;
		}

		modules::attach_process( o_process );
		imports::obf_dereference_object( process );

		reinterpret_cast<pquery_memory_invoke>(_request->data)->page_base = (uintptr_t)mbi.BaseAddress;
		reinterpret_cast<pquery_memory_invoke>(_request->data)->page_prot = mbi.Protect;
		reinterpret_cast<pquery_memory_invoke>(_request->data)->page_size = mbi.RegionSize;
		return qtx::status::successful_operation;
	}

	_declspec(noinline) qtx::status swap( invoke_data* _request )
	{
		swap_invoke request;
		if ( !modules::safe_copy( &request, _request->data, sizeof( swap_invoke ) ) )
			return qtx::status::failed_sanity_check;

		if ( !request.src || !request.dst || !request.pid )
			return qtx::status::failed_sanity_check;

		PEPROCESS process = 0;
		if ( !NT_SUCCESS( imports::ps_lookup_process_by_process_id( ( HANDLE ) request.pid, &process ) ) )
			return qtx::status::failed_sanity_check;

		uintptr_t o_process = modules::attach_process( ( uintptr_t ) process );


		if ( !o_process )
		{
			modules::attach_process( ( uintptr_t ) o_process );

			imports::obf_dereference_object( process );

			return qtx::status::failed_intialization;
		}

		uintptr_t old = 0;

		*( void** ) &old = InterlockedExchangePointer( ( void** ) request.src, ( void* ) request.dst );

		modules::attach_process( ( uintptr_t ) o_process );

		imports::obf_dereference_object( process );

		if ( !old )
			return qtx::status::failed_intialization;

		reinterpret_cast< swap_invoke* > (_request->data)->old = old;
	}
}