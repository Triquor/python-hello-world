#include <ntdef.h>
#include <C:\qtx FULL\qtx\impl\imports.h>
namespace crt
{
	template <typename t>
	__forceinline int strlen(t str) {
		if (!str)
		{
			return 0;
		}

		t buffer = str;

		while (*buffer)
		{
			*buffer++;
		}

		return (int)(buffer - str);
	}

	bool strcmp(const char* src, const char* dst)
	{
		if (!src || !dst)
		{
			return true;
		}

		const auto src_sz = crt::strlen(src);
		const auto dst_sz = crt::strlen(dst);

		if (src_sz != dst_sz)
		{
			return true;
		}

		for (int i = 0; i < src_sz; i++)
		{
			if (src[i] != dst[i])
			{
				return true;
			}
		}

		return false;
	}

	PVOID get_kernel_proc_address( const char* system_routine_name )
	{
		UNICODE_STRING name;
		ANSI_STRING ansi_str;

		imports::rtl_init_ansi_string( & ansi_str, system_routine_name );
		imports::rtl_ansi_string_to_unicode_string( & name, & ansi_str, TRUE );

		return imports::mm_get_system_routine_address( & name );
	}


	int a_toi( const char* value )
	{
		using _atoi = int(NTAPI*)(const char* str);
		static auto importer_atoi = static_cast<_atoi>(get_kernel_proc_address( ( "atoi" ) ));

		return importer_atoi( value );
	}
}

#define o(value) [&](){ return crt::a_toi(_(#value)); \
	}()

