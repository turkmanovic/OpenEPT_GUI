#include "chargerwnd.h"
#include "device.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QDoubleValidator>
#include <QIntValidator>

ChargerWnd::ChargerWnd(Device* device, QWidget *parent) :
    QWidget(parent),
    m_device(device)
{
    buildUi();

    connect(readPusb,  SIGNAL(clicked()), this, SLOT(onReadClicked()));
    connect(setPusb,   SIGNAL(clicked()), this, SLOT(onSetClicked()));
    connect(startPusb, SIGNAL(clicked()), this, SLOT(onStartChargingClicked()));
    connect(stopPusb,  SIGNAL(clicked()), this, SLOT(onStopChargingClicked()));

    /* Populate current values from the device. */
    onReadClicked();
}

void ChargerWnd::buildUi()
{
    QVBoxLayout* mainL = new QVBoxLayout(this);

    /* ---- Device information (read-only) ---- */
    QGroupBox*  infoBox = new QGroupBox("Device information", this);
    QFormLayout* infoL  = new QFormLayout(infoBox);
    hwSerialEdit  = new QLineEdit(infoBox);  hwSerialEdit->setReadOnly(true);
    fwVersionEdit = new QLineEdit(infoBox);  fwVersionEdit->setReadOnly(true);
    infoL->addRow("HW serial:",  hwSerialEdit);
    infoL->addRow("FW version:", fwVersionEdit);
    mainL->addWidget(infoBox);

    /* ---- Charging parameters ---- */
    QGroupBox*  paramBox = new QGroupBox("Charging parameters", this);
    QFormLayout* paramL  = new QFormLayout(paramBox);

    termVoltageEdit = new QLineEdit(paramBox);
    termVoltageEdit->setValidator(new QDoubleValidator(1.0, 5.0, 2, termVoltageEdit));
    paramL->addRow("Termination voltage [V]:", termVoltageEdit);

    termCurrentCombo = new QComboBox(paramBox);
    termCurrentCombo->addItem("Disabled", 0);
    termCurrentCombo->addItem("5 %",      1);
    termCurrentCombo->addItem("10 %",     2);
    termCurrentCombo->addItem("20 %",     3);
    paramL->addRow("Termination current:", termCurrentCombo);

    chargeCurrentEdit = new QLineEdit(paramBox);
    chargeCurrentEdit->setValidator(new QIntValidator(5, 1000, chargeCurrentEdit));
    paramL->addRow("Charging current [mA]:", chargeCurrentEdit);

    maxCurrentCombo = new QComboBox(paramBox);
    maxCurrentCombo->addItem("50 mA",   0);
    maxCurrentCombo->addItem("100 mA",  1);
    maxCurrentCombo->addItem("200 mA",  2);
    maxCurrentCombo->addItem("300 mA",  3);
    maxCurrentCombo->addItem("400 mA",  4);
    maxCurrentCombo->addItem("500 mA",  5);
    maxCurrentCombo->addItem("700 mA",  6);
    maxCurrentCombo->addItem("1100 mA", 7);
    paramL->addRow("Max charging current:", maxCurrentCombo);

    /* Start voltage is not yet exposed by the firmware (Phase 2). */
    startVoltageEdit = new QLineEdit(paramBox);
    startVoltageEdit->setEnabled(false);
    startVoltageEdit->setPlaceholderText("not supported by firmware yet");
    paramL->addRow("Start voltage [V]:", startVoltageEdit);

    QHBoxLayout* paramBtnL = new QHBoxLayout();
    readPusb = new QPushButton("Read", paramBox);
    setPusb  = new QPushButton("Set",  paramBox);
    paramBtnL->addWidget(readPusb);
    paramBtnL->addWidget(setPusb);
    paramL->addRow(paramBtnL);

    mainL->addWidget(paramBox);

    /* ---- Charging control ---- */
    QGroupBox*  ctrlBox = new QGroupBox("Charging control", this);
    QHBoxLayout* ctrlL  = new QHBoxLayout(ctrlBox);
    startPusb = new QPushButton("Start charging", ctrlBox);
    stopPusb  = new QPushButton("Stop charging",  ctrlBox);
    ctrlL->addWidget(startPusb);
    ctrlL->addWidget(stopPusb);
    mainL->addWidget(ctrlBox);

    statusLabel = new QLabel("", this);
    mainL->addWidget(statusLabel);

    mainL->addStretch(1);
}

int ChargerWnd::comboIndexForData(QComboBox* combo, int value)
{
    for(int i = 0; i < combo->count(); i++)
        if(combo->itemData(i).toInt() == value) return i;
    return -1;
}

void ChargerWnd::onReadClicked()
{
    if(m_device == nullptr) return;

    QString s;
    if(m_device->getChargerHWSerial(&s))  hwSerialEdit->setText(s);
    QString v;
    if(m_device->getChargerFWVersion(&v)) fwVersionEdit->setText(v);

    float tv;
    if(m_device->getChargerTermVoltage(&tv)) termVoltageEdit->setText(QString::number(tv, 'f', 2));

    int tc;
    if(m_device->getChargerTermCurrent(&tc))
    {
        int idx = comboIndexForData(termCurrentCombo, tc);
        if(idx >= 0) termCurrentCombo->setCurrentIndex(idx);
    }

    int cc;
    if(m_device->getChargerCurrent(&cc)) chargeCurrentEdit->setText(QString::number(cc));

    int mc;
    if(m_device->getChargerMaxChargingCurrent(&mc))
    {
        int idx = comboIndexForData(maxCurrentCombo, mc);
        if(idx >= 0) maxCurrentCombo->setCurrentIndex(idx);
    }

    statusLabel->setText("Parameters read from device.");
}

void ChargerWnd::onSetClicked()
{
    if(m_device == nullptr) return;

    bool ok = true;
    ok &= m_device->setChargerTermVoltage(termVoltageEdit->text().toFloat());
    ok &= m_device->setChargerTermCurrent(termCurrentCombo->currentData().toInt());
    ok &= m_device->setChargerCurrent(chargeCurrentEdit->text().toInt());
    ok &= m_device->setChargerMaxChargingCurrent(maxCurrentCombo->currentData().toInt());

    statusLabel->setText(ok ? "Parameters applied." : "Failed to apply one or more parameters.");

    /* Reflect what the device actually stored. */
    onReadClicked();
}

void ChargerWnd::onStartChargingClicked()
{
    if(m_device == nullptr) return;
    bool ok = m_device->setChargerStatus(true);
    statusLabel->setText(ok ? "Charging started." : "Failed to start charging.");
}

void ChargerWnd::onStopChargingClicked()
{
    if(m_device == nullptr) return;
    bool ok = m_device->setChargerStatus(false);
    statusLabel->setText(ok ? "Charging stopped." : "Failed to stop charging.");
}
