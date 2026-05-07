#pragma once

#include "core/QtAntDesignExport.h"

#include "core/AntStyleBase.h"

struct AntUploadFile;

class QT_ANT_DESIGN_EXPORT AntUploadStyle : public AntStyleBase
{
    Q_OBJECT

public:
    explicit AntUploadStyle(QStyle* style = nullptr);
    ~AntUploadStyle() override = default;

    void polish(QWidget* widget) override;
    void unpolish(QWidget* widget) override;
    void drawPrimitive(PrimitiveElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget = nullptr) const override;
    QSize sizeFromContents(ContentsType type, const QStyleOption* option, const QSize& size, const QWidget* widget = nullptr) const override;
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    void drawUpload(const QStyleOption* option, QPainter* painter, const QWidget* widget) const;

    void drawTriggerArea(QPainter* painter, const QRect& rect, bool hovered, bool disabled, bool dragger = false) const;
    void drawTextFileItem(QPainter* painter, const QRect& itemRect, const AntUploadFile& file,
                          bool hovered, bool removeHovered) const;
    void drawPictureFileItem(QPainter* painter, const QRect& itemRect, const AntUploadFile& file,
                             bool hovered) const;
    void drawPictureCardItem(QPainter* painter, const QRect& cardRect, const AntUploadFile& file,
                             bool hovered) const;

    void drawProgressBar(QPainter* painter, const QRect& rect, int percent) const;
    void drawCheckIcon(QPainter* painter, const QPoint& center, int size) const;
    void drawErrorIcon(QPainter* painter, const QPoint& center, int size) const;
    void drawCloseIcon(QPainter* painter, const QPoint& center, int size, const QColor& color) const;
    void drawPlusIcon(QPainter* painter, const QPoint& center, int size, const QColor& color) const;
    void drawUploadIcon(QPainter* painter, const QPoint& center, int size, const QColor& color) const;
    void drawInboxIcon(QPainter* painter, const QPoint& center, int size, const QColor& color) const;
    void drawPaperClipIcon(QPainter* painter, const QPoint& center, int size, const QColor& color) const;
    void drawLoadingIcon(QPainter* painter, const QPoint& center, int size, const QColor& color) const;
    void drawFileIcon(QPainter* painter, const QRect& rect, const QColor& color) const;
    void drawEyeIcon(QPainter* painter, const QPoint& center, int size, const QColor& color) const;
    void drawDeleteIcon(QPainter* painter, const QPoint& center, int size, const QColor& color) const;

    static QString fileIconText(const QString& fileName);
    static QString truncatedName(const QString& name, int maxWidth, const QFontMetrics& fm);
};
