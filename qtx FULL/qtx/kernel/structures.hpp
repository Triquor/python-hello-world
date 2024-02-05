#pragma once
#include <stdint.h>

///
/// Header options
///
///#define PRINT_DEBUG // Enable/disable(commented out) printf debugging into DebugView with this option.

typedef unsigned __int64 QWORD;

QWORD ResolveRelativeAddress(
	QWORD Instruction,
	DWORD OffsetOffset,
	DWORD InstructionSize
)
{

	QWORD Instr = ( QWORD ) Instruction;
	INT32 RipOffset = *( INT32* ) (Instr + OffsetOffset);
	QWORD ResolvedAddr = ( QWORD ) (Instr + InstructionSize + RipOffset);
	return ResolvedAddr;
}

typedef struct _SYSTEM_BIGPOOL_ENTRY
{
	union {
		PVOID VirtualAddress;
		ULONG_PTR NonPaged : 1;
	};
	ULONG_PTR SizeInBytes;
	union {
		UCHAR Tag [ 4 ];
		ULONG TagUlong;
	};
} SYSTEM_BIGPOOL_ENTRY, * PSYSTEM_BIGPOOL_ENTRY;

typedef struct _SYSTEM_BIGPOOL_INFORMATION {
	ULONG Count;
	SYSTEM_BIGPOOL_ENTRY AllocatedInfo [ ANYSIZE_ARRAY ];
} SYSTEM_BIGPOOL_INFORMATION, * PSYSTEM_BIGPOOL_INFORMATION;



///
/// Definitions
///

#ifdef PRINT_DEBUG
#define printf(text, ...) DbgPrintEx(DPFLTR_IHVBUS_ID, 0, _("[qtx-kernel-device]: " text), ##__VA_ARGS__)
#else
#define printf(text, ...) 
#endif

#define PFN_TO_PAGE(pfn) ( pfn << 12 )
#define dereference(ptr) (const uintptr_t)(ptr + *( int * )( ( BYTE * )ptr + 3 ) + 7)
#define in_range(x,a,b)    (x >= a && x <= b) 
#define get_bits( x )    (in_range((x&(~0x20)),'A','F') ? ((x&(~0x20)) - 'A' + 0xA) : (in_range(x,'0','9') ? x - '0' : 0))
#define get_byte( x )    (get_bits(x[0]) << 4 | get_bits(x[1]))
#define size_align(Size) ((Size + 0xFFF) & 0xFFFFFFFFFFFFF000)
#define to_lower_i(Char) ((Char >= 'A' && Char <= 'Z') ? (Char + 32) : Char)
#define to_lower_c(Char) ((Char >= (char*)'A' && Char <= (char*)'Z') ? (Char + 32) : Char)

#define rva(addr, size)       ((uintptr_t)((uintptr_t)(addr) + *(PINT)((uintptr_t)(addr) + ((size) - sizeof(INT))) + (size)))


//
// Protection Bits part of the internal memory manager Protection Mask, from:
// http://reactos.org/wiki/Techwiki:Memory_management_in_the_Windows_XP_kernel
// https://www.reactos.org/wiki/Techwiki:Memory_Protection_constants
// and public assertions.
//
#define MM_ZERO_ACCESS         0
#define MM_READONLY            1
#define MM_EXECUTE             2
#define MM_EXECUTE_READ        3
#define MM_READWRITE           4
#define MM_WRITECOPY           5
#define MM_EXECUTE_READWRITE   6
#define MM_EXECUTE_WRITECOPY   7
#define MM_PROTECT_ACCESS      7




///
/// Structures
///
typedef union _KWAIT_STATUS_REGISTER
{
	union
	{
		/* 0x0000 */ unsigned char Flags;
		struct /* bitfield */
		{
			/* 0x0000 */ unsigned char State : 3; /* bit position: 0 */
			/* 0x0000 */ unsigned char Affinity : 1; /* bit position: 3 */
			/* 0x0000 */ unsigned char Priority : 1; /* bit position: 4 */
			/* 0x0000 */ unsigned char Apc : 1; /* bit position: 5 */
			/* 0x0000 */ unsigned char UserApc : 1; /* bit position: 6 */
			/* 0x0000 */ unsigned char Alert : 1; /* bit position: 7 */
		}; /* bitfield */
	}; /* size: 0x0001 */
} KWAIT_STATUS_REGISTER, * PKWAIT_STATUS_REGISTER; /* size: 0x0001 */

