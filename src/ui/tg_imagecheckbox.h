/* ------------------------------------------------------------------
//  File      : tg_imagecheckbox.h
//  Created   : 2026-04-21
//  Purpose   : Sprite-based checkbox button (unchecked/checked)
// ----------------------------------------------------------------*/
#ifndef _TYPEGAME_TG_IMAGECHECKBOX_H_
#define _TYPEGAME_TG_IMAGECHECKBOX_H_

#include <QPixmap>
#include <QWidget>

class TgImageCheckBox : public QWidget
{
	Q_OBJECT
public:
	explicit TgImageCheckBox(const QPixmap &sprite, QWidget *pParent = nullptr);
	TgImageCheckBox(const TgImageCheckBox &) = delete;
	TgImageCheckBox &operator=(const TgImageCheckBox &) = delete;

	QSize sizeHint() const override;
	bool isChecked() const;
	void setChecked(bool checked);

signals:
	void toggled(bool checked);

protected:
	void paintEvent(QPaintEvent *pEvent) override;
	void mouseReleaseEvent(QMouseEvent *pEvent) override;
	void enterEvent(QEvent *pEvent) override;
	void leaveEvent(QEvent *pEvent) override;

private:
	QRect frameRect(bool checked, bool hovered) const;
	int frameCount() const;

	QPixmap m_sprite;
	bool m_checked = false;
	bool m_hovered = false;
};

#endif

