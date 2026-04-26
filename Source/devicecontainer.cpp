#include <QMessageBox>
#include "devicecontainer.h"

DeviceContainer::DeviceContainer(QObject *parent,  DeviceWnd* aDeviceWnd, Device* aDevice, QString aWsPath)
    : QObject{parent}
{
    deviceWnd       = aDeviceWnd;
    device          = aDevice;
    log             = new Log();
    fileProcessing  = new FileProcessing();
    log->assignLogWidget(deviceWnd->getLogWidget());
    wsPath          = aWsPath;
    consumptionProfileName = "";
    consumptionProfileNameSet = false;
    consumptionProfileNameExists = false;
    globalSaveToFileEnabled   = false;
    epEnabled           = false;

    elapsedTime     = 0;
    timer           = new QTimer();

    connect(timer,      SIGNAL(timeout()),                                           this, SLOT(onTimeout()));


    /*Device window signals*/
    connect(deviceWnd,  SIGNAL(sigWndClosed()),                                     this, SLOT(onDeviceWndClosed()));
    connect(deviceWnd,  SIGNAL(sigSaveToFileEnabled(bool)),                         this, SLOT(onDeviceWndSaveToFileChanged(bool)));
    connect(deviceWnd,  SIGNAL(sigEPEnable(bool)),                                  this, SLOT(onDeviceWndEPEnable(bool)));
    connect(deviceWnd,  SIGNAL(sigNewControlMessageRcvd(QString)),                  this, SLOT(onConsoleWndMessageRcvd(QString)));
    connect(deviceWnd,  SIGNAL(sigADCChanged(QString)),                             this, SLOT(onDeviceWndADCChanged(QString)));
    connect(deviceWnd,  SIGNAL(sigResolutionChanged(QString)),                      this, SLOT(onDeviceWndResolutionChanged(QString)));
    connect(deviceWnd,  SIGNAL(sigClockDivChanged(QString)),                        this, SLOT(onDeviceWndClockDivChanged(QString)));
    connect(deviceWnd,  SIGNAL(sigSamplesNoChanged(unsigned int)),                  this, SLOT(onDeviceWndSamplesNoChanged(unsigned int)));
    connect(deviceWnd,  SIGNAL(sigSampleTimeChanged(QString)),                      this, SLOT(onDeviceWndChannelSamplingTimeChanged(QString)));
    connect(deviceWnd,  SIGNAL(sigSamplingPeriodChanged(QString)),                  this, SLOT(onDeviceWndSamplingPeriodChanged(QString)));
    connect(deviceWnd,  SIGNAL(sigAvrRatioChanged(QString)),                        this, SLOT(onDeviceWndAvrRatioChanged(QString)));
    connect(deviceWnd,  SIGNAL(sigVOffsetChanged(QString)),                         this, SLOT(onDeviceWndVOffsetChanged(QString)));
    connect(deviceWnd,  SIGNAL(sigCOffsetChanged(QString)),                         this, SLOT(onDeviceWndCOffsetChanged(QString)));
    connect(deviceWnd,  SIGNAL(sigNewInterfaceSelected(QString)),                   this, SLOT(onDeviceWndInterfaceChanged(QString)));
    connect(deviceWnd,  SIGNAL(sigStartAcquisition()),                              this, SLOT(onDeviceWndAcquisitionStart()));
    connect(deviceWnd,  SIGNAL(sigStopAcquisition()),                               this, SLOT(onDeviceWndAcquisitionStop()));
    connect(deviceWnd,  SIGNAL(sigPauseAcquisition()),                              this, SLOT(onDeviceWndAcquisitionPause()));
    connect(deviceWnd,  SIGNAL(sigRefreshAcquisition()),                            this, SLOT(onDeviceWndAcquisitionRefresh()));
    connect(deviceWnd,  SIGNAL(sigAdvConfigurationReqested()),                      this, SLOT(onDeviceWndAdvConfGet()));
    connect(deviceWnd,  SIGNAL(sigAdvConfigurationChanged(QVariant)),               this, SLOT(onDeviceWndNewConfiguration(QVariant)));
    connect(deviceWnd,  SIGNAL(sigMaxNumberOfBuffersChanged(uint)),                 this, SLOT(onDeviceWndMaxNumberOfBuffersChanged(uint)));
    connect(deviceWnd,  SIGNAL(sigConsumptionTypeChanged(QString)),                 this, SLOT(onDeviceWndConsumptionTypeChanged(QString)));
    connect(deviceWnd,  SIGNAL(sigMeasurementTypeChanged(QString)),                 this, SLOT(onDeviceWndMeasurementTypeChanged(QString)));
    connect(deviceWnd,  SIGNAL(sigConsumptionProfileNameChanged(QString)),          this, SLOT(onDeviceWndConsumptionProfileNameChanged(QString)));
    connect(deviceWnd,  SIGNAL(sigCalibrationUpdated()),                            this, SLOT(onDeviceWndCalibrationUpdated()));
    connect(deviceWnd,  SIGNAL(sigLoadStatusChanged(bool)),                         this, SLOT(onDeviceWndLoadStatusChanged(bool)));
    connect(deviceWnd,  SIGNAL(sigPPathStatusChanged(bool)),                        this, SLOT(onDeviceWndPPathStatusChanged(bool)));
    connect(deviceWnd,  SIGNAL(sigBatteryStatusChanged(bool)),                      this, SLOT(onDeviceWndBatteryStatusChanged(bool)));
    connect(deviceWnd,  SIGNAL(sigResetProtection()),                               this, SLOT(onDeviceWndResetProtection()));
    connect(deviceWnd,  SIGNAL(sigLoadCurrentStatusChanged(bool)),                  this, SLOT(onDeviceWndLoadCurrentSetStatus(bool)));
    connect(deviceWnd,  SIGNAL(sigLoadCurrentChanged(unsigned int)),                this, SLOT(onDeviceWndLoadCurrentSetValue(unsigned int)));
    connect(deviceWnd,  SIGNAL(sigChargingCurrentStatusChanged(bool)),              this, SLOT(onDeviceWndChargingCurrentSetStatus(bool)));
    connect(deviceWnd,  SIGNAL(sigChargingCurrentChanged(unsigned int)),            this, SLOT(onDeviceWndChargingCurrentSetValue(unsigned int)));
    connect(deviceWnd,  SIGNAL(sigChargingTermCurrentChanged(unsigned int)),        this, SLOT(onDeviceWndChargingTermCurrentSetValue(unsigned int)));
    connect(deviceWnd,  SIGNAL(sigChargingTermVoltageChanged(float)),               this, SLOT(onDeviceWndChargingTermVoltageSetValue(float)));
    connect(deviceWnd,  SIGNAL(sigChDschSaveToFileToggled(bool)),                   this, SLOT(onDeviceWndChDschSaveToFileChanged(bool)));

    /*Device signals*/
    connect(device,     SIGNAL(sigControlLinkConnected()),                          this, SLOT(onDeviceControlLinkConnected()));
    connect(device,     SIGNAL(sigControlLinkDisconnected()),                       this, SLOT(onDeviceControlLinkDisconnected()));
    connect(device,     SIGNAL(sigStatusLinkNewDeviceAdded(QString)),               this, SLOT(onDeviceStatusLinkNewDeviceAdded(QString)));
    connect(device,     SIGNAL(sigStatusLinkNewMessageReceived(QString,QString)),   this, SLOT(onDeviceStatusLinkNewMessageReceived(QString,QString)));
    connect(device,     SIGNAL(sigNewResponseReceived(QString, bool)),              this, SLOT(onDeviceHandleControlMsgResponse(QString, bool)));
    connect(device,     SIGNAL(sigResolutionObtained(QString)),                     this, SLOT(onDeviceResolutionObtained(QString)));
    connect(device,     SIGNAL(sigChSampleTimeObtained(QString)),                   this, SLOT(onDeviceChSampleTimeObtained(QString)));
    connect(device,     SIGNAL(sigClockDivObtained(QString)),                       this, SLOT(onDeviceClkDivObtained(QString)));
    connect(device,     SIGNAL(sigSampleTimeObtained(QString)),                     this, SLOT(onDeviceSamplingPeriodObtained(QString)));
    connect(device,     SIGNAL(sigAdcInputClkObtained(QString)),                    this, SLOT(onDeviceAdcInClkObtained(QString)));
    connect(device,     SIGNAL(sigCOffsetObtained(QString)),                        this, SLOT(onDeviceCOffsetObtained(QString)));
    connect(device,     SIGNAL(sigVOffsetObtained(QString)),                        this, SLOT(onDeviceVOffsetObtained(QString)));
    connect(device,     SIGNAL(sigAvgRatio(QString)),                               this, SLOT(onDeviceAvgRatioChanged(QString)));
    connect(device,     SIGNAL(sigSamplingTimeChanged(double)),                     this, SLOT(onDeviceSamplingTimeChanged(double)));
    connect(device,     SIGNAL(sigAcqusitionStarted()),                             this, SLOT(onDeviceAcquisitonStarted()));
    connect(device,     SIGNAL(sigAcqusitionStopped()),                             this, SLOT(onDeviceAcquisitonStopped()));

    connect(device,     SIGNAL(sigPPathStateObtained(bool)),                        this,  SLOT(onDevicePPathStateObtained(bool)));
    connect(device,     SIGNAL(sigBatStateObtained(bool)),                          this,  SLOT(onDeviceBatStateObtained(bool)));
    connect(device,     SIGNAL(sigLoadStateObtained(bool)),                         this,  SLOT(onDeviceLoadStateObtained(bool)));
    connect(device,     SIGNAL(sigLoadCurrentObtained(int )),                       this,  SLOT(onDeviceLoadCurrentObtained(int)));
    connect(device,     SIGNAL(sigDACStateObtained(bool)),                          this,  SLOT(onDeviceDACStateObtained(bool)));
    connect(device,     SIGNAL(sigOVoltageObtained(bool)),                          this,  SLOT(onDeviceOVoltageObtained(bool)));
    connect(device,     SIGNAL(sigUVoltageObtained(bool)),                          this,  SLOT(onDeviceUVoltageObtained(bool)));
    connect(device,     SIGNAL(sigOCurrentObtained(bool)),                          this,  SLOT(onDeviceOCurrentObtained(bool)));
    connect(device,     SIGNAL(sigChargerCurrentObtained(int )),                    this,  SLOT(onDeviceChargerCurrentObtained(int)));
    connect(device,     SIGNAL(sigChargerTermCurrentObtained(int )),                this,  SLOT(onDeviceChargerTermCurrentObtained(int)));
    connect(device,     SIGNAL(sigChargerTermVoltageObtained(float )),              this,  SLOT(onDeviceChargerTermVoltageObtained(float)));
    connect(device,     SIGNAL(sigChargingDone()),                                  this,  SLOT(onDeviceChargingDone()));

    connect(device,     SIGNAL(sigVoltageCurrentSamplesReceived(QVector<double>,QVector<double>,QVector<double>, QVector<double>)),
            this, SLOT(onDeviceNewVoltageCurrentSamplesReceived(QVector<double>,QVector<double>,QVector<double>, QVector<double>)));
    connect(device,     SIGNAL(sigNewSamplesBuffersProcessingStatistics(double,uint,uint,uint, unsigned short)), this, SLOT(onDeviceNewSamplesBuffersProcessingStatistics(double,uint,uint,uint, unsigned short)));
    connect(device,     SIGNAL(sigNewConsumptionDataReceived(QVector<double>,QVector<double>, dataprocessing_consumption_mode_t)),
            this, SLOT(onDeviceNewConsumptionDataReceived(QVector<double>,QVector<double>, dataprocessing_consumption_mode_t)));
    connect(device,     SIGNAL(sigNewEBP(QVector<double>,QVector<double>)), this, SLOT(onDeviceNewEBP(QVector<double>,QVector<double>)));
    connect(device,     SIGNAL(sigNewEBPFull(double,double,QString)), this, SLOT(onDeviceNewEBPFull(double,double,QString)));
    connect(device,     SIGNAL(sigNewStatisticsReceived(dataprocessing_dev_info_t,dataprocessing_dev_info_t,dataprocessing_dev_info_t)),
            this, SLOT(onDeviceNewStatisticsReceived(dataprocessing_dev_info_t,dataprocessing_dev_info_t,dataprocessing_dev_info_t)));

    connect(device,     SIGNAL(sigChargingStatusChanged(charginganalysis_status_t)),
            this, SLOT(onDeviceMeasurementEnergyFlowStatusChanged(charginganalysis_status_t)));




    deviceWnd->setCalibrationData(device->getCalibrationData());

    log->printLogMessage("Device container successfully created", LOG_MESSAGE_TYPE_INFO);
    device->statusLinkServerCreate();
    device->epLinkServerCreate();

}

