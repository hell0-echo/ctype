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
#include "ui/tg_skinslider.h"
#include "ui/tg_bonuswordprovider.h"

#include <QDialog>
#include <QLineEdit>
#include <QLabel>
#include <QVBoxLayout>
#include <QPainter>
#include <QCoreApplication>
#include <QDir>
#include <QStackedWidget>

namespace
{
bool runSpaceSettings(TgConfig *pConfig, TgBonusWordProvider *pBonusProvider, QWidget *pParent)
{
	class SpaceSettingsDialog : public QDialog
	{
	public:
		explicit SpaceSettingsDialog(TgConfig *pConfig, TgBonusWordProvider *pBonusProvider, QWidget *pParent)
			: QDialog(pParent)
			, m_pConfig(pConfig)
			, m_pBonusProvider(pBonusProvider)
			, m_bg(m_rp.spacePixmap(QStringLiteral("APPLE_SETUP")))
		{
			setModal(true);
			setWindowFlags((windowFlags() & ~Qt::WindowContextHelpButtonHint) | Qt::Dialog);
			if (!m_bg.isNull())
				setFixedSize(m_bg.size());

			const QPixmap groove = m_rp.commonPixmap(QStringLiteral("SLIDER_BG"));
			const QPixmap handle = m_rp.commonPixmap(QStringLiteral("SLIDER_SLIDER"));
			m_pCount = new TgSkinSlider(groove, handle, Qt::Horizontal, this);
			m_pSpeed = new TgSkinSlider(groove, handle, Qt::Horizontal, this);
			m_pUp = new TgSkinSlider(groove, handle, Qt::Horizontal, this);

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

			m_pOk = new TgImageButton(m_rp.commonPixmap(QStringLiteral("OK")), this);
			m_pCancel = new TgImageButton(m_rp.commonPixmap(QStringLiteral("CANCEL")), this);
			m_pDefault = new TgImageButton(m_rp.commonPixmap(QStringLiteral("DEFAULT")), this);

			connect(m_pOk, &TgImageButton::clicked, this, &QDialog::accept);
			connect(m_pCancel, &TgImageButton::clicked, this, &QDialog::reject);
			connect(m_pDefault, &TgImageButton::clicked, this, [this]() { restoreDefaults(); });

			m_pKeyEdit = new QLineEdit(this);
			m_pKeyEdit->setEchoMode(QLineEdit::Password);
			m_pKeyEdit->setText(m_pConfig->spaceBonusApiKey());

			initCheckbox();
			buildFixedLayout();
		}

		bool changed() const
		{
			return m_pCount->value() != m_pConfig->spaceEnemyCount()
				|| m_pSpeed->value() != m_pConfig->spaceEnemySpeedLevel()
				|| m_pUp->value() != m_pConfig->spaceUpgradeIntervalSec()
				|| (m_pBonus ? m_pBonus->isChecked() : false) != m_pConfig->spaceBonusMode()
				|| (m_pKeyEdit ? m_pKeyEdit->text().trimmed() : QString()) != m_pConfig->spaceBonusApiKey();
		}

		void applyToConfig()
		{
			m_pConfig->setSpaceEnemyCount(m_pCount->value());
			m_pConfig->setSpaceEnemySpeedLevel(m_pSpeed->value());
			m_pConfig->setSpaceUpgradeIntervalSec(m_pUp->value());
			m_pConfig->setSpaceBonusMode(m_pBonus ? m_pBonus->isChecked() : false);
			m_pConfig->setSpaceBonusApiKey(m_pKeyEdit ? m_pKeyEdit->text().trimmed() : QString());
			if (m_pBonusProvider)
				m_pBonusProvider->setApiKey(m_pConfig->spaceBonusApiKey());
		}

	protected:
		void paintEvent(QPaintEvent *pEvent) override
		{
			Q_UNUSED(pEvent);
			QPainter painter(this);
			painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
			if (!m_bg.isNull())
				painter.drawPixmap(rect(), m_bg);
		}

	private:
		void buildFixedLayout()
		{
			// Fixed coordinates on APPLE_SETUP background (same pattern as AppleSettingsDialog::buildFixedLayout).
			const QSize bgSize = size();
			const qreal baseW = static_cast<qreal>(bgSize.width());
			const qreal baseH = static_cast<qreal>(bgSize.height());
			const qreal s = 1.0;
			Q_UNUSED(baseW);
			Q_UNUSED(baseH);

			const QFont font(QStringLiteral("SimHei"), 10, 10);

			auto applyLabel = [&](QLabel *lbl, const QString &text, int x, int y) {
				lbl->setFont(font);
				lbl->setText(text);
				lbl->adjustSize();
				lbl->move(static_cast<int>(x * s), static_cast<int>(y * s));
			};

			m_pLabelCount = new QLabel(this);
			m_pLabelSpeed = new QLabel(this);
			m_pLabelUp = new QLabel(this);
			m_pLabelBonus = new QLabel(this);
			m_pLabelApiKey = new QLabel(this);
			m_pCountLabel->setFont(font);
			m_pSpeedLabel->setFont(font);
			m_pUpLabel->setFont(font);

			applyLabel(m_pLabelCount, QString::fromUtf16(u"\u654c\u4eba\u6570\u91cf:"), 120, 25);
			applyLabel(m_pLabelSpeed, QString::fromUtf16(u"\u654c\u4eba\u901f\u5ea6:"), 120, 80);
			applyLabel(m_pLabelUp, QString::fromUtf16(u"\u5347\u7ea7\u95f4\u9694:"), 120, 135);
			applyLabel(m_pLabelBonus, QString::fromUtf16(u"\u5956\u52b1\u6a21\u5f0f:"), 120, 190);
			applyLabel(m_pLabelApiKey, QString::fromUtf16(u"API Key(DeepSeek):"), 32, 236);

			auto applySlider = [&](QSlider *slider, int x, int y) {
				slider->setFixedWidth(static_cast<int>(180 * s));
				slider->move(static_cast<int>(x * s), static_cast<int>(y * s));
			};

			applySlider(m_pCount, 220, 25);
			applySlider(m_pSpeed, 220, 80);
			applySlider(m_pUp, 220, 135);

			m_pCountLabel->move(static_cast<int>(415 * s), static_cast<int>(25 * s));
			m_pSpeedLabel->move(static_cast<int>(415 * s), static_cast<int>(80 * s));
			m_pUpLabel->move(static_cast<int>(415 * s), static_cast<int>(135 * s));

			if (m_pBonus)
			{
				m_pBonus->setFixedSize(m_pBonus->sizeHint());
				m_pBonus->move(static_cast<int>(220 * s), static_cast<int>(190 * s));
			}

			m_pKeyEdit->setFont(font);
			m_pKeyEdit->setFixedWidth(static_cast<int>(200 * s));
			m_pKeyEdit->move(static_cast<int>(220 * s), static_cast<int>(233 * s));

			m_pOk->setFixedSize(m_pOk->sizeHint());
			m_pCancel->setFixedSize(m_pCancel->sizeHint());
			m_pDefault->setFixedSize(m_pDefault->sizeHint());
			const int bottomMargin = static_cast<int>(18 * s);
			const int btnY = height() - bottomMargin - m_pOk->height();
			m_pOk->move(static_cast<int>(180 * s), btnY);
			m_pCancel->move(static_cast<int>(270 * s), btnY);
			m_pDefault->move(static_cast<int>(360 * s), btnY);
		}

		void initCheckbox()
		{
			const QPixmap sprite = m_rp.spacePixmap(QStringLiteral("CHECKBOX_BUTTON"));
			m_pBonus = new TgImageCheckBox(sprite, this);
			m_pBonus->setChecked(m_pConfig->spaceBonusMode());
		}

		void restoreDefaults()
		{
			m_pCount->setValue(3);
			m_pSpeed->setValue(3);
			m_pUp->setValue(120);
			if (m_pBonus)
				m_pBonus->setChecked(false);
			if (m_pKeyEdit)
				m_pKeyEdit->clear();
		}

		// Slider skin is drawn by TgSkinSlider (no QSS).

		TgConfig *m_pConfig = nullptr;
		TgBonusWordProvider *m_pBonusProvider = nullptr;
		TgResourceProvider m_rp;
		QPixmap m_bg;

		QSlider *m_pCount = nullptr;
		QSlider *m_pSpeed = nullptr;
		QSlider *m_pUp = nullptr;
		QLabel *m_pCountLabel = nullptr;
		QLabel *m_pSpeedLabel = nullptr;
		QLabel *m_pUpLabel = nullptr;
		QLabel *m_pLabelCount = nullptr;
		QLabel *m_pLabelSpeed = nullptr;
		QLabel *m_pLabelUp = nullptr;
		QLabel *m_pLabelBonus = nullptr;
		QLabel *m_pLabelApiKey = nullptr;
		QLineEdit *m_pKeyEdit = nullptr;
		TgImageCheckBox *m_pBonus = nullptr;
		TgImageButton *m_pOk = nullptr;
		TgImageButton *m_pCancel = nullptr;
		TgImageButton *m_pDefault = nullptr;
	};

	SpaceSettingsDialog dlg(pConfig, pBonusProvider, pParent);
	if (dlg.exec() != QDialog::Accepted)
		return false;
	const bool changed = dlg.changed();
	dlg.applyToConfig();
	return changed;
}
}

