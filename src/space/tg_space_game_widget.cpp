#include "tg_space_game_widget.h"
#include "tg_space_controller.h"
#include "tg_space_model.h"
#include "tg_space_view.h"
#include "tg_space_menu_widget.h"
#include "tg_space_rank_widget.h"
#include "common/tg_config.h"
#include "common/tg_audiomanager.h"
#include "common/tg_resourceprovider.h"
#include "common/tg_scorestore.h"
#include "ui/tg_dialogs.h"
#include "ui/tg_imagebutton.h"
#include "ui/tg_imagecheckbox.h"

#include <QDialog>
#include <QCheckBox>
#include <QLabel>
#include <QSlider>
#include <QVBoxLayout>
#include <QPainter>
#include <QCoreApplication>
#include <QDir>
#include <QStackedWidget>

namespace
{
    bool runSpaceSettings(TgConfig* pConfig, QWidget* pParent)
    {
        class SpaceSettingsDialog : public QDialog
        {
        public:
            explicit SpaceSettingsDialog(TgConfig* pConfig, QWidget* pParent)
                : QDialog(pParent)
                , m_pConfig(pConfig)
                , m_bg(m_rp.spacePixmap(QStringLiteral("APPLE_SETUP")))
            {
                setModal(true);
                setWindowFlags((windowFlags() & ~Qt::WindowContextHelpButtonHint) | Qt::Dialog);
                if (!m_bg.isNull())
                    setFixedSize(m_bg.size());

                m_pCount = new QSlider(Qt::Horizontal, this);
                m_pSpeed = new QSlider(Qt::Horizontal, this);
                m_pUp = new QSlider(Qt::Horizontal, this);

                m_pCount->setRange(1, 10);
                m_pSpeed->setRange(1, 10);
                m_pUp->setRange(30, 300);

                m_pCount->setValue(m_pConfig->spaceEnemyCount());
                m_pSpeed->setValue(m_pConfig->spaceEnemySpeedLevel());
                m_pUp->setValue(m_pConfig->spaceUpgradeIntervalSec());

                m_pCountLabel = new QLabel(QString::number(m_pCount->value()), this);
                m_pSpeedLabel = new QLabel(QString::number(m_pSpeed->value()), this);
                m_pUpLabel = new QLabel(QString::number(m_pUp->value()), this);

                connect(m_pCount, &QSlider::valueChanged, this, [this](int v) { m_pCountLabel->setText(QString::number(v)); });
                connect(m_pSpeed, &QSlider::valueChanged, this, [this](int v) { m_pSpeedLabel->setText(QString::number(v)); });
                connect(m_pUp, &QSlider::valueChanged, this, [this](int v) { m_pUpLabel->setText(QString::number(v)); });
                connect(m_pCount, &QSlider::sliderReleased, this, []() { TgAudioManager::instance().playCommon(QStringLiteral("GLIDE")); });
                connect(m_pSpeed, &QSlider::sliderReleased, this, []() { TgAudioManager::instance().playCommon(QStringLiteral("GLIDE")); });
                connect(m_pUp, &QSlider::sliderReleased, this, []() { TgAudioManager::instance().playCommon(QStringLiteral("GLIDE")); });

                applySliderSkin(m_pCount);
                applySliderSkin(m_pSpeed);
                applySliderSkin(m_pUp);

                m_pOk = new TgImageButton(m_rp.commonPixmap(QStringLiteral("OK")), this);
                m_pCancel = new TgImageButton(m_rp.commonPixmap(QStringLiteral("CANCEL")), this);
                m_pDefault = new TgImageButton(m_rp.commonPixmap(QStringLiteral("DEFAULT")), this);

                connect(m_pOk, &TgImageButton::clicked, this, &QDialog::accept);
                connect(m_pCancel, &TgImageButton::clicked, this, &QDialog::reject);
                connect(m_pDefault, &TgImageButton::clicked, this, [this]() { restoreDefaults(); });

                initCheckbox();
                layoutFixed();
            }

            bool changed() const
            {
                return m_pCount->value() != m_pConfig->spaceEnemyCount()
                    || m_pSpeed->value() != m_pConfig->spaceEnemySpeedLevel()
                    || m_pUp->value() != m_pConfig->spaceUpgradeIntervalSec()
                    || (m_pBonus ? m_pBonus->isChecked() : false) != m_pConfig->spaceBonusMode();
            }

            void applyToConfig()
            {
                m_pConfig->setSpaceEnemyCount(m_pCount->value());
                m_pConfig->setSpaceEnemySpeedLevel(m_pSpeed->value());
                m_pConfig->setSpaceUpgradeIntervalSec(m_pUp->value());
                m_pConfig->setSpaceBonusMode(m_pBonus ? m_pBonus->isChecked() : false);
            }

        protected:
            void paintEvent(QPaintEvent* pEvent) override
            {
                Q_UNUSED(pEvent);
                QPainter painter(this);
                painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
                if (!m_bg.isNull())
                    painter.drawPixmap(rect(), m_bg);
            }

        private:
            void layoutFixed()
            {
                const QFont font(QStringLiteral("SimHei"), 10, 10);

                auto applyLabel = [&](QLabel* lbl, const QString& text, int x, int y) {
                    lbl->setFont(font);
                    lbl->setText(text);
                    lbl->adjustSize();
                    lbl->move(x, y);
                };

                m_pLabelCount = new QLabel(this);
                m_pLabelSpeed = new QLabel(this);
                m_pLabelUp = new QLabel(this);
                m_pCountLabel->setFont(font);
                m_pSpeedLabel->setFont(font);
                m_pUpLabel->setFont(font);

                applyLabel(m_pLabelCount, QString::fromUtf16(u"\u654c\u4eba\u6570\u91cf:"), 120, 80);
                applyLabel(m_pLabelSpeed, QString::fromUtf16(u"\u654c\u4eba\u901f\u5ea6:"), 120, 140);
                applyLabel(m_pLabelUp, QString::fromUtf16(u"\u5347\u7ea7\u95f4\u9694:"), 120, 200);

                m_pCount->setFixedWidth(200);
                m_pSpeed->setFixedWidth(200);
                m_pUp->setFixedWidth(200);
                m_pCount->move(200, 80);
                m_pSpeed->move(200, 140);
                m_pUp->move(200, 200);

                m_pCountLabel->move(420, 80);
                m_pSpeedLabel->move(420, 140);
                m_pUpLabel->move(420, 200);

                if (m_pBonus)
                    m_pBonus->move(200, 260);

                m_pOk->setFixedSize(m_pOk->sizeHint());
                m_pCancel->setFixedSize(m_pCancel->sizeHint());
                m_pDefault->setFixedSize(m_pDefault->sizeHint());
                const int btnY = 330;
                m_pOk->move(180, btnY);
                m_pCancel->move(270, btnY);
                m_pDefault->move(360, btnY);
            }

            void initCheckbox()
            {
                m_pBonus = new QCheckBox(QString::fromUtf16(u"\u662f\u5426\u5f00\u542f\u5956\u52b1\u6a21\u5f0f"), this);
                m_pBonus->setChecked(m_pConfig->spaceBonusMode());

                const QString base = QCoreApplication::applicationDirPath() + QDir::separator()
                    + "res" + QDir::separator() + "Space" + QDir::separator()
                    + "Images" + QDir::separator();
                const QString indicatorPath = base + "CHECKBOX_BUTTON.png";

                const QString qss = QStringLiteral(R"(
QCheckBox:focus { outline: none; }
QCheckBox { spacing: 8px; }
QCheckBox::indicator { width: 20px; height: 20px; border: none; }
QCheckBox::indicator:unchecked { border-image: url(%1) 0 60 0 0 stretch; }
QCheckBox::indicator:unchecked:hover { border-image: url(%1) 0 40 0 20 stretch; }
QCheckBox::indicator:checked { border-image: url(%1) 0 20 0 40 stretch; }
QCheckBox::indicator:checked:hover { border-image: url(%1) 0 0 0 60 stretch; }
)").arg(indicatorPath);
                m_pBonus->setStyleSheet(qss);
            }

            void restoreDefaults()
            {
                m_pCount->setValue(3);
                m_pSpeed->setValue(3);
                m_pUp->setValue(120);
                if (m_pBonus)
                    m_pBonus->setChecked(false);
            }

            void applySliderSkin(QSlider* pSlider)
            {
                // Match the reference project's QSS (groove repeat-x, handle has normal/pressed halves).
                const QString base = QCoreApplication::applicationDirPath() + QDir::separator()
                    + "res" + QDir::separator() + "Common" + QDir::separator()
                    + "Images" + QDir::separator();
                const QString groovePath = base + "SLIDER_BG.png";
                const QString handlePath = base + "SLIDER_SLIDER.png";
                const QString qss = QStringLiteral(R"(
QSlider::groove:horizontal {
	background-image: url(%1);
	height: 4px;
	border: none;
	background-repeat: repeat-x;
}
QSlider::handle:horizontal {
	border-image: url(%2) 0 11 0 0 none none;
	border-width: 0;
	width: 11px;
	height: 23px;
	margin: -9.5px 0;
	border: none;
}
QSlider::handle:horizontal:pressed {
	border-image: url(%2) 0 0 0 11 none none;
	border-width: 0;
}
)").arg(groovePath, handlePath);
                pSlider->setStyleSheet(qss);
            }

            TgConfig* m_pConfig = nullptr;
            TgResourceProvider m_rp;
            QPixmap m_bg;

            QSlider* m_pCount = nullptr;
            QSlider* m_pSpeed = nullptr;
            QSlider* m_pUp = nullptr;
            QLabel* m_pCountLabel = nullptr;
            QLabel* m_pSpeedLabel = nullptr;
            QLabel* m_pUpLabel = nullptr;
            QLabel* m_pLabelCount = nullptr;
            QLabel* m_pLabelSpeed = nullptr;
            QLabel* m_pLabelUp = nullptr;
            QCheckBox* m_pBonus = nullptr;
            TgImageButton* m_pOk = nullptr;
            TgImageButton* m_pCancel = nullptr;
            TgImageButton* m_pDefault = nullptr;
        };

        SpaceSettingsDialog dlg(pConfig, pParent);
        if (dlg.exec() != QDialog::Accepted) return false;
        const bool changed = dlg.changed();
        dlg.applyToConfig();
        return changed;
    }
}

