/* ------------------------------------------------------------------
//  File      : tg_imagebutton.h
//  Created   : 2026-04-21
//  Purpose   : Sprite-based image button (normal/hover/pressed)
// ----------------------------------------------------------------*/
#ifndef _TYPEGAME_TG_IMAGEBUTTON_H_
#define _TYPEGAME_TG_IMAGEBUTTON_H_

#include <QPixmap>
#include <QWidget>

class TgImageButton : public QWidget
{
	Q_OBJECT
public:
	explicit TgImageButton(const QPixmap &sprite, QWidget *pParent = nullptr);
	TgImageButton(const TgImageButton &) = delete;
	TgImageButton &operator=(const TgImageButton &) = delete;

	QSize sizeHint() const override;
	void setEnabled(bool enabled);
	void setSprite(const QPixmap &sprite);

signals:
	void clicked();

protected:
	void paintEvent(QPaintEvent *pEvent) override;
	void enterEvent(QEvent *pEvent) override;
	void leaveEvent(QEvent *pEvent) override;
	void mousePressEvent(QMouseEvent *pEvent) override;
	void mouseReleaseEvent(QMouseEvent *pEvent) override;

private:
	enum class VisualState
	{
		Normal,
		Hover,
		Pressed,
		Disabled,
	};

	void updateState();
	QRect frameRect(VisualState state) const;
	QPixmap makeDisabled(const QPixmap &pm) const;

	QPixmap m_sprite;
	QPixmap m_disabled;
	VisualState m_state = VisualState::Normal;
	bool m_hovered = false;
	bool m_pressed = false;
};

#endif

