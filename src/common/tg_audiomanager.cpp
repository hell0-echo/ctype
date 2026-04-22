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
	// SFX effects are created on-demand into pools.
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

void TgAudioManager::preload()
{
	// Preload commonly used sounds to avoid first-hit blocking I/O.
	// Pools are created with desired sizes; setSource() will be invoked once per instance.
	const QStringList common = {
		QStringLiteral("TYPE"),
		QStringLiteral("BTN_CLICK"),
		QStringLiteral("ANIBTN_CLICK"),
		QStringLiteral("ANIBTN_ENTER"),
		QStringLiteral("GLIDE"),
	};
	for (const QString &n : common)
		ensurePool(QStringLiteral("Common"), n, desiredPoolSize(QStringLiteral("Common"), n));

	const QStringList apple = { QStringLiteral("APPLE_IN") };
	for (const QString &n : apple)
		ensurePool(QStringLiteral("Apple"), n, desiredPoolSize(QStringLiteral("Apple"), n));

	const QStringList space = {
		QStringLiteral("SPACE_SHOOT"),
		QStringLiteral("SPACE_BLAST"),
		QStringLiteral("SPACE_PLANEOUT"),
		QStringLiteral("SPACE_WORDOUT"),
		QStringLiteral("UPGRADE"),
	};
	for (const QString &n : space)
		ensurePool(QStringLiteral("Space"), n, desiredPoolSize(QStringLiteral("Space"), n));
}

void TgAudioManager::playCommon(const QString &logicalName)
{
	if (!m_enabled)
		return;
	if (m_paused)
		return;
	playFromPool(QStringLiteral("Common"), logicalName);
}

void TgAudioManager::playApple(const QString &logicalName)
{
	if (!m_enabled)
		return;
	if (m_paused)
		return;
	playFromPool(QStringLiteral("Apple"), logicalName);
}

void TgAudioManager::playSpace(const QString &logicalName)
{
	if (!m_enabled)
		return;
	if (m_paused)
		return;
	playFromPool(QStringLiteral("Space"), logicalName);
}

void TgAudioManager::ensureLoaded(QSoundEffect &effect, const QString &subDir, const QString &logicalName, bool loop)
{
	const QString path = soundPath(subDir, logicalName);
	const QUrl url = QUrl::fromLocalFile(path);
	if (effect.source() != url)
		effect.setSource(url);
	effect.setLoopCount(loop ? QSoundEffect::Infinite : 1);
}

int TgAudioManager::desiredPoolSize(const QString &subDir, const QString &logicalName) const
{
	Q_UNUSED(subDir);
	// TYPE is extremely high frequency.
	if (logicalName == QStringLiteral("TYPE"))
		return 10;
	// Short UI sounds can overlap a little.
	if (logicalName == QStringLiteral("BTN_CLICK") || logicalName == QStringLiteral("ANIBTN_CLICK") || logicalName == QStringLiteral("ANIBTN_ENTER"))
		return 3;
	// Game SFX can overlap (blast/spawn/shoot).
	if (logicalName.startsWith(QStringLiteral("SPACE_")) || logicalName == QStringLiteral("UPGRADE") || logicalName == QStringLiteral("APPLE_IN"))
		return 4;
	return 2;
}

void TgAudioManager::ensurePool(const QString &subDir, const QString &logicalName, int poolSize)
{
	const QString key = subDir + QLatin1Char('/') + logicalName;
	auto &vec = m_pools[key];
	if (vec.size() >= poolSize)
		return;

	const QString path = soundPath(subDir, logicalName);
	const QUrl url = QUrl::fromLocalFile(path);

	while (vec.size() < poolSize)
	{
		auto *e = new QSoundEffect(this);
		e->setVolume(kSfxVolume);
		e->setLoopCount(1);
		e->setSource(url); // preload source (may block once at startup, not during gameplay)
		vec.push_back(e);
	}
	if (!m_poolCursor.contains(key))
		m_poolCursor.insert(key, 0);
}

void TgAudioManager::playFromPool(const QString &subDir, const QString &logicalName)
{
	const int poolSize = desiredPoolSize(subDir, logicalName);
	ensurePool(subDir, logicalName, poolSize);

	const QString key = subDir + QLatin1Char('/') + logicalName;
	auto it = m_pools.find(key);
	if (it == m_pools.end() || it->isEmpty())
		return;

	QVector<QSoundEffect *> &vec = it.value();
	int &cursor = m_poolCursor[key];

	// Prefer a channel that is not currently playing.
	const int n = vec.size();
	int pick = -1;
	for (int i = 0; i < n; ++i)
	{
		const int idx = (cursor + i) % n;
		if (vec[idx] && !vec[idx]->isPlaying())
		{
			pick = idx;
			break;
		}
	}
	if (pick < 0)
		pick = cursor % n;
	cursor = (pick + 1) % n;

	QSoundEffect *e = vec[pick];
	if (nullptr == e)
		return;
	e->play();
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

