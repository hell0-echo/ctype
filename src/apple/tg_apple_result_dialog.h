/* ------------------------------------------------------------------
//  File      : tg_apple_result_dialog.h
//  Created   : 2026-04-23
//  Purpose   : Apple win/lose modal dialog (custom background + sprite buttons)
// ----------------------------------------------------------------*/
#ifndef _TYPEGAME_TG_APPLE_RESULT_DIALOG_H_
#define _TYPEGAME_TG_APPLE_RESULT_DIALOG_H_

#include <QDialog>
#include <QPixmap>

class TgImageButton;
class TgResourceProvider;

class TgAppleResultDialog : public QDialog
{
	Q_OBJECT
public:
	enum class Action
	{
		None,
		Replay,
		Next,
		End,
	};

	explicit TgAppleResultDialog(TgResourceProvider *pResources, bool isWin, QWidget *pParent = nullptr);
	TgAppleResultDialog(const TgAppleResultDialog &) = delete;
	TgAppleResultDialog &operator=(const TgAppleResultDialog &) = delete;

	Action action() const;

protected:
	void paintEvent(QPaintEvent *pEvent) override;

private slots:
	void onReplay();
	void onNext();
	void onEnd();

private:
	void layoutControls();

	TgResourceProvider *m_pResources = nullptr;
	QPixmap m_bg;
	bool m_isWin = false;
	Action m_action = Action::None;

	TgImageButton *m_pReplay = nullptr;
	TgImageButton *m_pNext = nullptr;
	TgImageButton *m_pEnd = nullptr;
};

#endif

