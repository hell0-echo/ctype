#include "tg_space_game_widget.h"
#include "tg_space_controller.h"
#include "tg_space_model.h"
#include "tg_space_view.h"
#include "common/tg_config.h"
#include "ui/tg_dialogs.h"

#include <QCheckBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSlider>
#include <QVBoxLayout>

namespace
{
    bool runSpaceSettings(TgConfig* pConfig, QWidget* pParent)
    {
        QDialog dlg(pParent);
        dlg.setWindowTitle(QStringLiteral("Space settings"));
        auto* pVBox = new QVBoxLayout(&dlg);
        auto* pForm = new QFormLayout;

        auto createSliderRow = [](QSlider*& slider, QLabel*& label, int min, int max, int val) {
            slider = new QSlider(Qt::Horizontal);
            slider->setRange(min, max);
            slider->setValue(val);
            label = new QLabel(QString::number(val));
            auto* row = new QHBoxLayout;
            row->addWidget(slider); row->addWidget(label);
            QObject::connect(slider, &QSlider::valueChanged, [label](int v) { label->setText(QString::number(v)); });
            return row;
            };

        QSlider* pCount, * pSpeed, * pUp;
        QLabel* pCountLabel, * pSpeedLabel, * pUpLabel;
        pForm->addRow(QStringLiteral("Max enemies"), createSliderRow(pCount, pCountLabel, 1, 10, pConfig->spaceEnemyCount()));
        pForm->addRow(QStringLiteral("Speed level"), createSliderRow(pSpeed, pSpeedLabel, 1, 10, pConfig->spaceEnemySpeedLevel()));
        pForm->addRow(QStringLiteral("Upgrade interval (s)"), createSliderRow(pUp, pUpLabel, 30, 300, pConfig->spaceUpgradeIntervalSec()));

        auto* pBonus = new QCheckBox;
        pBonus->setChecked(pConfig->spaceBonusMode());
        pForm->addRow(QStringLiteral("Bonus mode"), pBonus);

        pVBox->addLayout(pForm);
        auto* pBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::RestoreDefaults);
        pVBox->addWidget(pBox);