typedef struct _KTHREAD_META
{
	/* 0x0000 */ struct _DISPATCHER_HEADER Header;
	/* 0x0018 */ void* SListFaultAddress;
	/* 0x0020 */ unsigned __int64 QuantumTarget;
	/* 0x0028 */ void* InitialStack;
	/* 0x0030 */ void* volatile StackLimit;
	/* 0x0038 */ void* StackBase;
	/* 0x0040 */ unsigned __int64 ThreadLock;
	/* 0x0048 */ volatile unsigned __int64 CycleTime;
	/* 0x0050 */ unsigned long CurrentRunTime;
	/* 0x0054 */ unsigned long ExpectedRunTime;
	/* 0x0058 */ void* KernelStack;
	/* 0x0060 */ struct _XSAVE_FORMAT* StateSaveArea;
	/* 0x0068 */ struct _KSCHEDULING_GROUP* volatile SchedulingGroup;
	/* 0x0070 */ union _KWAIT_STATUS_REGISTER WaitRegister;
	/* 0x0071 */ volatile unsigned char Running;
	/* 0x0072 */ unsigned char Alerted[2];
	union
	{
		struct /* bitfield */
		{
			/* 0x0074 */ unsigned long AutoBoostActive : 1; /* bit position: 0 */
			/* 0x0074 */ unsigned long ReadyTransition : 1; /* bit position: 1 */
			/* 0x0074 */ unsigned long WaitNext : 1; /* bit position: 2 */
			/* 0x0074 */ unsigned long SystemAffinityActive : 1; /* bit position: 3 */
			/* 0x0074 */ unsigned long Alertable : 1; /* bit position: 4 */
			/* 0x0074 */ unsigned long UserStackWalkActive : 1; /* bit position: 5 */
			/* 0x0074 */ unsigned long ApcInterruptRequest : 1; /* bit position: 6 */
			/* 0x0074 */ unsigned long QuantumEndMigrate : 1; /* bit position: 7 */
			/* 0x0074 */ unsigned long UmsDirectedSwitchEnable : 1; /* bit position: 8 */
			/* 0x0074 */ unsigned long TimerActive : 1; /* bit position: 9 */
			/* 0x0074 */ unsigned long SystemThread : 1; /* bit position: 10 */
			/* 0x0074 */ unsigned long ProcessDetachActive : 1; /* bit position: 11 */
			/* 0x0074 */ unsigned long CalloutActive : 1; /* bit position: 12 */
			/* 0x0074 */ unsigned long ScbReadyQueue : 1; /* bit position: 13 */
			/* 0x0074 */ unsigned long ApcQueueable : 1; /* bit position: 14 */
			/* 0x0074 */ unsigned long ReservedStackInUse : 1; /* bit position: 15 */
			/* 0x0074 */ unsigned long UmsPerformingSyscall : 1; /* bit position: 16 */
			/* 0x0074 */ unsigned long TimerSuspended : 1; /* bit position: 17 */
			/* 0x0074 */ unsigned long SuspendedWaitMode : 1; /* bit position: 18 */
			/* 0x0074 */ unsigned long SuspendSchedulerApcWait : 1; /* bit position: 19 */
			/* 0x0074 */ unsigned long CetUserShadowStack : 1; /* bit position: 20 */
			/* 0x0074 */ unsigned long BypassProcessFreeze : 1; /* bit position: 21 */
			/* 0x0074 */ unsigned long CetKernelShadowStack : 1; /* bit position: 22 */
			/* 0x0074 */ unsigned long Reserved : 9; /* bit position: 23 */
		}; /* bitfield */
		/* 0x0074 */ long MiscFlags;
	}; /* size: 0x0004 */
} KTHREAD_META, * PKTHREAD_META; /* size: 0x0430 */

