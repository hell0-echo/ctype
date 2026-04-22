/* ------------------------------------------------------------------
//  File      : tg_space_rank_widget.h
//  Created   : 2026-04-21
//  Purpose   : Space rank list page (background + return button)
// ----------------------------------------------------------------*/
#ifndef _TYPEGAME_TG_SPACE_RANK_WIDGET_H_
#define _TYPEGAME_TG_SPACE_RANK_WIDGET_H_

#include <QWidget>

class TgImageButton;
class TgResourceProvider;
class TgScoreStore;

class TgSpaceRankWidget : public QWidget
{
	Q_OBJECT
public:
	explicit TgSpaceRankWidget(TgResourceProvider *pResources, TgScoreStore *pScores, QWidget *pParent = nullptr);
	TgSpaceRankWidget(const TgSpaceRankWidget &) = delete;
	TgSpaceRankWidget &operator=(const TgSpaceRankWidget &) = delete;

	void refreshLayout();

signals:
	void returnClicked();

protected:
	void paintEvent(QPaintEvent *pEvent) override;
	void resizeEvent(QResizeEvent *pEvent) override;

private:
	void layoutControls();

	TgResourceProvider *m_pResources = nullptr;
	TgScoreStore *m_pScores = nullptr;
	QPixmap m_bg;
	TgImageButton *m_pReturn = nullptr;
};

#endif

