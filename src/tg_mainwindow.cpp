#include "tg_mainwindow.h"

#include "apple/tg_apple_controller.h"
#include "apple/tg_apple_game_widget.h"
#include "apple/tg_apple_model.h"
#include "apple/tg_apple_view.h"
#include "space/tg_space_controller.h"
#include "space/tg_space_model.h"
#include "space/tg_space_view.h"
#include "ui/tg_bonuswordprovider.h"
#include "ui/tg_dialogs.h"

#include <QApplication>
#include <QCoreApplication>
#include <QCheckBox>
#include <QCloseEvent>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QSlider>
#include <QStackedWidget>
#include <QVBoxLayout>

#include <algorithm>

TgMainWindow::TgMainWindow(QWidget *pParent)
	: QMainWindow(pParent)
	, m_config(QStringLiteral("typegame"), QStringLiteral("typegame"))
	, m_scores(QCoreApplication::applicationDirPath() + QStringLiteral("/scores.ini"))
{
	m_config.load();
	m_scores.load();

	m_pStack = new QStackedWidget(this);
	setCentralWidget(m_pStack);

	buildHome();
	buildApplePage();
	buildSpacePage();

	onShowHome();
}

void TgMainWindow::buildHome()
{
	auto *pW = new QWidget;
	auto *pLay = new QVBoxLayout(pW);
	auto *pApple = new QPushButton(QStringLiteral("Apple rescue"));
	auto *pSpace = new QPushButton(QStringLiteral("Space battle"));
	auto *pExit = new QPushButton(QStringLiteral("Exit"));
	pLay->addStretch(1);
	pLay->addWidget(pApple);
	pLay->addWidget(pSpace);
	pLay->addWidget(pExit);
	pLay->addStretch(1);
	connect(pApple, &QPushButton::clicked, this, &TgMainWindow::onShowApple);
	connect(pSpace, &QPushButton::clicked, this, &TgMainWindow::onShowSpace);
	connect(pExit, &QPushButton::clicked, this, &TgMainWindow::close);
	m_pStack->addWidget(pW);
}

void TgMainWindow::buildApplePage()
{
	m_pAppleModel = new TgAppleModel();
	m_pAppleController = new TgAppleController(m_pAppleModel, this);
	m_pAppleView = new TgAppleView(m_pAppleModel, &m_resources, this);
	m_pApplePage = new TgAppleGameWidget(m_pAppleModel, m_pAppleController, m_pAppleView, &m_config, this);

	connect(m_pAppleController, &TgAppleController::modelUpdated, this, &TgMainWindow::onAppleModelUpdated);
	connect(m_pAppleController, &TgAppleController::roundEnded, this, &TgMainWindow::onAppleRoundEnded);

	m_pStack->addWidget(m_pApplePage);
}

void TgMainWindow::buildSpacePage()
{
	m_pSpaceModel = new TgSpaceModel();
	m_pBonusProvider = new TgBonusWordProvider(this);
	m_pBonusProvider->setEndpoint(m_config.spaceBonusApiEndpoint());
	m_pBonusProvider->setApiKey(m_config.spaceBonusApiKey());
	m_pSpaceController = new TgSpaceController(m_pSpaceModel, m_pBonusProvider, this);
	m_pSpaceView = new TgSpaceView(m_pSpaceModel, &m_resources, this);

	connect(m_pSpaceController, &TgSpaceController::modelUpdated, this, &TgMainWindow::onSpaceModelUpdated);
	connect(m_pSpaceController, &TgSpaceController::roundEnded, this, &TgMainWindow::onSpaceRoundEnded);

	connect(m_pSpaceView, &TgSpaceView::startGameRequested, this, &TgMainWindow::onSpaceStart);
	connect(m_pSpaceView, &TgSpaceView::continueGameRequested, this, &TgMainWindow::onSpaceContinue);
	connect(m_pSpaceView, &TgSpaceView::pauseRequested, this, &TgMainWindow::onSpacePause);
	connect(m_pSpaceView, &TgSpaceView::highScoreRequested, this, &TgMainWindow::onSpaceHighScore);
	connect(m_pSpaceView, &TgSpaceView::settingsRequested, this, &TgMainWindow::onSpaceSettings);
	connect(m_pSpaceView, &TgSpaceView::exitRequested, this, &TgMainWindow::onSpaceExitMenu);

	m_pSpaceModel->setMaxEnemies(m_config.spaceEnemyCount());
	m_pSpaceModel->setSpeedLevel(m_config.spaceEnemySpeedLevel());
	m_pSpaceModel->setUpgradeIntervalSec(m_config.spaceUpgradeIntervalSec());
	m_pSpaceModel->setBonusMode(m_config.spaceBonusMode());

	m_pStack->addWidget(m_pSpaceView);
}

