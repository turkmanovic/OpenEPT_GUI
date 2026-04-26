#include "configurationwnd.h"
#include "ui_configurationwnd.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QTabWidget>
#include <QTabBar>
#include <QScrollArea>
#include <QFrame>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QSizePolicy>
#include <QSignalBlocker>

#define CONFIG_LABEL_WIDTH     150
#define CONFIG_FIELD_WIDTH     170
#define CONFIG_UNIT_WIDTH      40
#define CONFIG_ROW_HEIGHT      28
#define CONFIG_BUTTON_WIDTH    120
#define CONFIG_BUTTON_HEIGHT   32

ConfigurationWnd::ConfigurationWnd(QWidget *parent,
                                   DeviceParameters *params) :
    QWidget(parent),
    ui(new Ui::ConfigurationWnd),
    m_params(params),
    tabWidget(nullptr),
    configurationStatusLabel(nullptr),
    changedFieldsLabel(nullptr),
    setConfigButton(nullptr),
    acquireConfigButton(nullptr)
{
    ui->setupUi(this);

    setMinimumSize(900, 700);
    resize(900, 700);

    rebuildUi();

    if(m_params != nullptr)
    {
        connect(m_params,
                &DeviceParameters::paramChanged,
                this,
                &ConfigurationWnd::onParamChanged);
    }
}

ConfigurationWnd::~ConfigurationWnd()
{
    delete ui;
}

void ConfigurationWnd::setParameters(DeviceParameters *params)
{
    if(m_params != nullptr)
    {
        disconnect(m_params,
                   &DeviceParameters::paramChanged,
                   this,
                   &ConfigurationWnd::onParamChanged);
    }

    m_params = params;

    if(m_params != nullptr)
    {
        connect(m_params,
                &DeviceParameters::paramChanged,
                this,
                &ConfigurationWnd::onParamChanged);
    }

    rebuildUi();
}

void ConfigurationWnd::setParamValue(const QString &key, const QString &value)
{
    setFieldValue(key, value, true);
}

void ConfigurationWnd::setFieldEditable(const QString &key, bool editable)
{
    if(fields.contains(key) == false)
    {
        return;
    }

    fields[key]->setReadOnly(!editable);
}

void ConfigurationWnd::setConfigurationAcquiredStatus(bool status)
{
    if(configurationStatusLabel == nullptr)
    {
        return;
    }

    configurationStatusLabel->setText(status ?
                                      "Device status: Configuration acquired successfully" :
                                      "Device status: Configuration acquire failed");

    refreshStatusBar();
}

void ConfigurationWnd::setConfigurationAppliedStatus(bool status)
{
    if(configurationStatusLabel == nullptr)
    {
        return;
    }

    if(status == true)
    {
        configurationStatusLabel->setText("Device status: Configuration applied successfully");

        for(auto it = fields.begin(); it != fields.end(); ++it)
        {
            appliedValues[it.key()] = it.value()->text();
        }
    }
    else
    {
        configurationStatusLabel->setText("Device status: Configuration apply failed");
    }

    refreshStatusBar();
}

void ConfigurationWnd::rebuildUi()
{
    clearUiState();

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    tabWidget = new QTabWidget(this);
    tabWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QTabBar *tabBar = tabWidget->tabBar();
    tabBar->setExpanding(true);
    tabBar->setUsesScrollButtons(false);

    if(m_params != nullptr)
    {
        QList<DeviceParams::GroupMeta> groups = m_params->getAllGroupMeta();

        for(const DeviceParams::GroupMeta &groupMeta : groups)
        {
            QWidget *tab = createTab(groupMeta.id);

            if(tab != nullptr)
            {
                tabWidget->addTab(tab, groupMeta.name);
            }
        }
    }

    mainLayout->addWidget(tabWidget);
    mainLayout->addLayout(createButtonsRow());
    mainLayout->addLayout(createStatusBarLayout());

    setLayout(mainLayout);

    refreshStatusBar();
}

