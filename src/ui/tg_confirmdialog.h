/* ------------------------------------------------------------------
//  File      : tg_confirmdialog.h
//  Created   : 2026-04-21
//  Purpose   : Modal confirm dialog with background + image buttons
// ----------------------------------------------------------------*/
#ifndef _TYPEGAME_TG_CONFIRMDIALOG_H_
#define _TYPEGAME_TG_CONFIRMDIALOG_H_

#include <QDialog>
#include <QPixmap>

class TgImageButton;
class TgResourceProvider;

class TgConfirmDialog : public QDialog
{
	Q_OBJECT
public:
	struct ButtonSpec
	{
		QPixmap m_sprite;
		bool m_accept = false;
	};

	explicit TgConfirmDialog(const QPixmap &background, const QString &message,
		const ButtonSpec &left, const ButtonSpec &right, QWidget *pParent = nullptr);
	TgConfirmDialog(const TgConfirmDialog &) = delete;
	TgConfirmDialog &operator=(const TgConfirmDialog &) = delete;

	bool resultAccepted() const;

protected:
	void paintEvent(QPaintEvent *pEvent) override;

private slots:
	void onLeftClicked();
	void onRightClicked();

private:
	void layoutControls();

	QPixmap m_bg;
	QString m_message;
	ButtonSpec m_left;
	ButtonSpec m_right;
	bool m_accepted = false;

	TgImageButton *m_pLeftBtn = nullptr;
	TgImageButton *m_pRightBtn = nullptr;
};

#endif

