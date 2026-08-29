#pragma once

#include "core/QtAntDesignExport.h"

#include <QByteArray>
#include <QString>
#include <QWidget>

#include <type_traits>

namespace AntWindowFrame
{
constexpr int LegacySoftwareShadowMargin = 14;
constexpr int LegacySoftwareShadowInnerClearance = 0;

struct NativeFrameOptions
{
    const char* forceLegacyFramePolicyProperty = nullptr;
    const char* usesNativeCaptionFrameProperty = nullptr;
    const char* dwmFrameMarginsProperty = nullptr;
    const char* dwmFrameApplyCountProperty = nullptr;
    const char* dwmFrameLastReasonProperty = nullptr;
    const char* legacyRoundedMaskAppliedProperty = nullptr;
    const char* legacyRoundedMaskFrameInsetProperty = nullptr;
    const char* legacyClassDropShadowEnabledProperty = nullptr;
    int cornerRadius = 8;
    bool translucentBackground = true;
    bool maximized = false;
    bool fullScreen = false;
    bool enableNativeCaption = true;
    bool enableMinimizeBox = true;
    bool enableMaximizeBox = true;
};

struct LegacySoftwareShadowOptions;
enum class LegacySoftwareShadowResult;
class QT_ANT_DESIGN_EXPORT LegacySoftwareShadowHandle;

QT_ANT_DESIGN_EXPORT LegacySoftwareShadowResult updateLegacySoftwareShadow(
    QWidget* owner,
    LegacySoftwareShadowHandle& shadowHandle,
    const LegacySoftwareShadowOptions& options);
QT_ANT_DESIGN_EXPORT LegacySoftwareShadowResult hideLegacySoftwareShadow(
    QWidget* owner,
    LegacySoftwareShadowHandle& shadowHandle,
    const QByteArray& enabledProperty,
    const QByteArray& clickThroughProperty);

// AntWindowFrame is installed as a low-level window integration API because
// AntWindow and AntDialog both expose behavior implemented by it.  The shadow
// itself is deliberately represented by a pointer-sized opaque handle: callers
// can inspect the generated QWidget, but cannot replace the handle's internal
// object with an unrelated QWidget.  Its trivial destructor preserves the
// historical AntWindow/AntDialog private-slot ABI while every access is checked
// against the library's private live-shadow registry.
class QT_ANT_DESIGN_EXPORT LegacySoftwareShadowHandle final
{
public:
    LegacySoftwareShadowHandle() noexcept = default;
    ~LegacySoftwareShadowHandle() = default;

    LegacySoftwareShadowHandle(const LegacySoftwareShadowHandle&) = delete;
    LegacySoftwareShadowHandle& operator=(const LegacySoftwareShadowHandle&) = delete;
    LegacySoftwareShadowHandle(LegacySoftwareShadowHandle&&) = delete;
    LegacySoftwareShadowHandle& operator=(LegacySoftwareShadowHandle&&) = delete;

    bool isNull() const noexcept;
    // Diagnostic escape hatch. Deleting the returned widget is tolerated and
    // the next update recreates it; reparenting it or changing native window
    // flags is unsupported because those operations break the owner contract.
    QWidget* widget() const noexcept;

private:
    QWidget* m_widget = nullptr;

    friend LegacySoftwareShadowResult updateLegacySoftwareShadow(
        QWidget*, LegacySoftwareShadowHandle&, const LegacySoftwareShadowOptions&);
    friend LegacySoftwareShadowResult hideLegacySoftwareShadow(
        QWidget*, LegacySoftwareShadowHandle&, const QByteArray&, const QByteArray&);
};

static_assert(sizeof(LegacySoftwareShadowHandle) == sizeof(QWidget*),
              "LegacySoftwareShadowHandle must preserve the historical private pointer slot size.");
static_assert(alignof(LegacySoftwareShadowHandle) == alignof(QWidget*),
              "LegacySoftwareShadowHandle must preserve the historical private pointer slot alignment.");
static_assert(std::is_trivially_destructible<LegacySoftwareShadowHandle>::value,
              "LegacySoftwareShadowHandle must preserve the historical trivial member destruction ABI.");

struct LegacySoftwareShadowOptions
{
    QString objectName;
    QByteArray enabledProperty;
    QByteArray marginProperty;
    QByteArray innerClearanceProperty;
    QByteArray geometryProperty;
    QByteArray geometryModeProperty;
    QByteArray devicePixelRatioProperty;
    QByteArray clickThroughProperty;
    bool enabled = false;
    int cornerRadius = 0;
};

enum class LegacySoftwareShadowResult
{
    Updated,
    Hidden,
    InvalidOwner,
    InvalidShadowWidget,
    OwnerMismatch,
    UnsupportedPlatform
};

QT_ANT_DESIGN_EXPORT bool legacyFramePolicyEnabled(const QWidget* widget, const char* forcePropertyName);
QT_ANT_DESIGN_EXPORT bool supportsNativeCaptionFrame();
QT_ANT_DESIGN_EXPORT bool usesNativeCaptionFrameForWidget(const QWidget* widget, const char* forcePropertyName);
QT_ANT_DESIGN_EXPORT void applyNativeFrame(QWidget* widget, const NativeFrameOptions& options);
// Compatibility path for code compiled against the original QWidget* based
// helper.  Unlike the historical implementation, this function validates that
// a non-null pointer is a library-created shadow belonging to owner and returns
// an explicit rejection result instead of casting arbitrary QWidget objects.
QT_ANT_DESIGN_EXPORT LegacySoftwareShadowResult tryUpdateLegacySoftwareShadow(
    QWidget* owner,
    QWidget*& shadowWidget,
    const QString& objectName,
    const char* enabledProperty,
    const char* marginProperty,
    const char* innerClearanceProperty,
    const char* geometryProperty,
    const char* geometryModeProperty,
    const char* dprProperty,
    const char* clickThroughProperty,
    bool enabled,
    int cornerRadius);
QT_ANT_DESIGN_EXPORT void updateLegacySoftwareShadow(QWidget* owner,
                                                     QWidget*& shadowWidget,
                                                     const QString& objectName,
                                                     const char* enabledProperty,
                                                     const char* marginProperty,
                                                     const char* innerClearanceProperty,
                                                     const char* geometryProperty,
                                                     const char* geometryModeProperty,
                                                     const char* dprProperty,
                                                     const char* clickThroughProperty,
                                                     bool enabled,
                                                     int cornerRadius);
QT_ANT_DESIGN_EXPORT void hideLegacySoftwareShadow(QWidget* owner,
                                                  QWidget* shadowWidget,
                                                  const char* enabledProperty,
                                                  const char* clickThroughProperty);
} // namespace AntWindowFrame
