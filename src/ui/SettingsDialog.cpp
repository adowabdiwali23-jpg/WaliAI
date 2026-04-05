#include "ui/SettingsDialog.h"
#include "state/PersonalizationState.h"
#include "infrastructure/DatabaseManager.h"

#include <QTabWidget>
#include <QDialogButtonBox>
#include <QGroupBox>

SettingsDialog::SettingsDialog(PersonalizationState &state, DatabaseManager &db,
                               QWidget *parent)
    : QDialog(parent), m_state(state), m_db(db)
{
    setWindowTitle("Settings");
    setMinimumSize(500, 550);
    setupUi();
    loadFromState();
}

void SettingsDialog::setupUi()
{
    auto *mainLayout = new QVBoxLayout(this);

    auto *tabs = new QTabWidget(this);

    // General tab
    auto *generalTab = new QWidget();
    auto *generalLayout = new QFormLayout(generalTab);
    generalLayout->setSpacing(10);

    m_userName = new QLineEdit(this);
    generalLayout->addRow("User Name:", m_userName);

    m_theme = new QComboBox(this);
    m_theme->addItems({"dark", "light"});
    generalLayout->addRow("Theme:", m_theme);

    m_memoryEnabled = new QCheckBox("Enable persistent memory", this);
    generalLayout->addRow("Memory:", m_memoryEnabled);

    m_uiMaxMessages = new QSpinBox(this);
    m_uiMaxMessages->setRange(100, 10000);
    m_uiMaxMessages->setSingleStep(100);
    generalLayout->addRow("Max UI Messages:", m_uiMaxMessages);

    tabs->addTab(generalTab, "General");

    // Model tab
    auto *modelTab = new QWidget();
    auto *modelLayout = new QFormLayout(modelTab);
    modelLayout->setSpacing(10);

    m_maxTokens = new QSpinBox(this);
    m_maxTokens->setRange(128, 8192);
    m_maxTokens->setSingleStep(128);
    modelLayout->addRow("Max Tokens:", m_maxTokens);

    m_temperature = new QDoubleSpinBox(this);
    m_temperature->setRange(0.0, 2.0);
    m_temperature->setSingleStep(0.1);
    m_temperature->setDecimals(2);
    modelLayout->addRow("Temperature:", m_temperature);

    m_contextSize = new QSpinBox(this);
    m_contextSize->setRange(1024, 32768);
    m_contextSize->setSingleStep(1024);
    modelLayout->addRow("Context Window:", m_contextSize);

    m_systemPrompt = new QTextEdit(this);
    m_systemPrompt->setMaximumHeight(120);
    modelLayout->addRow("System Prompt:", m_systemPrompt);

    tabs->addTab(modelTab, "Model");

    // Voice tab
    auto *voiceTab = new QWidget();
    auto *voiceLayout = new QFormLayout(voiceTab);
    voiceLayout->setSpacing(10);

    m_ttsEnabled = new QCheckBox("Enable Text-to-Speech", this);
    voiceLayout->addRow("TTS:", m_ttsEnabled);

    m_voiceProfile = new QComboBox(this);
    m_voiceProfile->addItems({"breeze", "ember", "cove", "juniper", "sky"});
    voiceLayout->addRow("Voice Profile:", m_voiceProfile);

    tabs->addTab(voiceTab, "Voice");

    mainLayout->addWidget(tabs);

    // Buttons
    auto *buttons = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    mainLayout->addWidget(buttons);

    connect(buttons, &QDialogButtonBox::accepted, this, [this]() {
        saveToState();
        accept();
    });
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    setStyleSheet(
        "QDialog { background-color: #1a202c; }"
        "QLabel { color: #e2e8f0; }"
        "QLineEdit, QTextEdit, QComboBox, QSpinBox, QDoubleSpinBox { "
        "  background-color: #2d3748; color: #e2e8f0; "
        "  border: 1px solid #4a5568; border-radius: 4px; padding: 4px; }"
        "QCheckBox { color: #e2e8f0; }"
        "QTabWidget::pane { border: 1px solid #4a5568; }"
        "QTabBar::tab { background-color: #2d3748; color: #e2e8f0; padding: 8px 16px; }"
        "QTabBar::tab:selected { background-color: #4299e1; }"
    );
}

void SettingsDialog::loadFromState()
{
    m_userName->setText(m_state.userName());
    m_systemPrompt->setPlainText(m_state.systemPrompt());
    m_theme->setCurrentText(m_state.theme());
    m_voiceProfile->setCurrentText(m_state.voiceProfile());
    m_maxTokens->setValue(m_state.maxTokens());
    m_temperature->setValue(m_state.temperature());
    m_contextSize->setValue(m_state.contextWindowSize());
    m_memoryEnabled->setChecked(m_state.memoryEnabled());
    m_ttsEnabled->setChecked(m_state.ttsEnabled());
    m_uiMaxMessages->setValue(m_state.uiMaxMessages());
}

void SettingsDialog::saveToState()
{
    m_state.setUserName(m_userName->text());
    m_state.setSystemPrompt(m_systemPrompt->toPlainText());
    m_state.setTheme(m_theme->currentText());
    m_state.setVoiceProfile(m_voiceProfile->currentText());
    m_state.setMaxTokens(m_maxTokens->value());
    m_state.setTemperature(m_temperature->value());
    m_state.setContextWindowSize(m_contextSize->value());
    m_state.setMemoryEnabled(m_memoryEnabled->isChecked());
    m_state.setTtsEnabled(m_ttsEnabled->isChecked());
    m_state.setUiMaxMessages(m_uiMaxMessages->value());

    m_db.saveSettings(m_state);
}
