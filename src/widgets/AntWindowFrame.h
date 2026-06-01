#pragma once

#include "core/QtAntDesignExport.h"

#include <QString>
#include <QWidget>

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

QT_ANT_DESIGN_EXPORT bool legacyFramePolicyEnabled(const QWidget* widget, const char* forcePropertyName);
QT_ANT_DESIGN_EXPORT bool supportsNativeCaptionFrame();
QT_ANT_DESIGN_EXPORT bool usesNativeCaptionFrameForWidget(const QWidget* widget, const char* forcePropertyName);
QT_ANT_DESIGN_EXPORT void applyNativeFrame(QWidget* widget, const NativeFrameOptions& options);
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
