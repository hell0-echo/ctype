#include "tg_apple_game_widget.h"

#include "tg_apple_controller.h"
#include "tg_apple_model.h"
#include "tg_apple_view.h"

#include "common/tg_config.h"
#include "common/tg_audiomanager.h"
#include "common/tg_resourceprovider.h"
#include "ui/tg_imagebutton.h"
#include "ui/tg_imagecheckbox.h"
#include "ui/tg_skinslider.h"
#include "ui/tg_dialogs.h"

#include <QApplication>
#include <QDialog>
#include <QLabel>
#include <QPushButton>
#include <QPainter>
#include <QCoreApplication>
#include <QDir>
#include <QVBoxLayout>
#include <QResizeEvent>

namespace
{
bool runAppleSettings(TgConfig *pConfig, QWidget *pParent)
{
	TgResourceProvider rp;
	class AppleSettingsDialog : public QDialog
	{
	public:
		explicit AppleSettingsDialog(TgConfig *pConfig, QWidget *pParent)
			: QDialog(pParent)
			, m_pConfig(pConfig)
			, m_bg(m_rp.applePixmap(QStringLiteral("APPLE_SETUP")))
			, m_checkSprite(m_rp.spacePixmap(QStringLiteral("CHECKBOX_BUTTON")))
		{
			setModal(true);
			setWindowFlags((windowFlags() & ~Qt::WindowContextHelpButtonHint) | Qt::Dialog);
			if (!m_bg.isNull())
				setFixedSize(m_bg.size());

			const QPixmap groove = m_rp.commonPixmap(QStringLiteral("SLIDER_BG"));
			const QPixmap handle = m_rp.commonPixmap(QStringLiteral("SLIDER_SLIDER"));
			m_pLevel = new TgSkinSlider(groove, handle, Qt::Horizontal, this);
			m_pFail = new TgSkinSlider(groove, handle, Qt::Horizontal, this);
			m_pWin = new TgSkinSlider(groove, handle, Qt::Horizontal, this);
			m_pMax = new TgSkinSlider(groove, handle, Qt::Horizontal, this);

			m_pLevel->setRange(0, 9);
			m_pFail->setRange(5, 50);
			m_pWin->setRange(100, 900);
			m_pMax->setRange(1, 5);

			m_pLevel->setValue(m_pConfig->appleLevel());
			m_pFail->setValue(m_pConfig->appleFailLimit());
			m_pWin->setValue(m_pConfig->appleWinTarget());
			m_pMax->setValue(m_pConfig->appleMaxOnScreen());

			m_pLevelLabel = new QLabel(QString::number(m_pLevel->value()), this);
			m_pFailLabel = new QLabel(QString::number(m_pFail->value()), this);
			m_pWinLabel = new QLabel(QString::number(m_pWin->value()), this);
			m_pMaxLabel = new QLabel(QString::number(m_pMax->value()), this);

			m_pSound = new TgImageCheckBox(m_checkSprite, this);
			m_pSound->setChecked(m_pConfig->appleSoundEnabled());

			connect(m_pLevel, &QSlider::valueChanged, this, [this](int v) { m_pLevelLabel->setText(QString::number(v)); });
			connect(m_pFail, &QSlider::valueChanged, this, [this](int v) { m_pFailLabel->setText(QString::number(v)); });
			connect(m_pWin, &QSlider::valueChanged, this, [this](int v) { m_pWinLabel->setText(QString::number(v)); });
			connect(m_pMax, &QSlider::valueChanged, this, [this](int v) { m_pMaxLabel->setText(QString::number(v)); });
			connect(m_pLevel, &QSlider::sliderReleased, this, []() { TgAudioManager::instance().playCommon(QStringLiteral("GLIDE")); });
			connect(m_pFail, &QSlider::sliderReleased, this, []() { TgAudioManager::instance().playCommon(QStringLiteral("GLIDE")); });
			connect(m_pWin, &QSlider::sliderReleased, this, []() { TgAudioManager::instance().playCommon(QStringLiteral("GLIDE")); });
			connect(m_pMax, &QSlider::sliderReleased, this, []() { TgAudioManager::instance().playCommon(QStringLiteral("GLIDE")); });

			m_pOk = new TgImageButton(m_rp.commonPixmap(QStringLiteral("OK")), this);
			m_pCancel = new TgImageButton(m_rp.commonPixmap(QStringLiteral("CANCEL")), this);
			m_pDefault = new TgImageButton(m_rp.commonPixmap(QStringLiteral("DEFAULT")), this);

			connect(m_pOk, &TgImageButton::clicked, this, &QDialog::accept);
			connect(m_pCancel, &TgImageButton::clicked, this, &QDialog::reject);
			connect(m_pDefault, &TgImageButton::clicked, this, [this]() { restoreDefaults(); });

			buildFixedLayout();
		}

		bool changed() const
		{
			return m_pLevel->value() != m_pConfig->appleLevel()
				|| m_pFail->value() != m_pConfig->appleFailLimit()
				|| m_pWin->value() != m_pConfig->appleWinTarget()
				|| m_pMax->value() != m_pConfig->appleMaxOnScreen()
				|| m_pSound->isChecked() != m_pConfig->appleSoundEnabled();
		}

		void applyToConfig()
		{
			m_pConfig->setAppleLevel(m_pLevel->value());
			m_pConfig->setAppleFailLimit(m_pFail->value());
			m_pConfig->setAppleWinTarget(m_pWin->value());
			m_pConfig->setAppleMaxOnScreen(m_pMax->value());
			m_pConfig->setAppleSoundEnabled(m_pSound->isChecked());
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
			// Follow the reference project WidgetSetUp: fixed coordinates on APPLE_SETUP background.
			// Base positions below are tuned for the original pixmap size; we apply a uniform scale if needed.
			const QSize bgSize = size();
			const qreal baseW = static_cast<qreal>(bgSize.width());
			const qreal baseH = static_cast<qreal>(bgSize.height());
			const qreal s = 1.0; // pixmap is used 1:1 in our dialog
			Q_UNUSED(baseW);
			Q_UNUSED(baseH);

			const QFont font(QStringLiteral("SimHei"), 10, 10);

			auto applyLabel = [&](QLabel *lbl, const QString &text, int x, int y, bool value) {
				lbl->setFont(font);
				lbl->setText(text);
				lbl->adjustSize();
				lbl->move(static_cast<int>(x * s), static_cast<int>(y * s));
				if (value)
					lbl->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
			};

			// Move the whole options area slightly upward for better visual balance.
			const int up = 60;

			applyLabel(m_pLevelLabel, QString::number(m_pLevel->value()), 415, 80 - up, true);
			applyLabel(m_pFailLabel, QString::number(m_pFail->value()), 415, 135 - up, true);
			applyLabel(m_pWinLabel, QString::number(m_pWin->value()), 415, 190 - up, true);
			applyLabel(m_pMaxLabel, QString::number(m_pMax->value()), 415, 245 - up, true);

			auto *lblLevel = new QLabel(this);
			auto *lblFail = new QLabel(this);
			auto *lblWin = new QLabel(this);
			auto *lblMax = new QLabel(this);
			auto *lblSound = new QLabel(this);

			applyLabel(lblLevel, QString::fromUtf16(u"\u6e38\u620f\u7b49\u7ea7:"), 100, 80 - up, false);
			applyLabel(lblFail, QString::fromUtf16(u"\u5931\u8d25\u82f9\u679c\u6570:"), 100, 135 - up, false);
			applyLabel(lblWin, QString::fromUtf16(u"\u6210\u529f\u82f9\u679c\u6570:"), 100, 190 - up, false);
			applyLabel(lblMax, QString::fromUtf16(u"\u540c\u5c4f\u82f9\u679c:"), 100, 245 - up, false);
			applyLabel(lblSound, QString::fromUtf16(u"\u97f3\u6548:"), 100, 300 - up, false);

			auto applySlider = [&](QSlider *slider, int x, int y) {
				slider->setFixedWidth(static_cast<int>(180 * s));
				slider->move(static_cast<int>(x * s), static_cast<int>(y * s));
			};

			applySlider(m_pLevel, 220, 80 - up);
			applySlider(m_pFail, 220, 135 - up);
			applySlider(m_pWin, 220, 190 - up);
			applySlider(m_pMax, 220, 245 - up);

			m_pSound->setFixedSize(m_pSound->sizeHint());
			m_pSound->move(static_cast<int>(200 * s), static_cast<int>((300 - up) * s));

			// Buttons (reference): OK(180,270) CANCEL(270,270) DEFAULT(360,270)
			m_pOk->setFixedSize(m_pOk->sizeHint());
			m_pCancel->setFixedSize(m_pCancel->sizeHint());
			m_pDefault->setFixedSize(m_pDefault->sizeHint());
			const int bottomMargin = static_cast<int>(18 * s);
			const int btnY = height() - bottomMargin - m_pOk->height();
			m_pOk->move(static_cast<int>(180 * s), btnY);
			m_pCancel->move(static_cast<int>(270 * s), btnY);
			m_pDefault->move(static_cast<int>(360 * s), btnY);
		}

		// Slider skin is drawn by TgSkinSlider (no QSS).

		void restoreDefaults()
		{
			m_pLevel->setValue(3);
			m_pFail->setValue(10);
			m_pWin->setValue(100);
			m_pMax->setValue(5);
			m_pSound->setChecked(true);
		}

		// applySliderSkin moved above (reference-matching)

		TgConfig *m_pConfig = nullptr;
		TgResourceProvider m_rp;
		QPixmap m_bg;
		QPixmap m_checkSprite;

		QSlider *m_pLevel = nullptr;
		QSlider *m_pFail = nullptr;
		QSlider *m_pWin = nullptr;
		QSlider *m_pMax = nullptr;
		QLabel *m_pLevelLabel = nullptr;
		QLabel *m_pFailLabel = nullptr;
		QLabel *m_pWinLabel = nullptr;
		QLabel *m_pMaxLabel = nullptr;
		TgImageCheckBox *m_pSound = nullptr;
		TgImageButton *m_pOk = nullptr;
		TgImageButton *m_pCancel = nullptr;
		TgImageButton *m_pDefault = nullptr;
	};

	AppleSettingsDialog dlg(pConfig, pParent);
	if (dlg.exec() != QDialog::Accepted)
		return false;
	const bool changed = dlg.changed();
	dlg.applyToConfig();
	return changed;
}
}

