/* ------------------------------------------------------------------
//  File      : tg_audiomanager.cpp
//  Created   : 2026-04-21
//  Purpose   : Centralized BGM/SFX with pause/resume sync
// ----------------------------------------------------------------*/
#include "tg_audiomanager.h"

#include <QCoreApplication>
#include <QDir>
#include <QUrl>

namespace
{
constexpr qreal kBgmVolume = 0.45;
constexpr qreal kSfxVolume = 0.75;
}

TgAudioManager &TgAudioManager::instance()
{
	static TgAudioManager inst;
	return inst;
}

TgAudioManager::TgAudioManager(QObject *pParent)
	: QObject(pParent)
{
	m_bgm.setVolume(kBgmVolume);
	m_sfxCommon.setVolume(kSfxVolume);
	m_sfxApple.setVolume(kSfxVolume);
	m_sfxSpace.setVolume(kSfxVolume);
}

void TgAudioManager::setEnabled(bool on)
{
	m_enabled = on;
	if (!m_enabled)
	{
		stopBgm();
	}
}

bool TgAudioManager::isEnabled() const
{
	return m_enabled;
}

void TgAudioManager::setPaused(bool paused)
{
	m_paused = paused;
	applyPausedToAll();
}

bool TgAudioManager::isPaused() const
{
	return m_paused;
}

void TgAudioManager::startBgm(Game game)
{
	m_bgmGame = game;
	if (!m_enabled || game == Game::None)
		return;

	QString logical;
	QString subDir;
	if (game == Game::Apple)
	{
		subDir = QStringLiteral("Apple");
		logical = QStringLiteral("APPLE_BG");
	}
	else if (game == Game::Space)
	{
		subDir = QStringLiteral("Space");
		logical = QStringLiteral("SPACE_BG");
	}
	else
	{
		return;
	}

	ensureLoaded(m_bgm, subDir, logical, true);
	if (m_paused)
		m_bgm.stop();
	else
		m_bgm.play();
}

void TgAudioManager::stopBgm()
{
	m_bgm.stop();
	m_bgmGame = Game::None;
}

void TgAudioManager::playCommon(const QString &logicalName)
{
	if (!m_enabled)
		return;
	ensureLoaded(m_sfxCommon, QStringLiteral("Common"), logicalName, false);
	if (!m_paused)
		m_sfxCommon.play();
}

void TgAudioManager::playApple(const QString &logicalName)
{
	if (!m_enabled)
		return;
	ensureLoaded(m_sfxApple, QStringLiteral("Apple"), logicalName, false);
	if (!m_paused)
		m_sfxApple.play();
}

void TgAudioManager::playSpace(const QString &logicalName)
{
	if (!m_enabled)
		return;
	ensureLoaded(m_sfxSpace, QStringLiteral("Space"), logicalName, false);
	if (!m_paused)
		m_sfxSpace.play();
}

void TgAudioManager::ensureLoaded(QSoundEffect &effect, const QString &subDir, const QString &logicalName, bool loop)
{
	const QString path = soundPath(subDir, logicalName);
	const QUrl url = QUrl::fromLocalFile(path);
	if (effect.source() != url)
		effect.setSource(url);
	effect.setLoopCount(loop ? QSoundEffect::Infinite : 1);
}

QString TgAudioManager::soundPath(const QString &subDir, const QString &logicalName) const
{
	const QString base = QCoreApplication::applicationDirPath() + QDir::separator() + "res";
	return base + QDir::separator() + subDir + QDir::separator() + "Sounds" + QDir::separator() + logicalName + ".wav";
}

void TgAudioManager::applyPausedToAll()
{
	if (m_paused)
	{
		m_bgm.stop();
	}
	else
	{
		if (m_bgmGame != Game::None && m_enabled)
			m_bgm.play();
	}
}

