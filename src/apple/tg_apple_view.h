#ifndef _TYPEGAME_TG_APPLE_VIEW_H_
#define _TYPEGAME_TG_APPLE_VIEW_H_

#include <QWidget>

class TgAppleModel;
class TgResourceProvider;

class TgAppleView : public QWidget
{
	Q_OBJECT

public:
	explicit TgAppleView(TgAppleModel *pModel, TgResourceProvider *pResources, QWidget *pParent = nullptr);

	TgAppleView(const TgAppleView &) = delete;
	TgAppleView &operator=(const TgAppleView &) = delete;

	void refreshMetrics();

protected:
	void paintEvent(QPaintEvent *pEvent) override;
	void resizeEvent(QResizeEvent *pEvent) override;
	void keyPressEvent(QKeyEvent *pEvent) override;

private:
	TgAppleModel *m_pModel = nullptr;
	TgResourceProvider *m_pResources = nullptr;
};

#endif
