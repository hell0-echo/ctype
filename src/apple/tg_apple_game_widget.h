#ifndef _TYPEGAME_TG_APPLE_GAME_WIDGET_H_
#define _TYPEGAME_TG_APPLE_GAME_WIDGET_H_

#include <QWidget>

class QPushButton;
class TgAppleController;
class TgAppleModel;
class TgAppleView;
class TgConfig;

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

	TgAppleModel *m_pModel = nullptr;
	TgAppleController *m_pController = nullptr;
	TgAppleView *m_pView = nullptr;
	TgConfig *m_pConfig = nullptr;

	QPushButton *m_pPauseBtn = nullptr;
	QPushButton *m_pStartBtn = nullptr;
	QPushButton *m_pEndBtn = nullptr;
	QPushButton *m_pSettingsBtn = nullptr;
	QPushButton *m_pExitBtn = nullptr;
};

#endif
