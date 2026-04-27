#ifndef DEVICEPARAMDEFS_H
#define DEVICEPARAMDEFS_H

#include "parameterdefs.h"

namespace DeviceParamDefs
{
    enum Group
    {
        DeviceConfig = 0,
        ApplicationConfig = 1,
        RuntimeState = 2,
        Calculated = 3
    };

    enum SubGroup
    {
        General = 0,
        Network = 1,
        Stream = 2,
        ADC = 3,
        EnergyPoint = 4,
        Processing = 5,
        Load = 6,
        Charger = 6,
        Battery = 7,
        Protection = 8,
        Statistics = 10,
        FileStorage = 11
    };

    QList<Params::GroupMeta> defaultGroupMeta();
    QList<Params::SubGroupMeta> defaultSubGroupMeta();
    QList<Params::Param> defaultParams();
}

#endif // DEVICEPARAMDEFS_H
