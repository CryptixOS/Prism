/*
 * Created by v1tr10l7 on 02.09.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include <Prism/Test/Test.hpp>
#include <Prism/Debug/Log.hpp>
#include <Prism/Debug/Logger.hpp>

using namespace Prism;

using namespace Prism::Literals;
#include <mutex>
struct Policy
{
    std::mutex  Mutex;

    inline void Lock() { Mutex.lock(); }
    inline void Unlock() { Mutex.unlock(); }
};

TEST_CASE(log_levels)
{
    Log::Log(LogLevel::eTrace, "Hello");
    Log::Log(LogLevel::eInfo, "Hello");
    Log::Log(LogLevel::eDebug, "Hello");
    Log::Log(LogLevel::eWarn, "Hello");
    Log::Log(LogLevel::eError, "Hello");
    Log::Log(LogLevel::eFatal, "Hello");

    Logger<Policy> logger("TestLogger");
    logger.Log(LogLevel::eDebug, "TestPayload");
}

struct CaptureSink final : public LogSinkBase
{
    String   LastPayload;
    String   LastTag;
    LogLevel LastLevel = LogLevel::eNone;
    usize    Count     = 0;

    void     Log(const LogMessage& message) override
    {
        LastPayload = message.Payload;
        LastTag     = message.Tag;
        LastLevel   = message.Level;
        ++Count;
    }
    void Flush() override {}
    void SetPattern(StringView) override {}
};

static_assert(IsSameV<LoggerFormatString<int>,
#if PRISM_LOGGER_USE_PRISM_FORMAT != 0
                      FormatString<int>
#else
                      fmt::format_string<int>
#endif
                      >);

TEST_CASE(logger_formats_payload)
{
    auto           sinkRef = Ref<CaptureSink>::Create();
    CaptureSink&   sink    = *sinkRef;
    Logger<Policy> logger("TestLogger");
    logger.AddSink(&sink);

    logger.Info("{} + {} = {}", 1, 2, 3);
    EXPECT_EQ(sink.LastPayload, "1 + 2 = 3"_s);
    EXPECT_EQ(sink.LastLevel, LogLevel::eInfo);
    EXPECT(sink.LastTag.Empty());

    logger.Warn("{:#x} {:>5}|", 255u, "ab");
    EXPECT_EQ(sink.LastPayload, "0xff    ab|"_s);

    logger.ErrorTag("VFS", "open({}) failed", StringView("/etc"));
    EXPECT_EQ(sink.LastPayload, "open(/etc) failed"_s);
    EXPECT_EQ(sink.LastTag, "VFS"_s);
    EXPECT_EQ(sink.LastLevel, LogLevel::eError);

    logger.Log(LogLevel::eInfo, "plain");
    EXPECT_EQ(sink.LastPayload, "plain"_s);

    logger.RemoveAllSinks();
}

TEST_CASE(logger_filters_before_formatting)
{
    auto           sinkRef = Ref<CaptureSink>::Create();
    CaptureSink&   sink    = *sinkRef;
    Logger<Policy> logger("TestLogger");
    logger.AddSink(&sink);
    logger.SetLogLevelFilter(LogLevel::eWarn);

    logger.Debug("{}", 1);
    logger.InfoTag("tag", "{}", 2);
    EXPECT_EQ(sink.Count, usize(0));
    logger.Fatal("{}", 3);
    EXPECT_EQ(sink.Count, usize(1));
    EXPECT_EQ(sink.LastPayload, "3"_s);

    logger.RemoveAllSinks();
}
