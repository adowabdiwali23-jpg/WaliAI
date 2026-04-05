#pragma once

#include <QDialog>
#include <QLineEdit>
#include <QComboBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QTextEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QFormLayout>

class PersonalizationState;
class DatabaseManager;

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(PersonalizationState &state, DatabaseManager &db,
                            QWidget *parent = nullptr);

private:
    void setupUi();
    void loadFromState();
    void saveToState();

    PersonalizationState &m_state;
    DatabaseManager &m_db;

    QLineEdit *m_userName = nullptr;
    QTextEdit *m_systemPrompt = nullptr;
    QComboBox *m_theme = nullptr;
    QComboBox *m_voiceProfile = nullptr;
    QSpinBox *m_maxTokens = nullptr;
    QDoubleSpinBox *m_temperature = nullptr;
    QSpinBox *m_contextSize = nullptr;
    QCheckBox *m_memoryEnabled = nullptr;
    QCheckBox *m_ttsEnabled = nullptr;
    QSpinBox *m_uiMaxMessages = nullptr;
};