void TgMainWindow::closeEvent(QCloseEvent *pEvent)
{
	m_config.save();
	QMainWindow::closeEvent(pEvent);
}

void TgMainWindow::onShowHome()
{
	m_pStack->setCurrentIndex(0);
}

void TgMainWindow::onShowApple()
{
	m_pAppleView->refreshMetrics();
	m_pStack->setCurrentIndex(1);
	m_pApplePage->syncUiState();
}

void TgMainWindow::onShowSpace()
{
	m_pSpaceView->refreshLayout();
	m_pSpaceView->setFocus();
	m_pStack->setCurrentIndex(2);
}

void TgMainWindow::onAppleModelUpdated()
{
	m_pAppleView->update();
}

void TgMainWindow::onAppleRoundEnded(bool isWin)
{
	m_pApplePage->syncUiState();
	if (isWin)
	{
		const int r = QMessageBox::question(this, QString(), QStringLiteral("Stage cleared. Increase level for next run?"),
			QMessageBox::Yes | QMessageBox::No);
		if (r == QMessageBox::Yes)
			m_pAppleModel->setLevel(std::min(9, m_pAppleModel->level() + 1));
		m_pApplePage->onStartClicked();
	}
	else
	{
		QMessageBox::information(this, QString(), QStringLiteral("Try again."));
	}
}

void TgMainWindow::onSpaceModelUpdated()
{
	m_pSpaceView->update();
}

void TgMainWindow::onSpaceRoundEnded()
{
	const QString name = QInputDialog::getText(this, QStringLiteral("High score"), QStringLiteral("Name:"));
	m_scores.submitScore(name, m_pSpaceModel->score());
	m_scores.save();
	m_pSpaceController->stopLoop();
	m_pSpaceModel->resetToInitial();
	m_pSpaceView->update();
	QMessageBox::information(this, QString(), QStringLiteral("Game over"));
}

void TgMainWindow::onSpaceStart()
{
	m_pSpaceModel->setMaxEnemies(m_config.spaceEnemyCount());
	m_pSpaceModel->setSpeedLevel(m_config.spaceEnemySpeedLevel());
	m_pSpaceModel->setUpgradeIntervalSec(m_config.spaceUpgradeIntervalSec());
	m_pSpaceModel->setBonusMode(m_config.spaceBonusMode());
	m_pSpaceModel->startFromMenu();
	m_pSpaceController->setBonusModeEnabled(m_config.spaceBonusMode());
	m_pSpaceController->startLoop();
	m_pSpaceView->setFocus();
}

void TgMainWindow::onSpaceContinue()
{
	m_pSpaceModel->resumeFromPause();
	m_pSpaceController->setPaused(false);
	m_pSpaceController->startLoop();
	m_pSpaceView->setFocus();
}

void TgMainWindow::onSpacePause()
{
	m_pSpaceModel->pauseToMenu();
	m_pSpaceController->setPaused(true);
	m_pSpaceView->update();
}

void TgMainWindow::onSpaceHighScore()
{
	QString text;
	const auto list = m_scores.topTen();
	int rank = 1;
	for (const TgScoreEntry &e : list)
	{
		text += QStringLiteral("%1. %2  %3\n").arg(rank).arg(e.m_name).arg(e.m_score);
		rank++;
	}
	if (text.isEmpty())
		text = QStringLiteral("(empty)");
	QMessageBox::information(this, QStringLiteral("High scores"), text);
}