DeviceContainer::~DeviceContainer()
{
    delete deviceWnd;
    delete device;
    delete log;
}

void DeviceContainer::onDeviceControlLinkDisconnected()
{
    log->printLogMessage("Device control link disconnected", LOG_MESSAGE_TYPE_WARNING);
    deviceWnd->setDeviceNetworkState(DEVICE_STATE_DISCONNECTED);
}

void DeviceContainer::onDeviceControlLinkConnected()
{
    log->printLogMessage("Device control link established", LOG_MESSAGE_TYPE_INFO);
    deviceWnd->setDeviceNetworkState(DEVICE_STATE_CONNECTED);
}

void DeviceContainer::onDeviceStatusLinkNewDeviceAdded(QString aDeviceIP)
{
    log->printLogMessage("Status link sucessfully establish with device(IP: " + aDeviceIP + ")", LOG_MESSAGE_TYPE_INFO);
}

void DeviceContainer::onDeviceStatusLinkNewMessageReceived(QString aDeviceIP, QString aMessage)
{
    log->printLogMessage("New message received from device (IP: " + aDeviceIP + ") :\" " + aMessage + "\"", LOG_MESSAGE_TYPE_INFO, LOG_MESSAGE_DEVICE_TYPE_DEVICE);
}

void DeviceContainer::onDeviceWndClosed()
{
    emit sigDeviceClosed(device);
}

