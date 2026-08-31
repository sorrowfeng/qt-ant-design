#include "AntLocale.h"

AntLocale* AntLocale::instance()
{
    static AntLocale locale;
    return &locale;
}

AntLocale::AntLocale(QObject* parent)
    : QObject(parent)
{
    rebuildTable();
}

Ant::LocaleLanguage AntLocale::language() const { return m_language; }

void AntLocale::setLanguage(Ant::LocaleLanguage language)
{
    if (m_language == language)
    {
        return;
    }
    m_language = language;
    rebuildTable();
    Q_EMIT languageChanged(m_language);
}

QString AntLocale::text(const QString& key) const
{
    return m_table.value(key);
}

QStringList AntLocale::keys() const
{
    return m_table.keys();
}

void AntLocale::rebuildTable()
{
    m_table.clear();
    if (m_language == Ant::LocaleLanguage::ChineseSimplified)
    {
        m_table.insert(QStringLiteral("Modal.okText"), QStringLiteral("确定"));
        m_table.insert(QStringLiteral("Modal.cancelText"), QStringLiteral("取消"));
        m_table.insert(QStringLiteral("Popconfirm.okText"), QStringLiteral("确定"));
        m_table.insert(QStringLiteral("Popconfirm.cancelText"), QStringLiteral("取消"));
        m_table.insert(QStringLiteral("Table.emptyText"), QStringLiteral("暂无数据"));
        m_table.insert(QStringLiteral("Pagination.itemsPerPage"), QStringLiteral("条/页"));
        m_table.insert(QStringLiteral("Pagination.jumpTo"), QStringLiteral("跳至"));
    }
    else
    {
        m_table.insert(QStringLiteral("Modal.okText"), QStringLiteral("OK"));
        m_table.insert(QStringLiteral("Modal.cancelText"), QStringLiteral("Cancel"));
        m_table.insert(QStringLiteral("Popconfirm.okText"), QStringLiteral("OK"));
        m_table.insert(QStringLiteral("Popconfirm.cancelText"), QStringLiteral("Cancel"));
        m_table.insert(QStringLiteral("Table.emptyText"), QStringLiteral("No data"));
        m_table.insert(QStringLiteral("Pagination.itemsPerPage"), QStringLiteral("/ page"));
        m_table.insert(QStringLiteral("Pagination.jumpTo"), QStringLiteral("Go to"));
    }
}
