#ifndef _TYPEGAME_TG_CONFIG_H_
#define _TYPEGAME_TG_CONFIG_H_

#include <QString>

class TgConfig
{
public:
	explicit TgConfig(const QString &organization, const QString &application);

	void load();
	void save() const;

	int appleLevel() const;
	void setAppleLevel(int level);

	int appleFailLimit() const;
	void setAppleFailLimit(int value);

	int appleWinTarget() const;
	void setAppleWinTarget(int value);

	int appleMaxOnScreen() const;
	void setAppleMaxOnScreen(int value);

	bool appleSoundEnabled() const;
	void setAppleSoundEnabled(bool on);

	int spaceEnemyCount() const;
	void setSpaceEnemyCount(int value);

	int spaceEnemySpeedLevel() const;
	void setSpaceEnemySpeedLevel(int value);

	int spaceUpgradeIntervalSec() const;
	void setSpaceUpgradeIntervalSec(int value);

	bool spaceBonusMode() const;
	void setSpaceBonusMode(bool on);

	QString spaceBonusApiKey() const;
	void setSpaceBonusApiKey(const QString &key);

private:
	QString m_filePath;

	int m_appleLevel = 3;
	int m_appleFailLimit = 10;
	int m_appleWinTarget = 100;
	int m_appleMaxOnScreen = 5;
	bool m_appleSound = true;

	int m_spaceEnemyCount = 3;
	int m_spaceEnemySpeedLevel = 3;
	int m_spaceUpgradeIntervalSec = 120;
	bool m_spaceBonusMode = false;
	QString m_spaceBonusApiKey;
};

#endif