TgAppleGameWidget::TgAppleGameWidget(TgAppleModel *pModel, TgAppleController *pController, TgAppleView *pView,
	TgConfig *pConfig, QWidget *pParent)
	: QWidget(pParent)
	, m_pModel(pModel)
	, m_pController(pController)
	, m_pView(pView)
	, m_pConfig(pConfig)
{
	auto *pRoot = new QVBoxLayout(this);
	pRoot->addWidget(m_pView, 1);

	// Overlay UI (bottom-left) like the original game.
	m_pOverlay = new QWidget(m_pView);
	m_pOverlay->setAttribute(Qt::WA_TransparentForMouseEvents, false);
	m_pOverlay->setAttribute(Qt::WA_NoSystemBackground, true);
	m_pOverlay->setAttribute(Qt::WA_TranslucentBackground, true);

	TgResourceProvider rp;
	m_pPauseBtn = new TgImageButton(rp.commonPixmap(QStringLiteral("PUBLIC_PAUSE")), m_pOverlay);
	m_pStartBtn = new TgImageButton(rp.commonPixmap(QStringLiteral("PUBLIC_START")), m_pOverlay);
	m_pEndBtn = new TgImageButton(rp.commonPixmap(QStringLiteral("PUBLIC_END")), m_pOverlay);
	m_pSettingsBtn = new TgImageButton(rp.commonPixmap(QStringLiteral("PUBLIC_SETUP")), m_pOverlay);
	m_pExitBtn = new TgImageButton(rp.commonPixmap(QStringLiteral("PUBLIC_EXIT")), m_pOverlay);

	connect(m_pPauseBtn, &TgImageButton::clicked, this, &TgAppleGameWidget::onPauseClicked);
	connect(m_pStartBtn, &TgImageButton::clicked, this, &TgAppleGameWidget::onStartClicked);
	connect(m_pEndBtn, &TgImageButton::clicked, this, &TgAppleGameWidget::onEndClicked);
	connect(m_pSettingsBtn, &TgImageButton::clicked, this, &TgAppleGameWidget::onSettingsClicked);
	connect(m_pExitBtn, &TgImageButton::clicked, this, &TgAppleGameWidget::onExitClicked);

	layoutOverlay();

	syncUiState();
}