void ConfigurationWnd::clearUiState()
{
    fields.clear();
    appliedValues.clear();
    displayNames.clear();

    tabWidget = nullptr;
    configurationStatusLabel = nullptr;
    changedFieldsLabel = nullptr;
    setConfigButton = nullptr;
    acquireConfigButton = nullptr;

    QLayout *oldLayout = layout();

    if(oldLayout != nullptr)
    {
        QLayoutItem *item = nullptr;

        while((item = oldLayout->takeAt(0)) != nullptr)
        {
            if(item->widget() != nullptr)
            {
                delete item->widget();
            }

            if(item->layout() != nullptr)
            {
                delete item->layout();
            }

            delete item;
        }

        delete oldLayout;
    }
}

QWidget *ConfigurationWnd::createTab(DeviceParams::Group group)
{
    QWidget *tab = new QWidget(this);
    QVBoxLayout *tabMainLayout = new QVBoxLayout(tab);

    QWidget *scrollContent = new QWidget(this);
    scrollContent->setLayout(createGroupLayout(group));

    QScrollArea *scrollArea = new QScrollArea(this);
    scrollArea->setWidget(scrollContent);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);

    tabMainLayout->addWidget(scrollArea);

    return tab;
}

QVBoxLayout *ConfigurationWnd::createGroupLayout(DeviceParams::Group group)
{
    QVBoxLayout *groupLayout = new QVBoxLayout();

    if(m_params == nullptr)
    {
        groupLayout->addStretch();
        return groupLayout;
    }

    QList<DeviceParams::SubGroupMeta> subGroups = m_params->getAllSubGroupMeta();

    for(const DeviceParams::SubGroupMeta &subGroupMeta : subGroups)
    {
        QList<DeviceParams::Param> params =
            m_params->getParamsBySubGroup(group, subGroupMeta.id);

        bool hasVisibleParam = false;

        for(const DeviceParams::Param &param : params)
        {
            if(param.meta.visible == true)
            {
                hasVisibleParam = true;
                break;
            }
        }

        if(hasVisibleParam == false)
        {
            continue;
        }

        QGroupBox *box = createSubGroupBox(group, subGroupMeta.id);

        if(box != nullptr)
        {
            groupLayout->addWidget(box);
        }
    }

    groupLayout->addStretch();

    return groupLayout;
}

QGroupBox *ConfigurationWnd::createSubGroupBox(DeviceParams::Group group,
                                               DeviceParams::SubGroup subGroup)
{
    if(m_params == nullptr)
    {
        return nullptr;
    }

    QList<DeviceParams::Param> params = m_params->getParamsBySubGroup(group, subGroup);

    QList<DeviceParams::Param> visibleParams;

    for(const DeviceParams::Param &param : params)
    {
        if(param.meta.visible == true)
        {
            visibleParams.append(param);
        }
    }

    if(visibleParams.isEmpty() == true)
    {
        return nullptr;
    }

    DeviceParams::SubGroupMeta subGroupMeta = m_params->getSubGroupMeta(subGroup);

    QGroupBox *groupBox = new QGroupBox(subGroupMeta.name, this);
    groupBox->setToolTip(subGroupMeta.description);
    groupBox->setLayout(createParamsGrid(visibleParams));

    return groupBox;
}

QGridLayout *ConfigurationWnd::createParamsGrid(const QList<DeviceParams::Param> &params)
{
    QGridLayout *gridLayout = new QGridLayout();

    for(int i = 0; i < params.size(); i++)
    {
        QWidget *paramWidget = createParamWidget(params[i]);

        int row = i / 3;
        int column = i % 3;

        gridLayout->addWidget(paramWidget, row, column);
    }

    gridLayout->setColumnStretch(0, 1);
    gridLayout->setColumnStretch(1, 1);
    gridLayout->setColumnStretch(2, 1);

    return gridLayout;
}