void DeviceContainer::onDeviceWndSaveToFileChanged(bool saveToFile)
{
    globalSaveToFileEnabled = saveToFile;
    writeSamplesToFileEnabled = globalSaveToFileEnabled;
    consumptionProfileName = "";
    consumptionProfileNameSet = false;
    consumptionProfileNameExists = false;
}

void DeviceContainer::onDeviceWndEPEnable(bool aEpEnabled)
{
    epEnabled = aEpEnabled;
    if(!device->setEPEnable(epEnabled))
    {
        log->printLogMessage("Unable to set EP State", LOG_MESSAGE_TYPE_ERROR);
    }
    else
    {
        log->printLogMessage("EP State successfully set", LOG_MESSAGE_TYPE_INFO);
    }
}

void DeviceContainer::onDeviceWndMaxNumberOfBuffersChanged(unsigned int maxNumber)
{
    if(device->setDataProcessingMaxNumberOfBuffers(maxNumber))
    {
        log->printLogMessage("Max number of samples buffers sucessfully configured", LOG_MESSAGE_TYPE_INFO);
    }
    else
    {
        log->printLogMessage("Unable to sucessfully configure max number of samples buffers", LOG_MESSAGE_TYPE_ERROR);
    }
}

void DeviceContainer::onDeviceWndConsumptionTypeChanged(QString aConsumptionType)
{
    dataprocessing_consumption_mode_t consumptionType;
    if(aConsumptionType == "Current") consumptionType = DATAPROCESSING_CONSUMPTION_MODE_CURRENT;
    if(aConsumptionType == "Cumulative") consumptionType = DATAPROCESSING_CONSUMPTION_MODE_CUMULATIVE;
    if(device->setDataProcessingConsumptionType(consumptionType))
    {
        log->printLogMessage("Consumption type: \"" + aConsumptionType + "\" successfully set", LOG_MESSAGE_TYPE_INFO);
    }
    else
    {
        log->printLogMessage("Unable to sucessfully configure Consumption type", LOG_MESSAGE_TYPE_ERROR);
    }
}

