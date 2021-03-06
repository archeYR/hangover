/*
 * Copyright 2017 André Hentschel
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 */

/* NOTE: The guest side uses mingw's headers. The host side uses Wine's headers. */

#include <ntstatus.h>
#define WIN32_NO_STATUS
#include <windows.h>
#include <stdio.h>
#include <winternl.h>
#include <ntdef.h>

#include "thunk/qemu_winternl.h"

#include "windows-user-services.h"
#include "dll_list.h"
#include "qemu_ntdll.h"

#ifdef QEMU_DLL_GUEST

typedef enum _KEY_INFORMATION_CLASS
{
    KeyBasicInformation,
    KeyNodeInformation,
    KeyFullInformation,
    KeyNameInformation,
    KeyCachedInformation,
    KeyFlagsInformation,
    KeyVirtualizationInformation,
    KeyHandleTagsInformation,
    MaxKeyInfoClass
} KEY_INFORMATION_CLASS;

typedef enum _KEY_VALUE_INFORMATION_CLASS
{
    KeyValueBasicInformation,
    KeyValueFullInformation,
    KeyValuePartialInformation,
    KeyValueFullInformationAlign64,
    KeyValuePartialInformationAlign64
} KEY_VALUE_INFORMATION_CLASS;

typedef void *PKEY_MULTIPLE_VALUE_INFORMATION, *PRTL_QUERY_REGISTRY_TABLE;

#else

#include <wine/debug.h>
WINE_DEFAULT_DEBUG_CHANNEL(qemu_ntdll);

#endif


struct qemu_NtCreateKey
{
    struct qemu_syscall super;
    uint64_t retkey;
    uint64_t access;
    uint64_t attr;
    uint64_t TitleIndex;
    uint64_t class;
    uint64_t options;
    uint64_t dispos;
};

#ifdef QEMU_DLL_GUEST

WINBASEAPI NTSTATUS WINAPI NtCreateKey(PHANDLE retkey, ACCESS_MASK access, const OBJECT_ATTRIBUTES *attr, ULONG TitleIndex, const UNICODE_STRING *class, ULONG options, PULONG dispos)
{
    struct qemu_NtCreateKey call;
    call.super.id = QEMU_SYSCALL_ID(CALL_NTCREATEKEY);
    call.access = (ULONG_PTR)access;
    call.attr = (ULONG_PTR)attr;
    call.TitleIndex = (ULONG_PTR)TitleIndex;
    call.class = (ULONG_PTR)class;
    call.options = (ULONG_PTR)options;
    call.dispos = (ULONG_PTR)dispos;

    if (!retkey || !attr)
        return STATUS_ACCESS_VIOLATION;

    qemu_syscall(&call.super);

    *retkey = (HANDLE)(ULONG_PTR)call.retkey;

    return call.super.iret;
}

#else

void qemu_NtCreateKey(struct qemu_syscall *call)
{
    struct qemu_NtCreateKey *c = (struct qemu_NtCreateKey *)call;
    OBJECT_ATTRIBUTES stack, *attr = &stack;
    UNICODE_STRING cls_stack, *class = &cls_stack;
    UNICODE_STRING name;
    HANDLE retkey;
    WINE_TRACE("\n");

#if GUEST_BIT == HOST_BIT
    attr = QEMU_G2H(c->attr);
    class = QEMU_G2H(c->class);
#else
    OBJECT_ATTRIBUTES_g2h(attr, QEMU_G2H(c->attr), &name);
    if (c->class)
        UNICODE_STRING_g2h(class, QEMU_G2H(c->class));
    else
        class = NULL;
#endif

    c->super.iret = NtCreateKey(&retkey, c->access, attr, c->TitleIndex, class, c->options, QEMU_G2H(c->dispos));
    c->retkey = QEMU_H2G(retkey);
}

#endif

struct qemu_NtCreateKeyTransacted
{
    struct qemu_syscall super;
    uint64_t retkey;
    uint64_t access;
    uint64_t attr;
    uint64_t TitleIndex;
    uint64_t class;
    uint64_t options;
    uint64_t transacted;
    uint64_t dispos;
};

#ifdef QEMU_DLL_GUEST

WINBASEAPI NTSTATUS WINAPI NtCreateKeyTransacted(PHANDLE retkey, ACCESS_MASK access, const OBJECT_ATTRIBUTES *attr, ULONG TitleIndex, const UNICODE_STRING *class, ULONG options, HANDLE transacted, ULONG *dispos)
{
    struct qemu_NtCreateKeyTransacted call;
    call.super.id = QEMU_SYSCALL_ID(CALL_NTCREATEKEYTRANSACTED);
    call.retkey = (ULONG_PTR)retkey;
    call.access = (ULONG_PTR)access;
    call.attr = (ULONG_PTR)attr;
    call.TitleIndex = (ULONG_PTR)TitleIndex;
    call.class = (ULONG_PTR)class;
    call.options = (ULONG_PTR)options;
    call.transacted = (ULONG_PTR)transacted;
    call.dispos = (ULONG_PTR)dispos;

    qemu_syscall(&call.super);

    return call.super.iret;
}

#else

void qemu_NtCreateKeyTransacted(struct qemu_syscall *call)
{
    struct qemu_NtCreateKeyTransacted *c = (struct qemu_NtCreateKeyTransacted *)call;
    WINE_FIXME("Unverified!\n");
    c->super.iret = NtCreateKeyTransacted(QEMU_G2H(c->retkey), c->access, QEMU_G2H(c->attr), c->TitleIndex, QEMU_G2H(c->class), c->options, QEMU_G2H(c->transacted), QEMU_G2H(c->dispos));
}

#endif

struct qemu_NtRenameKey
{
    struct qemu_syscall super;
    uint64_t handle;
    uint64_t name;
};

#ifdef QEMU_DLL_GUEST

WINBASEAPI NTSTATUS WINAPI NtRenameKey(HANDLE handle, UNICODE_STRING *name)
{
    struct qemu_NtRenameKey call;
    call.super.id = QEMU_SYSCALL_ID(CALL_NTRENAMEKEY);
    call.handle = (ULONG_PTR)handle;
    call.name = (ULONG_PTR)name;

    qemu_syscall(&call.super);

    return call.super.iret;
}

#else

void qemu_NtRenameKey(struct qemu_syscall *call)
{
    struct qemu_NtRenameKey *c = (struct qemu_NtRenameKey *)call;
    WINE_FIXME("Unverified!\n");
    c->super.iret = NtRenameKey(QEMU_G2H(c->handle), QEMU_G2H(c->name));
}

#endif

struct qemu_RtlpNtCreateKey
{
    struct qemu_syscall super;
    uint64_t retkey;
    uint64_t access;
    uint64_t attr;
    uint64_t TitleIndex;
    uint64_t class;
    uint64_t options;
    uint64_t dispos;
};

#ifdef QEMU_DLL_GUEST

