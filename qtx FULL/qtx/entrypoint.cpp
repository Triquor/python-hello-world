/*
							  ,----,
							,/   .`|
			,----..       ,`   .'  :,--,     ,--,
		   /   /   \    ;    ;     /|'. \   / .`|
		  /   .     : .'___,/    ,' ; \ `\ /' / ;
		 .   /   ;.  \|    :     |  `. \  /  / .'
		.   ;   /  ` ;;    |.';  ;   \  \/  / ./
		;   |  ; \ ; |`----'  |  |    \  \.'  /
		|   :  | ; | '    '   :  ;     \  ;  ;
		.   |  ' ' ' :    |   |  '    / \  \  \
		'   ;  \; /  |    '   :  |   ;  /\  \  \
		 \   \  ',  . \   ;   |.'  ./__;  \  ;  \
		  ;   :      ; |  '---'    |   : / \  \  ;
		   \   \ .'`--"            ;   |/   \  ' |
			`---`                  `---'     `--`

		-- qtx_kernel_device project by interpreter


*/

#include <ntifs.h>
#include <windef.h>
#include <cstdint>
#include <intrin.h>
#include <ntimage.h>

#include <C:\QTX FULL\qtx\kernel\xor.h>
#include <C:\QTX FULL\qtx\kernel\structures.hpp>

#include <C:\QTX FULL\qtx\impl\communication/interface.h>

#include <C:\QTX FULL\qtx\impl\imports.h>
#include <C:\QTX FULL\qtx\impl\scanner.h>
#include <C:\QTX FULL\qtx\impl\modules.h>

#include <C:\QTX FULL\qtx\requests\get_module_base.cpp>
#include <C:\QTX FULL\qtx\requests\physical_operations.cpp>
#include <C:\QTX FULL\qtx\requests\read_physical_memory.cpp>
#include <C:\QTX FULL\qtx\requests\write_physical_memory.cpp>
#include <C:\QTX FULL\qtx\requests\signature_scanner.cpp>
#include <C:\QTX FULL\qtx\requests\virtual_address_memory.cpp>
#include <C:\QTX FULL\qtx\requests\query_memory.cpp>

#include <C:\QTX FULL\qtx\impl\invoked.h>

#define rva(addr, size) ((uintptr_t)((uintptr_t)(addr) + *(PINT)((uintptr_t)(addr) + ((size) - sizeof(INT))) + (size)))


// LISTEN, DON'T FREAK OUT WEIRDO. I know what I'm doing, you have the backup anyway. Btw you were sig scanning win32k vs win32kfull (what you needed)
intptr_t null_sub( )
{
	return 0;
}

_declspec(noinline) auto hook( ) -> qtx::status
{
	std::uintptr_t service = modules::get_kernel_module( _( "win32kfull.sys" ) );
	if ( !service ) return qtx_failed_check;

	std::uintptr_t mpFnidPfn = scanner::find_pattern( service, _( "\x48\x8D\x05\x00\x00\x00\x00\x41\x83\xC2\x06" ), _( "xxx????xxxx" ) );
	if ( !imports::mm_is_address_valid( ( void* ) mpFnidPfn ) ) return qtx_failed_check;

	/*
		as i've mentioned a numerous amount of times, the shorter the signature the more supportive the data ptr is.
		this one is avaliable on almost all machines, i've done countless hours of reversing and debugging to ensure
		that this data ptr is stable and safe for use, and it's not the best but it's really efficent and works great.
	*/

	uintptr_t* pFnPtrTable = ( uintptr_t* ) rva( mpFnidPfn, 7 );
	//algo: [((uint8_t)a6 + 6) & 0x1F]
	uint8_t index = 122;

	index = (( uint8_t ) index + 6) & 0x1F;
	if ( imports::mm_is_address_valid( ( void* ) pFnPtrTable [ index ] ) )
	{
		qtx::o_hook = ( decltype(qtx::o_hook) ) pFnPtrTable [ index ];
		pFnPtrTable [ index ] = ( uintptr_t ) qtx::f_hook;
		printf( "hook successful, return needed.\n" );
	}
	else
	{
		qtx::o_hook = ( decltype(qtx::o_hook) ) null_sub;
		pFnPtrTable [ index ] = ( uintptr_t ) qtx::f_hook;
		printf( "hook successful, no return needed.\n" );
	}

	return qtx_successful_operation;
}