QWidget *ConfigurationWnd::createParamWidget(const DeviceParams::Param &param)
{
    QWidget *controlWidget = new QWidget(this);
    QVBoxLayout *controlLayout = new QVBoxLayout(controlWidget);
    controlLayout->setContentsMargins(0, 0, 0, 0);

    QLabel *nameLabel = new QLabel(param.meta.displayName, this);
    nameLabel->setMinimumHeight(CONFIG_ROW_HEIGHT);
    nameLabel->setToolTip(param.meta.description);

    QHBoxLayout *fieldLayout = new QHBoxLayout();

    QLineEdit *field = new QLineEdit(this);
    field->setFixedWidth(CONFIG_FIELD_WIDTH);
    field->setMinimumHeight(CONFIG_ROW_HEIGHT);
    field->setToolTip(param.meta.description);

    QLabel *unitLabel = new QLabel(param.meta.unit, this);
    unitLabel->setFixedWidth(CONFIG_UNIT_WIDTH);
    unitLabel->setMinimumHeight(CONFIG_ROW_HEIGHT);

    fieldLayout->addWidget(field);
    fieldLayout->addWidget(unitLabel);
    fieldLayout->addStretch();

    controlLayout->addWidget(nameLabel);
    controlLayout->addLayout(fieldLayout);

    registerField(param, field);

    return controlWidget;
}

QHBoxLayout *ConfigurationWnd::createButtonsRow()
{
    QHBoxLayout *buttonsLayout = new QHBoxLayout();

    configurationStatusLabel = new QLabel("Device status: Configuration not acquired", this);
    configurationStatusLabel->setMinimumHeight(CONFIG_ROW_HEIGHT);

    setConfigButton = new QPushButton("Set config", this);
    setConfigButton->setFixedSize(CONFIG_BUTTON_WIDTH, CONFIG_BUTTON_HEIGHT);

    acquireConfigButton = new QPushButton("Acquire config", this);
    acquireConfigButton->setFixedSize(CONFIG_BUTTON_WIDTH, CONFIG_BUTTON_HEIGHT);

    buttonsLayout->addWidget(configurationStatusLabel);
    buttonsLayout->addStretch();
    buttonsLayout->addWidget(setConfigButton);
    buttonsLayout->addWidget(acquireConfigButton);

    connect(setConfigButton,
            &QPushButton::clicked,
            this,
            &ConfigurationWnd::onSetConfigClicked);

    connect(acquireConfigButton,
            &QPushButton::clicked,
            this,
            &ConfigurationWnd::onAcquireConfigClicked);

    return buttonsLayout;
}

QVBoxLayout *ConfigurationWnd::createStatusBarLayout()
{
    QVBoxLayout *statusLayout = new QVBoxLayout();

    changedFieldsLabel = new QLabel("Changed: None", this);
    changedFieldsLabel->setMinimumHeight(CONFIG_ROW_HEIGHT);

    statusLayout->addWidget(changedFieldsLabel);

    return statusLayout;
}

void ConfigurationWnd::registerField(const DeviceParams::Param &param,
                                     QLineEdit *field)
{
    const QString key = param.meta.key;

    fields[key] = field;
    displayNames[key] = param.meta.displayName;

    if (param.initialized)
    {
        field->setText(param.value.toString());
        appliedValues[key] = param.value.toString();
        field->setStyleSheet("");
        field->setReadOnly(!m_params->isEditable(key));
    }
    else
    {
        field->setText("");
        field->setStyleSheet("QLineEdit { background-color: #e0e0e0; color: #707070; }");
        field->setPlaceholderText("Not acquired");
        appliedValues[key] = QString();
        field->setReadOnly(true);
    }


    connect(field,
            &QLineEdit::textChanged,
            this,
            &ConfigurationWnd::onFieldChanged);
}

void ConfigurationWnd::setFieldValue(const QString &key,
                                     const QString &value,
                                     bool markAsApplied)
{
    if(fields.contains(key) == false)
        return;

    QSignalBlocker blocker(fields[key]);

    fields[key]->setText(value);
    fields[key]->setPlaceholderText("");
    fields[key]->setStyleSheet("");
    if(m_params->getParam(key).meta.access != DeviceParams::Access::ReadWrite){
        fields[key]->setReadOnly(true);
        fields[key]->setStyleSheet("QLineEdit { background-color: #e0e0e0; color: #707070; }");
    }
    else
    {
        fields[key]->setReadOnly(false);
    }

    if(markAsApplied == true)
        appliedValues[key] = value;

    refreshStatusBar();
}

