/* ------------------------------------------------------------------
//  File      : tg_audiomanager.h
//  Created   : 2026-04-21
//  Purpose   : Centralized BGM/SFX with pause/resume sync
// ----------------------------------------------------------------*/
#ifndef _TYPEGAME_TG_AUDIOMANAGER_H_
#define _TYPEGAME_TG_AUDIOMANAGER_H_

#include <QObject>
#include <QSoundEffect>
#include <QString>

class TgAudioManager : public QObject
{
	Q_OBJECT
public:
	enum class Game
	{
		None,
		Apple,
		Space,
	};

	static TgAudioManager &instance();

	void setEnabled(bool on);
	bool isEnabled() const;

	void setPaused(bool paused);
	bool isPaused() const;

	void startBgm(Game game);
	void stopBgm();

	void playCommon(const QString &logicalName);
	void playApple(const QString &logicalName);
	void playSpace(const QString &logicalName);

private:
	explicit TgAudioManager(QObject *pParent = nullptr);
	TgAudioManager(const TgAudioManager &) = delete;
	TgAudioManager &operator=(const TgAudioManager &) = delete;

	void ensureLoaded(QSoundEffect &effect, const QString &subDir, const QString &logicalName, bool loop);
	QString soundPath(const QString &subDir, const QString &logicalName) const;
	void applyPausedToAll();

	bool m_enabled = true;
	bool m_paused = false;
	Game m_bgmGame = Game::None;

	QSoundEffect m_bgm;
	QSoundEffect m_sfxCommon;
	QSoundEffect m_sfxApple;
	QSoundEffect m_sfxSpace;
};

#endif

