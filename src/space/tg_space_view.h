#ifndef _TYPEGAME_TG_SPACE_VIEW_H_
#define _TYPEGAME_TG_SPACE_VIEW_H_

#include <QRect>
#include <QWidget>

class TgResourceProvider;
class TgSpaceModel;

class TgSpaceView : public QWidget
{
	Q_OBJECT

public:
	explicit TgSpaceView(TgSpaceModel *pModel, TgResourceProvider *pResources, QWidget *pParent = nullptr);

	TgSpaceView(const TgSpaceView &) = delete;
	TgSpaceView &operator=(const TgSpaceView &) = delete;

	void refreshLayout();

signals:
	void startGameRequested();
	void continueGameRequested();
	void highScoreRequested();
	void settingsRequested();
	void exitRequested();
	void pauseRequested();

protected:
	void paintEvent(QPaintEvent *pEvent) override;
	void resizeEvent(QResizeEvent *pEvent) override;
	void mousePressEvent(QMouseEvent *pEvent) override;
	void keyPressEvent(QKeyEvent *pEvent) override;

private:
	void layoutMenu();

	TgSpaceModel *m_pModel = nullptr;
	TgResourceProvider *m_pResources = nullptr;

	QRect m_rectStart;
	QRect m_rectScores;
	QRect m_rectSettings;
	QRect m_rectExit;
};

#endif