typedef struct _RTL_PROCESS_MODULE_INFORMATION
{
	HANDLE Section;
	PVOID MappedBase;
	PVOID ImageBase;
	ULONG ImageSize;
	ULONG Flags;
	USHORT LoadOrderIndex;
	USHORT InitOrderIndex;
	USHORT LoadCount;
	USHORT OffsetToFileName;
	UCHAR  FullPathName[256];
} RTL_PROCESS_MODULE_INFORMATION, * PRTL_PROCESS_MODULE_INFORMATION;

typedef struct _RTL_PROCESS_MODULES
{
	ULONG NumberOfModules;
	RTL_PROCESS_MODULE_INFORMATION Modules[1];
} RTL_PROCESS_MODULES, * PRTL_PROCESS_MODULES;

typedef struct _LDR_DATA_TABLE_ENTRY {
	LIST_ENTRY InLoadOrderModuleList;
	LIST_ENTRY InMemoryOrderModuleList;
	LIST_ENTRY InInitializationOrderModuleList;
	PVOID DllBase;
	PVOID EntryPoint;
	ULONG SizeOfImage;
	UNICODE_STRING FullDllName;
	UNICODE_STRING BaseDllName;
	ULONG Flags;
	USHORT LoadCount;
	USHORT TlsIndex;
	LIST_ENTRY HashLinks;
	PVOID SectionPointer;
	ULONG CheckSum;
	ULONG TimeDateStamp;
} LDR_DATA_TABLE_ENTRY, * PLDR_DATA_TABLE_ENTRY;

typedef struct _RTL_CRITICAL_SECTION
{
	VOID* DebugInfo;
	LONG LockCount;
	LONG RecursionCount;
	PVOID OwningThread;
	PVOID LockSemaphore;
	ULONG SpinCount;
} RTL_CRITICAL_SECTION, * PRTL_CRITICAL_SECTION;

typedef struct _PEB_LDR_DATA {
	ULONG Length;
	BOOLEAN Initialized;
	PVOID SsHandle;
	LIST_ENTRY ModuleListLoadOrder;
	LIST_ENTRY ModuleListMemoryOrder;
	LIST_ENTRY ModuleListInitOrder;
} PEB_LDR_DATA, * PPEB_LDR_DATA;

