#pragma once

#include <QString>

#include "core/AntTypes.h"
#include "widgets/AntTypography.h"

class QWidget;
class QVBoxLayout;
class AntCard;

namespace example::pages
{
QWidget* wrapPage(QWidget* page);

// Uniform page scaffold: a bare QWidget with a VBox layout using the shared
// margins (32, 24, 32, 24) and spacing (16) that every example page repeats.
// Use structured bindings: `auto [page, layout] = makePage();`
struct PageScaffold
{
    QWidget* page = nullptr;
    QVBoxLayout* layout = nullptr;
};
PageScaffold makePage();

// Titled AntCard already added to `layout`. Returns the card so the caller can
// fill `card->bodyLayout()`; replaces the repeated
// `new AntCard(title)` + `layout->addWidget(card)` pair.
AntCard* makeCard(QVBoxLayout* layout, const QString& title);

AntTypography* makeText(const QString& text, QWidget* parent = nullptr,
                        Ant::TypographyType type = Ant::TypographyType::Default);
AntTypography* makeSecondaryText(const QString& text, QWidget* parent = nullptr);
AntTypography* makeParagraph(const QString& text, QWidget* parent = nullptr,
                             Ant::TypographyType type = Ant::TypographyType::Default);
}
