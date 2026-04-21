#include "tg_apple_game_widget.h"

#include "tg_apple_controller.h"
#include "tg_apple_model.h"
#include "tg_apple_view.h"

#include "common/tg_config.h"
#include "ui/tg_dialogs.h"

#include <QApplication>
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
bool runAppleSettings(TgConfig *pConfig, QWidget *pParent)
{
	QDialog dlg(pParent);
	dlg.setWindowTitle(QStringLiteral("Settings"));
	auto *pVBox = new QVBoxLayout(&dlg);
	auto *pForm = new QFormLayout;

	QSlider *pLevel = new QSlider(Qt::Horizontal);
	pLevel->setRange(0, 9);
	pLevel->setValue(pConfig->appleLevel());
	QLabel *pLevelLabel = new QLabel(QString::number(pConfig->appleLevel()));
	auto *pRow = new QHBoxLayout;
	pRow->addWidget(pLevel);
	pRow->addWidget(pLevelLabel);
	QObject::connect(pLevel, &QSlider::valueChanged, [pLevelLabel](int v) { pLevelLabel->setText(QString::number(v)); });
	pForm->addRow(QStringLiteral("Level"), pRow);

	QSlider *pFail = new QSlider(Qt::Horizontal);
	pFail->setRange(5, 50);
	pFail->setValue(pConfig->appleFailLimit());
	QLabel *pFailLabel = new QLabel(QString::number(pConfig->appleFailLimit()));
	auto *pRow2 = new QHBoxLayout;
	pRow2->addWidget(pFail);
	pRow2->addWidget(pFailLabel);
	QObject::connect(pFail, &QSlider::valueChanged, [pFailLabel](int v) { pFailLabel->setText(QString::number(v)); });
	pForm->addRow(QStringLiteral("Fail limit"), pRow2);

	QSlider *pWin = new QSlider(Qt::Horizontal);
	pWin->setRange(100, 900);
	pWin->setValue(pConfig->appleWinTarget());
	QLabel *pWinLabel = new QLabel(QString::number(pConfig->appleWinTarget()));
	auto *pRow3 = new QHBoxLayout;
	pRow3->addWidget(pWin);
	pRow3->addWidget(pWinLabel);
	QObject::connect(pWin, &QSlider::valueChanged, [pWinLabel](int v) { pWinLabel->setText(QString::number(v)); });
	pForm->addRow(QStringLiteral("Win target"), pRow3);

	QSlider *pMax = new QSlider(Qt::Horizontal);
	pMax->setRange(1, 5);
	pMax->setValue(pConfig->appleMaxOnScreen());
	QLabel *pMaxLabel = new QLabel(QString::number(pConfig->appleMaxOnScreen()));
	auto *pRow4 = new QHBoxLayout;
	pRow4->addWidget(pMax);
	pRow4->addWidget(pMaxLabel);
	QObject::connect(pMax, &QSlider::valueChanged, [pMaxLabel](int v) { pMaxLabel->setText(QString::number(v)); });
	pForm->addRow(QStringLiteral("Max apples"), pRow4);

	auto *pSound = new QCheckBox;
	pSound->setChecked(pConfig->appleSoundEnabled());
	pForm->addRow(QStringLiteral("Sound"), pSound);

	pVBox->addLayout(pForm);

	auto *pBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::RestoreDefaults);
	pVBox->addWidget(pBox);

	QObject::connect(pBox, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
	QObject::connect(pBox, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
	QObject::connect(pBox->button(QDialogButtonBox::RestoreDefaults), &QPushButton::clicked, [&]() {
		pLevel->setValue(3);
		pFail->setValue(10);
		pWin->setValue(100);
		pMax->setValue(5);
		pSound->setChecked(true);
	});

	const int code = dlg.exec();
	if (code != QDialog::Accepted)
		return false;
	const bool changed = pLevel->value() != pConfig->appleLevel()
		|| pFail->value() != pConfig->appleFailLimit()
		|| pWin->value() != pConfig->appleWinTarget()
		|| pMax->value() != pConfig->appleMaxOnScreen()
		|| pSound->isChecked() != pConfig->appleSoundEnabled();
	pConfig->setAppleLevel(pLevel->value());
	pConfig->setAppleFailLimit(pFail->value());
	pConfig->setAppleWinTarget(pWin->value());
	pConfig->setAppleMaxOnScreen(pMax->value());
	pConfig->setAppleSoundEnabled(pSound->isChecked());
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
	auto *pBar = new QHBoxLayout;
	m_pPauseBtn = new QPushButton(QStringLiteral("Pause"));
	m_pStartBtn = new QPushButton(QStringLiteral("Start"));
	m_pEndBtn = new QPushButton(QStringLiteral("End"));
	m_pSettingsBtn = new QPushButton(QStringLiteral("Settings"));
	m_pExitBtn = new QPushButton(QStringLiteral("Exit"));
	pBar->addWidget(m_pPauseBtn);
	pBar->addWidget(m_pStartBtn);
	pBar->addWidget(m_pEndBtn);
	pBar->addWidget(m_pSettingsBtn);
	pBar->addWidget(m_pExitBtn);
	pRoot->addLayout(pBar);

	connect(m_pPauseBtn, &QPushButton::clicked, this, &TgAppleGameWidget::onPauseClicked);
	connect(m_pStartBtn, &QPushButton::clicked, this, &TgAppleGameWidget::onStartClicked);
	connect(m_pEndBtn, &QPushButton::clicked, this, &TgAppleGameWidget::onEndClicked);
	connect(m_pSettingsBtn, &QPushButton::clicked, this, &TgAppleGameWidget::onSettingsClicked);
	connect(m_pExitBtn, &QPushButton::clicked, this, &TgAppleGameWidget::onExitClicked);

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
	m_pPauseBtn->setText(paused ? QStringLiteral("Resume") : QStringLiteral("Pause"));
	m_pStartBtn->setEnabled(!playing);
	m_pEndBtn->setEnabled(playing || paused);
}

void TgAppleGameWidget::onPauseClicked()
{
	if (nullptr == m_pModel || nullptr == m_pController)
		return;
	if (m_pModel->state() == TgAppleGameState::PlayingAppleState)
	{
		m_pModel->setState(TgAppleGameState::PausedAppleState);
		m_pController->setPaused(true);
	}
	else if (m_pModel->state() == TgAppleGameState::PausedAppleState)
	{
		m_pModel->setState(TgAppleGameState::PlayingAppleState);
		m_pController->setPaused(false);
	}
	updateButtonStates();
	m_pView->update();
}

void TgAppleGameWidget::onStartClicked()
{
	if (nullptr == m_pModel || nullptr == m_pController)
		return;
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
	updateButtonStates();
	m_pView->update();
}

void TgAppleGameWidget::onSettingsClicked()
{
	if (nullptr == m_pConfig)
		return;
	const bool changed = runAppleSettings(m_pConfig, this);
	if (!changed)
		return;
	if (!tg::AskApplySettingsNow(this))
		return;
	if (nullptr != m_pModel)
	{
		m_pModel->setLevel(m_pConfig->appleLevel());
		m_pModel->setFailLimit(m_pConfig->appleFailLimit());
		m_pModel->setWinTarget(m_pConfig->appleWinTarget());
		m_pModel->setMaxApplesOnScreen(m_pConfig->appleMaxOnScreen());
	}
	m_pConfig->save();
}

void TgAppleGameWidget::onExitClicked()
{
	if (tg::AskExitApplication(this))
		qApp->quit();
}