void DeviceContainer::onDeviceWndMeasurementTypeChanged(QString aMeasurementType)
{
    dataprocessing_measurement_mode_t measurementType;
    if(aMeasurementType == "Current") measurementType = DATAPROCESSING_MEASUREMENT_MODE_CURRENT;
    if(aMeasurementType == "Voltage") measurementType = DATAPROCESSING_MEASUREMENT_MODE_VOLTAGE;
    if(device->setDataProcessingMeasurementType(measurementType))
    {
        log->printLogMessage("Measurement type: \"" + aMeasurementType + "\" successfully set", LOG_MESSAGE_TYPE_INFO);
    }
    else
    {
        log->printLogMessage("Unable to sucessfully configure Consumption type", LOG_MESSAGE_TYPE_ERROR);
    }
}

void DeviceContainer::onDeviceWndConsumptionProfileNameChanged(QString aConsumptionProfileName)
{
    QString fullPath;
    if(consumptionProfileName == aConsumptionProfileName)
    {
        log->printLogMessage("Consumption profile " + consumptionProfileName+ " already exists", LOG_MESSAGE_TYPE_ERROR);
        return;
    }
    consumptionProfileName = aConsumptionProfileName;
    consumptionProfileNameSet = true;
    if(!createSubDir(consumptionProfileName, fullPath) )
    {
        log->printLogMessage("Consumption profile " + consumptionProfileName+ " already exists", LOG_MESSAGE_TYPE_WARNING);
        fileProcessing->open(FILEPROCESSING_TYPE_SAMPLES, fullPath);
        fileProcessing->setSamplesFileHeader("Voltage and Current samples");
        fileProcessing->setConsumptionFileHeader("Consumption samples");
        fileProcessing->setSummaryFileHeader("Acquisition info");
        fileProcessing->setEPFileHeader("Energy point info");
        consumptionProfileNameExists = true;
        return;
    }
    if(fileProcessing->open(FILEPROCESSING_TYPE_SAMPLES, fullPath))
    {
        log->printLogMessage("Directory for new consumption profile " + consumptionProfileName + " succesfully created", LOG_MESSAGE_TYPE_INFO);
        fileProcessing->setSamplesFileHeader("Voltage and Current samples");
        fileProcessing->setConsumptionFileHeader("Consumption samples");
        fileProcessing->setSummaryFileHeader("Acquisition info");
        fileProcessing->setEPFileHeader("Energy point info");
        consumptionProfileNameExists = false;
    }
    else
    {
        log->printLogMessage("Unable to open samples log file (Path = " + consumptionProfileName + ")", LOG_MESSAGE_TYPE_ERROR);
    }
}

void DeviceContainer::onDeviceWndLoadStatusChanged(bool status)
{
    if(device->setLoadStatus(status))
    {
        deviceWnd->setLoadState(status);
    }
    else
    {

    }
}

void DeviceContainer::onDeviceWndPPathStatusChanged(bool status)
{
    if(device->setPPathStatus(status))
    {
        deviceWnd->setPPathState(status);
    }
    else
    {

    }
}

void DeviceContainer::onDeviceWndBatteryStatusChanged(bool status)
{
    if(device->setBatStatus(status))
    {
        deviceWnd->setBatState(status);
    }
    else
    {

    }
}

void DeviceContainer::onDeviceWndResetProtection()
{
    if(device->latchTrigger())
    {
        log->printLogMessage("Protection Reset", LOG_MESSAGE_TYPE_INFO);
    }
    else
    {

    }
}

void DeviceContainer::onConsoleWndMessageRcvd(QString msg)
{
    /* call device funtion sendControl Msg -> */
    device->sendControlMsg(msg);
}

void DeviceContainer::onDeviceHandleControlMsgResponse(QString msg, bool exeStatus)
{
    /* call deviceWnd function with recieved msg from FW <- */
    deviceWnd->printConsoleMsg(msg, exeStatus);
}



void DeviceContainer::onDeviceWndSamplesNoChanged(unsigned int newSamplesNo)
{
    /* call deviceWnd function with recieved msg from FW <- */
    if(!device->setSamplesNo(newSamplesNo))
    {
        log->printLogMessage("Unable to set samples no: " + QString::number(newSamplesNo), LOG_MESSAGE_TYPE_ERROR);
    }
    else
    {
        log->printLogMessage(QString::number(newSamplesNo) + " sucessfully set", LOG_MESSAGE_TYPE_INFO);
    }
}


