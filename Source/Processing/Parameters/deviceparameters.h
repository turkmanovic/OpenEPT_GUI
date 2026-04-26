#ifndef DEVICEPARAMETERS_H
#define DEVICEPARAMETERS_H

#include <QObject>
#include <QMap>
#include <QJsonObject>

#include "deviceparamdefs.h"

class DeviceParameters : public QObject
{
    Q_OBJECT

public:
    explicit DeviceParameters(QObject *parent = nullptr);

    bool hasParam(const QString &key) const;

    QString getParamValue(const QString &key) const;
    QVariant getParamVariant(const QString &key) const;

    bool setParamValue(const QString &key, const QString &value);
    bool setParamVariant(const QString &key, const QVariant &value);

    bool isInitialized(const QString &key) const;
    void setParamInitialized(const QString &key, bool initialized);

    DeviceParams::ParamMeta getParamMeta(const QString &key) const;
    DeviceParams::Param getParam(const QString &key) const;

    QList<DeviceParams::Param> getAllParams() const;
    QList<DeviceParams::Param> getParamsByGroup(DeviceParams::Group group) const;
    QList<DeviceParams::Param> getParamsBySubGroup(DeviceParams::Group group,
                                                   DeviceParams::SubGroup subGroup) const;

    DeviceParams::GroupMeta getGroupMeta(DeviceParams::Group group) const;
    DeviceParams::SubGroupMeta getSubGroupMeta(DeviceParams::SubGroup subGroup) const;

    QList<DeviceParams::GroupMeta> getAllGroupMeta() const;
    QList<DeviceParams::SubGroupMeta> getAllSubGroupMeta() const;

    bool isEditable(const QString &key) const;
    bool isVisible(const QString &key) const;
    bool isSavable(const QString &key) const;
    bool isLoadable(const QString &key) const;
    bool isRuntimeOnly(const QString &key) const;

    void resetDefaults();

    QJsonObject toJson(bool includeSaveOnly = true) const;
    bool fromJson(const QJsonObject &object);

signals:
    void paramChanged(QString key, QString value);
    void paramVariantChanged(QString key, QVariant value);

private:
    void initGroupMeta();
    void initSubGroupMeta();
    void initParams();

    bool validateValue(const DeviceParams::ParamMeta &meta,
                       const QVariant &value) const;

    QVariant normalizeValue(const DeviceParams::ParamMeta &meta,
                            const QVariant &value,
                            bool *ok = nullptr) const;

private:
    QMap<QString, DeviceParams::Param> m_params;

    QMap<DeviceParams::Group, DeviceParams::GroupMeta> m_groupMeta;
    QMap<DeviceParams::SubGroup, DeviceParams::SubGroupMeta> m_subGroupMeta;
};

#endif // DEVICEPARAMETERS_H