typedef struct _PEB
{
	UCHAR InheritedAddressSpace;
	UCHAR ReadImageFileExecOptions;
	UCHAR BeingDebugged;
	UCHAR BitField;
	ULONG ImageUsesLargePages : 1;
	ULONG IsProtectedProcess : 1;
	ULONG IsLegacyProcess : 1;
	ULONG IsImageDynamicallyRelocated : 1;
	ULONG SpareBits : 4;
	PVOID Mutant;
	PVOID ImageBaseAddress;
	PPEB_LDR_DATA Ldr;
	VOID* ProcessParameters;
	PVOID SubSystemData;
	PVOID ProcessHeap;
	PRTL_CRITICAL_SECTION FastPebLock;
	PVOID AtlThunkSListPtr;
	PVOID IFEOKey;
	ULONG CrossProcessFlags;
	ULONG ProcessInJob : 1;
	ULONG ProcessInitializing : 1;
	ULONG ReservedBits0 : 30;
	union
	{
		PVOID KernelCallbackTable;
		PVOID UserSharedInfoPtr;
	};
	ULONG SystemReserved[1];
	ULONG SpareUlong;
	VOID* FreeList;
	ULONG TlsExpansionCounter;
	PVOID TlsBitmap;
	ULONG TlsBitmapBits[2];
	PVOID ReadOnlySharedMemoryBase;
	PVOID HotpatchInformation;
	VOID** ReadOnlyStaticServerData;
	PVOID AnsiCodePageData;
	PVOID OemCodePageData;
	PVOID UnicodeCaseTableData;
	ULONG NumberOfProcessors;
	ULONG NtGlobalFlag;
	LARGE_INTEGER CriticalSectionTimeout;
	ULONG HeapSegmentReserve;
	ULONG HeapSegmentCommit;
	ULONG HeapDeCommitTotalFreeThreshold;
	ULONG HeapDeCommitFreeBlockThreshold;
	ULONG NumberOfHeaps;
	ULONG MaximumNumberOfHeaps;
	VOID** ProcessHeaps;
	PVOID GdiSharedHandleTable;
	PVOID ProcessStarterHelper;
	ULONG GdiDCAttributeList;
	PRTL_CRITICAL_SECTION LoaderLock;
	ULONG OSMajorVersion;
	ULONG OSMinorVersion;
	WORD OSBuildNumber;
	WORD OSCSDVersion;
	ULONG OSPlatformId;
	ULONG ImageSubsystem;
	ULONG ImageSubsystemMajorVersion;
	ULONG ImageSubsystemMinorVersion;
	ULONG ImageProcessAffinityMask;
	ULONG GdiHandleBuffer[34];
	PVOID PostProcessInitRoutine;
	PVOID TlsExpansionBitmap;
	ULONG TlsExpansionBitmapBits[32];
	ULONG SessionId;
	ULARGE_INTEGER AppCompatFlags;
	ULARGE_INTEGER AppCompatFlagsUser;
	PVOID pShimData;
	PVOID AppCompatInfo;
	UNICODE_STRING CSDVersion;
	VOID* ActivationContextData;
	VOID* ProcessAssemblyStorageMap;
	VOID* SystemDefaultActivationContextData;
	VOID* SystemAssemblyStorageMap;
	ULONG MinimumStackCommit;
	VOID* FlsCallback;
	LIST_ENTRY FlsListHead;
	PVOID FlsBitmap;
	ULONG FlsBitmapBits[4];
	ULONG FlsHighIndex;
	PVOID WerRegistrationData;
	PVOID WerShipAssertPtr;
} PEB, * PPEB;

typedef enum _SYSTEM_INFORMATION_CLASS
{
	SystemBasicInformation,
	SystemProcessorInformation,
	SystemPerformanceInformation,
	SystemTimeOfDayInformation,
	SystemPathInformation,
	SystemProcessInformation,
	SystemCallCountInformation,
	SystemDeviceInformation,
	SystemProcessorPerformanceInformation,
	SystemFlagsInformation,
	SystemCallTimeInformation,
	SystemModuleInformation,
	SystemLocksInformation,
	SystemStackTraceInformation,
	SystemPagedPoolInformation,
	SystemNonPagedPoolInformation,
	SystemHandleInformation,
	SystemObjectInformation,
	SystemPageFileInformation,
	SystemVdmInstemulInformation,
	SystemVdmBopInformation,
	SystemFileCacheInformation,
	SystemPoolTagInformation,
	SystemInterruptInformation,
	SystemDpcBehaviorInformation,
	SystemFullMemoryInformation,
	SystemLoadGdiDriverInformation,
	SystemUnloadGdiDriverInformation,
	SystemTimeAdjustmentInformation,
	SystemSummaryMemoryInformation,
	SystemNextEventIdInformation,
	SystemEventIdsInformation,
	SystemCrashDumpInformation,
	SystemExceptionInformation,
	SystemCrashDumpStateInformation,
	SystemKernelDebuggerInformation,
	SystemContextSwitchInformation,
	SystemRegistryQuotaInformation,
	SystemExtendServiceTableInformation,
	SystemPrioritySeperation,
	SystemPlugPlayBusInformation,
	SystemDockInformation,
	SystemProcessorSpeedInformation,
	SystemCurrentTimeZoneInformation,
	SystemLookasideInformation,
	SystemBigPoolInformation = 0x42
} SYSTEM_INFORMATION_CLASS, * PSYSTEM_INFORMATION_CLASS;