void DeviceContainer::onDeviceWndSamplingPeriodChanged(QString time)
{
    bool conversionOk;
    double  numericValue = time.toDouble(&conversionOk);

    if(!device->setSamplingPeriod(time))
    {
        log->printLogMessage("Unable to set sampling time: " + time, LOG_MESSAGE_TYPE_ERROR);
    }
    else
    {
        log->printLogMessage("Sampling time: " + time + " sucessfully set", LOG_MESSAGE_TYPE_INFO);
    }
}

void DeviceContainer::onDeviceWndInterfaceChanged(QString interfaceIp)
{
    int streamID = -1;
    if(!device->createStreamLink(interfaceIp, 11223, &streamID))
    {
        log->printLogMessage("Unable to create stream link: ", LOG_MESSAGE_TYPE_ERROR);
        deviceWnd->setDeviceInterfaceSelectionState(DEVICE_INTERFACE_SELECTION_STATE_UNDEFINED);
    }
    else
    {
        log->printLogMessage("Stream link ( sid="+ QString::number(streamID) + " ) sucessfully created: ", LOG_MESSAGE_TYPE_INFO);
        if(!device->establishEPLink(interfaceIp))
        {
            log->printLogMessage("Unable to create ep link: ", LOG_MESSAGE_TYPE_ERROR);
        }
        else
        {
            log->printLogMessage("Ep link ( port="+ QString::number(8000) + " ) sucessfully created: ", LOG_MESSAGE_TYPE_INFO);
        }
        deviceWnd->setDeviceInterfaceSelectionState(DEVICE_INTERFACE_SELECTION_STATE_SELECTED);
        device->acquireDeviceConfiguration(DEVICE_ADC_EXTERNAL);
        if(!device->establishStatusLink(interfaceIp))
        {
            log->printLogMessage("Unable to create status link: ", LOG_MESSAGE_TYPE_ERROR);
        }
        else
        {
            log->printLogMessage("Status link ( port="+ QString::number(8818) + " ) sucessfully created: ", LOG_MESSAGE_TYPE_INFO);
        }
    }
}


void DeviceContainer::onDeviceWndAcquisitionStart()
{
    if(globalSaveToFileEnabled && (!consumptionProfileNameSet))
    {
        QMessageBox msgBox;
        msgBox.setWindowIcon(QIcon(QPixmap(":/images/NewSet/stopHand.png")));
        msgBox.setWindowTitle("Error: Unable to start Acquisition");
        msgBox.setText("Set consumption profile name or disable \"Save to file\"");
        msgBox.exec();
        return;
    }
    if(globalSaveToFileEnabled && consumptionProfileNameExists)
    {
        QMessageBox msgBox;
        msgBox.setWindowIcon(QIcon(QPixmap(":/images/NewSet/stopHand.png")));
        msgBox.setWindowTitle("Warning: Consumption profile already exists");
        msgBox.setInformativeText("Consumption profile already exists. Continuing will overwrite previous data. Do you want to proceed?");
        msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Cancel);
        int ret = msgBox.exec();

        switch (ret) {
        case QMessageBox::Ok:
            fileProcessing->reOpenFiles();
            consumptionProfileNameExists = false;
            break;
        case QMessageBox::Cancel:
            return;
        default:
            return;
        }
    }
    if(!device->acquisitionStart())
    {
        log->printLogMessage("Unable to start acquistion", LOG_MESSAGE_TYPE_ERROR);
    }
    else
    {
        log->printLogMessage("Acquisition sucessfully started", LOG_MESSAGE_TYPE_INFO);
        deviceWnd->setDeviceAcqState(DEVICE_ACQ_ACTIVE);
        if(globalSaveToFileEnabled)
        {
            fileProcessing->appendSummaryFile("EP Enabled: " + QString::number(epEnabled));
            fileProcessing->appendSummaryFile("Acquisiton start: " + QDateTime::currentDateTime().toString());
        }
    }
}

void DeviceContainer::onDeviceWndAcquisitionStop()
{
    if(!device->acquisitionStop())
    {
        log->printLogMessage("Unable to stop acquistion", LOG_MESSAGE_TYPE_ERROR);
    }
    else
    {
        log->printLogMessage("Acquisition sucessfully stoped", LOG_MESSAGE_TYPE_INFO);
        if(globalSaveToFileEnabled && (consumptionProfileNameExists == false))
        {
            fileProcessing->appendSummaryFile("Acquisiton stop: " + QDateTime::currentDateTime().toString());
            consumptionProfileNameExists = true;
        }
        deviceWnd->setDeviceAcqState(DEVICE_ACQ_PAUSE);
    }
}