TgSpaceGameWidget::TgSpaceGameWidget(TgSpaceModel* pModel, TgSpaceController* pController,
    TgSpaceView* pView, TgConfig* pConfig, TgScoreStore* pScores, TgResourceProvider* pResources,
    TgBonusWordProvider* pBonusWords, QWidget* pParent)
    : QWidget(pParent)
    , m_pModel(pModel)
    , m_pController(pController)
    , m_pView(pView)
    , m_pConfig(pConfig)
    , m_pScores(pScores)
    , m_pResources(pResources)
    , m_pBonusWords(pBonusWords)
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
    if (!m_pConfig || !m_pModel || !m_pController) return;
    const bool hadMidGame = (m_pModel->state() == TgSpaceGameState::PlayingSpaceState
        || m_pModel->state() == TgSpaceGameState::PausedSpaceState);

    const bool changed = runSpaceSettings(m_pConfig, m_pBonusWords, this);
    if (!changed) return;
    if (!tg::AskApplySettingsNow(this)) return;

    m_pModel->setMaxEnemies(m_pConfig->spaceEnemyCount());
    m_pModel->setSpeedLevel(m_pConfig->spaceEnemySpeedLevel());
    m_pModel->setUpgradeIntervalSec(m_pConfig->spaceUpgradeIntervalSec());
    m_pModel->setBonusMode(m_pConfig->spaceBonusMode());
    m_pController->setBonusModeEnabled(m_pConfig->spaceBonusMode());
    m_pConfig->save();

    // Yes: restart if a round was in progress (apply settings on a new game).
    if (hadMidGame) {
        onStartClicked();
        showGamePage();
    } else {
        m_pView->update();
    }
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

void TgSpaceGameWidget::showMainMenu()
{
    showMenuPage();
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
    // Confirm exit, then return to launcher (Home).
    if (!tg::AskExitApplication(this))
        return;
    onExitClicked(); // resets state and emits exitToLauncherRequested()
}

void TgSpaceGameWidget::onRankReturn()
{
    showMenuPage();
}