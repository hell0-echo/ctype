#include "tg_dialogs.h"

#include <QMessageBox>

namespace tg
{
bool AskApplySettingsNow(QWidget *pParent)
{
	const int r = QMessageBox::question(pParent, QString(),
		QString::fromUtf8("Settings changed. Apply now?"),
		QMessageBox::Yes | QMessageBox::No,
		QMessageBox::Yes);
	return r == QMessageBox::Yes;
}

bool AskExitApplication(QWidget *pParent)
{
	const int r = QMessageBox::question(pParent, QString(),
		QString::fromUtf8("Exit the game?"),
		QMessageBox::Yes | QMessageBox::No,
		QMessageBox::No);
	return r == QMessageBox::Yes;
}

}
