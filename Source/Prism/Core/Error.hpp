/*
 * Created by v1tr10l7 on 15.07.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Core/Details/Conditional.inl>
#include <Prism/Utility/Expected.hpp>

#if PRISM_TARGET_CRYPTIX != 0
    #include <cerrno>
#endif

namespace Prism
{
#ifndef Stringify
    #define Stringify(x) PrismStringify(x)
#endif

#if PRISM_TARGET_CRYPTIX != 0
    using PosixErrorCode = std::errno_t;
#else
    using PosixErrorCode = isize;
#endif
    // enum PosixErrorCode
    // {
    //     EPERM           = 1,
    //     ENOENT          = 2,
    //     ESRCH           = 3,
    //     EINTR           = 4,
    //     EIO             = 5,
    //     ENXIO           = 6,
    //     E2BIG           = 7,
    //     ENOEXEC         = 8,
    //     EBADF           = 9,
    //     ECHILD          = 10,
    //     EAGAIN          = 11,
    //     ENOMEM          = 12,
    //     EACCES          = 13,
    //     EFAULT          = 14,
    //     ENOTBLK         = 15,
    //     EBUSY           = 16,
    //     EEXIST          = 17,
    //     EXDEV           = 18,
    //     ENODEV          = 19,
    //     ENOTDIR         = 20,
    //     EISDIR          = 21,
    //     EINVAL          = 22,
    //     ENFILE          = 23,
    //     EMFILE          = 24,
    //     ENOTTY          = 25,
    //     ETXTBSY         = 26,
    //     EFBIG           = 27,
    //     ENOSPC          = 28,
    //     ESPIPE          = 29,
    //     EROFS           = 30,
    //     EMLINK          = 31,
    //     EPIPE           = 32,
    //     EDOM            = 33,
    //     ERANGE          = 34,
    //     EDEADLK         = 35,
    //     ENAMETOOLONG    = 36,
    //     ENOLCK          = 37,
    //     ENOSYS          = 38,
    //     ENOTEMPTY       = 39,
    //     ELOOP           = 40,
    //     EWOULDBLOCK     = EAGAIN,
    //     ENOMSG          = 42,
    //     EIDRM           = 43,
    //     ECHRNG          = 44,
    //     EL2NSYNC        = 45,
    //     EL3HLT          = 46,
    //     EL3RST          = 47,
    //     ELNRNG          = 48,
    //     EUNATCH         = 49,
    //     ENOCSI          = 50,
    //     EL2HLT          = 51,
    //     EBADE           = 52,
    //     EBADR           = 53,
    //     EXFULL          = 54,
    //     ENOANO          = 55,
    //     EBADRQC         = 56,
    //     EBADSLT         = 57,
    //     EDEADLOCK       = EDEADLK,
    //     EBFONT          = 59,
    //     ENOSTR          = 60,
    //     ENODATA         = 61,
    //     ETIME           = 62,
    //     ENOSR           = 63,
    //     ENONET          = 64,
    //     ENOPKG          = 65,
    //     EREMOTE         = 66,
    //     ENOLINK         = 67,
    //     EADV            = 68,
    //     ESRMNT          = 69,
    //     ECOMM           = 70,
    //     EPROTO          = 71,
    //     EMULTIHOP       = 72,
    //     EDOTDOT         = 73,
    //     EBADMSG         = 74,
    //     EOVERFLOW       = 75,
    //     ENOTUNIQ        = 76,
    //     EBADFD          = 77,
    //     EREMCHG         = 78,
    //     ELIBACC         = 79,
    //     ELIBBAD         = 80,
    //     ELIBSCN         = 81,
    //     ELIBMAX         = 82,
    //     ELIBEXEC        = 83,
    //     EILSEQ          = 84,
    //     ERESTART        = 85,
    //     ESTRPIPE        = 86,
    //     EUSERS          = 87,
    //     ENOTSOCK        = 88,
    //     EDESTADDRREQ    = 89,
    //     EMSGSIZE        = 90,
    //     EPROTOTYPE      = 91,
    //     ENOPROTOOPT     = 92,
    //     EPROTONOSUPPORT = 93,
    //     ESOCKTNOSUPPORT = 94,
    //     EOPNOTSUPP      = 95,
    //     ENOTSUP         = EOPNOTSUPP,
    //     EPFNOSUPPORT    = 96,
    //     EAFNOSUPPORT    = 97,
    //     EADDRINUSE      = 98,
    //     EADDRNOTAVAIL   = 99,
    //     ENETDOWN        = 100,
    //     ENETUNREACH     = 101,
    //     ENETRESET       = 102,
    //     ECONNABORTED    = 103,
    //     ECONNRESET      = 104,
    //     ENOBUFS         = 105,
    //     EISCONN         = 106,
    //     ENOTCONN        = 107,
    //     ESHUTDOWN       = 108,
    //     ETOOMANYREFS    = 109,
    //     ETIMEDOUT       = 110,
    //     ECONNREFUSED    = 111,
    //     EHOSTDOWN       = 112,
    //     EHOSTUNREACH    = 113,
    //     EALREADY        = 114,
    //     EINPROGRESS     = 115,
    //     ESTALE          = 116,
    //     EUCLEAN         = 117,
    //     ENOTNAM         = 118,
    //     ENAVAIL         = 119,
    //     EISNAM          = 120,
    //     EREMOTEIO       = 121,
    //     EDQUOT          = 122,
    //     ENOMEDIUM       = 123,
    //     EMEDIUMTYPE     = 124,
    //     ECANCELED       = 125,
    //     ENOKEY          = 126,
    //     EKEYEXPIRED     = 127,
    //     EKEYREVOKED     = 128,
    //     EKEYREJECTED    = 129,
    //     EOWNERDEAD      = 130,
    //     ENOTRECOVERABLE = 131,
    //     ERFKILL         = 132,
    //     EHWPOISON       = 133
    // };

    using ErrorCode = PosixErrorCode;
    using Error     = Unexpected<ErrorCode>;

    template <typename R>
    using ErrorOr = Expected<R, ErrorCode>;
}; // namespace Prism

#define PM_TryOrRetFmt(expr, return_value, ...)                                \
    ({                                                                         \
        auto result = (expr);                                                  \
        if (!result)                                                           \
        {                                                                      \
            LogError(fmt::format(__VA_ARGS__).data());                         \
            return (return_value);                                             \
        }                                                                      \
        Prism::Move(result.Value());                                           \
    })

#define PM_TryOrRetMsg(expr, return_value)                                     \
    PM_TryOrRetFmt(expr, return_value, msg)
#define PM_TryOrRetVal(expr, return_value)                                     \
    ({                                                                         \
        auto result = (expr);                                                  \
        if (!result) return (return_value);                                    \
        Prism::Move(result.Value());                                           \
    })
#define PM_TryOrRetCode(expr, code_on_fail)                                    \
    PM_TryOrRetVal(expr, Error(code_on_fail))
#define PM_TryOrRet(expr) PM_TryOrRetCode(expr, result.Error())
#define PM_TryAcquire(expr)                                                    \
    ({                                                                         \
        auto result = (expr);                                                  \
        if (!result) return;                                                   \
        Prism::Move(result.Value());                                           \
    })
#define PM_Try(expr)                                                           \
    ({                                                                         \
        auto result = (expr);                                                  \
        if (!result) return;                                                   \
    })

#if PRISM_TARGET_CRYPTIX != 0
using Prism::Error;
using Prism::ErrorCode;
using Prism::ErrorOr;
    #define TryOrRetFmt(expr, return_value, ...)                               \
        PM_TryOrRetFmt(expr, return_value, __VA_ARGS__)
    #define TryOrRetMsg(expr, return_value, msg)                               \
        PM_TryOrRetMsg(expr, return_value, msg)
    #define TryOrRetVal(expr, val)           PM_TryOrRetVal(expr, val)
    #define TryOrRetCode(expr, code_on_fail) PM_TryOrRetCode(expr, code_on_fail)
    #define TryOrRet(expr)                   PM_TryOrRet(expr)
    #define TryAcquire(expr)                 PM_TryAcquire(expr)
    #define Try(expr)                        PM_Try(expr)
#endif
