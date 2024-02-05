#pragma once

namespace request
{
	_declspec(noinline) qtx::status allocate_memory( invoke_data* _request )
	{
		allocate_invoke request;
		if (!modules::safe_copy( &request, _request->data, sizeof( allocate_invoke ) ))
			return qtx::status::failed_sanity_check;

		// sanity check (check for invalid values)
		if (!request.base || !request.pid || !request.size)
		{
			reinterpret_cast<pallocate_invoke>(_request->data)->base = 0;
			printf( "allocate_memory sanity check failed.\n" );
			return qtx::status::failed_sanity_check;
		}
		// end sanity check

		PEPROCESS process = 0;
		if (!NT_SUCCESS(imports::ps_lookup_process_by_process_id( (HANDLE)request.pid, &process )))
		{
			reinterpret_cast<pallocate_invoke>(_request->data)->base = 0;
			imports::obf_dereference_object( process );
			printf( "process sanity check failed.\n" );
			return qtx::status::failed_sanity_check;
		}

		uintptr_t start = request.base, end = start + request.size;
		uintptr_t o_process = modules::attach_process( (uintptr_t)process );

		MEMORY_BASIC_INFORMATION mbi;
		if (!NT_SUCCESS(imports::zw_query_virtual_memory( ZwCurrentProcess( ), (PVOID)start, MemoryBasicInformation, &mbi, sizeof( mbi ), 0 ) ))
		{
			printf( "query memory failed.\n" );
			modules::attach_process( o_process );
			imports::obf_dereference_object( process );
			reinterpret_cast<pallocate_invoke>(_request->data)->base = 0;
			return qtx::status::failed_sanity_check;
		}

		PMMVAD_SHORT vad = imports::mi_allocate_vad( start, end, 1 );
		if (!vad)
		{
			printf( "unable to allocate vad.\n" );
			modules::attach_process( o_process );
			imports::obf_dereference_object( process );
			reinterpret_cast<pallocate_invoke>(_request->data)->base = 0;
			return qtx::status::failed_sanity_check;
		}

		PMMVAD_FLAGS flags = (PMMVAD_FLAGS)&vad->u.LongFlags;
		flags->Protection = MM_EXECUTE_READWRITE;
		flags->NoChange = 0;

		if (!NT_SUCCESS(imports::mi_insert_vad_charges( vad, process )))
		{
			printf( "unable to insert vad charges.\n" );
			imports::ex_free_pool_with_tag( vad, NULL );
			modules::attach_process( o_process );
			imports::obf_dereference_object( process );
			reinterpret_cast<pallocate_invoke>(_request->data)->base = 0;
			return qtx::status::failed_sanity_check;
		}
		imports::mi_insert_vad( vad, process );

		modules::attach_process( o_process );
		imports::obf_dereference_object( process );

		reinterpret_cast<pallocate_invoke>(_request->data)->base = start;
		return qtx::status::successful_operation;
	}
	
	_declspec(noinline) qtx::status unlink_nodes( invoke_data* _request )
	{
		remove_node_invoke request;
		if (!modules::safe_copy( &request, _request->data, sizeof( remove_node_invoke ) ))
			return qtx::status::failed_sanity_check;

		// sanity check (check for invalid values)
		if (!request.address || !request.pid )
		{
			printf( "unlinking vad nodes failed sanity check.\n" );
			return qtx::status::failed_sanity_check;
		}
		// end sanity check

		PEPROCESS process = 0;
		if (!NT_SUCCESS( imports::ps_lookup_process_by_process_id( (HANDLE)request.pid, &process ) ))
		{
			printf( "unlinking vad nodes failed sanity check.\n" );
			imports::obf_dereference_object( process );
			return qtx::status::failed_sanity_check;
		}

		PMMVAD_SHORT vad_short = NULL;
		PMM_AVL_TABLE table = (PMM_AVL_TABLE)((PUCHAR)process + 0x7D8);

		modules::find_vad( process, request.address, &vad_short );
		imports::rtl_avl_remove_node( table, reinterpret_cast<PMMADDRESS_NODE>(vad_short) );

		return qtx::status::successful_operation;
	}

	_declspec(noinline) qtx::status free( invoke_data* _request )
	{
		free_invoke request;
		if (!modules::safe_copy( &request, _request->data, sizeof( free_invoke ) ))
			return qtx::status::failed_sanity_check;

		// sanity check (check for invalid values)
		if (!request.address || !request.pid)
		{
			printf( "unlinking vad nodes failed sanity check.\n" );
			return qtx::status::failed_sanity_check;
		}
		// end sanity check

		PEPROCESS process = 0;
		if (!NT_SUCCESS( imports::ps_lookup_process_by_process_id( (HANDLE)request.pid, &process ) ))
		{
			printf( "unlinking vad nodes failed sanity check.\n" );
			imports::obf_dereference_object( process );
			return qtx::status::failed_sanity_check;
		}

		uintptr_t o_process = modules::attach_process( (uintptr_t)process );

		SIZE_T size = 0;
		PVOID address = (PVOID)request.address;

		imports::zw_free_virtual_memory( imports::io_get_current_process( ), &address, &size, 0x00008000 );

		modules::attach_process( o_process );

		imports::obf_dereference_object( process );

		return qtx::status::successful_operation;
	}
}