WINBASEAPI NTSTATUS WINAPI RtlpNtCreateKey(PHANDLE retkey, ACCESS_MASK access, const OBJECT_ATTRIBUTES *attr, ULONG TitleIndex, const UNICODE_STRING *class, ULONG options, PULONG dispos)
{
    struct qemu_RtlpNtCreateKey call;
    call.super.id = QEMU_SYSCALL_ID(CALL_RTLPNTCREATEKEY);
    call.access = (ULONG_PTR)access;
    call.attr = (ULONG_PTR)attr;
    call.TitleIndex = (ULONG_PTR)TitleIndex;
    call.class = (ULONG_PTR)class;
    call.options = (ULONG_PTR)options;
    call.dispos = (ULONG_PTR)dispos;

    if (!retkey || !attr)
        return STATUS_ACCESS_VIOLATION;

    qemu_syscall(&call.super);

    *retkey = (HANDLE)(ULONG_PTR)call.retkey;

    return call.super.iret;
}

#else

void qemu_RtlpNtCreateKey(struct qemu_syscall *call)
{
    struct qemu_RtlpNtCreateKey *c = (struct qemu_RtlpNtCreateKey *)call;
    OBJECT_ATTRIBUTES stack, *attr = &stack;
    UNICODE_STRING cls_stack, *class = &cls_stack;
    UNICODE_STRING name;
    HANDLE retkey;
    WINE_TRACE("\n");

#if GUEST_BIT == HOST_BIT
    attr = QEMU_G2H(c->attr);
    class = QEMU_G2H(c->class);
#else
    OBJECT_ATTRIBUTES_g2h(attr, QEMU_G2H(c->attr), &name);
    if (c->class)
        UNICODE_STRING_g2h(class, QEMU_G2H(c->class));
    else
        class = NULL;
#endif

    c->super.iret = RtlpNtCreateKey(&retkey, c->access, attr, c->TitleIndex, class, c->options, QEMU_G2H(c->dispos));
    c->retkey = QEMU_H2G(retkey);
}

#endif

struct qemu_NtOpenKeyEx
{
    struct qemu_syscall super;
    uint64_t retkey;
    uint64_t access;
    uint64_t attr;
    uint64_t options;
};

#ifdef QEMU_DLL_GUEST

WINBASEAPI NTSTATUS WINAPI NtOpenKeyEx(PHANDLE retkey, ACCESS_MASK access, const OBJECT_ATTRIBUTES *attr, ULONG options)
{
    struct qemu_NtOpenKeyEx call;
    call.super.id = QEMU_SYSCALL_ID(CALL_NTOPENKEYEX);
    call.access = (ULONG_PTR)access;
    call.attr = (ULONG_PTR)attr;
    call.options = (ULONG_PTR)options;

    if (!retkey || !attr)
        return STATUS_ACCESS_VIOLATION;

    qemu_syscall(&call.super);
    *retkey = (HANDLE)(ULONG_PTR)call.retkey;

    return call.super.iret;
}

#else

void qemu_NtOpenKeyEx(struct qemu_syscall *call)
{
    struct qemu_NtOpenKeyEx *c = (struct qemu_NtOpenKeyEx *)call;
    OBJECT_ATTRIBUTES stack, *attr = &stack;
    UNICODE_STRING name;
    HANDLE retkey;
    WINE_TRACE("\n");

#if GUEST_BIT == HOST_BIT
    attr = QEMU_G2H(c->attr);
#else
    OBJECT_ATTRIBUTES_g2h(attr, QEMU_G2H(c->attr), &name);
#endif

    c->super.iret = NtOpenKeyEx(&retkey, c->access, attr, c->options);
    c->retkey = QEMU_H2G(retkey);
}

#endif

struct qemu_NtOpenKey
{
    struct qemu_syscall super;
    uint64_t retkey;
    uint64_t access;
    uint64_t attr;
};

#ifdef QEMU_DLL_GUEST

WINBASEAPI NTSTATUS WINAPI NtOpenKey(PHANDLE retkey, ACCESS_MASK access, const OBJECT_ATTRIBUTES *attr)
{
    struct qemu_NtOpenKey call;
    call.super.id = QEMU_SYSCALL_ID(CALL_NTOPENKEY);
    call.access = (ULONG_PTR)access;
    call.attr = (ULONG_PTR)attr;

    if (!retkey || !attr)
        return STATUS_ACCESS_VIOLATION;

    qemu_syscall(&call.super);
    *retkey = (HANDLE)(ULONG_PTR)call.retkey;

    return call.super.iret;
}

#else

void qemu_NtOpenKey(struct qemu_syscall *call)
{
    struct qemu_NtOpenKey *c = (struct qemu_NtOpenKey *)call;
    OBJECT_ATTRIBUTES stack, *attr = &stack;
    UNICODE_STRING name;
    HANDLE retkey;
    WINE_TRACE("\n");

#if GUEST_BIT == HOST_BIT
    attr = QEMU_G2H(c->attr);
#else
    OBJECT_ATTRIBUTES_g2h(attr, QEMU_G2H(c->attr), &name);
#endif

    c->super.iret = NtOpenKey(&retkey, c->access, attr);
    c->retkey = QEMU_H2G(retkey);
}

#endif

struct qemu_NtOpenKeyTransactedEx
{
    struct qemu_syscall super;
    uint64_t retkey;
    uint64_t access;
    uint64_t attr;
    uint64_t options;
    uint64_t transaction;
};

#ifdef QEMU_DLL_GUEST

WINBASEAPI NTSTATUS WINAPI NtOpenKeyTransactedEx(PHANDLE retkey, ACCESS_MASK access, const OBJECT_ATTRIBUTES *attr, ULONG options, HANDLE transaction)
{
    struct qemu_NtOpenKeyTransactedEx call;
    call.super.id = QEMU_SYSCALL_ID(CALL_NTOPENKEYTRANSACTEDEX);
    call.retkey = (ULONG_PTR)retkey;
    call.access = (ULONG_PTR)access;
    call.attr = (ULONG_PTR)attr;
    call.options = (ULONG_PTR)options;
    call.transaction = (ULONG_PTR)transaction;

    qemu_syscall(&call.super);

    return call.super.iret;
}

#else

void qemu_NtOpenKeyTransactedEx(struct qemu_syscall *call)
{
    struct qemu_NtOpenKeyTransactedEx *c = (struct qemu_NtOpenKeyTransactedEx *)call;
    WINE_FIXME("Unverified!\n");
    c->super.iret = NtOpenKeyTransactedEx(QEMU_G2H(c->retkey), c->access, QEMU_G2H(c->attr), c->options, QEMU_G2H(c->transaction));
}

#endif

struct qemu_NtOpenKeyTransacted
{
    struct qemu_syscall super;
    uint64_t retkey;
    uint64_t access;
    uint64_t attr;
    uint64_t transaction;
};

#ifdef QEMU_DLL_GUEST

WINBASEAPI NTSTATUS WINAPI NtOpenKeyTransacted(PHANDLE retkey, ACCESS_MASK access, const OBJECT_ATTRIBUTES *attr, HANDLE transaction)
{
    struct qemu_NtOpenKeyTransacted call;
    call.super.id = QEMU_SYSCALL_ID(CALL_NTOPENKEYTRANSACTED);
    call.retkey = (ULONG_PTR)retkey;
    call.access = (ULONG_PTR)access;
    call.attr = (ULONG_PTR)attr;
    call.transaction = (ULONG_PTR)transaction;

    qemu_syscall(&call.super);

    return call.super.iret;
}

#else

