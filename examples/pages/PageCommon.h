#pragma once

#include <QString>

#include "core/AntTypes.h"
#include "widgets/AntTypography.h"

class QScrollArea;
class QWidget;

namespace example::pages
{
QScrollArea* wrapPage(QWidget* page);
AntTypography* makeText(const QString& text, QWidget* parent = nullptr,
                        Ant::TypographyType type = Ant::TypographyType::Default);
AntTypography* makeSecondaryText(const QString& text, QWidget* parent = nullptr);
AntTypography* makeParagraph(const QString& text, QWidget* parent = nullptr,
                             Ant::TypographyType type = Ant::TypographyType::Default);
}