bool TgMainWindow::runSpaceSettings()
{
	QDialog dlg(this);
	dlg.setWindowTitle(QStringLiteral("Space settings"));
	auto *pVBox = new QVBoxLayout(&dlg);
	auto *pForm = new QFormLayout;

	auto *pCount = new QSlider(Qt::Horizontal);
	pCount->setRange(1, 10);
	pCount->setValue(m_config.spaceEnemyCount());
	auto *pCountLabel = new QLabel(QString::number(m_config.spaceEnemyCount()));
	auto *pRow1 = new QHBoxLayout;
	pRow1->addWidget(pCount);
	pRow1->addWidget(pCountLabel);
	connect(pCount, &QSlider::valueChanged, [pCountLabel](int v) { pCountLabel->setText(QString::number(v)); });
	pForm->addRow(QStringLiteral("Max enemies"), pRow1);

	auto *pSpeed = new QSlider(Qt::Horizontal);
	pSpeed->setRange(1, 10);
	pSpeed->setValue(m_config.spaceEnemySpeedLevel());
	auto *pSpeedLabel = new QLabel(QString::number(m_config.spaceEnemySpeedLevel()));
	auto *pRow2 = new QHBoxLayout;
	pRow2->addWidget(pSpeed);
	pRow2->addWidget(pSpeedLabel);
	connect(pSpeed, &QSlider::valueChanged, [pSpeedLabel](int v) { pSpeedLabel->setText(QString::number(v)); });
	pForm->addRow(QStringLiteral("Speed level"), pRow2);

	auto *pUp = new QSlider(Qt::Horizontal);
	pUp->setRange(30, 300);
	pUp->setValue(m_config.spaceUpgradeIntervalSec());
	auto *pUpLabel = new QLabel(QString::number(m_config.spaceUpgradeIntervalSec()));
	auto *pRow3 = new QHBoxLayout;
	pRow3->addWidget(pUp);
	pRow3->addWidget(pUpLabel);
	connect(pUp, &QSlider::valueChanged, [pUpLabel](int v) { pUpLabel->setText(QString::number(v)); });
	pForm->addRow(QStringLiteral("Upgrade interval (s)"), pRow3);

	auto *pBonus = new QCheckBox;
	pBonus->setChecked(m_config.spaceBonusMode());
	pForm->addRow(QStringLiteral("Bonus mode"), pBonus);

	pVBox->addLayout(pForm);
	auto *pBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::RestoreDefaults);
	pVBox->addWidget(pBox);
	connect(pBox, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
	connect(pBox, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
	connect(pBox->button(QDialogButtonBox::RestoreDefaults), &QPushButton::clicked, [&]() {
		pCount->setValue(3);
		pSpeed->setValue(3);
		pUp->setValue(120);
		pBonus->setChecked(false);
	});

	if (dlg.exec() != QDialog::Accepted)
		return false;
	const bool changed = pCount->value() != m_config.spaceEnemyCount()
		|| pSpeed->value() != m_config.spaceEnemySpeedLevel()
		|| pUp->value() != m_config.spaceUpgradeIntervalSec()
		|| pBonus->isChecked() != m_config.spaceBonusMode();
	m_config.setSpaceEnemyCount(pCount->value());
	m_config.setSpaceEnemySpeedLevel(pSpeed->value());
	m_config.setSpaceUpgradeIntervalSec(pUp->value());
	m_config.setSpaceBonusMode(pBonus->isChecked());
	return changed;
}

void TgMainWindow::onSpaceSettings()
{
	const bool changed = runSpaceSettings();
	if (!changed)
		return;
	if (!tg::AskApplySettingsNow(this))
		return;
	m_pSpaceModel->setMaxEnemies(m_config.spaceEnemyCount());
	m_pSpaceModel->setSpeedLevel(m_config.spaceEnemySpeedLevel());
	m_pSpaceModel->setUpgradeIntervalSec(m_config.spaceUpgradeIntervalSec());
	m_pSpaceModel->setBonusMode(m_config.spaceBonusMode());
	m_pBonusProvider->setEndpoint(m_config.spaceBonusApiEndpoint());
	m_pBonusProvider->setApiKey(m_config.spaceBonusApiKey());
	m_pSpaceController->setBonusModeEnabled(m_config.spaceBonusMode());
	m_config.save();
}

void TgMainWindow::onSpaceExitMenu()
{
	m_pSpaceController->stopLoop();
	m_pSpaceModel->resetToInitial();
	onShowHome();
}