        QObject::connect(pBox, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
        QObject::connect(pBox, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
        QObject::connect(pBox->button(QDialogButtonBox::RestoreDefaults), &QPushButton::clicked, [&]() {
            pCount->setValue(3); pSpeed->setValue(3); pUp->setValue(120); pBonus->setChecked(false);
            });

        if (dlg.exec() != QDialog::Accepted) return false;

        const bool changed = pCount->value() != pConfig->spaceEnemyCount() ||
            pSpeed->value() != pConfig->spaceEnemySpeedLevel() ||
            pUp->value() != pConfig->spaceUpgradeIntervalSec() ||
            pBonus->isChecked() != pConfig->spaceBonusMode();
        pConfig->setSpaceEnemyCount(pCount->value());
        pConfig->setSpaceEnemySpeedLevel(pSpeed->value());
        pConfig->setSpaceUpgradeIntervalSec(pUp->value());
        pConfig->setSpaceBonusMode(pBonus->isChecked());
        return changed;
    }
}

TgSpaceGameWidget::TgSpaceGameWidget(TgSpaceModel* pModel, TgSpaceController* pController,
    TgSpaceView* pView, TgConfig* pConfig, QWidget* pParent)
    : QWidget(pParent), m_pModel(pModel), m_pController(pController), m_pView(pView), m_pConfig(pConfig)
{
    auto* pRoot = new QVBoxLayout(this);
    pRoot->addWidget(m_pView, 1);

    auto* pBar = new QHBoxLayout;
    m_pStartBtn = new QPushButton(QStringLiteral("Start"));
    m_pPauseBtn = new QPushButton(QStringLiteral("Pause"));
    m_pHighScoreBtn = new QPushButton(QStringLiteral("High Scores"));
    m_pSettingsBtn = new QPushButton(QStringLiteral("Settings"));
    m_pExitBtn = new QPushButton(QStringLiteral("Exit"));

    pBar->addWidget(m_pStartBtn);
    pBar->addWidget(m_pPauseBtn);
    pBar->addWidget(m_pHighScoreBtn);
    pBar->addWidget(m_pSettingsBtn);
    pBar->addWidget(m_pExitBtn);
    pRoot->addLayout(pBar);

    connect(m_pStartBtn, &QPushButton::clicked, this, &TgSpaceGameWidget::onStartClicked);
    connect(m_pPauseBtn, &QPushButton::clicked, this, &TgSpaceGameWidget::onPauseClicked);
    connect(m_pHighScoreBtn, &QPushButton::clicked, this, &TgSpaceGameWidget::onHighScoreClicked);
    connect(m_pSettingsBtn, &QPushButton::clicked, this, &TgSpaceGameWidget::onSettingsClicked);
    connect(m_pExitBtn, &QPushButton::clicked, this, &TgSpaceGameWidget::onExitClicked);

    syncUiState();
}

void TgSpaceGameWidget::syncUiState() { updateButtonStates(); }

void TgSpaceGameWidget::updateButtonStates() {
    if (!m_pModel) return;
    const TgSpaceGameState st = m_pModel->state();
    const bool playing = (st == TgSpaceGameState::PlayingSpaceState);
    const bool paused = (st == TgSpaceGameState::PausedSpaceState);
    const bool initialOrEnd = (st == TgSpaceGameState::InitialSpaceState || st == TgSpaceGameState::EndSpaceState);

    m_pPauseBtn->setEnabled(playing || paused);
    m_pPauseBtn->setText(paused ? QStringLiteral("Resume") : QStringLiteral("Pause"));
    m_pStartBtn->setEnabled(!playing);
    m_pStartBtn->setText(initialOrEnd ? QStringLiteral("Start") : QStringLiteral("Continue"));
    m_pExitBtn->setEnabled(!playing && !paused);
}

void TgSpaceGameWidget::onStartClicked() {
    if (!m_pModel || !m_pController) return;
    m_pModel->setMaxEnemies(m_pConfig->spaceEnemyCount());
    m_pModel->setSpeedLevel(m_pConfig->spaceEnemySpeedLevel());
    m_pModel->setUpgradeIntervalSec(m_pConfig->spaceUpgradeIntervalSec());
    m_pModel->setBonusMode(m_pConfig->spaceBonusMode());
    m_pModel->startFromMenu();
    m_pController->setBonusModeEnabled(m_pConfig->spaceBonusMode());
    m_pController->startLoop();
    m_pView->setFocus();
    updateButtonStates();
}

void TgSpaceGameWidget::onPauseClicked() {
    if (!m_pModel || !m_pController) return;
    if (m_pModel->state() == TgSpaceGameState::PlayingSpaceState) {
        m_pModel->pauseToMenu();
        m_pController->setPaused(true);
        m_pController->stopLoop();
    }
    else if (m_pModel->state() == TgSpaceGameState::PausedSpaceState) {
        m_pModel->resumeFromPause();
        m_pController->setPaused(false);
        m_pController->startLoop();
    }
    m_pView->update();
    updateButtonStates();
}

void TgSpaceGameWidget::onHighScoreClicked() { emit highScoreRequested(); }

void TgSpaceGameWidget::onSettingsClicked() {
    if (!m_pConfig) return;
    const bool changed = runSpaceSettings(m_pConfig, this);
    if (!changed) return;
    if (!tg::AskApplySettingsNow(this)) return;

    m_pModel->setMaxEnemies(m_pConfig->spaceEnemyCount());
    m_pModel->setSpeedLevel(m_pConfig->spaceEnemySpeedLevel());
    m_pModel->setUpgradeIntervalSec(m_pConfig->spaceUpgradeIntervalSec());
    m_pModel->setBonusMode(m_pConfig->spaceBonusMode());
    m_pController->setBonusModeEnabled(m_pConfig->spaceBonusMode());
    m_pConfig->save();
    m_pView->update();
}

void TgSpaceGameWidget::onExitClicked() {
    if (m_pController) m_pController->stopLoop();
    if (m_pModel) m_pModel->resetToInitial();
    m_pView->update();
    emit exitToLauncherRequested();
}

void TgSpaceGameWidget::onRoundEnded() {
    if (m_pController) m_pController->stopLoop();
    m_pView->update();
    updateButtonStates();
}

void TgSpaceGameWidget::refreshLayout() {
    if (m_pView) m_pView->refreshLayout();
    updateButtonStates();
}