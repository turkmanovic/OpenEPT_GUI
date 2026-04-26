#ifndef CONFIGURATIONWND_H
#define CONFIGURATIONWND_H

#include <QWidget>
#include <QMap>
#include <QString>
#include <QStringList>

#include "Processing/Parameters/deviceparameters.h"

class QLineEdit;
class QLabel;
class QPushButton;
class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QGroupBox;
class QTabWidget;
class QLayout;

namespace Ui {
class ConfigurationWnd;
}

class ConfigurationWnd : public QWidget
{
    Q_OBJECT

public:
    explicit ConfigurationWnd(QWidget *parent = nullptr,
                              DeviceParameters *params = nullptr);
    ~ConfigurationWnd();

    void setParameters(DeviceParameters *params);

    void setParamValue(const QString &key, const QString &value);
    void setFieldEditable(const QString &key, bool editable);

    void setConfigurationAcquiredStatus(bool status);
    void setConfigurationAppliedStatus(bool status);

signals:
    void sigDeviceConfigSet(QMap<QString, QString> changedFields);
    void sigApplicationConfigSet(QMap<QString, QString> changedFields);
    void sigConfigSet(QMap<QString, QString> changedFields);

    void sigDeviceConfigAcquireRequest();

private slots:
    void onFieldChanged();
    void onParamChanged(QString key, QString value);
    void onSetConfigClicked();
    void onAcquireConfigClicked();

private:
    Ui::ConfigurationWnd *ui;

    DeviceParameters *m_params;

    QTabWidget *tabWidget;

    QLabel *configurationStatusLabel;
    QLabel *changedFieldsLabel;

    QPushButton *setConfigButton;
    QPushButton *acquireConfigButton;

    QMap<QString, QLineEdit*> fields;
    QMap<QString, QString> appliedValues;
    QMap<QString, QString> displayNames;

    void rebuildUi();
    void clearUiState();

    QWidget *createTab(DeviceParams::Group group);
    QVBoxLayout *createGroupLayout(DeviceParams::Group group);
    QGroupBox *createSubGroupBox(DeviceParams::Group group,
                                 DeviceParams::SubGroup subGroup);

    QGridLayout *createParamsGrid(const QList<DeviceParams::Param> &params);
    QWidget *createParamWidget(const DeviceParams::Param &param);

    QHBoxLayout *createButtonsRow();
    QVBoxLayout *createStatusBarLayout();

    void registerField(const DeviceParams::Param &param,
                       QLineEdit *field);

    void setFieldValue(const QString &key,
                       const QString &value,
                       bool markAsApplied);

    QMap<QString, QString> getChangedFields() const;
    QMap<QString, QString> getChangedFields(DeviceParams::Group group) const;

    QStringList getChangedFieldDisplayNames() const;

    void refreshStatusBar();

    bool isDeviceParam(const QString &key) const;
    bool isApplicationParam(const QString &key) const;
};

#endif // CONFIGURATIONWND_H