void TgAppleGameWidget::syncUiState()
{
	updateButtonStates();
}

void TgAppleGameWidget::updateButtonStates()
{
	if (nullptr == m_pModel)
		return;
	const TgAppleGameState st = m_pModel->state();
	const bool playing = (st == TgAppleGameState::PlayingAppleState);
	const bool paused = (st == TgAppleGameState::PausedAppleState);
	m_pPauseBtn->setEnabled(playing || paused);
	m_pStartBtn->setEnabled(!playing);
	m_pEndBtn->setEnabled(playing || paused);
}

void TgAppleGameWidget::layoutOverlay()
{
	if (nullptr == m_pOverlay || nullptr == m_pView)
		return;
	m_pOverlay->setGeometry(m_pView->rect());

	// Follow the reference project coordinates (800x600 base):
	// Pause(118,514) Start(198,526) End(163,483) Setup(150,552) Exit(20,540)
	const qreal baseW = 800.0;
	const qreal baseH = 600.0;
	const qreal sx = static_cast<qreal>(m_pOverlay->width()) / baseW;
	const qreal sy = static_cast<qreal>(m_pOverlay->height()) / baseH;
	const qreal s = std::min(sx, sy);

	auto applyScaled = [&](TgImageButton *btn, int x, int y) {
		if (!btn)
			return;
		const QSize raw = btn->sizeHint();
		const QSize scaled(std::max(1, static_cast<int>(raw.width() * s * 1.12f)),
			std::max(1, static_cast<int>(raw.height() * s * 1.12f)));
		btn->setFixedSize(scaled);
		btn->move(static_cast<int>(x * sx), static_cast<int>(y * sy));
	};

	applyScaled(m_pPauseBtn, 119, 512);
	applyScaled(m_pStartBtn, 199, 521);
	applyScaled(m_pEndBtn, 164, 479);
	applyScaled(m_pSettingsBtn, 151, 547);
	applyScaled(m_pExitBtn, 20, 540);
}

