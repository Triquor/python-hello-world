#pragma once

namespace physical_memory_operations
{
#define PAGE_OFFSET_SIZE 12
	static const UINT64 PMASK = (~0xfull << 8) & 0xfffffffffull;

#define win_1803 17134
#define win_1809 17763
#define win_1903 18362
#define win_1909 18363
#define win_2004 19041
#define win_20H2 19569
#define win_21H1 20180

	INT32 get_winver( ) {
		RTL_OSVERSIONINFOW ver = { 0 };
		imports::rtl_get_version( &ver );
		switch ( ver.dwBuildNumber )
		{
		case win_1803:
			return 0x0278;
			break;
		case win_1809:
			return 0x0278;
			break;
		case win_1903:
			return 0x0280;
			break;
		case win_1909:
			return 0x0280;
			break;
		case win_2004:
			return 0x0388;
			break;
		case win_20H2:
			return 0x0388;
			break;
		case win_21H1:
			return 0x0388;
			break;
		default:
			return 0x0388;
		}
	}

	UINT64 get_process_cr3( const PEPROCESS pProcess ) {
		PUCHAR process = ( PUCHAR ) pProcess;
		ULONG_PTR process_dirbase = *( PULONG_PTR ) (process + 0x28);
		if ( process_dirbase == 0 )
		{
			INT32 UserDirOffset = get_winver( );
			ULONG_PTR process_userdirbase = *( PULONG_PTR ) (process + UserDirOffset);
			return process_userdirbase;
		}
		return process_dirbase;
	}

	NTSTATUS read( PVOID target_address, PVOID buffer, SIZE_T size, SIZE_T* bytes_read ) {
		MM_COPY_ADDRESS to_read = { 0 };
		to_read.PhysicalAddress.QuadPart = ( LONGLONG ) target_address;
		return imports::mm_copy_memory( buffer, to_read, size, MM_COPY_MEMORY_PHYSICAL, bytes_read );
	}

	UINT64 translate_linear( UINT64 directoryTableBase, UINT64 virtualAddress ) {
		directoryTableBase &= ~0xf;

		UINT64 pageOffset = virtualAddress & ~(~0ul << PAGE_OFFSET_SIZE);
		UINT64 pte = ((virtualAddress >> 12) & (0x1ffll));
		UINT64 pt = ((virtualAddress >> 21) & (0x1ffll));
		UINT64 pd = ((virtualAddress >> 30) & (0x1ffll));
		UINT64 pdp = ((virtualAddress >> 39) & (0x1ffll));

		SIZE_T readsize = 0;
		UINT64 pdpe = 0;
		read( PVOID( directoryTableBase + 8 * pdp ), &pdpe, sizeof( pdpe ), &readsize );
		if ( ~pdpe & 1 )
			return 0;

		UINT64 pde = 0;
		read( PVOID( (pdpe & PMASK) + 8 * pd ), &pde, sizeof( pde ), &readsize );
		if ( ~pde & 1 )
			return 0;

		/* 1GB large page, use pde's 12-34 bits */
		if ( pde & 0x80 )
			return (pde & (~0ull << 42 >> 12)) + (virtualAddress & ~(~0ull << 30));

		UINT64 pteAddr = 0;
		read( PVOID( (pde & PMASK) + 8 * pt ), &pteAddr, sizeof( pteAddr ), &readsize );
		if ( ~pteAddr & 1 )
			return 0;

		/* 2MB large page */
		if ( pteAddr & 0x80 )
			return (pteAddr & PMASK) + (virtualAddress & ~(~0ull << 21));

		virtualAddress = 0;
		read( PVOID( (pteAddr & PMASK) + 8 * pte ), &virtualAddress, sizeof( virtualAddress ), &readsize );
		virtualAddress &= PMASK;

		if ( !virtualAddress )
			return 0;

		return virtualAddress + pageOffset;
	}

	ULONG64 find_min( INT32 g, SIZE_T f ) {
		INT32 h = ( INT32 ) f;
		ULONG64 result = 0;

		result = (((g) < (h)) ? (g) : (h));

		return result;
	}

	NTSTATUS write( PVOID target_address, PVOID buffer, SIZE_T size, SIZE_T* bytes_read )
	{
		if ( !target_address )
			return STATUS_UNSUCCESSFUL;

		PHYSICAL_ADDRESS AddrToWrite = { 0 };
		AddrToWrite.QuadPart = LONGLONG( target_address );

		PVOID pmapped_mem = imports::mm_map_io_space_ex( AddrToWrite, size, PAGE_READWRITE );

		if ( !pmapped_mem )
			return STATUS_UNSUCCESSFUL;

		memcpy( pmapped_mem, buffer, size );

		*bytes_read = size;
		imports::mm_unmap_io_space( pmapped_mem, size );
		return STATUS_SUCCESS;
	}
}