TgSpaceGameWidget::TgSpaceGameWidget(TgSpaceModel* pModel, TgSpaceController* pController,
    TgSpaceView* pView, TgConfig* pConfig, TgScoreStore* pScores, TgResourceProvider* pResources, QWidget* pParent)
    : QWidget(pParent)
    , m_pModel(pModel)
    , m_pController(pController)
    , m_pView(pView)
    , m_pConfig(pConfig)
    , m_pScores(pScores)
    , m_pResources(pResources)
{
    auto* pRoot = new QVBoxLayout(this);
    pRoot->setContentsMargins(0, 0, 0, 0);

    m_pStack = new QStackedWidget(this);
    pRoot->addWidget(m_pStack, 1);

    m_pMenu = new TgSpaceMenuWidget(m_pResources, m_pStack);
    m_pRank = new TgSpaceRankWidget(m_pResources, m_pScores, m_pStack);

    m_pStack->addWidget(m_pMenu); // index 0
    m_pStack->addWidget(m_pRank); // index 1
    m_pStack->addWidget(m_pView); // index 2

    connect(m_pMenu, &TgSpaceMenuWidget::startClicked, this, &TgSpaceGameWidget::onMenuStart);
    connect(m_pMenu, &TgSpaceMenuWidget::rankClicked, this, &TgSpaceGameWidget::onMenuRank);
    connect(m_pMenu, &TgSpaceMenuWidget::optionClicked, this, &TgSpaceGameWidget::onMenuOption);
    connect(m_pMenu, &TgSpaceMenuWidget::exitClicked, this, &TgSpaceGameWidget::onMenuExit);
    connect(m_pRank, &TgSpaceRankWidget::returnClicked, this, &TgSpaceGameWidget::onRankReturn);

    if (m_pView)
        connect(m_pView, &TgSpaceView::escapePressed, this, &TgSpaceGameWidget::onEscapeMenuRequested);

    showMenuPage();
    syncUiState();
}

