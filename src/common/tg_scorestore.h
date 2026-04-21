#ifndef _TYPEGAME_TG_SCORESTORE_H_
#define _TYPEGAME_TG_SCORESTORE_H_

#include <QString>
#include <vector>

struct TgScoreEntry
{
	QString m_name;
	int m_score = 0;
};

class TgScoreStore
{
public:
	explicit TgScoreStore(const QString &filePath);

	void load();
	void save() const;

	void submitScore(const QString &name, int score);
	std::vector<TgScoreEntry> topTen() const;

private:
	QString m_filePath;
	std::vector<TgScoreEntry> m_entries;
};

#endif
