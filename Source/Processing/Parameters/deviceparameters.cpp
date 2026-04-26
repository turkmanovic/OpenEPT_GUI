#include "deviceparameters.h"

#include <algorithm>

DeviceParameters::DeviceParameters(QObject *parent)
    : QObject(parent)
{
    initGroupMeta();
    initSubGroupMeta();
    initParams();
}

void DeviceParameters::initGroupMeta()
{
    m_groupMeta.clear();

    const auto groups = DeviceParams::defaultGroupMeta();
    for (const auto &group : groups)
    {
        m_groupMeta.insert(group.id, group);
    }
}

void DeviceParameters::initSubGroupMeta()
{
    m_subGroupMeta.clear();

    const auto subGroups = DeviceParams::defaultSubGroupMeta();
    for (const auto &subGroup : subGroups)
    {
        m_subGroupMeta.insert(subGroup.id, subGroup);
    }
}

void DeviceParameters::initParams()
{
    m_params.clear();

    const auto params = DeviceParams::defaultParams();
    for (const auto &param : params)
    {
        m_params.insert(param.meta.key, param);
    }
}

bool DeviceParameters::hasParam(const QString &key) const
{
    return m_params.contains(key);
}

QString DeviceParameters::getParamValue(const QString &key) const
{
    if (!m_params.contains(key))
        return QString();

    const QVariant value = m_params.value(key).value;

    if (value.type() == QVariant::Bool)
        return value.toBool() ? "true" : "false";

    return value.toString();
}

QVariant DeviceParameters::getParamVariant(const QString &key) const
{
    if (!m_params.contains(key))
        return QVariant();

    return m_params.value(key).value;
}

bool DeviceParameters::setParamValue(const QString &key, const QString &value)
{
    if (!m_params.contains(key))
        return false;

    return setParamVariant(key, value);
}
bool DeviceParameters::isInitialized(const QString &key) const
{
    if (!m_params.contains(key))
        return false;

    return m_params.value(key).initialized;
}

void DeviceParameters::setParamInitialized(const QString &key, bool initialized)
{
    if (!m_params.contains(key))
        return;

    m_params[key].initialized = initialized;

    emit paramChanged(key, getParamValue(key));
    emit paramVariantChanged(key, m_params[key].value);
}
bool DeviceParameters::setParamVariant(const QString &key, const QVariant &value)
{
    if (!m_params.contains(key))
        return false;

    DeviceParams::Param &param = m_params[key];

    bool normalizeOk = false;
    QVariant normalizedValue = normalizeValue(param.meta, value, &normalizeOk);

    if (!normalizeOk)
        return false;

    if (!validateValue(param.meta, normalizedValue))
        return false;

    if (param.value == normalizedValue && param.initialized!=false)
        return true;

    param.value = normalizedValue;
    param.initialized = true;

    emit paramChanged(key, getParamValue(key));
    emit paramVariantChanged(key, param.value);

    return true;
}

DeviceParams::ParamMeta DeviceParameters::getParamMeta(const QString &key) const
{
    if (!m_params.contains(key))
        return DeviceParams::ParamMeta();

    return m_params.value(key).meta;
}

DeviceParams::Param DeviceParameters::getParam(const QString &key) const
{
    if (!m_params.contains(key))
        return DeviceParams::Param();

    return m_params.value(key);
}

QList<DeviceParams::Param> DeviceParameters::getAllParams() const
{
    QList<DeviceParams::Param> result = m_params.values();

    std::sort(result.begin(), result.end(),
              [](const DeviceParams::Param &a, const DeviceParams::Param &b)
              {
                  if (a.meta.group != b.meta.group)
                      return static_cast<int>(a.meta.group) < static_cast<int>(b.meta.group);

                  if (a.meta.subGroup != b.meta.subGroup)
                      return static_cast<int>(a.meta.subGroup) < static_cast<int>(b.meta.subGroup);

                  return a.meta.order < b.meta.order;
              });

    return result;
}

QList<DeviceParams::Param> DeviceParameters::getParamsByGroup(DeviceParams::Group group) const
{
    QList<DeviceParams::Param> result;

    for (const auto &param : m_params)
    {
        if (param.meta.group == group)
            result.append(param);
    }

    std::sort(result.begin(), result.end(),
              [](const DeviceParams::Param &a, const DeviceParams::Param &b)
              {
                  if (a.meta.subGroup != b.meta.subGroup)
                      return static_cast<int>(a.meta.subGroup) < static_cast<int>(b.meta.subGroup);

                  return a.meta.order < b.meta.order;
              });

    return result;
}

QList<DeviceParams::Param> DeviceParameters::getParamsBySubGroup(DeviceParams::Group group,
                                                                 DeviceParams::SubGroup subGroup) const
{
    QList<DeviceParams::Param> result;

    for (const auto &param : m_params)
    {
        if ((param.meta.group == group) && (param.meta.subGroup == subGroup))
            result.append(param);
    }

    std::sort(result.begin(), result.end(),
              [](const DeviceParams::Param &a, const DeviceParams::Param &b)
              {
                  return a.meta.order < b.meta.order;
              });

    return result;
}

DeviceParams::GroupMeta DeviceParameters::getGroupMeta(DeviceParams::Group group) const
{
    return m_groupMeta.value(group);
}

DeviceParams::SubGroupMeta DeviceParameters::getSubGroupMeta(DeviceParams::SubGroup subGroup) const
{
    return m_subGroupMeta.value(subGroup);
}

