#pragma once

#define QTXAPI inline

namespace qtx
{
	intptr_t( __fastcall* o_hook )(intptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t);
	//inline __int64( __fastcall* o_hook )(void*) = nullptr;

	enum status : int32_t
	{
		failed_signature_scan = 0,
		failed_intialization = 0,
		failed_sanity_check = 0,
		successful_operation = STATUS_SUCCESS,
		failed_get_module_base = 0,
	};
}

#define qtx_successful_operation qtx::status::successful_operation
#define qtx_failed_check qtx::status::failed_sanity_check

namespace imports
{
	struct m_imported
	{
		uintptr_t ex_allocate_pool;
		uintptr_t zw_query_system_information;
		uintptr_t ex_free_pool_with_tag;
		uintptr_t ex_get_previous_mode;
		uintptr_t ke_get_current_thread;
		uintptr_t rtl_init_ansi_string;
		uintptr_t rtl_ansi_string_to_unicode_string;
		uintptr_t mm_get_system_routine_address;
		uintptr_t mm_copy_virtual_memory;
		uintptr_t io_get_current_process;
		uintptr_t ps_lookup_process_by_process_id;
		uintptr_t ps_get_process_peb;
		uintptr_t rtl_compare_unicode_string;
		uintptr_t rtl_free_unicode_string;
		uintptr_t rtl_get_version;
		uintptr_t mm_map_io_space_ex;
		uintptr_t mm_unmap_io_space;
		uintptr_t obf_dereference_object;
		uintptr_t mm_copy_memory;
		uintptr_t ps_get_process_section_base_address;
		uintptr_t mm_is_address_valid;
		uintptr_t zw_query_virtual_memory;
		uintptr_t rtl_avl_remove_node;
		uintptr_t zw_free_virtual_memory;
	};

	m_imported imported;

	QTXAPI NTSTATUS zw_query_system_information(SYSTEM_INFORMATION_CLASS SystemInformationClass, PVOID SystemInformation, ULONG SystemInformationLength, PULONG ReturnLength)
	{
		return reinterpret_cast<NTSTATUS(*)(SYSTEM_INFORMATION_CLASS, PVOID, ULONG, PULONG)> (imported.zw_query_system_information)(SystemInformationClass, SystemInformation, SystemInformationLength, ReturnLength);
	}

	QTXAPI PVOID ex_allocate_pool(POOL_TYPE PoolType, SIZE_T NumberOfBytes)
	{
		return reinterpret_cast<PVOID(*)(POOL_TYPE, SIZE_T)>(imported.ex_allocate_pool)(PoolType, NumberOfBytes);
	}
	
	QTXAPI void ex_free_pool_with_tag(PVOID P, ULONG TAG)
	{
		return reinterpret_cast<void(*)(PVOID, ULONG)> (imported.ex_free_pool_with_tag)(P, TAG);
	}

	QTXAPI KPROCESSOR_MODE ex_get_previous_mode( )
	{
		return reinterpret_cast<KPROCESSOR_MODE( * )()>(imported.ex_get_previous_mode)();
	}

	QTXAPI PKTHREAD ke_get_current_thread( )
	{
		return reinterpret_cast<PKTHREAD( * )()>(imported.ke_get_current_thread)();
	}

	QTXAPI VOID rtl_init_ansi_string( PANSI_STRING DestinationString, PCSZ SourceString )
	{
		return reinterpret_cast<VOID(*)(PANSI_STRING, PCSZ)> (imported.rtl_init_ansi_string)(DestinationString, SourceString);
	}

	QTXAPI NTSTATUS rtl_ansi_string_to_unicode_string( PUNICODE_STRING DestinationString, PCANSI_STRING SourceString, BOOLEAN AllocateDestinationString )
	{
		return reinterpret_cast<NTSTATUS( * )(PUNICODE_STRING, PCANSI_STRING, BOOLEAN)> (imported.rtl_ansi_string_to_unicode_string)(DestinationString, SourceString, AllocateDestinationString);
	}

	QTXAPI PVOID mm_get_system_routine_address( PUNICODE_STRING SystemRoutineName )
	{
		return reinterpret_cast<PVOID( * )(PUNICODE_STRING)> (imported.mm_get_system_routine_address)(SystemRoutineName);
	}

	QTXAPI NTSTATUS mm_copy_virtual_memory( PEPROCESS SourceProcess, PVOID SourceAddress, PEPROCESS TargetProcess, PVOID TargetAddress, SIZE_T BufferSize, KPROCESSOR_MODE PreviousMode, PSIZE_T ReturnSize )
	{
		return reinterpret_cast<NTSTATUS( * )(PEPROCESS, PVOID, PEPROCESS, PVOID, SIZE_T, KPROCESSOR_MODE, PSIZE_T)> (imported.mm_copy_virtual_memory)(SourceProcess, SourceAddress, TargetProcess, TargetAddress, BufferSize, PreviousMode, ReturnSize);
	}

	QTXAPI PEPROCESS io_get_current_process( )
	{
		return reinterpret_cast<PEPROCESS( * )()> (imported.io_get_current_process)();
	}