typedef struct _MDL_INFORMATION
{
	MDL* mdl;
	uintptr_t va;
}MDL_INFORMATION, * PMDL_INFORMATION;

typedef union _VIRTUAL_ADDRESS
{
	PVOID value;
	struct
	{
		ULONG64 offset : 12;
		ULONG64 pt_index : 9;
		ULONG64 pd_index : 9;
		ULONG64 pdpt_index : 9;
		ULONG64 pml4_index : 9;
		ULONG64 reserved : 16;
	};
} VIRTUAL_ADDRESS, * PVIRTUAL_ADDRESS;
struct _EX_PUSH_LOCK
{
	union
	{
		struct
		{
			ULONG Locked : 1;                                                 //0x0
			ULONG Waiting : 1;                                                //0x0
			ULONG Waking : 1;                                                 //0x0
			ULONG MultipleShared : 1;                                         //0x0
			ULONG Shared : 28;                                                //0x0
		};
		ULONG Value;                                                        //0x0
		VOID* Ptr;                                                          //0x0
	};
};

typedef struct _MMVAD_FLAGS
{
	ULONG Lock : 1;
	ULONG LockContended : 1;
	ULONG DeleteInProgress : 1;
	ULONG NoChange : 1;
	ULONG VadType : 3;
	ULONG Protection : 5;
	ULONG PreferredNode : 6;
	ULONG PageSize : 2;
	ULONG PrivateMemory : 1;
} MMVAD_FLAGS, * PMMVAD_FLAGS;

struct _MMVAD_FLAGS1
{
	unsigned long CommitCharge : 31;
	unsigned long MemCommit : 1;
};

struct _MMVAD_FLAGS2
{
	unsigned long FileOffset : 24;
	unsigned long Large : 1;
	unsigned long TrimBehind : 1;
	unsigned long Inherit : 1;
	unsigned long CopyOnWrite : 1;
	unsigned long NoValidationNeeded : 1;
	unsigned long Spare : 3;
};

union ___unnamed1952 // Size=4
{
	unsigned long LongFlags1; // Size=4 Offset=0
	struct _MMVAD_FLAGS1 VadFlags1; // Size=4 Offset=0
};
union ___unnamed1951 // Size=4
{
	unsigned long LongFlags; // Size=4 Offset=0
	struct _MMVAD_FLAGS VadFlags; // Size=4 Offset=0
};
typedef struct _MMVAD_SHORT
{
	union
	{
		struct
		{
			struct _MMVAD_SHORT* NextVad;
			VOID* ExtraCreateInfo;
		};
		struct _RTL_BALANCED_NODE VadNode;
	};
	ULONG StartingVpn;
	ULONG EndingVpn;
	UCHAR StartingVpnHigh;
	UCHAR EndingVpnHigh;
	UCHAR CommitChargeHigh;
	UCHAR SpareNT64VadUChar;
	LONG ReferenceCount;
	_EX_PUSH_LOCK PushLock;
	union ___unnamed1951 u;
	union ___unnamed1952 u1;
	struct _MI_VAD_EVENT_BLOCK* EventList;
} MMVAD_SHORT, * PMMVAD_SHORT;

typedef struct _MM_AVL_NODE // Size=24
{
	struct _MM_AVL_NODE* LeftChild; // Size=8 Offset=0
	struct _MM_AVL_NODE* RightChild; // Size=8 Offset=8

	union ___unnamed1666 // Size=8
	{
		struct
		{
			__int64 Balance : 2; // Size=8 Offset=0 BitOffset=0 BitCount=2
		};
		struct _MM_AVL_NODE* Parent; // Size=8 Offset=0
	} u1;
} MM_AVL_NODE, * PMM_AVL_NODE, * PMMADDRESS_NODE;

typedef struct _RTL_AVL_TREE // Size=8
{
	PMM_AVL_NODE BalancedRoot;
	void* NodeHint;
	unsigned __int64 NumberGenericTableElements;
} RTL_AVL_TREE, * PRTL_AVL_TREE, MM_AVL_TABLE, * PMM_AVL_TABLE;