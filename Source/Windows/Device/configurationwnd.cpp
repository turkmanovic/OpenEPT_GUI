#include "configurationwnd.h"
#include "ui_configurationwnd.h"
#include "Processing/Parameters/deviceparamdefs.h"

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
#include <QFile>
#include <QFileDialog>

#define CONFIG_LABEL_WIDTH     150
#define CONFIG_FIELD_WIDTH     170
#define CONFIG_UNIT_WIDTH      40
#define CONFIG_ROW_HEIGHT      28
#define CONFIG_BUTTON_WIDTH    32
#define CONFIG_BUTTON_HEIGHT   32

ConfigurationWnd::ConfigurationWnd(QWidget *parent,
                                   ParameterStore *params) :
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
                &ParameterStore::paramChanged,
                this,
                &ConfigurationWnd::onParamChanged);
    }
}

ConfigurationWnd::~ConfigurationWnd()
{
    delete ui;
}

void ConfigurationWnd::setParameters(ParameterStore *params)
{
    if(m_params != nullptr)
    {
        disconnect(m_params,
                   &ParameterStore::paramChanged,
                   this,
                   &ConfigurationWnd::onParamChanged);
    }

    m_params = params;

    if(m_params != nullptr)
    {
        connect(m_params,
                &ParameterStore::paramChanged,
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

void ConfigurationWnd::setBDContent(const QString &content)
{
    if(bdContentTextEdit == nullptr)
        return;

    QString formatted;
    int bytesPerLine = 16;

    /* content je HEX string: 2 char = 1 byte */
    int totalBytes = content.size() / 2;

    for(int i = 0; i < totalBytes; i++)
    {
        if(i % bytesPerLine == 0)
        {
            formatted += QString("%1: ")
                         .arg(i, 8, 16, QChar('0'))
                         .toUpper();
        }

        QString byte = content.mid(i * 2, 2).toUpper();
        formatted += byte + " ";

        if((i + 1) % bytesPerLine == 0)
        {
            formatted += "\n";
        }
    }

    bdContentTextEdit->setPlainText(formatted);
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
        QList<Params::GroupMeta> groups = m_params->getAllGroupMeta();

        for(const Params::GroupMeta &groupMeta : groups)
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

QWidget *ConfigurationWnd::createTab(Params::GroupId group)
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

QVBoxLayout *ConfigurationWnd::createGroupLayout(Params::GroupId group)
{
    QVBoxLayout *groupLayout = new QVBoxLayout();

    if(m_params == nullptr)
    {
        groupLayout->addStretch();
        return groupLayout;
    }

    QList<Params::SubGroupMeta> subGroups = m_params->getAllSubGroupMeta();

    for(const Params::SubGroupMeta &subGroupMeta : subGroups)
    {
        QList<Params::Param> params =
            m_params->getParamsBySubGroup(group, subGroupMeta.id);

        bool hasVisibleParam = false;

        for(const Params::Param &param : params)
        {
            if(param.meta.visible == true)
            {
                hasVisibleParam = true;
                break;
            }
        }

        bool isFileStorage =
            (subGroupMeta.id == static_cast<Params::SubGroupId>(DeviceParamDefs::FileStorage));

        if(hasVisibleParam == false && isFileStorage == false)
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

QGroupBox *ConfigurationWnd::createSubGroupBox(Params::GroupId group,
                                               Params::SubGroupId subGroup)
{
    if(m_params == nullptr)
    {
        return nullptr;
    }

    QList<Params::Param> params = m_params->getParamsBySubGroup(group, subGroup);

    QList<Params::Param> visibleParams;

    bool isFileStorage =
        (subGroup == static_cast<Params::SubGroupId>(DeviceParamDefs::FileStorage));

    for(const Params::Param &param : params)
    {
        if(param.meta.visible == true )
        {
            visibleParams.append(param);
        }
    }

    if(visibleParams.isEmpty() == true )
    {
        return nullptr;
    }

    Params::SubGroupMeta subGroupMeta = m_params->getSubGroupMeta(subGroup);

    QGroupBox *groupBox = new QGroupBox(subGroupMeta.name, this);
    groupBox->setToolTip(subGroupMeta.description);

    if(group == static_cast<Params::GroupId>(DeviceParamDefs::DeviceConfig) &&
       true == isFileStorage)
    {
        QVBoxLayout *vLayout = new QVBoxLayout();


        vLayout->addLayout(createParamsGrid(visibleParams));

        vLayout->addWidget(createBDMemoryWidget());

        groupBox->setLayout(vLayout);
    }
    else
    {
        QGridLayout *grid = createParamsGrid(visibleParams);
        groupBox->setLayout(grid);
    }

    return groupBox;
}

QGridLayout *ConfigurationWnd::createParamsGrid(const QList<Params::Param> &params)
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

QWidget *ConfigurationWnd::createParamWidget(const Params::Param &param)
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

    setConfigButton = new QPushButton("Set", this);
    setConfigButton->setFixedSize(80, CONFIG_BUTTON_HEIGHT);

    acquireConfigButton = new QPushButton("Get", this);
    acquireConfigButton->setFixedSize(80, CONFIG_BUTTON_HEIGHT);

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

void ConfigurationWnd::registerField(const Params::Param &param,
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
    if(m_params->getParam(key).meta.access != Params::Access::ReadWrite){
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

QMap<QString, QString> ConfigurationWnd::getChangedFields(Params::GroupId group) const
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

        Params::ParamMeta meta = m_params->getParamMeta(it.key());

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

    return m_params->getParamMeta(key).group ==
           static_cast<Params::GroupId>(DeviceParamDefs::Group::DeviceConfig);
}

bool ConfigurationWnd::isApplicationParam(const QString &key) const
{
    if(m_params == nullptr || m_params->hasParam(key) == false)
    {
        return false;
    }

    return m_params->getParamMeta(key).group ==
            static_cast<Params::GroupId>(DeviceParamDefs::Group::ApplicationConfig);
}
void ConfigurationWnd::onBDGetClicked()
{
    emit sigBDContentGetRequest();
}

void ConfigurationWnd::onBDUpdateClicked()
{
    QString filePath = QFileDialog::getOpenFileName(
                this,
                "Select file to upload",
                "",
                "Binary (*.bin);;Text (*.txt *.hex);;All Files (*)");

    if(filePath.isEmpty())
        return;

    QFile file(filePath);

    if(!file.open(QIODevice::ReadOnly))
        return;

    QByteArray data;

    /* ================= BIN FILE ================= */
    if(filePath.endsWith(".bin"))
    {
        data = file.readAll();
    }
    else
    {
        /* ================= TEXT / HEX FILE ================= */
        QString text = file.readAll();

        QString clean = text;
        clean.remove(' ');
        clean.remove('\n');
        clean.remove('\r');

        /* ako je HEX */
        QRegularExpression hexRegex("^[0-9A-Fa-f]+$");

        if(hexRegex.match(clean).hasMatch())
        {
            data = QByteArray::fromHex(clean.toUtf8());
        }
        else
        {
            /* fallback: ASCII tekst */
            data = text.toUtf8();
        }
    }

    file.close();

    if(data.isEmpty())
        return;

    emit sigBDContentSetRequest(data);
}

void ConfigurationWnd::onBDFormatClicked()
{
    emit sigBDFormatRequest();
}

void ConfigurationWnd::setBDProgress(int percent, const QString &text)
{
    if(bdProgressBar == nullptr || bdProgressLabel == nullptr)
        return;

    bdProgressBar->setVisible(true);
    bdProgressLabel->setVisible(true);

    bdProgressBar->setValue(percent);
    bdProgressBar->update();
    bdProgressLabel->setText(text);
}

void ConfigurationWnd::resetBDProgress()
{
    if(bdProgressBar == nullptr || bdProgressLabel == nullptr)
        return;

    bdProgressBar->setValue(0);
    bdProgressBar->setVisible(false);
    bdProgressLabel->setVisible(false);
}


QWidget* ConfigurationWnd::createBDMemoryWidget()
{
    QWidget *container = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(container);

    /* Header */
    QHBoxLayout *headerLayout = new QHBoxLayout();

    QLabel *title = new QLabel("Memory Content", this);
    QFont font = title->font();
    font.setBold(true);
    title->setFont(font);

    QString btnStyle = R"(
    QPushButton {
        background: transparent;
        border: none;
    }
    QPushButton:hover {
        background-color: rgba(255,255,255,30);
    }
    QPushButton:pressed {
        background-color: rgba(255,255,255,60);
    }
    QToolTip {
        background-color: white;
        color: black;
        border: 1px solid #ccc;
    }
    )";

    bdGetButton = new QPushButton(this);
    bdGetButton->setFixedSize(CONFIG_BUTTON_WIDTH, CONFIG_BUTTON_HEIGHT);
    bdGetButton->setIcon(QIcon(":/images/NewSet/upload.png"));
    bdGetButton->setIconSize(QSize(24, 24));
    bdGetButton->setToolTip("Get memory content");

    bdUpdateButton = new QPushButton(this);
    bdUpdateButton->setFixedSize(CONFIG_BUTTON_WIDTH, CONFIG_BUTTON_HEIGHT);
    bdUpdateButton->setIcon(QIcon(":/images/NewSet/download.png"));
    bdUpdateButton->setIconSize(QSize(24, 24));
    bdUpdateButton->setToolTip("Update memory content");

    bdFormatButton = new QPushButton(this);
    bdFormatButton->setFixedSize(CONFIG_BUTTON_WIDTH, CONFIG_BUTTON_HEIGHT);
    bdFormatButton->setIcon(QIcon(":/images/NewSet/format.png"));
    bdFormatButton->setIconSize(QSize(24, 24));
    bdFormatButton->setToolTip("Format memory");

    bdGetButton->setStyleSheet(btnStyle);
    bdUpdateButton->setStyleSheet(btnStyle);
    bdFormatButton->setStyleSheet(btnStyle);

    headerLayout->addWidget(title);
    headerLayout->addStretch();



    /* ===== Progress Section ===== */



    QHBoxLayout *progressLayout = new QHBoxLayout();


    bdProgressBar = new QProgressBar(this);
    bdProgressBar->setRange(0, 100);
    bdProgressBar->setValue(0);
    bdProgressBar->setTextVisible(true);
    bdProgressBar->setMinimumHeight(18);
    bdProgressBar->setMaximumHeight(18);

    bdProgressLabel = new QLabel("Idle", this);
    bdProgressLabel->setMinimumHeight(CONFIG_ROW_HEIGHT);
    bdProgressLabel->setMaximumHeight(CONFIG_ROW_HEIGHT);

    /* inicijalno sakriveno */
    bdProgressBar->setVisible(true);
    bdProgressLabel->setVisible(true);

    progressLayout->addWidget(bdProgressBar);
    progressLayout->addWidget(bdProgressLabel);

    headerLayout->addLayout(progressLayout);
    headerLayout->addStretch();
    headerLayout->addWidget(bdGetButton);
    headerLayout->addWidget(bdUpdateButton);
    headerLayout->addWidget(bdFormatButton);


    layout->addLayout(headerLayout);

    /* ===== Text area ===== */

    /* Text area */
    bdContentTextEdit = new QTextEdit(this);
    bdContentTextEdit->setReadOnly(true);
    bdContentTextEdit->setMinimumHeight(250);

    QFont mono("Courier New");
    bdContentTextEdit->setFont(mono);

    layout->addWidget(bdContentTextEdit);

    /* connections */
    connect(bdGetButton,
            &QPushButton::clicked,
            this,
            &ConfigurationWnd::onBDGetClicked);

    connect(bdUpdateButton,
            &QPushButton::clicked,
            this,
            &ConfigurationWnd::onBDUpdateClicked);

    connect(bdFormatButton,
            &QPushButton::clicked,
            this,
            &ConfigurationWnd::onBDFormatClicked);

    return container;
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
        getChangedFields(static_cast<Params::GroupId>(DeviceParamDefs::Group::DeviceConfig));

    QMap<QString, QString> changedApplicationFields =
        getChangedFields(static_cast<Params::GroupId>(DeviceParamDefs::Group::ApplicationConfig));

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
