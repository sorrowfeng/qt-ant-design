#pragma once

#include <QString>

class AntTypography;
class QScrollArea;
class QWidget;

namespace example::pages
{
QScrollArea* wrapPage(QWidget* page);
AntTypography* createSectionTitle(const QString& title);
}