QMap<QString, QString> ConfigurationWnd::getChangedFields() const
{
    QMap<QString, QString> changedFields;

    for(auto it = fields.constBegin(); it != fields.constEnd(); ++it)
    {
        const QString key = it.key();

        if(m_params != nullptr)
        {
            if(m_params->isEditable(key) == false)
                continue;

            if(m_params->isInitialized(key) == false && it.value()->text().isEmpty())
                continue;
        }

        const QString currentValue = it.value()->text();
        const QString appliedValue = appliedValues.value(key);

        if(currentValue != appliedValue)
        {
            changedFields[key] = currentValue;
        }
    }

    return changedFields;
}

QMap<QString, QString> ConfigurationWnd::getChangedFields(DeviceParams::Group group) const
{
    QMap<QString, QString> changedFields;

    if(m_params == nullptr)
    {
        return changedFields;
    }

    QMap<QString, QString> allChangedFields = getChangedFields();

    for(auto it = allChangedFields.constBegin(); it != allChangedFields.constEnd(); ++it)
    {
        if(m_params->hasParam(it.key()) == false)
        {
            continue;
        }

        DeviceParams::ParamMeta meta = m_params->getParamMeta(it.key());

        if(meta.group == group)
        {
            changedFields[it.key()] = it.value();
        }
    }

    return changedFields;
}

QStringList ConfigurationWnd::getChangedFieldDisplayNames() const
{
    QStringList changedNames;

    QMap<QString, QString> changedFields = getChangedFields();

    for(auto it = changedFields.constBegin(); it != changedFields.constEnd(); ++it)
    {
        changedNames.append(displayNames.value(it.key(), it.key()));
    }

    return changedNames;
}

void ConfigurationWnd::refreshStatusBar()
{
    if(changedFieldsLabel == nullptr)
    {
        return;
    }

    QStringList changedNames = getChangedFieldDisplayNames();

    if(changedNames.isEmpty() == true)
    {
        changedFieldsLabel->setText("Changed: None");
    }
    else
    {
        changedFieldsLabel->setText("Changed: " + changedNames.join(", "));

        if(configurationStatusLabel != nullptr)
        {
            configurationStatusLabel->setText("Device status: Configuration changed, not applied");
        }
    }
}

bool ConfigurationWnd::isDeviceParam(const QString &key) const
{
    if(m_params == nullptr || m_params->hasParam(key) == false)
    {
        return false;
    }

    return m_params->getParamMeta(key).group == DeviceParams::Group::DeviceConfig;
}

bool ConfigurationWnd::isApplicationParam(const QString &key) const
{
    if(m_params == nullptr || m_params->hasParam(key) == false)
    {
        return false;
    }

    return m_params->getParamMeta(key).group == DeviceParams::Group::ApplicationConfig;
}

void ConfigurationWnd::onFieldChanged()
{
    refreshStatusBar();
}

void ConfigurationWnd::onParamChanged(QString key, QString value)
{
    setFieldValue(key, value, true);
}

void ConfigurationWnd::onSetConfigClicked()
{
    QMap<QString, QString> changedFields = getChangedFields();

    if(changedFields.isEmpty() == true)
    {
        if(configurationStatusLabel != nullptr)
        {
            configurationStatusLabel->setText("Device status: No configuration changes detected");
        }

        return;
    }

    QMap<QString, QString> changedDeviceFields =
        getChangedFields(DeviceParams::Group::DeviceConfig);

    QMap<QString, QString> changedApplicationFields =
        getChangedFields(DeviceParams::Group::ApplicationConfig);

    if(configurationStatusLabel != nullptr)
    {
        configurationStatusLabel->setText("Device status: Configuration apply requested");
    }

    if(changedDeviceFields.isEmpty() == false)
    {
        emit sigDeviceConfigSet(changedDeviceFields);
    }

    if(changedApplicationFields.isEmpty() == false)
    {
        emit sigApplicationConfigSet(changedApplicationFields);
    }

    emit sigConfigSet(changedFields);
}

void ConfigurationWnd::onAcquireConfigClicked()
{
    if(configurationStatusLabel != nullptr)
    {
        configurationStatusLabel->setText("Device status: Configuration acquire requested");
    }

    emit sigDeviceConfigAcquireRequest();
}