void qemu_NtOpenKeyTransacted(struct qemu_syscall *call)
{
    struct qemu_NtOpenKeyTransacted *c = (struct qemu_NtOpenKeyTransacted *)call;
    WINE_FIXME("Unverified!\n");
    c->super.iret = NtOpenKeyTransacted(QEMU_G2H(c->retkey), c->access, QEMU_G2H(c->attr), QEMU_G2H(c->transaction));
}

#endif

struct qemu_RtlpNtOpenKey
{
    struct qemu_syscall super;
    uint64_t retkey;
    uint64_t access;
    uint64_t attr;
};

#ifdef QEMU_DLL_GUEST

WINBASEAPI NTSTATUS WINAPI RtlpNtOpenKey(PHANDLE retkey, ACCESS_MASK access, OBJECT_ATTRIBUTES *attr)
{
    struct qemu_RtlpNtOpenKey call;
    call.super.id = QEMU_SYSCALL_ID(CALL_RTLPNTOPENKEY);
    call.access = (ULONG_PTR)access;
    call.attr = (ULONG_PTR)attr;

    if (!retkey || !attr)
        return STATUS_ACCESS_VIOLATION;

    qemu_syscall(&call.super);

    *retkey = (HANDLE)(ULONG_PTR)call.retkey;

    return call.super.iret;
}

#else

/* TODO: Add RtlpNtOpenKey to Wine headers? */
extern NTSTATUS WINAPI RtlpNtOpenKey(PHANDLE retkey, ACCESS_MASK access, OBJECT_ATTRIBUTES *attr);
void qemu_RtlpNtOpenKey(struct qemu_syscall *call)
{
    struct qemu_RtlpNtOpenKey *c = (struct qemu_RtlpNtOpenKey *)call;
    OBJECT_ATTRIBUTES stack, *attr = &stack;
    UNICODE_STRING name;
    HANDLE retkey;
    WINE_TRACE("\n");

#if GUEST_BIT == HOST_BIT
    attr = QEMU_G2H(c->attr);
#else
    OBJECT_ATTRIBUTES_g2h(attr, QEMU_G2H(c->attr), &name);
#endif

    c->super.iret = RtlpNtOpenKey(&retkey, c->access, attr);
    c->retkey = QEMU_H2G(retkey);
}

#endif

struct qemu_NtDeleteKey
{
    struct qemu_syscall super;
    uint64_t hkey;
};

#ifdef QEMU_DLL_GUEST

WINBASEAPI NTSTATUS WINAPI NtDeleteKey(HANDLE hkey)
{
    struct qemu_NtDeleteKey call;
    call.super.id = QEMU_SYSCALL_ID(CALL_NTDELETEKEY);
    call.hkey = (LONG_PTR)hkey;

    qemu_syscall(&call.super);

    return call.super.iret;
}

#else

void qemu_NtDeleteKey(struct qemu_syscall *call)
{
    struct qemu_NtDeleteKey *c = (struct qemu_NtDeleteKey *)call;
    WINE_FIXME("Unverified!\n");
    c->super.iret = NtDeleteKey(QEMU_G2H(c->hkey));
}

#endif

struct qemu_RtlpNtMakeTemporaryKey
{
    struct qemu_syscall super;
    uint64_t hkey;
};

#ifdef QEMU_DLL_GUEST

WINBASEAPI NTSTATUS WINAPI RtlpNtMakeTemporaryKey(HANDLE hkey)
{
    struct qemu_RtlpNtMakeTemporaryKey call;
    call.super.id = QEMU_SYSCALL_ID(CALL_RTLPNTMAKETEMPORARYKEY);
    call.hkey = (LONG_PTR)hkey;

    qemu_syscall(&call.super);

    return call.super.iret;
}

#else

/* TODO: Add RtlpNtMakeTemporaryKey to Wine headers? */
extern NTSTATUS WINAPI RtlpNtMakeTemporaryKey(HANDLE hkey);
void qemu_RtlpNtMakeTemporaryKey(struct qemu_syscall *call)
{
    struct qemu_RtlpNtMakeTemporaryKey *c = (struct qemu_RtlpNtMakeTemporaryKey *)call;
    WINE_FIXME("Unverified!\n");
    c->super.iret = RtlpNtMakeTemporaryKey(QEMU_G2H(c->hkey));
}

#endif

struct qemu_NtDeleteValueKey
{
    struct qemu_syscall super;
    uint64_t hkey;
    uint64_t name;
};

#ifdef QEMU_DLL_GUEST

WINBASEAPI NTSTATUS WINAPI NtDeleteValueKey(HANDLE hkey, const UNICODE_STRING *name)
{
    struct qemu_NtDeleteValueKey call;
    call.super.id = QEMU_SYSCALL_ID(CALL_NTDELETEVALUEKEY);
    call.hkey = (LONG_PTR)hkey;
    call.name = (ULONG_PTR)name;

    qemu_syscall(&call.super);

    return call.super.iret;
}

#else

void qemu_NtDeleteValueKey(struct qemu_syscall *call)
{
    struct qemu_NtDeleteValueKey *c = (struct qemu_NtDeleteValueKey *)call;
    UNICODE_STRING name_stack, *name = &name_stack;
    WINE_FIXME("Unverified!\n");

#if GUEST_BIT == HOST_BIT
    name = QEMU_G2H(c->name);
#else
    UNICODE_STRING_g2h(name, QEMU_G2H(c->name));
#endif

    c->super.iret = NtDeleteValueKey(QEMU_G2H(c->hkey), name);
}

#endif

struct qemu_NtEnumerateKey
{
    struct qemu_syscall super;
    uint64_t handle;
    uint64_t index;
    uint64_t info_class;
    uint64_t info;
    uint64_t length;
    uint64_t result_len;
};

#ifdef QEMU_DLL_GUEST

WINBASEAPI NTSTATUS WINAPI NtEnumerateKey(HANDLE handle, ULONG index, KEY_INFORMATION_CLASS info_class, void *info, DWORD length, DWORD *result_len)
{
    struct qemu_NtEnumerateKey call;
    call.super.id = QEMU_SYSCALL_ID(CALL_NTENUMERATEKEY);
    call.handle = (ULONG_PTR)handle;
    call.index = (ULONG_PTR)index;
    call.info_class = (ULONG_PTR)info_class;
    call.info = (ULONG_PTR)info;
    call.length = (ULONG_PTR)length;
    call.result_len = (ULONG_PTR)result_len;

    qemu_syscall(&call.super);

    return call.super.iret;
}

#else

void qemu_NtEnumerateKey(struct qemu_syscall *call)
{
    struct qemu_NtEnumerateKey *c = (struct qemu_NtEnumerateKey *)call;
    WINE_FIXME("Unverified!\n");
    c->super.iret = NtEnumerateKey(QEMU_G2H(c->handle), c->index, c->info_class, QEMU_G2H(c->info), c->length, QEMU_G2H(c->result_len));
}

#endif

struct qemu_RtlpNtEnumerateSubKey
{
    struct qemu_syscall super;
    uint64_t handle;
    uint64_t out;
    uint64_t index;
};

#ifdef QEMU_DLL_GUEST

WINBASEAPI NTSTATUS WINAPI RtlpNtEnumerateSubKey(HANDLE handle, UNICODE_STRING *out, ULONG index)
{
    struct qemu_RtlpNtEnumerateSubKey call;
    call.super.id = QEMU_SYSCALL_ID(CALL_RTLPNTENUMERATESUBKEY);
    call.handle = (ULONG_PTR)handle;
    call.out = (ULONG_PTR)out;
    call.index = (ULONG_PTR)index;

    qemu_syscall(&call.super);

    return call.super.iret;
}

