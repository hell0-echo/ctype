#include "tg_dialogs.h"

#include "common/tg_resourceprovider.h"
#include "ui/tg_confirmdialog.h"

#include <QString>

namespace tg
{
bool AskApplySettingsNow(QWidget *pParent)
{
	const TgResourceProvider rp;
	TgConfirmDialog dlg(
		rp.commonPixmap(QStringLiteral("MAIN_DLG_BG")),
		QString::fromUtf16(u"\u8bbe\u7f6e\u5df2\u66f4\u6539\uff0c\u662f\u5426\u7acb\u5373\u751f\u6548\uff1f"),
		TgConfirmDialog::ButtonSpec{ rp.commonPixmap(QStringLiteral("YES")), true },
		TgConfirmDialog::ButtonSpec{ rp.commonPixmap(QStringLiteral("NO")), false },
		pParent);
	dlg.exec();
	return dlg.resultAccepted();
}

bool AskExitApplication(QWidget *pParent)
{
	const TgResourceProvider rp;
	TgConfirmDialog dlg(
		rp.commonPixmap(QStringLiteral("MAIN_DLG_BG")),
		QString::fromUtf16(u"\u786e\u5b9a\u8981\u9000\u51fa\u6e38\u620f\u5417\uff1f"),
		TgConfirmDialog::ButtonSpec{ rp.commonPixmap(QStringLiteral("MAIN_DLG_REPLAY")), false },
		TgConfirmDialog::ButtonSpec{ rp.commonPixmap(QStringLiteral("MAIN_DLG_EXIT")), true },
		pParent);
	dlg.exec();
	return dlg.resultAccepted();
}

}
