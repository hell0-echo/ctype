/* ------------------------------------------------------------------
//  File      : tg_space_menu_widget.h
//  Created   : 2026-04-21
//  Purpose   : Space main menu page (sprite buttons)
// ----------------------------------------------------------------*/
#ifndef _TYPEGAME_TG_SPACE_MENU_WIDGET_H_
#define _TYPEGAME_TG_SPACE_MENU_WIDGET_H_

#include <QWidget>

class TgImageButton;
class TgResourceProvider;

class TgSpaceMenuWidget : public QWidget
{
	Q_OBJECT
public:
	explicit TgSpaceMenuWidget(TgResourceProvider *pResources, QWidget *pParent = nullptr);
	TgSpaceMenuWidget(const TgSpaceMenuWidget &) = delete;
	TgSpaceMenuWidget &operator=(const TgSpaceMenuWidget &) = delete;

	void setStartAsReturn(bool isReturn);
	void refreshLayout();

signals:
	void startClicked();
	void rankClicked();
	void optionClicked();
	void exitClicked();

protected:
	void paintEvent(QPaintEvent *pEvent) override;
	void resizeEvent(QResizeEvent *pEvent) override;

private:
	void layoutButtons();

	TgResourceProvider *m_pResources = nullptr;
	QPixmap m_bg;
	QPixmap m_startSprite;
	QPixmap m_returnSprite;
	bool m_startIsReturn = false;

	TgImageButton *m_pStart = nullptr;
	TgImageButton *m_pRank = nullptr;
	TgImageButton *m_pOption = nullptr;
	TgImageButton *m_pExit = nullptr;
};

#endif