#else

void qemu_RtlpNtEnumerateSubKey(struct qemu_syscall *call)
{
    struct qemu_RtlpNtEnumerateSubKey *c = (struct qemu_RtlpNtEnumerateSubKey *)call;
    WINE_FIXME("Unverified!\n");
    c->super.iret = RtlpNtEnumerateSubKey(QEMU_G2H(c->handle), QEMU_G2H(c->out), c->index);
}

#endif

struct qemu_NtQueryKey
{
    struct qemu_syscall super;
    uint64_t handle;
    uint64_t info_class;
    uint64_t info;
    uint64_t length;
    uint64_t result_len;
};

#ifdef QEMU_DLL_GUEST

WINBASEAPI NTSTATUS WINAPI NtQueryKey(HANDLE handle, KEY_INFORMATION_CLASS info_class, void *info, DWORD length, DWORD *result_len)
{
    struct qemu_NtQueryKey call;
    call.super.id = QEMU_SYSCALL_ID(CALL_NTQUERYKEY);
    call.handle = (ULONG_PTR)handle;
    call.info_class = (ULONG_PTR)info_class;
    call.info = (ULONG_PTR)info;
    call.length = (ULONG_PTR)length;
    call.result_len = (ULONG_PTR)result_len;

    qemu_syscall(&call.super);

    return call.super.iret;
}

#else

void qemu_NtQueryKey(struct qemu_syscall *call)
{
    struct qemu_NtQueryKey *c = (struct qemu_NtQueryKey *)call;
    WINE_FIXME("Unverified!\n");
    c->super.iret = NtQueryKey(QEMU_G2H(c->handle), c->info_class, QEMU_G2H(c->info), c->length, QEMU_G2H(c->result_len));
}

#endif

struct qemu_NtEnumerateValueKey
{
    struct qemu_syscall super;
    uint64_t handle;
    uint64_t index;
    uint64_t info_class;
    uint64_t info;
    uint64_t length;
    uint64_t result_len;
};

#ifdef QEMU_DLL_GUEST

WINBASEAPI NTSTATUS WINAPI NtEnumerateValueKey(HANDLE handle, ULONG index, KEY_VALUE_INFORMATION_CLASS info_class, void *info, DWORD length, DWORD *result_len)
{
    struct qemu_NtEnumerateValueKey call;
    call.super.id = QEMU_SYSCALL_ID(CALL_NTENUMERATEVALUEKEY);
    call.handle = (ULONG_PTR)handle;
    call.index = (ULONG_PTR)index;
    call.info_class = (ULONG_PTR)info_class;
    call.info = (ULONG_PTR)info;
    call.length = (ULONG_PTR)length;
    call.result_len = (ULONG_PTR)result_len;

    qemu_syscall(&call.super);

    return call.super.iret;
}

#else

void qemu_NtEnumerateValueKey(struct qemu_syscall *call)
{
    struct qemu_NtEnumerateValueKey *c = (struct qemu_NtEnumerateValueKey *)call;
    WINE_FIXME("Unverified!\n");
    c->super.iret = NtEnumerateValueKey(QEMU_G2H(c->handle), c->index, c->info_class, QEMU_G2H(c->info), c->length, QEMU_G2H(c->result_len));
}

#endif

struct qemu_NtQueryValueKey
{
    struct qemu_syscall super;
    uint64_t handle;
    uint64_t name;
    uint64_t info_class;
    uint64_t info;
    uint64_t length;
    uint64_t result_len;
};

#ifdef QEMU_DLL_GUEST

WINBASEAPI NTSTATUS WINAPI NtQueryValueKey(HANDLE handle, const UNICODE_STRING *name,
        KEY_VALUE_INFORMATION_CLASS info_class, void *info, DWORD length, DWORD *result_len)
{
    struct qemu_NtQueryValueKey call;
    call.super.id = QEMU_SYSCALL_ID(CALL_NTQUERYVALUEKEY);
    call.handle = (ULONG_PTR)handle;
    call.name = (ULONG_PTR)name;
    call.info_class = (ULONG_PTR)info_class;
    call.info = (ULONG_PTR)info;
    call.length = (ULONG_PTR)length;
    call.result_len = (ULONG_PTR)result_len;

    qemu_syscall(&call.super);

    return call.super.iret;
}

#else

void qemu_NtQueryValueKey(struct qemu_syscall *call)
{
    struct qemu_NtQueryValueKey *c = (struct qemu_NtQueryValueKey *)call;
    UNICODE_STRING stack, *name = &stack;
    
    /* All 3 info classes implemented in Wine have the same layout on 32 and 64 bit. */
    WINE_TRACE("\n");
#if GUEST_BIT == HOST_BIT
    name = QEMU_G2H(c->name);
#else
    if (QEMU_G2H(c->name))
        UNICODE_STRING_g2h(name, QEMU_G2H(c->name));
    else
        name = NULL;
#endif

    c->super.iret = NtQueryValueKey(QEMU_G2H(c->handle), name, c->info_class, QEMU_G2H(c->info),
            c->length, QEMU_G2H(c->result_len));
}

#endif

struct qemu_RtlpNtQueryValueKey
{
    struct qemu_syscall super;
    uint64_t handle;
    uint64_t result_type;
    uint64_t dest;
    uint64_t result_len;
    uint64_t unknown;
};

#ifdef QEMU_DLL_GUEST

WINBASEAPI NTSTATUS WINAPI RtlpNtQueryValueKey(HANDLE handle, ULONG *result_type, PBYTE dest, DWORD *result_len, void *unknown)
{
    struct qemu_RtlpNtQueryValueKey call;
    call.super.id = QEMU_SYSCALL_ID(CALL_RTLPNTQUERYVALUEKEY);
    call.handle = (ULONG_PTR)handle;
    call.result_type = (ULONG_PTR)result_type;
    call.dest = (ULONG_PTR)dest;
    call.result_len = (ULONG_PTR)result_len;
    call.unknown = (ULONG_PTR)unknown;

    qemu_syscall(&call.super);

    return call.super.iret;
}

#else

/* TODO: Add RtlpNtQueryValueKey to Wine headers? */
extern NTSTATUS WINAPI RtlpNtQueryValueKey(HANDLE handle, ULONG *result_type, PBYTE dest, DWORD *result_len, void *unknown);
void qemu_RtlpNtQueryValueKey(struct qemu_syscall *call)
{
    struct qemu_RtlpNtQueryValueKey *c = (struct qemu_RtlpNtQueryValueKey *)call;
    WINE_FIXME("Unverified!\n");
    c->super.iret = RtlpNtQueryValueKey(QEMU_G2H(c->handle), QEMU_G2H(c->result_type), QEMU_G2H(c->dest), QEMU_G2H(c->result_len), QEMU_G2H(c->unknown));
}

#endif

struct qemu_NtFlushKey
{
    struct qemu_syscall super;
    uint64_t key;
};

#ifdef QEMU_DLL_GUEST

WINBASEAPI NTSTATUS WINAPI NtFlushKey(HANDLE key)
{
    struct qemu_NtFlushKey call;
    call.super.id = QEMU_SYSCALL_ID(CALL_NTFLUSHKEY);
    call.key = (ULONG_PTR)key;

    qemu_syscall(&call.super);

    return call.super.iret;
}