	QTXAPI NTSTATUS ps_lookup_process_by_process_id( HANDLE ProcessId, PEPROCESS* Process )
	{
		return reinterpret_cast<NTSTATUS( * )(HANDLE, PEPROCESS*)> (imported.ps_lookup_process_by_process_id)(ProcessId, Process);
	}

	QTXAPI PPEB ps_get_process_peb( PEPROCESS Process )
	{
		return reinterpret_cast<PPEB( * )(PEPROCESS)> (imported.ps_get_process_peb)(Process);
	}

	QTXAPI LONG rtl_compare_unicode_string( PCUNICODE_STRING String1, PCUNICODE_STRING String2, BOOLEAN CaseInSensitive )
	{
		return reinterpret_cast<LONG( * )(PCUNICODE_STRING, PCUNICODE_STRING, BOOLEAN)> (imported.rtl_compare_unicode_string)(String1, String2, CaseInSensitive);
	}

	QTXAPI VOID rtl_free_unicode_string( PUNICODE_STRING UnicodeString )
	{
		return reinterpret_cast<VOID( * )(PUNICODE_STRING)> (imported.rtl_free_unicode_string)(UnicodeString);
	}

	QTXAPI NTSTATUS rtl_get_version( PRTL_OSVERSIONINFOW lpVersionInformation )
	{
		return reinterpret_cast<NTSTATUS( * )(PRTL_OSVERSIONINFOW)> (imported.rtl_get_version)(lpVersionInformation);
	}

	QTXAPI PVOID mm_map_io_space_ex( PHYSICAL_ADDRESS PhysicalAddress, SIZE_T NumberOfBytes, ULONG Protect )
	{
		return reinterpret_cast<PVOID( * )(PHYSICAL_ADDRESS, SIZE_T, ULONG)>(imported.mm_map_io_space_ex)(PhysicalAddress, NumberOfBytes, Protect);
	}

	QTXAPI VOID mm_unmap_io_space( PVOID BaseAddress, SIZE_T NumberOfBytes )
	{
		return reinterpret_cast<VOID( * )(PVOID, SIZE_T)>(imported.mm_unmap_io_space)(BaseAddress, NumberOfBytes);
	}

	QTXAPI LONG_PTR obf_dereference_object( PVOID Object )
	{
		return reinterpret_cast<LONG_PTR( * )(PVOID)>(imported.obf_dereference_object)(Object);
	}

	QTXAPI NTSTATUS mm_copy_memory( PVOID TargetAddress, MM_COPY_ADDRESS SourceAddress, SIZE_T NumberOfBytes, ULONG Flags, PSIZE_T NumberOfBytesTransferred )
	{
		return reinterpret_cast<NTSTATUS( * )(PVOID, MM_COPY_ADDRESS, SIZE_T, ULONG, PSIZE_T)>(imported.mm_copy_memory)(TargetAddress, SourceAddress, NumberOfBytes, Flags, NumberOfBytesTransferred);
	}

	QTXAPI PVOID ps_get_process_section_base_address( PEPROCESS Process )
	{
		return reinterpret_cast<PVOID( * )(PEPROCESS)>(imported.ps_get_process_section_base_address)(Process);
	}

	QTXAPI BOOLEAN mm_is_address_valid( PVOID VirtualAddress )
	{
		return reinterpret_cast<BOOLEAN( * )(PVOID)>(imported.mm_is_address_valid)(VirtualAddress);
	}

	QTXAPI NTSTATUS zw_query_virtual_memory( HANDLE ProcessHandle, PVOID BaseAddress, MEMORY_INFORMATION_CLASS MemoryInformationClass, PVOID MemoryInformation, SIZE_T MemoryInformationLength, PSIZE_T ReturnLength )
	{
		return reinterpret_cast<NTSTATUS( * )(HANDLE, PVOID, MEMORY_INFORMATION_CLASS, PVOID, SIZE_T, PSIZE_T)>(imported.zw_query_virtual_memory)(ProcessHandle, BaseAddress, MemoryInformationClass, MemoryInformation, MemoryInformationLength, ReturnLength);
	}

	QTXAPI PVOID rtl_avl_remove_node( PRTL_AVL_TREE pTree, PMMADDRESS_NODE pNode )
	{
		return reinterpret_cast<PVOID( * )(PRTL_AVL_TREE, PMMADDRESS_NODE)>(imported.rtl_avl_remove_node)(pTree, pNode);
	}

	QTXAPI NTSTATUS zw_free_virtual_memory( HANDLE ProcessHandle, PVOID* BaseAddress, PSIZE_T RegionSize, ULONG FreeType )
	{
		return reinterpret_cast<NTSTATUS( * )(HANDLE, PVOID*, PSIZE_T, ULONG)>(imported.zw_free_virtual_memory)(ProcessHandle, BaseAddress, RegionSize, FreeType);
	}

	// External Imports
	PMMVAD_SHORT(*mi_allocate_vad)( UINT_PTR start, UINT_PTR end, LOGICAL deletable );

	NTSTATUS(*mi_insert_vad_charges)( PMMVAD_SHORT vad, PEPROCESS process );

	VOID(*mi_insert_vad)( PMMVAD_SHORT vad, PEPROCESS process );
}