namespace request
{
	_declspec(noinline) qtx::status scan_signature( invoke_data* request )
	{
		pattern_invoke data = { 0 };
		uintptr_t o_process = 0;

		if (!modules::safe_copy( &data, request->data, sizeof( pattern_invoke ) ))
			return qtx::status::failed_sanity_check;

		if (!data.pid || !data.base)
			return qtx::status::failed_sanity_check;

		PEPROCESS process = 0;
		if (imports::ps_lookup_process_by_process_id( (HANDLE)data.pid, &process ) != qtx::status::successful_operation)
			return qtx::status::failed_sanity_check;

		o_process = modules::attach_process( (uintptr_t)process );
		if (!o_process) return qtx::status::failed_sanity_check;

		const auto address = scanner::find_pattern( data.base, data.signature );

		if (!address)
		{
			modules::attach_process( o_process );
			imports::obf_dereference_object( process );

			return qtx::status::failed_sanity_check;
		}

		modules::attach_process( o_process );
		imports::obf_dereference_object( process );

		reinterpret_cast<pattern_invoke*> (request->data)->address = address;
		return qtx::status::successful_operation;
	}
}