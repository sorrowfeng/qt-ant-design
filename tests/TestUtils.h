#pragma once

// Shared test helpers used across the qt-ant-design test suite.
// Kept header-only so tests can include it without an extra translation unit.

#include <QCoreApplication>
#include <QElapsedTimer>
#include <QTest>

#include <functional>

namespace AntTestUtils
{

// Poll `predicate` until it returns true or `timeoutMs` elapses, processing
// events and yielding between checks. Returns the predicate's final value.
// The default timeout is deliberately short so failing conditions fail fast.
inline bool waitUntil(const std::function<bool()>& predicate, int timeoutMs = 700)
{
    QElapsedTimer timer;
    timer.start();
    while (timer.elapsed() < timeoutMs)
    {
        QCoreApplication::processEvents();
        if (predicate())
        {
            return true;
        }
        QTest::qWait(10);
    }
    QCoreApplication::processEvents();
    return predicate();
}

} // namespace AntTestUtils