void TgAppleGameWidget::resizeEvent(QResizeEvent *pEvent)
{
	QWidget::resizeEvent(pEvent);
	layoutOverlay();
}

void TgAppleGameWidget::onPauseClicked()
{
	if (nullptr == m_pModel || nullptr == m_pController)
		return;
	if (m_pModel->state() == TgAppleGameState::PlayingAppleState)
	{
		m_pModel->setState(TgAppleGameState::PausedAppleState);
		m_pController->setPaused(true);
		TgAudioManager::instance().setPaused(true);
	}
	else if (m_pModel->state() == TgAppleGameState::PausedAppleState)
	{
		m_pModel->setState(TgAppleGameState::PlayingAppleState);
		m_pController->setPaused(false);
		TgAudioManager::instance().setPaused(false);
	}
	updateButtonStates();
	m_pView->update();
}

void TgAppleGameWidget::onStartClicked()
{
	if (nullptr == m_pModel || nullptr == m_pController)
		return;
	TgAudioManager::instance().setEnabled(m_pConfig ? m_pConfig->appleSoundEnabled() : true);
	TgAudioManager::instance().setPaused(false);
	TgAudioManager::instance().startBgm(TgAudioManager::Game::Apple);
	m_pModel->setLevel(m_pConfig->appleLevel());
	m_pModel->setFailLimit(m_pConfig->appleFailLimit());
	m_pModel->setWinTarget(m_pConfig->appleWinTarget());
	m_pModel->setMaxApplesOnScreen(m_pConfig->appleMaxOnScreen());
	m_pModel->startNewRound();
	m_pController->setPaused(false);
	m_pController->startLoop();
	updateButtonStates();
	m_pView->setFocus();
}

