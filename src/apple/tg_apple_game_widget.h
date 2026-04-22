#ifndef _TYPEGAME_TG_APPLE_GAME_WIDGET_H_
#define _TYPEGAME_TG_APPLE_GAME_WIDGET_H_

#include <QWidget>

class TgAppleController;
class TgAppleModel;
class TgAppleView;
class TgConfig;
class TgImageButton;

class TgAppleGameWidget : public QWidget
{
	Q_OBJECT

public:
	explicit TgAppleGameWidget(TgAppleModel *pModel, TgAppleController *pController, TgAppleView *pView,
		TgConfig *pConfig, QWidget *pParent = nullptr);

	TgAppleGameWidget(const TgAppleGameWidget &) = delete;
	TgAppleGameWidget &operator=(const TgAppleGameWidget &) = delete;

	void syncUiState();

public slots:
	void onPauseClicked();
	void onStartClicked();
	void onEndClicked();
	void onSettingsClicked();
	void onExitClicked();

private:
	void updateButtonStates();
	void layoutOverlay();

protected:
	void resizeEvent(QResizeEvent *pEvent) override;

	TgAppleModel *m_pModel = nullptr;
	TgAppleController *m_pController = nullptr;
	TgAppleView *m_pView = nullptr;
	TgConfig *m_pConfig = nullptr;

	QWidget *m_pOverlay = nullptr;
	TgImageButton *m_pPauseBtn = nullptr;
	TgImageButton *m_pStartBtn = nullptr;
	TgImageButton *m_pEndBtn = nullptr;
	TgImageButton *m_pSettingsBtn = nullptr;
	TgImageButton *m_pExitBtn = nullptr;
};

#endif
