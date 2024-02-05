#pragma once
#define BATTLEYE_SUPPORT
#ifdef BATTLEYE_SUPPORT
#define APC_TOGGLE(bOnOff) if (thread->ApcQueueable == bOnOff) thread->ApcQueueable = !bOnOff;
#else
#define APC_TOGGLE(bOnOff) 
#endif

namespace qtx
{
	//real type: intptr_t (__fastcall *)(intptr_t, uintptr_t, intptr_t, intptr_t, intptr_t)

	// a1, a2 are junk. a3, a4, a5 are useable.
	intptr_t __fastcall f_hook( intptr_t a1, uintptr_t a2, uintptr_t a3, uintptr_t a4, uintptr_t a5)
	{
		PKTHREAD_META thread = ( ( PKTHREAD_META ) ( (uintptr_t ) imports::ke_get_current_thread( )));

		APC_TOGGLE( TRUE );

		invoke_data data{};
		data.unique = ~invoke_unique; //it will never be invoke_unique.
		// complicated, yes. but it will do everything.
		// ensures 'invoke_data' is ***memory copied*** from a3
		if (!imports::mm_is_address_valid( (void*)a3 )
			|| imports::ex_get_previous_mode( ) != UserMode
			|| !(modules::safe_copy( &data, (void*)a3, sizeof( invoke_data ) && data.unique != invoke_unique )))
		{
			APC_TOGGLE( FALSE );
			return qtx::o_hook( a1, a2, a3, a4, a5 );
		}

		invoke_data* qtx = (invoke_data*)a3;
		switch (qtx->code)
		{
			case invoke_guarded_region:
			{
				if ( request::find_guarded_region( qtx ) != qtx::status::successful_operation )
				{
					APC_TOGGLE( FALSE );
					return 0;
				}
				break;
			}
			
	 		case invoke_base:
			{
				if (request::get_module_base( qtx ) != qtx::status::successful_operation)
				{
					APC_TOGGLE( FALSE );
					return 0;
				}
				break;
			}

			case invoke_read:
			{
				if (request::read_memory( qtx ) != qtx::status::successful_operation)
				{
					APC_TOGGLE( FALSE );
					return 0;
				}
				break;
			}

			case invoke_remove_node:
			{
				if (request::unlink_nodes( qtx ) != qtx::status::successful_operation)
				{
					APC_TOGGLE( FALSE );
					return 0;
				}
				break;
			}

			case invoke_free:
			{
				if (request::free( qtx ) != qtx::status::successful_operation)
				{
					APC_TOGGLE( FALSE );
					return 0;
				}
				break;
			}
			
			case invoke_write:
			{
				if (request::write_memory( qtx ) != qtx::status::successful_operation)
				{
					APC_TOGGLE( FALSE );
					return 0;
				}
				break;
			}

			case invoke_pattern:
			{
				if (request::scan_signature( qtx ) != qtx::status::successful_operation)
				{
					APC_TOGGLE( FALSE );
					return 0;
				}
				break;
			}

			case invoke_allocate:
			{
				if (request::allocate_memory( qtx ) != qtx::status::successful_operation)
				{
					APC_TOGGLE( FALSE );
					return 0;
				}
				break;
			}

			case invoke_swap:
			{
				if ( request::swap( qtx ) != qtx::status::successful_operation )
				{
					APC_TOGGLE( FALSE );
					return 0;
				}
				break;
			}

			case invoke_query_memory:
			{
				if (request::query_memory( qtx ) != qtx::status::successful_operation)
				{
					APC_TOGGLE( FALSE );
					return 0;
				}
				break;
			}

			case invoke_unload:
			{
				dereference( ( uintptr_t ) qtx::o_hook );
				//dereference( ( uintptr_t ) qtx::f_hook );

				reinterpret_cast< punload_invoke >(qtx->data)->unloaded = true;

				APC_TOGGLE( FALSE );

				return 0;
			}
		}

		APC_TOGGLE( FALSE );
		return invoke_success;
	}
}