#else

void qemu_NtFlushKey(struct qemu_syscall *call)
{
    struct qemu_NtFlushKey *c = (struct qemu_NtFlushKey *)call;
    WINE_FIXME("Unverified!\n");
    c->super.iret = NtFlushKey(QEMU_G2H(c->key));
}

#endif

struct qemu_NtLoadKey
{
    struct qemu_syscall super;
    uint64_t attr;
    uint64_t file;
};

#ifdef QEMU_DLL_GUEST

WINBASEAPI NTSTATUS WINAPI NtLoadKey(const OBJECT_ATTRIBUTES *attr, OBJECT_ATTRIBUTES *file)
{
    struct qemu_NtLoadKey call;
    call.super.id = QEMU_SYSCALL_ID(CALL_NTLOADKEY);
    call.attr = (ULONG_PTR)attr;
    call.file = (ULONG_PTR)file;

    qemu_syscall(&call.super);

    return call.super.iret;
}

#else

void qemu_NtLoadKey(struct qemu_syscall *call)
{
    struct qemu_NtLoadKey *c = (struct qemu_NtLoadKey *)call;
    WINE_FIXME("Unverified!\n");
    c->super.iret = NtLoadKey(QEMU_G2H(c->attr), QEMU_G2H(c->file));
}

#endif

struct qemu_NtLoadKey2
{
    struct qemu_syscall super;
    uint64_t attr;
    uint64_t file;
    uint64_t flags;
};

#ifdef QEMU_DLL_GUEST

WINBASEAPI NTSTATUS WINAPI NtLoadKey2(OBJECT_ATTRIBUTES *attr, OBJECT_ATTRIBUTES *file, ULONG flags)
{
    struct qemu_NtLoadKey2 call;
    call.super.id = QEMU_SYSCALL_ID(CALL_NTLOADKEY2);
    call.attr = (ULONG_PTR)attr;
    call.file = (ULONG_PTR)file;
    call.flags = (ULONG_PTR)flags;

    qemu_syscall(&call.super);

    return call.super.iret;
}

#else

/* TODO: Add NtLoadKey2 to Wine headers? */
extern NTSTATUS WINAPI NtLoadKey2(OBJECT_ATTRIBUTES *attr, OBJECT_ATTRIBUTES *file, ULONG flags);
void qemu_NtLoadKey2(struct qemu_syscall *call)
{
    struct qemu_NtLoadKey2 *c = (struct qemu_NtLoadKey2 *)call;
    WINE_FIXME("Unverified!\n");
    c->super.iret = NtLoadKey2(QEMU_G2H(c->attr), QEMU_G2H(c->file), c->flags);
}

#endif

struct qemu_NtNotifyChangeMultipleKeys
{
    struct qemu_syscall super;
    uint64_t KeyHandle;
    uint64_t Count;
    uint64_t SubordinateObjects;
    uint64_t Event;
    uint64_t ApcRoutine;
    uint64_t ApcContext;
    uint64_t IoStatusBlock;
    uint64_t CompletionFilter;
    uint64_t WatchSubtree;
    uint64_t ChangeBuffer;
    uint64_t Length;
    uint64_t Asynchronous;
};

#ifdef QEMU_DLL_GUEST

WINBASEAPI NTSTATUS WINAPI NtNotifyChangeMultipleKeys(HANDLE KeyHandle, ULONG Count, OBJECT_ATTRIBUTES *SubordinateObjects, HANDLE Event, PIO_APC_ROUTINE ApcRoutine, PVOID ApcContext, PIO_STATUS_BLOCK IoStatusBlock, ULONG CompletionFilter, BOOLEAN WatchSubtree, PVOID ChangeBuffer, ULONG Length, BOOLEAN Asynchronous)
{
    struct qemu_NtNotifyChangeMultipleKeys call;
    call.super.id = QEMU_SYSCALL_ID(CALL_NTNOTIFYCHANGEMULTIPLEKEYS);
    call.KeyHandle = (ULONG_PTR)KeyHandle;
    call.Count = (ULONG_PTR)Count;
    call.SubordinateObjects = (ULONG_PTR)SubordinateObjects;
    call.Event = (ULONG_PTR)Event;
    call.ApcRoutine = (ULONG_PTR)ApcRoutine;
    call.ApcContext = (ULONG_PTR)ApcContext;
    call.IoStatusBlock = (ULONG_PTR)IoStatusBlock;
    call.CompletionFilter = (ULONG_PTR)CompletionFilter;
    call.WatchSubtree = (ULONG_PTR)WatchSubtree;
    call.ChangeBuffer = (ULONG_PTR)ChangeBuffer;
    call.Length = (ULONG_PTR)Length;
    call.Asynchronous = (ULONG_PTR)Asynchronous;

    qemu_syscall(&call.super);

    return call.super.iret;
}

#else

void qemu_NtNotifyChangeMultipleKeys(struct qemu_syscall *call)
{
    struct qemu_NtNotifyChangeMultipleKeys *c = (struct qemu_NtNotifyChangeMultipleKeys *)call;
    WINE_FIXME("Unverified!\n");
    c->super.iret = NtNotifyChangeMultipleKeys(QEMU_G2H(c->KeyHandle), c->Count, QEMU_G2H(c->SubordinateObjects), QEMU_G2H(c->Event), QEMU_G2H(c->ApcRoutine), QEMU_G2H(c->ApcContext), QEMU_G2H(c->IoStatusBlock), c->CompletionFilter, c->WatchSubtree, QEMU_G2H(c->ChangeBuffer), c->Length, c->Asynchronous);
}

#endif

struct qemu_NtNotifyChangeKey
{
    struct qemu_syscall super;
    uint64_t KeyHandle;
    uint64_t Event;
    uint64_t ApcRoutine;
    uint64_t ApcContext;
    uint64_t IoStatusBlock;
    uint64_t CompletionFilter;
    uint64_t WatchSubtree;
    uint64_t ChangeBuffer;
    uint64_t Length;
    uint64_t Asynchronous;
};

#ifdef QEMU_DLL_GUEST

WINBASEAPI NTSTATUS WINAPI NtNotifyChangeKey(HANDLE KeyHandle, HANDLE Event, PIO_APC_ROUTINE ApcRoutine, PVOID ApcContext, PIO_STATUS_BLOCK IoStatusBlock, ULONG CompletionFilter, BOOLEAN WatchSubtree, PVOID ChangeBuffer, ULONG Length, BOOLEAN Asynchronous)
{
    struct qemu_NtNotifyChangeKey call;
    call.super.id = QEMU_SYSCALL_ID(CALL_NTNOTIFYCHANGEKEY);
    call.KeyHandle = (ULONG_PTR)KeyHandle;
    call.Event = (ULONG_PTR)Event;
    call.ApcRoutine = (ULONG_PTR)ApcRoutine;
    call.ApcContext = (ULONG_PTR)ApcContext;
    call.IoStatusBlock = (ULONG_PTR)IoStatusBlock;
    call.CompletionFilter = (ULONG_PTR)CompletionFilter;
    call.WatchSubtree = (ULONG_PTR)WatchSubtree;
    call.ChangeBuffer = (ULONG_PTR)ChangeBuffer;
    call.Length = (ULONG_PTR)Length;
    call.Asynchronous = (ULONG_PTR)Asynchronous;

    qemu_syscall(&call.super);

    return call.super.iret;
}