void DeviceContainer::onDeviceWndAcquisitionPause()
{
    if(!device->acquisitionPause())
    {
        log->printLogMessage("Unable to pause acquistion", LOG_MESSAGE_TYPE_ERROR);
    }
    else
    {
        log->printLogMessage("Acquisition sucessfully paused", LOG_MESSAGE_TYPE_INFO);
          if(globalSaveToFileEnabled && (consumptionProfileNameExists == false))
        {
            fileProcessing->appendSummaryFile("Acquisiton stop: " + QDateTime::currentDateTime().toString());
            consumptionProfileNameExists = true;
        }
        deviceWnd->setDeviceAcqState(DEVICE_ACQ_PAUSE);
    }
}

void DeviceContainer::onDeviceWndAcquisitionRefresh()
{
    device->acquireDeviceConfiguration();
}

void DeviceContainer::onDeviceWndAdvConfGet()
{
    device->acquireDeviceConfiguration();
}

void DeviceContainer::onDeviceWndNewConfiguration(QVariant newConfig)
{
    advConfigurationData config = newConfig.value<advConfigurationData>();

    if(config.samplingTime != "")
    {
        if(!device->setSamplingPeriod(config.samplingTime))
        {
            log->printLogMessage("Unable to set sampling time: " + config.samplingTime, LOG_MESSAGE_TYPE_ERROR);
        }
        else
        {
            log->printLogMessage("Sampling time: " + config.samplingTime + "[ms] - sucessfully set", LOG_MESSAGE_TYPE_INFO);
            deviceWnd->setSamplingPeriod(config.samplingTime);
        }
    }

}

void DeviceContainer::onDeviceSamplingPeriodObtained(QString stime)
{
    if(!deviceWnd->setSamplingPeriod(stime))
    {
        log->printLogMessage("Unable to show obtained sampling time: " + stime + "us", LOG_MESSAGE_TYPE_ERROR);
    }
    else
    {
        log->printLogMessage("Sampling time sucessfully obained and presented ", LOG_MESSAGE_TYPE_INFO);
    }
}

void DeviceContainer::onDeviceSamplingTimeChanged(double value)
{
    deviceWnd->setStatisticsSamplingTime(value);
}

void DeviceContainer::onDeviceLoadStateObtained(bool state)
{
    if(!deviceWnd->setLoadState(state))
    {
        log->printLogMessage("Unable to obtain load state", LOG_MESSAGE_TYPE_ERROR);
    }
    else
    {
        log->printLogMessage("Load state sucessfully obained and presented ", LOG_MESSAGE_TYPE_INFO);
    }
}

void DeviceContainer::onDeviceBatStateObtained(bool state)
{
    if(!deviceWnd->setBatState(state))
    {
        log->printLogMessage("Unable to obtain battery state", LOG_MESSAGE_TYPE_ERROR);
    }
    else
    {
        log->printLogMessage("Battery state sucessfully obained and presented ", LOG_MESSAGE_TYPE_INFO);
    }
}

void DeviceContainer::onDevicePPathStateObtained(bool state)
{
    if(!deviceWnd->setPPathState(state))
    {
        log->printLogMessage("Unable to obtain PPath state", LOG_MESSAGE_TYPE_ERROR);
    }
    else
    {
        log->printLogMessage("PPath state sucessfully obained and presented ", LOG_MESSAGE_TYPE_INFO);
    }
}

void DeviceContainer::onDeviceChargerStateObtained(bool state)
{
    if(!deviceWnd->setChargerState(state))
    {
        log->printLogMessage("Unable to obtain Charger state", LOG_MESSAGE_TYPE_ERROR);
    }
    else
    {
        log->printLogMessage("Charger state sucessfully obained and presented ", LOG_MESSAGE_TYPE_INFO);
    }
}

void DeviceContainer::onDeviceDACStateObtained(bool state)
{
    if(!deviceWnd->setDACState(state))
    {
        log->printLogMessage("Unable to obtain DAC state", LOG_MESSAGE_TYPE_ERROR);
    }
    else
    {
        log->printLogMessage("DAC state sucessfully obained and presented ", LOG_MESSAGE_TYPE_INFO);
    }
}

void DeviceContainer::onDeviceLoadCurrentObtained(int current)
{
    if(!deviceWnd->setLoadCurrent(current))
    {
        log->printLogMessage("Unable to obtain load current", LOG_MESSAGE_TYPE_ERROR);
    }
    else
    {
        log->printLogMessage("Load current sucessfully obained and presented ", LOG_MESSAGE_TYPE_INFO);
    }
}

void DeviceContainer::onDeviceUVoltageObtained(bool state)
{
    if(!deviceWnd->setUVoltageIndication(state))
    {
        log->printLogMessage("Unable to obtain Under Voltage protection state", LOG_MESSAGE_TYPE_ERROR);
    }
    else
    {
        log->printLogMessage("Under Voltage protection state sucessfully obained and presented ", LOG_MESSAGE_TYPE_INFO);
    }
}

void DeviceContainer::onDeviceOVoltageObtained(bool state)
{
    if(!deviceWnd->setOVoltageIndication(state))
    {
        log->printLogMessage("Unable to obtain Over Voltage protection state", LOG_MESSAGE_TYPE_ERROR);
    }
    else
    {
        log->printLogMessage("Over Voltage protection state sucessfully obained and presented ", LOG_MESSAGE_TYPE_INFO);
    }
}