void TgSpaceGameWidget::syncUiState() { updateButtonStates(); }

void TgSpaceGameWidget::updateButtonStates() {
    if (!m_pModel) return;
    const TgSpaceGameState st = m_pModel->state();
    const bool playing = (st == TgSpaceGameState::PlayingSpaceState);
    const bool paused = (st == TgSpaceGameState::PausedSpaceState);
    if (m_pMenu)
        m_pMenu->setStartAsReturn(paused);
    Q_UNUSED(playing);
}

void TgSpaceGameWidget::onStartClicked() {
    if (!m_pModel || !m_pController) return;
    TgAudioManager::instance().setEnabled(true);
    TgAudioManager::instance().setPaused(false);
    TgAudioManager::instance().startBgm(TgAudioManager::Game::Space);
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
        TgAudioManager::instance().setPaused(true);
    }
    else if (m_pModel->state() == TgSpaceGameState::PausedSpaceState) {
        m_pModel->resumeFromPause();
        m_pController->setPaused(false);
        m_pController->startLoop();
        TgAudioManager::instance().setPaused(false);
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
    TgAudioManager::instance().stopBgm();
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

void TgSpaceGameWidget::showMenuPage()
{
    if (m_pStack)
        m_pStack->setCurrentIndex(0);
    if (m_pMenu)
        m_pMenu->refreshLayout();
    updateButtonStates();
}

void TgSpaceGameWidget::showRankPage()
{
    if (m_pStack)
        m_pStack->setCurrentIndex(1);
    if (m_pRank)
        m_pRank->refreshLayout();
}

void TgSpaceGameWidget::showGamePage()
{
    if (m_pStack)
        m_pStack->setCurrentIndex(2);
    if (m_pView)
        m_pView->setFocus();
}

void TgSpaceGameWidget::onEscapeMenuRequested()
{
    if (!m_pModel || !m_pController)
        return;
    if (m_pModel->state() == TgSpaceGameState::PlayingSpaceState)
    {
        m_pModel->pauseToMenu();
        m_pController->setPaused(true);
        m_pController->stopLoop();
        TgAudioManager::instance().setPaused(true);
    }
    showMenuPage();
}

void TgSpaceGameWidget::onMenuStart()
{
    if (!m_pModel || !m_pController)
        return;
    if (m_pModel->state() == TgSpaceGameState::PausedSpaceState)
    {
        // Return to game.
        m_pModel->resumeFromPause();
        m_pController->setPaused(false);
        m_pController->startLoop();
        TgAudioManager::instance().setPaused(false);
        showGamePage();
        return;
    }

    // Start new round.
    onStartClicked();
    showGamePage();
}

void TgSpaceGameWidget::onMenuRank()
{
    showRankPage();
}

void TgSpaceGameWidget::onMenuOption()
{
    onSettingsClicked();
}

void TgSpaceGameWidget::onMenuExit()
{
    onExitClicked();
}

void TgSpaceGameWidget::onRankReturn()
{
    showMenuPage();
}