#else

void qemu_NtNotifyChangeKey(struct qemu_syscall *call)
{
    struct qemu_NtNotifyChangeKey *c = (struct qemu_NtNotifyChangeKey *)call;
    WINE_FIXME("Unverified!\n");
    c->super.iret = NtNotifyChangeKey(QEMU_G2H(c->KeyHandle), QEMU_G2H(c->Event), QEMU_G2H(c->ApcRoutine), QEMU_G2H(c->ApcContext), QEMU_G2H(c->IoStatusBlock), c->CompletionFilter, c->WatchSubtree, QEMU_G2H(c->ChangeBuffer), c->Length, c->Asynchronous);
}

#endif

struct qemu_NtQueryMultipleValueKey
{
    struct qemu_syscall super;
    uint64_t KeyHandle;
    uint64_t ListOfValuesToQuery;
    uint64_t NumberOfItems;
    uint64_t MultipleValueInformation;
    uint64_t Length;
    uint64_t ReturnLength;
};

#ifdef QEMU_DLL_GUEST

#if __MINGW64_VERSION_MAJOR < 7
WINBASEAPI NTSTATUS WINAPI NtQueryMultipleValueKey(HANDLE KeyHandle, PKEY_MULTIPLE_VALUE_INFORMATION ListOfValuesToQuery, ULONG NumberOfItems, PVOID MultipleValueInformation, ULONG Length, PULONG ReturnLength)
#else
WINBASEAPI NTSTATUS NTAPI NtQueryMultipleValueKey(HANDLE KeyHandle, PKEY_VALUE_ENTRY ListOfValuesToQuery, ULONG NumberOfItems, PVOID MultipleValueInformation, PULONG Length, PULONG ReturnLength)
#endif
{
    struct qemu_NtQueryMultipleValueKey call;
    call.super.id = QEMU_SYSCALL_ID(CALL_NTQUERYMULTIPLEVALUEKEY);
    call.KeyHandle = (ULONG_PTR)KeyHandle;
    call.ListOfValuesToQuery = (ULONG_PTR)ListOfValuesToQuery;
    call.NumberOfItems = (ULONG_PTR)NumberOfItems;
    call.MultipleValueInformation = (ULONG_PTR)MultipleValueInformation;
    call.Length = (ULONG_PTR)Length;
    call.ReturnLength = (ULONG_PTR)ReturnLength;

    qemu_syscall(&call.super);

    return call.super.iret;
}

#else

void qemu_NtQueryMultipleValueKey(struct qemu_syscall *call)
{
    struct qemu_NtQueryMultipleValueKey *c = (struct qemu_NtQueryMultipleValueKey *)call;
    WINE_FIXME("Unverified!\n");
    c->super.iret = NtQueryMultipleValueKey(QEMU_G2H(c->KeyHandle), QEMU_G2H(c->ListOfValuesToQuery), c->NumberOfItems, QEMU_G2H(c->MultipleValueInformation), c->Length, QEMU_G2H(c->ReturnLength));
}

#endif

struct qemu_NtReplaceKey
{
    struct qemu_syscall super;
    uint64_t ObjectAttributes;
    uint64_t Key;
    uint64_t ReplacedObjectAttributes;
};

#ifdef QEMU_DLL_GUEST

WINBASEAPI NTSTATUS WINAPI NtReplaceKey(POBJECT_ATTRIBUTES ObjectAttributes, HANDLE Key, POBJECT_ATTRIBUTES ReplacedObjectAttributes)
{
    struct qemu_NtReplaceKey call;
    call.super.id = QEMU_SYSCALL_ID(CALL_NTREPLACEKEY);
    call.ObjectAttributes = (ULONG_PTR)ObjectAttributes;
    call.Key = (ULONG_PTR)Key;
    call.ReplacedObjectAttributes = (ULONG_PTR)ReplacedObjectAttributes;

    qemu_syscall(&call.super);

    return call.super.iret;
}

#else

void qemu_NtReplaceKey(struct qemu_syscall *call)
{
    struct qemu_NtReplaceKey *c = (struct qemu_NtReplaceKey *)call;
    WINE_FIXME("Unverified!\n");
    c->super.iret = NtReplaceKey(QEMU_G2H(c->ObjectAttributes), QEMU_G2H(c->Key), QEMU_G2H(c->ReplacedObjectAttributes));
}

#endif

struct qemu_NtRestoreKey
{
    struct qemu_syscall super;
    uint64_t KeyHandle;
    uint64_t FileHandle;
    uint64_t RestoreFlags;
};

#ifdef QEMU_DLL_GUEST

WINBASEAPI NTSTATUS WINAPI NtRestoreKey(HANDLE KeyHandle, HANDLE FileHandle, ULONG RestoreFlags)
{
    struct qemu_NtRestoreKey call;
    call.super.id = QEMU_SYSCALL_ID(CALL_NTRESTOREKEY);
    call.KeyHandle = (ULONG_PTR)KeyHandle;
    call.FileHandle = (ULONG_PTR)FileHandle;
    call.RestoreFlags = (ULONG_PTR)RestoreFlags;

    qemu_syscall(&call.super);

    return call.super.iret;
}

#else

void qemu_NtRestoreKey(struct qemu_syscall *call)
{
    struct qemu_NtRestoreKey *c = (struct qemu_NtRestoreKey *)call;
    WINE_FIXME("Unverified!\n");
    c->super.iret = NtRestoreKey(QEMU_G2H(c->KeyHandle), QEMU_G2H(c->FileHandle), c->RestoreFlags);
}

#endif

struct qemu_NtSaveKey
{
    struct qemu_syscall super;
    uint64_t KeyHandle;
    uint64_t FileHandle;
};

#ifdef QEMU_DLL_GUEST

WINBASEAPI NTSTATUS WINAPI NtSaveKey(HANDLE KeyHandle, HANDLE FileHandle)
{
    struct qemu_NtSaveKey call;
    call.super.id = QEMU_SYSCALL_ID(CALL_NTSAVEKEY);
    call.KeyHandle = (ULONG_PTR)KeyHandle;
    call.FileHandle = (ULONG_PTR)FileHandle;

    qemu_syscall(&call.super);

    return call.super.iret;
}

#else

void qemu_NtSaveKey(struct qemu_syscall *call)
{
    struct qemu_NtSaveKey *c = (struct qemu_NtSaveKey *)call;
    WINE_FIXME("Unverified!\n");
    c->super.iret = NtSaveKey(QEMU_G2H(c->KeyHandle), QEMU_G2H(c->FileHandle));
}

#endif

struct qemu_NtSetInformationKey
{
    struct qemu_syscall super;
    uint64_t KeyHandle;
    uint64_t KeyInformationClass;
    uint64_t KeyInformation;
    uint64_t KeyInformationLength;
};

#ifdef QEMU_DLL_GUEST