void DeviceContainer::onDeviceOCurrentObtained(bool state)
{
    if(!deviceWnd->setOCurrentIndication(state))
    {
        log->printLogMessage("Unable to obtain Over Current protection state", LOG_MESSAGE_TYPE_ERROR);
    }
    else
    {
        log->printLogMessage("Over Current protection state sucessfully obained and presented ", LOG_MESSAGE_TYPE_INFO);
    }
}

void DeviceContainer::onDeviceChargingDone()
{
    if(!deviceWnd->chargingDone())
    {
        log->printLogMessage("Unable to obtain Over Current protection state", LOG_MESSAGE_TYPE_ERROR);
    }
    else
    {
        log->printLogMessage("Charging done ", LOG_MESSAGE_TYPE_INFO);
    }
}

void DeviceContainer::onDeviceChargerCurrentObtained(int current)
{
    if(!deviceWnd->setChargerCurrent(current))
    {
        log->printLogMessage("Unable to obtain charge current", LOG_MESSAGE_TYPE_ERROR);
    }
    else
    {
        log->printLogMessage("Charge current sucessfully obained and presented ", LOG_MESSAGE_TYPE_INFO);
    }
}

void DeviceContainer::onDeviceChargerTermCurrentObtained(int current)
{
    if(!deviceWnd->setChargerTermCurrent(current))
    {
        log->printLogMessage("Unable to obtain charge termination current", LOG_MESSAGE_TYPE_ERROR);
    }
    else
    {
        log->printLogMessage("Charge termination current sucessfully obained and presented ", LOG_MESSAGE_TYPE_INFO);
    }
}

void DeviceContainer::onDeviceChargerTermVoltageObtained(float voltage)
{
    if(!deviceWnd->setChargerTermVoltage(voltage))
    {
        log->printLogMessage("Unable to obtain charge termination voltage", LOG_MESSAGE_TYPE_ERROR);
    }
    else
    {
        log->printLogMessage("Charge termination voltage sucessfully obained and presented ", LOG_MESSAGE_TYPE_INFO);
    }
}


void DeviceContainer::onDeviceWndLoadCurrentSetValue(unsigned int current)
{
    if(device->setLoadCurrent(current))
    {
        log->printLogMessage("Load current sucessfully set: [" + QString::number(current) + " mA]", LOG_MESSAGE_TYPE_INFO);
    }
    else
    {
        log->printLogMessage("Unable to set load current", LOG_MESSAGE_TYPE_ERROR);
    }
}

void DeviceContainer::onDeviceWndLoadCurrentSetStatus(bool status)
{
    QString statusStr = status ? "Enabled" : "Disabled";
    if(device->setDACStatus(status))
    {
        log->printLogMessage("DAC status successfully set: " + statusStr, LOG_MESSAGE_TYPE_INFO);
        deviceWnd->setLoadCurrentStatus(status);
    }
    else
    {
        log->printLogMessage("Unable to set DAC status", LOG_MESSAGE_TYPE_ERROR);
    }
}

void DeviceContainer::onDeviceWndChargingCurrentSetValue(unsigned int current)
{
    if(device->setChargerCurrent(current))
    {
        log->printLogMessage("Charging current sucessfully set: " + QString::number(current) + " [mA]", LOG_MESSAGE_TYPE_INFO);
    }
    else
    {
        log->printLogMessage("Unable to set Charging current", LOG_MESSAGE_TYPE_ERROR);
    }
}

void DeviceContainer::onDeviceWndChargingTermCurrentSetValue(unsigned int current)
{
    if(device->setChargerTermCurrent(current))
    {
        log->printLogMessage("Charging termination current sucessfully set: " + QString::number(current) + " [%]", LOG_MESSAGE_TYPE_INFO);
    }
    else
    {
        log->printLogMessage("Unable to set Charging termination current", LOG_MESSAGE_TYPE_ERROR);
    }
}

void DeviceContainer::onDeviceWndChargingTermVoltageSetValue(float voltage)
{
    if(device->setChargerTermVoltage(voltage))
    {
        log->printLogMessage("Charging termination voltage sucessfully set: " + QString::number(voltage,'g',3) + " [V]", LOG_MESSAGE_TYPE_INFO);
    }
    else
    {
        log->printLogMessage("Unable to set Charging termination voltage", LOG_MESSAGE_TYPE_ERROR);
    }
}

void DeviceContainer::onDeviceWndChargingCurrentSetStatus(bool status)
{
    QString statusStr = status ? "Enabled" : "Disabled";
    if(device->setChargerStatus(status))
//    if(true)
    {
        log->printLogMessage("Charger status successfully set: " + statusStr, LOG_MESSAGE_TYPE_INFO);
        deviceWnd->setChargingCurrentStatus(status);
    }
    else
    {
        log->printLogMessage("Unable to set Charger status", LOG_MESSAGE_TYPE_ERROR);
    }
}

