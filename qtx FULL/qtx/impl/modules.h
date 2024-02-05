namespace modules
{
    _declspec(noinline) void* get_system_information(SYSTEM_INFORMATION_CLASS information_class)
    {
        unsigned long size = o(32);
        char buffer[32];

        imports::zw_query_system_information(information_class, buffer, size, &size);

        void* info = imports::ex_allocate_pool(NonPagedPool, size);

        if (!info)
            return nullptr;

        if (!NT_SUCCESS(imports::zw_query_system_information(information_class, info, size, &size)))
        {
            imports::ex_free_pool_with_tag(info, o(0));
            return nullptr;
        }

        return info;

        return (void*)(NULL);
    }

    _declspec(noinline) uintptr_t get_kernel_module(const char* name)
    {
        const auto to_lower = [](char* string) -> const char*
        {
            for (char* pointer = string; *pointer != '\0'; ++pointer)
            {
                *pointer = (char)(short)tolower(*pointer);
            }

            return string;
        };

        const PRTL_PROCESS_MODULES info = (PRTL_PROCESS_MODULES)get_system_information(SystemModuleInformation);

        if (!info)
            return NULL;

        for (size_t i = o(0); i < info->NumberOfModules; ++i)
        {
            const auto& mod = info->Modules[i];

            if (crt::strcmp(to_lower_c((char*)mod.FullPathName + mod.OffsetToFileName), name) == o(0))
            {
                const void* address = mod.ImageBase;
                imports::ex_free_pool_with_tag(info, o(0));
                return (uintptr_t)address;
            }
        }

        imports::ex_free_pool_with_tag(info, o(0));
        return NULL;
    }

    _declspec(noinline) bool safe_copy( void* dst, void* src, size_t size )
    {
        SIZE_T bytes = 0;

        if (imports::mm_copy_virtual_memory( imports::io_get_current_process( ), src, imports::io_get_current_process( ), dst, size, KernelMode, &bytes ) == STATUS_SUCCESS && bytes == size)
        {
            return true;
        }

        return false;
    }


    _declspec(noinline) TABLE_SEARCH_RESULT mi_find_node_or_parent( IN PMM_AVL_TABLE table, ULONG_PTR starting_vpn, PMMADDRESS_NODE* node_or_parent )
    {
        PMMADDRESS_NODE child;
        PMMADDRESS_NODE node_to_examine;
        PMMVAD_SHORT    vpn_compare;
        ULONG_PTR       start_vpn;
        ULONG_PTR       end_vpn;

        if (table->NumberGenericTableElements == 0)
            return TableEmptyTree;

        node_to_examine = (PMMADDRESS_NODE)(table->BalancedRoot);

        for (;;)
        {
            vpn_compare = (PMMVAD_SHORT)node_to_examine;
            start_vpn = vpn_compare->StartingVpn;
            end_vpn = vpn_compare->EndingVpn;

            if (starting_vpn < start_vpn)
            {
                child = node_to_examine->LeftChild;
                if (child != NULL)
                {
                    node_to_examine = child;
                }
                else
                {
                    *node_or_parent = node_to_examine;
                    return TableInsertAsLeft;
                }
            }
            else if (starting_vpn <= end_vpn)
            {
                *node_or_parent = node_to_examine;
                return TableFoundNode;
            }
            else
            {
                child = node_to_examine->RightChild;
                if (child != NULL)
                {
                    node_to_examine = child;
                }
                else
                {
                    *node_or_parent = node_to_examine;
                    return TableInsertAsRight;
                }
            }
        };
    }

    _declspec(noinline)  NTSTATUS find_vad( PEPROCESS process, ULONG_PTR address, PMMVAD_SHORT* result )
    {
        NTSTATUS status = STATUS_SUCCESS;
        ULONG_PTR vpn_start = address >> PAGE_SHIFT;

        ASSERT( process != NULL && result != NULL );
        if (process == NULL || result == NULL)
            return STATUS_INVALID_PARAMETER;

        PMM_AVL_TABLE table = (PMM_AVL_TABLE)((PUCHAR)process + 0x7D8);
        PMM_AVL_NODE node = (table->BalancedRoot);

        if (mi_find_node_or_parent( table, vpn_start, &node ) == TableFoundNode)
        {
            *result = (PMMVAD_SHORT)node;
        }
        else
        {
            status = STATUS_NOT_FOUND;
        }

        return status;
    }


    _declspec(noinline) uintptr_t attach_process( uintptr_t process )
    {
        auto current_thread = (uintptr_t)imports::ke_get_current_thread( );
        if (!current_thread)
            return 0;

        auto apc_state = *(uintptr_t*)(current_thread + 0x98);
        auto old_process = *(uintptr_t*)(apc_state + 0x20);
        *(uintptr_t*)(apc_state + 0x20) = process;

        auto dir_table_base = *(uintptr_t*)(process + 0x28);
        __writecr3( dir_table_base );

        return old_process;
    }

    _declspec(noinline) auto is_windows_11( )
    {
        ULONG build_number = *(ULONG*)(o( 2147353184 ));

        if (build_number >= o( 22000 ))
            return true;

        return false;
    }
}