#if __MINGW64_VERSION_MAJOR < 7
WINBASEAPI NTSTATUS WINAPI NtSetInformationKey(HANDLE KeyHandle, const int KeyInformationClass, PVOID KeyInformation, ULONG KeyInformationLength)
#else
WINBASEAPI NTSTATUS NTAPI NtSetInformationKey(HANDLE KeyHandle, KEY_SET_INFORMATION_CLASS KeyInformationClass, PVOID KeyInformation, ULONG KeyInformationLength)
#endif
{
    struct qemu_NtSetInformationKey call;
    call.super.id = QEMU_SYSCALL_ID(CALL_NTSETINFORMATIONKEY);
    call.KeyHandle = (ULONG_PTR)KeyHandle;
    call.KeyInformationClass = (ULONG_PTR)KeyInformationClass;
    call.KeyInformation = (ULONG_PTR)KeyInformation;
    call.KeyInformationLength = (ULONG_PTR)KeyInformationLength;

    qemu_syscall(&call.super);

    return call.super.iret;
}

#else

void qemu_NtSetInformationKey(struct qemu_syscall *call)
{
    struct qemu_NtSetInformationKey *c = (struct qemu_NtSetInformationKey *)call;
    WINE_FIXME("Unverified!\n");
    c->super.iret = NtSetInformationKey(QEMU_G2H(c->KeyHandle), c->KeyInformationClass, QEMU_G2H(c->KeyInformation), c->KeyInformationLength);
}

#endif

struct qemu_NtSetValueKey
{
    struct qemu_syscall super;
    uint64_t hkey;
    uint64_t name;
    uint64_t TitleIndex;
    uint64_t type;
    uint64_t data;
    uint64_t count;
};

#ifdef QEMU_DLL_GUEST

WINBASEAPI NTSTATUS WINAPI NtSetValueKey(HANDLE hkey, const UNICODE_STRING *name, ULONG TitleIndex, ULONG type,
        const void *data, ULONG count)
{
    struct qemu_NtSetValueKey call;
    call.super.id = QEMU_SYSCALL_ID(CALL_NTSETVALUEKEY);
    call.hkey = (LONG_PTR)hkey;
    call.name = (ULONG_PTR)name;
    call.TitleIndex = (ULONG_PTR)TitleIndex;
    call.type = (ULONG_PTR)type;
    call.data = (ULONG_PTR)data;
    call.count = (ULONG_PTR)count;

    qemu_syscall(&call.super);

    return call.super.iret;
}

#else

void qemu_NtSetValueKey(struct qemu_syscall *call)
{
    struct qemu_NtSetValueKey *c = (struct qemu_NtSetValueKey *)call;
    UNICODE_STRING stack, *name = &stack;

    WINE_TRACE("\n");
#if GUEST_BIT == HOST_BIT
    name = QEMU_G2H(c->name);
#else
    if (QEMU_G2H(c->name))
        UNICODE_STRING_g2h(name, QEMU_G2H(c->name));
    else
        name = NULL;
#endif

    c->super.iret = NtSetValueKey(QEMU_G2H(c->hkey), name, c->TitleIndex,
            c->type, QEMU_G2H(c->data), c->count);
}

#endif

struct qemu_RtlpNtSetValueKey
{
    struct qemu_syscall super;
    uint64_t hkey;
    uint64_t type;
    uint64_t data;
    uint64_t count;
};

#ifdef QEMU_DLL_GUEST

WINBASEAPI NTSTATUS WINAPI RtlpNtSetValueKey(HANDLE hkey, ULONG type, const void *data, ULONG count)
{
    struct qemu_RtlpNtSetValueKey call;
    call.super.id = QEMU_SYSCALL_ID(CALL_RTLPNTSETVALUEKEY);
    call.hkey = (LONG_PTR)hkey;
    call.type = (ULONG_PTR)type;
    call.data = (ULONG_PTR)data;
    call.count = (ULONG_PTR)count;

    qemu_syscall(&call.super);

    return call.super.iret;
}

#else

/* TODO: Add RtlpNtSetValueKey to Wine headers? */
extern NTSTATUS WINAPI RtlpNtSetValueKey(HANDLE hkey, ULONG type, const void *data, ULONG count);
void qemu_RtlpNtSetValueKey(struct qemu_syscall *call)
{
    struct qemu_RtlpNtSetValueKey *c = (struct qemu_RtlpNtSetValueKey *)call;
    WINE_FIXME("Unverified!\n");
    c->super.iret = RtlpNtSetValueKey(QEMU_G2H(c->hkey), c->type, QEMU_G2H(c->data), c->count);
}

#endif

struct qemu_NtUnloadKey
{
    struct qemu_syscall super;
    uint64_t attr;
};

#ifdef QEMU_DLL_GUEST

WINBASEAPI NTSTATUS WINAPI NtUnloadKey(POBJECT_ATTRIBUTES attr)
{
    struct qemu_NtUnloadKey call;
    call.super.id = QEMU_SYSCALL_ID(CALL_NTUNLOADKEY);
    call.attr = (ULONG_PTR)attr;

    qemu_syscall(&call.super);

    return call.super.iret;
}

#else

void qemu_NtUnloadKey(struct qemu_syscall *call)
{
    struct qemu_NtUnloadKey *c = (struct qemu_NtUnloadKey *)call;
    WINE_FIXME("Unverified!\n");
    c->super.iret = NtUnloadKey(QEMU_G2H(c->attr));
}

#endif

struct qemu_RtlFormatCurrentUserKeyPath
{
    struct qemu_syscall super;
    uint64_t KeyPath;
};

#ifdef QEMU_DLL_GUEST

WINBASEAPI NTSTATUS WINAPI RtlFormatCurrentUserKeyPath(PUNICODE_STRING KeyPath)
{
    struct qemu_RtlFormatCurrentUserKeyPath call;
    call.super.id = QEMU_SYSCALL_ID(CALL_RTLFORMATCURRENTUSERKEYPATH);
    call.KeyPath = (ULONG_PTR)KeyPath;

    qemu_syscall(&call.super);

    return call.super.iret;
}

#else

void qemu_RtlFormatCurrentUserKeyPath(struct qemu_syscall *call)
{
    struct qemu_RtlFormatCurrentUserKeyPath *c = (struct qemu_RtlFormatCurrentUserKeyPath *)call;
    struct qemu_UNICODE_STRING *path32;
    UNICODE_STRING stack, *path = &stack;

    WINE_FIXME("Unverified!\n");
#if HOST_BIT == GUEST_BIT
    path = QEMU_G2H(c->KeyPath);
#else
    path32 = QEMU_G2H(c->KeyPath);
    if (!path32)
        path = NULL;
#endif

    c->super.iret = RtlFormatCurrentUserKeyPath(path);

#if HOST_BIT != GUEST_BIT
    if (c->super.iret == STATUS_SUCCESS)
        UNICODE_STRING_h2g(path32, path);
#endif
}

#endif

struct qemu_RtlOpenCurrentUser
{
    struct qemu_syscall super;
    uint64_t DesiredAccess;
    uint64_t KeyHandle;
};

#ifdef QEMU_DLL_GUEST

WINBASEAPI NTSTATUS WINAPI RtlOpenCurrentUser(ACCESS_MASK DesiredAccess, PHANDLE KeyHandle)
{
    struct qemu_RtlOpenCurrentUser call;
    call.super.id = QEMU_SYSCALL_ID(CALL_RTLOPENCURRENTUSER);
    call.DesiredAccess = DesiredAccess;
    call.KeyHandle = (ULONG_PTR)KeyHandle;

    qemu_syscall(&call.super);
    *KeyHandle = (HANDLE)(ULONG_PTR)call.KeyHandle;

    return call.super.iret;
}

