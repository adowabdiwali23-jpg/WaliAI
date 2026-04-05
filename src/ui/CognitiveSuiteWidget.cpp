#include "ui/CognitiveSuiteWidget.h"
#include "control/StateManager.h"
#include "state/CognitiveState.h"

CognitiveSuiteWidget::CognitiveSuiteWidget(StateManager &state, QWidget *parent)
    : QWidget(parent), m_state(state)
{
    setupUi();
    connectToggles();
    refreshState();
}

void CognitiveSuiteWidget::setupUi()
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(12, 12, 12, 12);
    layout->setSpacing(8);

    auto *titleLabel = new QLabel("Cognitive Suite");
    titleLabel->setStyleSheet("QLabel { color: #e2e8f0; font-size: 14px; font-weight: bold; }");
    layout->addWidget(titleLabel);

    auto *separator = new QFrame();
    separator->setFrameShape(QFrame::HLine);
    separator->setStyleSheet("QFrame { color: #4a5568; }");
    layout->addWidget(separator);

    QString checkboxStyle =
        "QCheckBox { color: #e2e8f0; font-size: 13px; spacing: 8px; }"
        "QCheckBox::indicator { width: 18px; height: 18px; border-radius: 4px; "
        "border: 2px solid #4a5568; background-color: #2d3748; }"
        "QCheckBox::indicator:checked { background-color: #4299e1; border-color: #4299e1; }";

    m_search = new QCheckBox("Search", this);
    m_search->setToolTip("Enable web search for queries");
    m_search->setStyleSheet(checkboxStyle);
    layout->addWidget(m_search);

    m_research = new QCheckBox("Research", this);
    m_research->setToolTip("Enable deep web research mode");
    m_research->setStyleSheet(checkboxStyle);
    layout->addWidget(m_research);

    m_deepThink = new QCheckBox("DeepThink", this);
    m_deepThink->setToolTip("Enable extended reasoning and analysis");
    m_deepThink->setStyleSheet(checkboxStyle);
    layout->addWidget(m_deepThink);

    layout->addStretch();

    setStyleSheet("QWidget { background-color: #171923; }");
    setFixedWidth(180);
}

void CognitiveSuiteWidget::connectToggles()
{
    connect(m_search, &QCheckBox::toggled, this, [this](bool checked) {
        m_state.cognitiveState().setSearchEnabled(checked);
        emit stateToggled("search", checked);
    });
    connect(m_research, &QCheckBox::toggled, this, [this](bool checked) {
        m_state.cognitiveState().setResearchEnabled(checked);
        emit stateToggled("research", checked);
    });
    connect(m_deepThink, &QCheckBox::toggled, this, [this](bool checked) {
        m_state.cognitiveState().setDeepThinkEnabled(checked);
        emit stateToggled("deep_think", checked);
    });
}

void CognitiveSuiteWidget::refreshState()
{
    const auto &cs = m_state.cognitiveState();
    m_search->setChecked(cs.searchEnabled());
    m_research->setChecked(cs.researchEnabled());
    m_deepThink->setChecked(cs.deepThinkEnabled());
}
