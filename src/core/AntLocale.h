#pragma once

#include "QtAntDesignExport.h"

#include <QHash>
#include <QObject>
#include <QString>
#include <QStringList>

#include "AntTypes.h"

// 内建文案本地化（对应上游 LocaleProvider）。
// 当前覆盖 Modal / Popconfirm / Table 等组件的内建按钮与空态文案，
// 通过 text("Modal.okText") 形式的键访问；setLanguage 后组件自动跟随。
class QT_ANT_DESIGN_EXPORT AntLocale : public QObject
{
    Q_OBJECT
    Q_PROPERTY(Ant::LocaleLanguage language READ language WRITE setLanguage NOTIFY languageChanged)

public:
    static AntLocale* instance();

    Ant::LocaleLanguage language() const;
    void setLanguage(Ant::LocaleLanguage language);

    // 按 "组件.键" 取内建文案，例如 "Modal.okText"。未知键返回空字符串。
    QString text(const QString& key) const;
    QStringList keys() const;

Q_SIGNALS:
    void languageChanged(Ant::LocaleLanguage language);

private:
    explicit AntLocale(QObject* parent = nullptr);
    void rebuildTable();

    Ant::LocaleLanguage m_language = Ant::LocaleLanguage::English;
    QHash<QString, QString> m_table;
};

#define antLocale AntLocale::instance()
