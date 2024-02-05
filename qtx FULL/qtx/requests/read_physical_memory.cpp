#pragma once

namespace request
{
    NTSTATUS find_guarded_region( invoke_data* request )
    {
        guarded_region_invoke data = { 0 };
		uintptr_t address = { 0 };

  //      if ( !modules::safe_copy( &data, request->data, sizeof( guarded_region_invoke ) ) )
  //          return 0;

		//ULONG infoLen = 0;
		//NTSTATUS status = imports::zw_query_system_information( SystemBigPoolInformation, &infoLen, 0, &infoLen );
		//PSYSTEM_BIGPOOL_INFORMATION pPoolInfo = 0;

		//while ( status == STATUS_INFO_LENGTH_MISMATCH )
		//{
		//	if ( pPoolInfo )
		//		imports::ex_free_pool_with_tag( pPoolInfo, 0 );

		//	pPoolInfo = ( PSYSTEM_BIGPOOL_INFORMATION ) imports::ex_allocate_pool( NonPagedPool, infoLen );
		//	status = imports::zw_query_system_information( SystemBigPoolInformation, pPoolInfo, infoLen, &infoLen );
		//}

		//if ( pPoolInfo )
		//{
		//	for ( unsigned int i = 0; i < pPoolInfo->Count; i++ )
		//	{
		//		SYSTEM_BIGPOOL_ENTRY* Entry = &pPoolInfo->AllocatedInfo [ i ];
		//		PVOID VirtualAddress;
		//		VirtualAddress = ( PVOID ) (( uintptr_t ) Entry->VirtualAddress & ~1ull);
		//		SIZE_T SizeInBytes = Entry->SizeInBytes;
		//		BOOLEAN NonPaged = Entry->NonPaged;

		//		if ( NonPaged && SizeInBytes == 0x200000 )
		//		{
		//			if ( Entry->TagUlong == 'TnoC' ) {
		//				RtlCopyMemory( &address, &VirtualAddress, sizeof( VirtualAddress ) );
		//			}
		//		}
		//	}

		//	imports::ex_free_pool_with_tag( pPoolInfo, 0 );
		//}

		reinterpret_cast< guarded_region_invoke* > (request->data)->buffer = address;

        return qtx::status::successful_operation;
    }

    NTSTATUS read_memory( invoke_data* request )
    {
        read_invoke data = { 0 };

        if (!modules::safe_copy( &data, request->data, sizeof( read_invoke ) ))
            return 0;

		if ( !data.address || !data.pid || !data.buffer || !data.size || data.address >= 0x7FFFFFFFFFFF )
			return 0;

		PEPROCESS process = 0;
		if ( !NT_SUCCESS( imports::ps_lookup_process_by_process_id( ( HANDLE ) data.pid, &process ) ) )
		{
			printf( "process sanity check failed.\n" );
			return 0;
		}

		//if ( data.address >= 0x7FFFFFFFFFFF || data.phys )
		//{

		//	ULONGLONG process_base = physical_memory_operations::get_process_cr3( process );
		//	imports::obf_dereference_object( process );

		//	SIZE_T this_offset = NULL;
		//	SIZE_T total_size = data.size;

		//	INT64 physical_address = physical_memory_operations::translate_linear( process_base, ( ULONG64 ) data.address + this_offset );
		//	if ( !physical_address )
		//		return qtx::status::failed_sanity_check;;

		//	ULONG64 final_size = physical_memory_operations::find_min( PAGE_SIZE - (physical_address & 0xFFF), total_size );
		//	SIZE_T bytes_trough = NULL;

		//	if ( !NT_SUCCESS( physical_memory_operations::read( PVOID( physical_address ), ( PVOID ) ((ULONG64) reinterpret_cast< read_invoke* > (request->data)->buffer + this_offset), final_size, &bytes_trough ) ) )
		//	{
		//		imports::obf_dereference_object( process ); 

		//		return qtx::status::failed_sanity_check;
		//	}

		//	return qtx::status::failed_sanity_check;
		//}

		printf( "mmcopyvirt: cp, addr %llx, pid %i, %llx, size %i\n", data.address, data.pid, reinterpret_cast<write_invoke*>(request->data)->buffer, data.size );
		size_t bytes = 0;
		if (imports::mm_copy_virtual_memory( process, (void*)data.address, imports::io_get_current_process( ), (void*)reinterpret_cast<read_invoke*> (request->data)->buffer, data.size, UserMode, &bytes ) != STATUS_SUCCESS || bytes != data.size)
		{
			printf( "mmcopyvirtmem failed.\n" );
			imports::obf_dereference_object( process );
			return qtx::status::failed_sanity_check;
		}

		imports::obf_dereference_object( process );

        return qtx::status::successful_operation;
    }
}