QList<DeviceParams::GroupMeta> DeviceParameters::getAllGroupMeta() const
{
    QList<DeviceParams::GroupMeta> result = m_groupMeta.values();

    std::sort(result.begin(), result.end(),
              [](const DeviceParams::GroupMeta &a, const DeviceParams::GroupMeta &b)
              {
                  return a.order < b.order;
              });

    return result;
}

QList<DeviceParams::SubGroupMeta> DeviceParameters::getAllSubGroupMeta() const
{
    QList<DeviceParams::SubGroupMeta> result = m_subGroupMeta.values();

    std::sort(result.begin(), result.end(),
              [](const DeviceParams::SubGroupMeta &a, const DeviceParams::SubGroupMeta &b)
              {
                  return a.order < b.order;
              });

    return result;
}

bool DeviceParameters::isEditable(const QString &key) const
{
    if (!m_params.contains(key))
        return false;

    return m_params.value(key).meta.access == DeviceParams::Access::ReadWrite;
}

bool DeviceParameters::isVisible(const QString &key) const
{
    if (!m_params.contains(key))
        return false;

    return m_params.value(key).meta.visible;
}

bool DeviceParameters::isSavable(const QString &key) const
{
    if (!m_params.contains(key))
        return false;

    const auto storage = m_params.value(key).meta.storage;

    return storage == DeviceParams::Storage::SaveOnly ||
           storage == DeviceParams::Storage::LoadSave;
}

bool DeviceParameters::isLoadable(const QString &key) const
{
    if (!m_params.contains(key))
        return false;

    return m_params.value(key).meta.storage == DeviceParams::Storage::LoadSave;
}

bool DeviceParameters::isRuntimeOnly(const QString &key) const
{
    if (!m_params.contains(key))
        return false;

    return m_params.value(key).meta.access == DeviceParams::Access::RuntimeOnly;
}

void DeviceParameters::resetDefaults()
{
    for (auto it = m_params.begin(); it != m_params.end(); ++it)
    {
        it.value().value = it.value().meta.defaultValue;

        emit paramChanged(it.key(), getParamValue(it.key()));
        emit paramVariantChanged(it.key(), it.value().value);
    }
}

QJsonObject DeviceParameters::toJson(bool includeSaveOnly) const
{
    QJsonObject object;

    for (auto it = m_params.constBegin(); it != m_params.constEnd(); ++it)
    {
        const auto &param = it.value();

        if (param.meta.storage == DeviceParams::Storage::None)
            continue;

        if (!includeSaveOnly &&
            param.meta.storage == DeviceParams::Storage::SaveOnly)
            continue;

        object.insert(param.meta.key, QJsonValue::fromVariant(param.value));
    }

    return object;
}

bool DeviceParameters::fromJson(const QJsonObject &object)
{
    bool allOk = true;

    for (auto it = object.constBegin(); it != object.constEnd(); ++it)
    {
        const QString key = it.key();

        if (!m_params.contains(key))
        {
            allOk = false;
            continue;
        }

        if (!isLoadable(key))
            continue;

        if (!setParamVariant(key, it.value().toVariant()))
            allOk = false;
    }

    return allOk;
}

bool DeviceParameters::validateValue(const DeviceParams::ParamMeta &meta,
                                     const QVariant &value) const
{
    if (!meta.allowedValues.isEmpty())
    {
        if (!meta.allowedValues.contains(value.toString(), Qt::CaseInsensitive))
            return false;
    }

    if (meta.minValue.isValid())
    {
        bool okValue = false;
        bool okMin = false;

        const double v = value.toDouble(&okValue);
        const double min = meta.minValue.toDouble(&okMin);

        if (okValue && okMin && v < min)
            return false;
    }

    if (meta.maxValue.isValid())
    {
        bool okValue = false;
        bool okMax = false;

        const double v = value.toDouble(&okValue);
        const double max = meta.maxValue.toDouble(&okMax);

        if (okValue && okMax && v > max)
            return false;
    }

    return true;
}

QVariant DeviceParameters::normalizeValue(const DeviceParams::ParamMeta &meta,
                                          const QVariant &value,
                                          bool *ok) const
{
    if (ok)
        *ok = true;

    const QVariant defaultValue = meta.defaultValue;

    if (!defaultValue.isValid())
        return value;

    switch (defaultValue.type())
    {
    case QVariant::Bool:
    {
        if (value.type() == QVariant::Bool)
            return value.toBool();

        const QString str = value.toString().trimmed().toLower();

        if (str == "true" || str == "1" || str == "yes" || str == "enabled")
            return true;

        if (str == "false" || str == "0" || str == "no" || str == "disabled")
            return false;

        if (ok)
            *ok = false;

        return QVariant();
    }

    case QVariant::Int:
    {
        bool conversionOk = false;
        const int converted = value.toInt(&conversionOk);

        if (ok)
            *ok = conversionOk;

        return converted;
    }

    case QVariant::UInt:
    {
        bool conversionOk = false;
        const uint converted = value.toUInt(&conversionOk);

        if (ok)
            *ok = conversionOk;

        return converted;
    }

    case QVariant::LongLong:
    {
        bool conversionOk = false;
        const qlonglong converted = value.toLongLong(&conversionOk);

        if (ok)
            *ok = conversionOk;

        return converted;
    }

    case QVariant::ULongLong:
    {
        bool conversionOk = false;
        const qulonglong converted = value.toULongLong(&conversionOk);

        if (ok)
            *ok = conversionOk;

        return converted;
    }

    case QVariant::Double:
    {
        bool conversionOk = false;
        const double converted = value.toDouble(&conversionOk);

        if (ok)
            *ok = conversionOk;

        return converted;
    }

    case QVariant::String:
    {
        return value.toString();
    }

    default:
        return value;
    }
}