__declspec(noinline) auto null_pfn( PMDL mdl ) -> qtx::status
{
	PPFN_NUMBER mdl_pages = MmGetMdlPfnArray( mdl );
	if ( !mdl_pages )
		return qtx_failed_check;

	ULONG mdl_page_count = ADDRESS_AND_SIZE_TO_SPAN_PAGES( MmGetMdlVirtualAddress( mdl ), MmGetMdlByteCount( mdl ) );

	ULONG null_pfn = 0x0;

	MM_COPY_ADDRESS source_address = { 0 };
	source_address.VirtualAddress = &null_pfn;

	for ( ULONG i = 0; i < mdl_page_count; i++ )
	{
		size_t bytes = 0;
		imports::mm_copy_memory( &mdl_pages [ i ], source_address, sizeof( ULONG ), MM_COPY_MEMORY_VIRTUAL, &bytes );
	}

	return qtx_successful_operation;
}

_declspec(noinline) auto vad( ) -> qtx::status
{
	// get core kernel
	const uintptr_t ntoskrnl = modules::get_kernel_module( _( "ntoskrnl.exe" ) );
	if ( !ntoskrnl ) return qtx_failed_check;

	//mi_allocate_vad
	uintptr_t faddr = scanner::find_pattern( ntoskrnl, _( "\x44\x8D\x42\x02\xE8\x00\x00\x00\x00\x48\x89\x43\x08" ), _( "xxxxx????xxxx" ) );
	if ( !faddr ) return qtx_failed_check;

	faddr += 4;
	faddr = rva( faddr, 5 );

	printf( "mi_allocate_vad: 0x%llx\n", faddr );

	imports::mi_allocate_vad = ( decltype(imports::mi_allocate_vad) ) faddr;

	//mi_insert_vad_charges
	faddr = scanner::find_pattern( ntoskrnl, _( "\xE8\x00\x00\x00\x00\x8B\xF0\x85\xC0\x0F\x88\x00\x00\x00\x00\x48\x8B\xD3" ), _( "x????xxxxxx????xxx" ) );
	if ( !ntoskrnl ) return qtx_failed_check;

	faddr = rva( faddr, 5 );
	printf( "mi_insert_vad_charges: 0x%llx\n", faddr );

	imports::mi_insert_vad_charges = ( decltype(imports::mi_insert_vad_charges) ) faddr;

	//mi_insert_vad
	faddr = scanner::find_pattern( ntoskrnl, _( "\xE8\x00\x00\x00\x00\x8B\x5B\x30" ), _( "x????xxx" ) );
	if ( !ntoskrnl ) return qtx_failed_check;

	faddr = rva( faddr, 5 );
	printf( "mi_insert_vad: 0x%llx\n", faddr );

	imports::mi_insert_vad = ( decltype(imports::mi_insert_vad) ) faddr;

	return qtx_successful_operation;
}

_declspec(noinline) auto nmi_callbacks( ) -> qtx::status
{
	// get core kernel
	const uintptr_t ntoskrnl = modules::get_kernel_module( _( "ntoskrnl.exe" ) );
	if ( !ntoskrnl )
	{
		printf( "ntoskrnl: 0x%llx\n", ntoskrnl );
		return qtx_failed_check;
	}
	//pattern_idt 
	uintptr_t faddr = scanner::find_pattern( ntoskrnl, _( "\xE8\x00\x00\x00\x00\x83\xCB\xFF\x48\x8B\xD6" ), _( "x????xxxxxx" ) );
	if ( !faddr )
	{
		printf( "faddr: 0x%llx\n", faddr );
		return qtx_failed_check;
	}

	if ( faddr )
	{
		faddr = ResolveRelativeAddress( faddr, 1, 5 );
		faddr += 0x1a;
		faddr = ResolveRelativeAddress( faddr, 3, 7 );

		*( QWORD* ) (faddr + 0x38) = *( QWORD* ) (faddr + 0x1A0);
		*( QWORD* ) (faddr + 0x40) = *( QWORD* ) (faddr + 0x1A8);
	}

	return qtx_successful_operation;
}

_declspec(noinline) NTSTATUS DriverEntry( const uintptr_t mdl, imports::m_imported imports )
{
	imports::imported = imports;

	if ( null_pfn( reinterpret_cast< PMDL > (mdl) ) != qtx_successful_operation )
		return qtx::status::failed_intialization;

	if ( vad( ) != qtx_successful_operation )
		return qtx::status::failed_intialization;

	if ( nmi_callbacks( ) != qtx_successful_operation )
		return qtx::status::failed_intialization;

	if ( hook( ) != qtx_successful_operation )
		return qtx::status::failed_intialization;

	printf( "qtx successfully loaded\n" );

	return qtx::status::successful_operation;
}