void TgAppleGameWidget::onEndClicked()
{
	if (nullptr == m_pModel || nullptr == m_pController)
		return;
	m_pController->stopLoop();
	m_pModel->resetIdle();
	TgAudioManager::instance().stopBgm();
	updateButtonStates();
	m_pView->update();
}

void TgAppleGameWidget::onSettingsClicked()
{
	if (nullptr == m_pConfig || nullptr == m_pModel)
		return;

	const TgAppleGameState stateAtOpen = m_pModel->state();
	const bool hadMidRound = (stateAtOpen == TgAppleGameState::PlayingAppleState
		|| stateAtOpen == TgAppleGameState::PausedAppleState);
	const bool wasActivelyPlaying = (stateAtOpen == TgAppleGameState::PlayingAppleState);

	// Pause only when the round is live (not already paused).
	if (wasActivelyPlaying)
	{
		m_pModel->setState(TgAppleGameState::PausedAppleState);
		if (m_pController)
			m_pController->setPaused(true);
		TgAudioManager::instance().setPaused(true);
		updateButtonStates();
		if (m_pView)
			m_pView->update();
	}

	const bool changed = runAppleSettings(m_pConfig, this);
	if (!changed)
	{
		if (wasActivelyPlaying)
		{
			m_pModel->setState(TgAppleGameState::PlayingAppleState);
			if (m_pController)
				m_pController->setPaused(false);
			TgAudioManager::instance().setPaused(false);
			updateButtonStates();
			if (m_pView)
				m_pView->update();
		}
		return;
	}
	if (!tg::AskApplySettingsNow(this))
	{
		if (wasActivelyPlaying)
		{
			m_pModel->setState(TgAppleGameState::PlayingAppleState);
			if (m_pController)
				m_pController->setPaused(false);
			TgAudioManager::instance().setPaused(false);
			updateButtonStates();
			if (m_pView)
				m_pView->update();
		}
		return;
	}

	TgAudioManager::instance().setEnabled(m_pConfig->appleSoundEnabled());
	m_pModel->setLevel(m_pConfig->appleLevel());
	m_pModel->setFailLimit(m_pConfig->appleFailLimit());
	m_pModel->setWinTarget(m_pConfig->appleWinTarget());
	m_pModel->setMaxApplesOnScreen(m_pConfig->appleMaxOnScreen());
	m_pConfig->save();

	// Yes: restart round if a game was in progress (do not continue the same round).
	if (hadMidRound)
	{
		onStartClicked();
	}
	else
	{
		updateButtonStates();
		if (m_pView)
			m_pView->update();
	}
}

void TgAppleGameWidget::onExitClicked()
{
	// Confirm exit, then return to launcher (Home) instead of quitting the app.
	if (!tg::AskExitApplication(this))
		return;
	if (m_pController)
		m_pController->stopLoop();
	if (m_pModel)
		m_pModel->resetIdle();
	TgAudioManager::instance().stopBgm();
	updateButtonStates();
	if (m_pView)
		m_pView->update();
	emit exitToLauncherRequested();
}