#else

void qemu_RtlOpenCurrentUser(struct qemu_syscall *call)
{
    struct qemu_RtlOpenCurrentUser *c = (struct qemu_RtlOpenCurrentUser *)call;
    HANDLE key;

    WINE_TRACE("\n");
    c->super.iret = RtlOpenCurrentUser(c->DesiredAccess, &key);
    c->KeyHandle = (ULONG_PTR)key;
}

#endif

struct qemu_RtlQueryRegistryValues
{
    struct qemu_syscall super;
    uint64_t RelativeTo;
    uint64_t Path;
    uint64_t QueryTable;
    uint64_t Context;
    uint64_t Environment;
};

#ifdef QEMU_DLL_GUEST

WINBASEAPI NTSTATUS WINAPI RtlQueryRegistryValues(ULONG RelativeTo, PCWSTR Path, PRTL_QUERY_REGISTRY_TABLE QueryTable, PVOID Context, PVOID Environment)
{
    struct qemu_RtlQueryRegistryValues call;
    call.super.id = QEMU_SYSCALL_ID(CALL_RTLQUERYREGISTRYVALUES);
    call.RelativeTo = (ULONG_PTR)RelativeTo;
    call.Path = (ULONG_PTR)Path;
    call.QueryTable = (ULONG_PTR)QueryTable;
    call.Context = (ULONG_PTR)Context;
    call.Environment = (ULONG_PTR)Environment;

    qemu_syscall(&call.super);

    return call.super.iret;
}

#else

void qemu_RtlQueryRegistryValues(struct qemu_syscall *call)
{
    struct qemu_RtlQueryRegistryValues *c = (struct qemu_RtlQueryRegistryValues *)call;
    WINE_FIXME("Unverified!\n");
    c->super.iret = RtlQueryRegistryValues(c->RelativeTo, QEMU_G2H(c->Path), QEMU_G2H(c->QueryTable), QEMU_G2H(c->Context), QEMU_G2H(c->Environment));
}

#endif

struct qemu_RtlCheckRegistryKey
{
    struct qemu_syscall super;
    uint64_t RelativeTo;
    uint64_t Path;
};

#ifdef QEMU_DLL_GUEST

WINBASEAPI NTSTATUS WINAPI RtlCheckRegistryKey(ULONG RelativeTo, PWSTR Path)
{
    struct qemu_RtlCheckRegistryKey call;
    call.super.id = QEMU_SYSCALL_ID(CALL_RTLCHECKREGISTRYKEY);
    call.RelativeTo = (ULONG_PTR)RelativeTo;
    call.Path = (ULONG_PTR)Path;

    qemu_syscall(&call.super);

    return call.super.iret;
}

#else

void qemu_RtlCheckRegistryKey(struct qemu_syscall *call)
{
    struct qemu_RtlCheckRegistryKey *c = (struct qemu_RtlCheckRegistryKey *)call;
    WINE_FIXME("Unverified!\n");
    c->super.iret = RtlCheckRegistryKey(c->RelativeTo, QEMU_G2H(c->Path));
}

#endif

struct qemu_RtlDeleteRegistryValue
{
    struct qemu_syscall super;
    uint64_t RelativeTo;
    uint64_t Path;
    uint64_t ValueName;
};

#ifdef QEMU_DLL_GUEST

WINBASEAPI NTSTATUS WINAPI RtlDeleteRegistryValue(ULONG RelativeTo, PCWSTR Path, PCWSTR ValueName)
{
    struct qemu_RtlDeleteRegistryValue call;
    call.super.id = QEMU_SYSCALL_ID(CALL_RTLDELETEREGISTRYVALUE);
    call.RelativeTo = (ULONG_PTR)RelativeTo;
    call.Path = (ULONG_PTR)Path;
    call.ValueName = (ULONG_PTR)ValueName;

    qemu_syscall(&call.super);

    return call.super.iret;
}

#else

void qemu_RtlDeleteRegistryValue(struct qemu_syscall *call)
{
    struct qemu_RtlDeleteRegistryValue *c = (struct qemu_RtlDeleteRegistryValue *)call;
    WINE_FIXME("Unverified!\n");
    c->super.iret = RtlDeleteRegistryValue(c->RelativeTo, QEMU_G2H(c->Path), QEMU_G2H(c->ValueName));
}

#endif

struct qemu_RtlWriteRegistryValue
{
    struct qemu_syscall super;
    uint64_t RelativeTo;
    uint64_t path;
    uint64_t name;
    uint64_t type;
    uint64_t data;
    uint64_t length;
};

#ifdef QEMU_DLL_GUEST

WINBASEAPI NTSTATUS WINAPI RtlWriteRegistryValue(ULONG RelativeTo, PCWSTR path, PCWSTR name, ULONG type, PVOID data, ULONG length)
{
    struct qemu_RtlWriteRegistryValue call;
    call.super.id = QEMU_SYSCALL_ID(CALL_RTLWRITEREGISTRYVALUE);
    call.RelativeTo = (ULONG_PTR)RelativeTo;
    call.path = (ULONG_PTR)path;
    call.name = (ULONG_PTR)name;
    call.type = (ULONG_PTR)type;
    call.data = (ULONG_PTR)data;
    call.length = (ULONG_PTR)length;

    qemu_syscall(&call.super);

    return call.super.iret;
}

#else

void qemu_RtlWriteRegistryValue(struct qemu_syscall *call)
{
    struct qemu_RtlWriteRegistryValue *c = (struct qemu_RtlWriteRegistryValue *)call;
    WINE_FIXME("Unverified!\n");
    c->super.iret = RtlWriteRegistryValue(c->RelativeTo, QEMU_G2H(c->path), QEMU_G2H(c->name), c->type, QEMU_G2H(c->data), c->length);
}

#endif

struct qemu_NtQueryLicenseValue
{
    struct qemu_syscall super;
    uint64_t name;
    uint64_t result_type;
    uint64_t data;
    uint64_t length;
    uint64_t result_len;
};

#ifdef QEMU_DLL_GUEST

WINBASEAPI NTSTATUS WINAPI NtQueryLicenseValue(const UNICODE_STRING *name, ULONG *result_type, PVOID data, ULONG length, ULONG *result_len)
{
    struct qemu_NtQueryLicenseValue call;
    call.super.id = QEMU_SYSCALL_ID(CALL_NTQUERYLICENSEVALUE);
    call.name = (ULONG_PTR)name;
    call.result_type = (ULONG_PTR)result_type;
    call.data = (ULONG_PTR)data;
    call.length = (ULONG_PTR)length;
    call.result_len = (ULONG_PTR)result_len;

    qemu_syscall(&call.super);

    return call.super.iret;
}

#else

void qemu_NtQueryLicenseValue(struct qemu_syscall *call)
{
    struct qemu_NtQueryLicenseValue *c = (struct qemu_NtQueryLicenseValue *)call;
    WINE_FIXME("Unverified!\n");
    c->super.iret = NtQueryLicenseValue(QEMU_G2H(c->name), QEMU_G2H(c->result_type), QEMU_G2H(c->data), c->length, QEMU_G2H(c->result_len));
}

#endif