void DeviceContainer::onDeviceWndChDschSaveToFileChanged(bool saveToFile)
{

    if(!globalSaveToFileEnabled)
    {
        log->printLogMessage("Global save to file option is not enabled", LOG_MESSAGE_TYPE_ERROR);
        return;
    }
    if(!consumptionProfileNameSet)
    {
        log->printLogMessage("Unable to enable write to file. Please set profile name ", LOG_MESSAGE_TYPE_ERROR);
        return;
    }
    chDschSaveToFileEnabled = saveToFile;
    writeSamplesToFileEnabled = globalSaveToFileEnabled && chDschSaveToFileEnabled;

    log->printLogMessage("Write samples to file set to: " + QString(writeSamplesToFileEnabled ? "Enabled": "Disabled"), LOG_MESSAGE_TYPE_INFO);

}

void DeviceContainer::onDeviceAcquisitonStarted()
{
    elapsedTime = 0;
    timer->start(1000);

}

void DeviceContainer::onDeviceAcquisitonStopped()
{
    elapsedTime = 0;
    timer->stop();
    deviceWnd->setStatisticsElapsedTime(elapsedTime);
}

void DeviceContainer::onTimeout()
{
    elapsedTime += 1;
    deviceWnd->setStatisticsElapsedTime(elapsedTime);
}

void DeviceContainer::onDeviceNewVoltageCurrentSamplesReceived(QVector<double> voltage, QVector<double> current, QVector<double> voltageKeys, QVector<double> currentKeys)
{
    deviceWnd->plotVoltageValues(voltage, voltageKeys);
    deviceWnd->plotCurrentValues(current, currentKeys);
    if(writeSamplesToFileEnabled)
    {
        fileProcessing->appendSampleDataQueued(voltage, voltageKeys, current, currentKeys);
    }
}

void DeviceContainer::onDeviceNewConsumptionDataReceived(QVector<double> consumption, QVector<double> keys, dataprocessing_consumption_mode_t mode)
{
    deviceWnd->plotConsumptionValues(consumption, keys);
    if(writeSamplesToFileEnabled)
    {
        fileProcessing->appendConsumptionQueued(consumption, keys);
    }
}

void DeviceContainer::onDeviceNewStatisticsReceived(dataprocessing_dev_info_t voltageStat, dataprocessing_dev_info_t currentStat, dataprocessing_dev_info_t consumptionStat)
{
    device_stat_info statInfo;
    statInfo.voltageAvg = voltageStat.average;
    statInfo.voltageMax = voltageStat.max;
    statInfo.voltageMin = voltageStat.min;
    statInfo.currentAvg = currentStat.average;
    statInfo.currentMax = currentStat.max;
    statInfo.currentMin = currentStat.min;
    statInfo.consumptionAvg = consumptionStat.average;
    statInfo.consumptionMax = consumptionStat.max;
    statInfo.consumptionMin = consumptionStat.min;
    deviceWnd->showStatistic(statInfo);
}

void DeviceContainer::onDeviceNewSamplesBuffersProcessingStatistics(double dropRate, unsigned int dropPacketsNo, unsigned int fullReceivedBuffersNo, unsigned int lastBufferID, unsigned short ebp)
{
    deviceWnd->setStatisticsData(dropRate, dropPacketsNo, fullReceivedBuffersNo, lastBufferID);
}

void DeviceContainer::onDeviceNewEBP(QVector<double> ebpValues, QVector<double> keys)
{
    //deviceWnd->plotAppendConsumptionEBP(ebpValues, keys);
}

void DeviceContainer::onDeviceNewEBPFull(double value, double key, QString name)
{
    deviceWnd->plotConsumptionEBPWithName(value, key, name);
    if(writeSamplesToFileEnabled)
    {
        fileProcessing->appendEPQueued(name, key);
    }
    log->printLogMessage("New Energy point received (Value: " + QString::number(value) + "; Key: " + QString::number(key) + "; Name: " + name + ")", LOG_MESSAGE_TYPE_INFO);
}

void DeviceContainer::onDeviceMeasurementEnergyFlowStatusChanged(charginganalysis_status_t status)
{
    QString msg = "Charging status changed to ";
    QString wndMsg;
    switch(status)
    {
    case CHARGINGANALYSIS_STATUS_UNKNOWN:
        msg += "Uknown";
        wndMsg = "Uknown";
        break;
    case CHARGINGANALYSIS_STATUS_IDLE:
        msg += "Idle";
        wndMsg = "Idle";
        break;
    case CHARGINGANALYSIS_STATUS_CHARGING:
        msg += "Charging";
        wndMsg = "Charging";
        break;
    case CHARGINGANALYSIS_STATUS_DISCHARGING:
        msg += "Discharging";
        wndMsg = "Discharging";
        break;

    }
    deviceWnd->setChargingStatus(wndMsg);
    log->printLogMessage(msg, LOG_MESSAGE_TYPE_INFO);
}

void DeviceContainer::onDeviceWndCalibrationUpdated()
{
    device->calibrationUpdated();
}





bool        DeviceContainer::createSubDir(const QString &subDirName, QString &fullPath) {
    QDir dir(wsPath);

    fullPath = wsPath + "/" + subDirName;

    // Check if the main directory exists
    if (!dir.exists()) return false;

    // Check if the subdirectory already exists
    if (dir.exists(subDirName)) return false;

    // Try to create the subdirectory
    if (!dir.mkdir(subDirName)) return false;

    return true;
}
