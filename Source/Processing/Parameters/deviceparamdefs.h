#ifndef DEVICEPARAMDEFS_H
#define DEVICEPARAMDEFS_H

#include <QList>
#include <QString>
#include <QStringList>
#include <QVariant>

namespace DeviceParams
{
    enum class Group
    {
        DeviceConfig,
        ApplicationConfig,
        RuntimeState,
        Calculated
    };

    enum class SubGroup
    {
        General,
        Network,
        Stream,
        ADC,
        EnergyPoint,
        Processing,
        Load,
        Charger,
        Battery,
        Protection,
        Statistics,
        FileStorage
    };

    enum class Access
    {
        ReadWrite,
        ReadOnly,
        RuntimeOnly
    };

    enum class Storage
    {
        None,
        SaveOnly,
        LoadSave
    };

    enum class Target
    {
        Device,
        Application,
        Runtime,
        Calculated
    };

    struct GroupMeta
    {
        Group id;
        QString name;
        QString description;
        int order;
    };

    struct SubGroupMeta
    {
        SubGroup id;
        QString name;
        QString description;
        int order;
    };

    struct ParamMeta
    {
        QString key;
        QString displayName;
        QString description;
        QString unit;

        Group group;
        SubGroup subGroup;
        Access access;
        Storage storage;
        Target target;

        QVariant defaultValue;
        QVariant minValue;
        QVariant maxValue;
        QStringList allowedValues;

        bool visible;
        int order;
    };

    struct Param
    {
        ParamMeta meta;
        QVariant value;
        bool initialized;
    };

    QList<GroupMeta> defaultGroupMeta();
    QList<SubGroupMeta> defaultSubGroupMeta();
    QList<Param> defaultParams();
}

#endif // DEVICEPARAMDEFS_H
