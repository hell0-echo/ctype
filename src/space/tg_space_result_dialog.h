/* ------------------------------------------------------------------
//  File      : tg_space_result_dialog.h
//  Created   : 2026-04-27
//  Purpose   : Space game over modal dialog (frameless, custom background + sprite buttons)
// ----------------------------------------------------------------*/
#ifndef _TYPEGAME_TG_SPACE_RESULT_DIALOG_H_
#define _TYPEGAME_TG_SPACE_RESULT_DIALOG_H_

#include <QDialog>
#include <QPixmap>

class TgImageButton;
class TgResourceProvider;

class TgSpaceResultDialog : public QDialog
{
	Q_OBJECT
public:
	enum class Action
	{
		None,
		Replay,
		End
	};

	explicit TgSpaceResultDialog(TgResourceProvider *pResources, QWidget *pParent = nullptr);
	TgSpaceResultDialog(const TgSpaceResultDialog &) = delete;
	TgSpaceResultDialog &operator=(const TgSpaceResultDialog &) = delete;

	Action action() const;

protected:
	void paintEvent(QPaintEvent *pEvent) override;

private slots:
	void onReplay();
	void onEnd();

private:
	void layoutControls();

	TgResourceProvider *m_pResources = nullptr;
	QPixmap m_bg;
	Action m_action = Action::None;

	TgImageButton *m_pReplay = nullptr;
	TgImageButton *m_pEnd = nullptr;
};

#endif

