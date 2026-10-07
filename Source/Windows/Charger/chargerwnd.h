#ifndef CHARGERWND_H
#define CHARGERWND_H

#include <QWidget>

class Device;
class QLineEdit;
class QComboBox;
class QLabel;
class QPushButton;

/**
 * @brief Dedicated settings window for a Charger device (BQ25180).
 *
 * Unlike EPP devices (which open the full DeviceWnd), a Charger device opens
 * only this focused window: charging parameters, start/stop control and device
 * identification. It talks to the charger through the Device charger wrappers,
 * which in turn use the serial ControlLink.
 */
class ChargerWnd : public QWidget
{
    Q_OBJECT
public:
    explicit ChargerWnd(Device* device, QWidget *parent = nullptr);

private slots:
    void    onReadClicked();
    void    onSetClicked();
    void    onStartChargingClicked();
    void    onStopChargingClicked();

private:
    void    buildUi();
    int     comboIndexForData(QComboBox* combo, int value);

    Device*         m_device;

    QLineEdit*      hwSerialEdit;
    QLineEdit*      fwVersionEdit;
    QLineEdit*      startVoltageEdit;   /* firmware support pending (Phase 2) */
    QLineEdit*      termVoltageEdit;
    QComboBox*      termCurrentCombo;
    QLineEdit*      chargeCurrentEdit;
    QComboBox*      maxCurrentCombo;
    QLabel*         statusLabel;
    QPushButton*    readPusb;
    QPushButton*    setPusb;
    QPushButton*    startPusb;
    QPushButton*    stopPusb;
};

#endif // CHARGERWND_H
