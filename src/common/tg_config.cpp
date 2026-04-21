#include "tg_config.h"

#include <QCoreApplication>
#include <QDir>
#include <QSettings>

namespace
{
QString resolveIniPath(const QString &organization, const QString &application)
{
	const QString base = QCoreApplication::applicationDirPath();
	return base + QDir::separator() + "typegame.ini";
}
}

TgConfig::TgConfig(const QString &organization, const QString &application)
	: m_filePath(resolveIniPath(organization, application))
{
	Q_UNUSED(organization);
	Q_UNUSED(application);
}

void TgConfig::load()
{
	QSettings s(m_filePath, QSettings::IniFormat);
	m_appleLevel = s.value("apple/level", 3).toInt();
	m_appleFailLimit = s.value("apple/fail_limit", 10).toInt();
	m_appleWinTarget = s.value("apple/win_target", 100).toInt();
	m_appleMaxOnScreen = s.value("apple/max_on_screen", 5).toInt();
	m_appleSound = s.value("apple/sound", true).toBool();

	m_spaceEnemyCount = s.value("space/enemy_count", 3).toInt();
	m_spaceEnemySpeedLevel = s.value("space/speed_level", 3).toInt();
	m_spaceUpgradeIntervalSec = s.value("space/upgrade_interval_sec", 120).toInt();
	m_spaceBonusMode = s.value("space/bonus_mode", false).toBool();
	m_spaceBonusApiEndpoint = s.value("space/bonus_api_endpoint", QString()).toString();
	m_spaceBonusApiKey = s.value("space/bonus_api_key", QString()).toString();
}

void TgConfig::save() const
{
	QSettings s(m_filePath, QSettings::IniFormat);
	s.setValue("apple/level", m_appleLevel);
	s.setValue("apple/fail_limit", m_appleFailLimit);
	s.setValue("apple/win_target", m_appleWinTarget);
	s.setValue("apple/max_on_screen", m_appleMaxOnScreen);
	s.setValue("apple/sound", m_appleSound);

	s.setValue("space/enemy_count", m_spaceEnemyCount);
	s.setValue("space/speed_level", m_spaceEnemySpeedLevel);
	s.setValue("space/upgrade_interval_sec", m_spaceUpgradeIntervalSec);
	s.setValue("space/bonus_mode", m_spaceBonusMode);
	s.setValue("space/bonus_api_endpoint", m_spaceBonusApiEndpoint);
	s.setValue("space/bonus_api_key", m_spaceBonusApiKey);
	s.sync();
}

int TgConfig::appleLevel() const
{
	return m_appleLevel;
}

void TgConfig::setAppleLevel(int level)
{
	m_appleLevel = level;
}

int TgConfig::appleFailLimit() const
{
	return m_appleFailLimit;
}

void TgConfig::setAppleFailLimit(int value)
{
	m_appleFailLimit = value;
}

int TgConfig::appleWinTarget() const
{
	return m_appleWinTarget;
}

void TgConfig::setAppleWinTarget(int value)
{
	m_appleWinTarget = value;
}

int TgConfig::appleMaxOnScreen() const
{
	return m_appleMaxOnScreen;
}

void TgConfig::setAppleMaxOnScreen(int value)
{
	m_appleMaxOnScreen = value;
}

bool TgConfig::appleSoundEnabled() const
{
	return m_appleSound;
}

void TgConfig::setAppleSoundEnabled(bool on)
{
	m_appleSound = on;
}

int TgConfig::spaceEnemyCount() const
{
	return m_spaceEnemyCount;
}

void TgConfig::setSpaceEnemyCount(int value)
{
	m_spaceEnemyCount = value;
}

int TgConfig::spaceEnemySpeedLevel() const
{
	return m_spaceEnemySpeedLevel;
}

void TgConfig::setSpaceEnemySpeedLevel(int value)
{
	m_spaceEnemySpeedLevel = value;
}

int TgConfig::spaceUpgradeIntervalSec() const
{
	return m_spaceUpgradeIntervalSec;
}

void TgConfig::setSpaceUpgradeIntervalSec(int value)
{
	m_spaceUpgradeIntervalSec = value;
}

bool TgConfig::spaceBonusMode() const
{
	return m_spaceBonusMode;
}

void TgConfig::setSpaceBonusMode(bool on)
{
	m_spaceBonusMode = on;
}

QString TgConfig::spaceBonusApiEndpoint() const
{
	return m_spaceBonusApiEndpoint;
}

void TgConfig::setSpaceBonusApiEndpoint(const QString &url)
{
	m_spaceBonusApiEndpoint = url;
}

QString TgConfig::spaceBonusApiKey() const
{
	return m_spaceBonusApiKey;
}

void TgConfig::setSpaceBonusApiKey(const QString &key)
{
	m_spaceBonusApiKey = key;
}
