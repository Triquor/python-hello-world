#pragma once
#include "C:\QTX FULL\qtx\impl\imports.h"

namespace request
{
	_declspec(noinline) qtx::status get_module_base( invoke_data* request )
	{
		base_invoke data = { 0 };
		uintptr_t out = 0;
		uintptr_t o_process = 0;

		if (!modules::safe_copy( &data, request->data, sizeof( base_invoke ) ))
			return qtx::status::failed_sanity_check;

		if (!data.pid )
			return qtx::status::failed_sanity_check;

		ANSI_STRING ansi_name;
		imports::rtl_init_ansi_string( &ansi_name, data.name );

		UNICODE_STRING compare_name;
		imports::rtl_ansi_string_to_unicode_string( &compare_name, &ansi_name, TRUE );

		PEPROCESS process = 0;
		if (imports::ps_lookup_process_by_process_id( (HANDLE)data.pid, &process) != qtx::status::successful_operation )
			return qtx::status::failed_sanity_check;

		if (!data.name)
		{
			uintptr_t base = (uintptr_t)imports::ps_get_process_section_base_address( process );

			imports::rtl_free_unicode_string( &compare_name );
			imports::obf_dereference_object( process );

			reinterpret_cast<base_invoke*> (request->data)->handle = out;
			return qtx::status::successful_operation;
		}

		o_process = modules::attach_process( (uintptr_t)process );
		if (!o_process)
			return qtx::status::failed_sanity_check;

		PPEB pPeb = imports::ps_get_process_peb( process );

		if (pPeb)
		{
			PPEB_LDR_DATA pLdr = (PPEB_LDR_DATA)pPeb->Ldr;

			if (pLdr)
			{
				for (PLIST_ENTRY listEntry = (PLIST_ENTRY)pLdr->ModuleListLoadOrder.Flink;
					listEntry != &pLdr->ModuleListLoadOrder;
					listEntry = (PLIST_ENTRY)listEntry->Flink) {


					PLDR_DATA_TABLE_ENTRY pEntry = CONTAINING_RECORD( listEntry, LDR_DATA_TABLE_ENTRY, InLoadOrderModuleList );
					printf( "modules: %wZ\n", pEntry->BaseDllName );

					if (imports::rtl_compare_unicode_string( &pEntry->BaseDllName, &compare_name, TRUE ) == 0)
					{
						out = (uint64_t)pEntry->DllBase;
						break;
					}
				}
			}
		}

		modules::attach_process( o_process );

		imports::rtl_free_unicode_string( &compare_name );
		imports::obf_dereference_object( process );

		reinterpret_cast<base_invoke*> (request->data)->handle = out;

		return qtx::status::successful_operation;
	}
}