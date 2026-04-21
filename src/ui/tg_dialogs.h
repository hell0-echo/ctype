#ifndef _TYPEGAME_TG_DIALOGS_H_
#define _TYPEGAME_TG_DIALOGS_H_

#include <QString>

class QWidget;

namespace tg
{
bool AskApplySettingsNow(QWidget *pParent);
bool AskExitApplication(QWidget *pParent);
}

#endif
