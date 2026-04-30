#ifndef CONFIGURATIONWND_H
#define CONFIGURATIONWND_H

#include <QWidget>
#include <QMap>
#include <QString>
#include <QStringList>
#include <QTextEdit>
#include <QFont>
#include <QProgressBar>

#include "Processing/Parameters/parameterstore.h"
#include "Processing/Parameters/deviceparamdefs.h"

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
                              ParameterStore *params = nullptr);
    ~ConfigurationWnd();

    void setParameters(ParameterStore *params);

    void setParamValue(const QString &key, const QString &value);
    void setFieldEditable(const QString &key, bool editable);

    void setConfigurationAcquiredStatus(bool status);
    void setConfigurationAppliedStatus(bool status);
    void setBDContent(const QString &content);
    void setBDProgress(int percent, const QString &text);
    void resetBDProgress();

signals:
    void sigDeviceConfigSet(QMap<QString, QString> changedFields);
    void sigApplicationConfigSet(QMap<QString, QString> changedFields);
    void sigConfigSet(QMap<QString, QString> changedFields);

    void sigDeviceConfigAcquireRequest();

    void sigBDContentGetRequest();
    void sigBDContentSetRequest(QByteArray content);
    void sigBDFormatRequest();


private slots:
    void onFieldChanged();
    void onParamChanged(QString key, QString value);
    void onSetConfigClicked();
    void onAcquireConfigClicked();

    void onBDGetClicked();
    void onBDUpdateClicked();
    void onBDFormatClicked();

private:
    Ui::ConfigurationWnd *ui;

    ParameterStore *m_params;

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

    QWidget *createTab(Params::GroupId group);
    QVBoxLayout *createGroupLayout(Params::GroupId group);
    QGroupBox *createSubGroupBox(Params::GroupId group,
                                 Params::SubGroupId subGroup);

    QGridLayout *createParamsGrid(const QList<Params::Param> &params);
    QWidget *createParamWidget(const Params::Param &param);

    QHBoxLayout *createButtonsRow();
    QVBoxLayout *createStatusBarLayout();

    void registerField(const Params::Param &param,
                       QLineEdit *field);

    void setFieldValue(const QString &key,
                       const QString &value,
                       bool markAsApplied);

    QMap<QString, QString> getChangedFields() const;
    QMap<QString, QString> getChangedFields(Params::GroupId group) const;

    QStringList getChangedFieldDisplayNames() const;

    void refreshStatusBar();

    bool isDeviceParam(const QString &key) const;
    bool isApplicationParam(const QString &key) const;

    QTextEdit *bdContentTextEdit;
    QPushButton *bdGetButton;
    QPushButton *bdUpdateButton;
    QPushButton *bdFormatButton;
    QProgressBar *bdProgressBar;
    QLabel *bdProgressLabel;
    QWidget* createBDMemoryWidget();
};

#endif // CONFIGURATIONWND_H
