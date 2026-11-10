/*
 * Created by v1tr10l7 on 07.11.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include <Prism/Test/Test.hpp>

#include <errno.h>
#include <signal.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <unistd.h>

#ifdef __linux__
    #include <sys/prctl.h>
#endif

namespace Prism::Test
{
    bool Crash::RunImpl(StringView description, ThunkFn thunk, void* context,
                        int expectedSignal)
    {
        if (IsReportingEnabled()) Warnln("Testing \"{}\"", description);

        pid_t pid = fork();
        if (pid < 0)
        {
            Warnln("{}: fork() failed: {}", Details::FailTag(), strerror(errno));
            return false;
        }
        if (pid == 0)
        {
            // Child: no core dumps for the crashes we provoke on purpose.
            rlimit noCore{0, 0};
            setrlimit(RLIMIT_CORE, &noCore);
#ifdef __linux__
            // Also keeps piped core_pattern handlers (systemd-coredump) out.
            prctl(PR_SET_DUMPABLE, 0, 0, 0, 0);
#endif

            auto failure = thunk(context);
            _exit(failure == Failure::eDidNotCrash ? 0 : 2);
        }

        int status = 0;
        while (waitpid(pid, &status, 0) < 0)
            if (errno != EINTR) return false;

        bool        exited    = WIFEXITED(status);
        bool        signalled = WIFSIGNALED(status);
        int         signal    = signalled ? WTERMSIG(status) : 0;
        int         code      = exited ? WEXITSTATUS(status) : -1;

        const char* problem   = nullptr;
        if (expectedSignal == 0)
        {
            if (signalled) problem = "crashed";
            else if (code != 0) problem = "reported an unexpected error";
        }
        else if (!signalled)
            problem = code == 0 ? "did not crash" : "reported an unexpected error";
        else if (expectedSignal != AnySignal && signal != expectedSignal)
            problem = "crashed with the wrong signal";

        if (!problem) return true;
        if (!IsReportingEnabled()) return false;

        if (signalled)
            Warnln("{}: \"{}\" {} (signal {}: {}, expected {})",
                   Details::FailTag(), description, problem, signal,
                   strsignal(signal), expectedSignal);
        else
            Warnln("{}: \"{}\" {} (exit code {})", Details::FailTag(),
                   description, problem, code);
        return false;
    }
} // namespace Prism::Test
