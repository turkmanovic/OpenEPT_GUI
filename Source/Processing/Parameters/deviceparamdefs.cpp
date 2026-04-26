#include "deviceparamdefs.h"

namespace DeviceParams
{

QList<GroupMeta> defaultGroupMeta()
{
    return {
        {
            Group::DeviceConfig,
            "Device Configuration",
            "Parameters stored on the device or directly related to device setup.",
            0
        },
        {
            Group::ApplicationConfig,
            "Application Configuration",
            "Parameters used only by the application.",
            1
        },
        {
            Group::RuntimeState,
            "Runtime State",
            "Dynamic parameters updated during execution.",
            2
        },
        {
            Group::Calculated,
            "Calculated Values",
            "Values calculated by the application.",
            3
        }
    };
}

QList<SubGroupMeta> defaultSubGroupMeta()
{
    return {
        {
            SubGroup::General,
            "General",
            "General device parameters.",
            0
        },
        {
            SubGroup::Network,
            "Network",
            "IP addresses, ports and communication endpoints.",
            1
        },
        {
            SubGroup::Stream,
            "Stream",
            "Streaming service parameters.",
            2
        },
        {
            SubGroup::ADC,
            "ADC",
            "ADC-related configuration and detected hardware parameters.",
            3
        },
        {
            SubGroup::EnergyPoint,
            "Energy Point",
            "Energy point processing and reporting parameters.",
            4
        },
        {
            SubGroup::Processing,
            "Processing",
            "Application-side data processing parameters.",
            5
        },
        {
            SubGroup::Load,
            "Load",
            "Load control and load-related runtime values.",
            6
        },
        {
            SubGroup::Charger,
            "Charger",
            "Battery charger configuration and runtime values.",
            7
        },
        {
            SubGroup::Battery,
            "Battery",
            "Battery path and battery-related runtime values.",
            8
        },
        {
            SubGroup::Protection,
            "Protection",
            "Protection and fault indication states.",
            9
        },
        {
            SubGroup::Statistics,
            "Statistics",
            "Runtime statistics and monitoring values.",
            10
        },
        {
            SubGroup::FileStorage,
            "File Storage",
            "Application-side file saving parameters.",
            11
        }
    };
}

QList<Param> defaultParams()
{
    return {
        {
            {
                "deviceName",
                "Device Name",
                "User-defined device name.",
                "",
                Group::DeviceConfig,
                SubGroup::General,
                Access::ReadWrite,
                Storage::LoadSave,
                Target::Device,
                "",
                QVariant(),
                QVariant(),
                {},
                true,
                0
            },
            "",
            false
        },
        {
            {
                "deviceIp",
                "Device IP",
                "IP address used for communication with the device.",
                "",
                Group::DeviceConfig,
                SubGroup::Network,
                Access::ReadWrite,
                Storage::LoadSave,
                Target::Device,
                "192.168.1.100",
                QVariant(),
                QVariant(),
                {},
                true,
                1
            },
            "192.168.1.100",
            false
        },
        {
            {
                "controlPort",
                "Control Port",
                "TCP port used for the control link.",
                "",
                Group::DeviceConfig,
                SubGroup::Network,
                Access::ReadWrite,
                Storage::LoadSave,
                Target::Device,
                0,
                0,
                65535,
                {},
                true,
                2
            },
            0,
            false
        },
        {
            {
                "statusLinkPort",
                "Status Link Port",
                "Local port used for receiving device status messages.",
                "",
                Group::DeviceConfig,
                SubGroup::Network,
                Access::ReadWrite,
                Storage::LoadSave,
                Target::Device,
                8818,
                0,
                65535,
                {},
                true,
                3
            },
            0,
            false
        },
        {
            {
                "energyPointLinkPort",
                "Energy Point Link Port",
                "Local port used for energy point communication.",
                "",
                Group::DeviceConfig,
                SubGroup::Network,
                Access::ReadWrite,
                Storage::LoadSave,
                Target::Device,
                8000,
                0,
                65535,
                {},
                true,
                4
            },
            0,
            false
        },
        {
            {
                "streamLinkPort",
                "Stream Link Port",
                "Local port used for stream data reception.",
                "",
                Group::DeviceConfig,
                SubGroup::Stream,
                Access::ReadWrite,
                Storage::LoadSave,
                Target::Device,
                11223,
                0,
                65535,
                {},
                true,
                5
            },
            0,
            false
        },
        {
            {
                "streamPacketSize",
                "Stream Packet Size",
                "Number of samples (per channel) expected in one stream packet.",
                "Samples",
                Group::DeviceConfig,
                SubGroup::Stream,
                Access::ReadWrite,
                Storage::LoadSave,
                Target::Device,
                0,
                0,
                QVariant(),
                {},
                true,
                6
            },
            250,
            true
        },
        {
            {
                "samplingPeriod",
                "Sampling Period",
                "ADC sampling period configured on the device.",
                "us",
                Group::DeviceConfig,
                SubGroup::ADC,
                Access::ReadWrite,
                Storage::LoadSave,
                Target::Device,
                1.0,
                0.0,
                QVariant(),
                {},
                true,
                7
            },
            1.0,
            false
        },
        {
            {
                "epEnabled",
                "EP Enabled",
                "Enables or disables energy point processing.",
                "",
                Group::DeviceConfig,
                SubGroup::EnergyPoint,
                Access::ReadWrite,
                Storage::LoadSave,
                Target::Device,
                false,
                QVariant(),
                QVariant(),
                {"false", "true"},
                true,
                8
            },
            false,
            false
        },
        {
            {
                "adcResolution",
                "ADC Resolution",
                "Detected ADC resolution. This value is read-only.",
                "bit",
                Group::DeviceConfig,
                SubGroup::ADC,
                Access::ReadOnly,
                Storage::SaveOnly,
                Target::Device,
                0,
                QVariant(),
                QVariant(),
                {"10", "12", "14", "16"},
                true,
                9
            },
            0,
            false
        },
        {
            {
                "adcType",
                "ADC Type",
                "Detected ADC type used by the device.",
                "",
                Group::DeviceConfig,
                SubGroup::ADC,
                Access::ReadOnly,
                Storage::SaveOnly,
                Target::Device,
                "Unknown",
                QVariant(),
                QVariant(),
                {"Unknown", "Internal", "External"},
                true,
                10
            },
            "Unknown",
            false
        },
        {
            {
                "maxNumberOfBuffers",
                "Max Number of Buffers",
                "Maximum number of sample buffers collected by the application.",
                "",
                Group::ApplicationConfig,
                SubGroup::Processing,
                Access::ReadWrite,
                Storage::LoadSave,
                Target::Application,
                0,
                0,
                QVariant(),
                {},
                true,
                0
            },
            5,
            true
        },
        {
            {
                "consumptionType",
                "Consumption Type",
                "Application-side consumption calculation mode.",
                "",
                Group::ApplicationConfig,
                SubGroup::Processing,
                Access::ReadOnly,
                Storage::LoadSave,
                Target::Application,
                "Combined",
                QVariant(),
                QVariant(),
                {"Combined"},
                true,
                1
            },
            "Combined",
            false
        },
        {
            {
                "streamId",
                "Stream ID",
                "Runtime stream identifier created by the device.",
                "",
                Group::RuntimeState,
                SubGroup::Stream,
                Access::RuntimeOnly,
                Storage::None,
                Target::Runtime,
                -1,
                QVariant(),
                QVariant(),
                {},
                true,
                0
            },
            -1,
            false
        },
        {
            {
                "powerPathState",
                "Power Path State",
                "Current power path state.",
                "",
                Group::RuntimeState,
                SubGroup::Battery,
                Access::RuntimeOnly,
                Storage::None,
                Target::Runtime,
                false,
                QVariant(),
                QVariant(),
                {"false", "true"},
                true,
                1
            },
            false,
            false
        },
        {
            {
                "loadState",
                "Load State",
                "Current load enable state.",
                "",
                Group::RuntimeState,
                SubGroup::Load,
                Access::RuntimeOnly,
                Storage::None,
                Target::Runtime,
                false,
                QVariant(),
                QVariant(),
                {"false", "true"},
                true,
                2
            },
            false,
            false
        },
        {
            {
                "loadCurrent",
                "Load Current",
                "Current load discharge current.",
                "mA",
                Group::RuntimeState,
                SubGroup::Load,
                Access::RuntimeOnly,
                Storage::None,
                Target::Runtime,
                0,
                0,
                QVariant(),
                {},
                true,
                3
            },
            0,
            false
        },
        {
            {
                "dacState",
                "DAC State",
                "Current DAC enable state.",
                "",
                Group::RuntimeState,
                SubGroup::Load,
                Access::RuntimeOnly,
                Storage::None,
                Target::Runtime,
                false,
                QVariant(),
                QVariant(),
                {"false", "true"},
                true,
                4
            },
            false,
            false
        },
        {
            {
                "batteryState",
                "Battery State",
                "Current battery path state.",
                "",
                Group::RuntimeState,
                SubGroup::Battery,
                Access::RuntimeOnly,
                Storage::None,
                Target::Runtime,
                false,
                QVariant(),
                QVariant(),
                {"false", "true"},
                true,
                5
            },
            false,
            false
        },
        {
            {
                "underVoltageStatus",
                "Under Voltage Status",
                "Under-voltage protection indication.",
                "",
                Group::RuntimeState,
                SubGroup::Protection,
                Access::RuntimeOnly,
                Storage::None,
                Target::Runtime,
                false,
                QVariant(),
                QVariant(),
                {"false", "true"},
                true,
                6
            },
            false,
            false
        },
        {
            {
                "overVoltageStatus",
                "Over Voltage Status",
                "Over-voltage protection indication.",
                "",
                Group::RuntimeState,
                SubGroup::Protection,
                Access::RuntimeOnly,
                Storage::None,
                Target::Runtime,
                false,
                QVariant(),
                QVariant(),
                {"false", "true"},
                true,
                7
            },
            false,
            false
        },
        {
            {
                "overCurrentStatus",
                "Over Current Status",
                "Over-current protection indication.",
                "",
                Group::RuntimeState,
                SubGroup::Protection,
                Access::RuntimeOnly,
                Storage::None,
                Target::Runtime,
                false,
                QVariant(),
                QVariant(),
                {"false", "true"},
                true,
                8
            },
            false,
            false
        },
        {
            {
                "calculatedAdcSamplingTime",
                "Calculated ADC Sampling Time",
                "ADC sampling time calculated by the application.",
                "ms",
                Group::Calculated,
                SubGroup::ADC,
                Access::ReadOnly,
                Storage::SaveOnly,
                Target::Calculated,
                0.0,
                0.0,
                QVariant(),
                {},
                true,
                0
            },
            0.0,
            false
        }
    };
}

}
