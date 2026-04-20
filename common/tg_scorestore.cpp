#include "tg_scorestore.h"

#include <QSettings>
#include <algorithm>

namespace
{
constexpr int kMaxEntries = 10;
}

TgScoreStore::TgScoreStore(const QString &filePath)
	: m_filePath(filePath)
{
}

void TgScoreStore::load()
{
	m_entries.clear();
	QSettings s(m_filePath, QSettings::IniFormat);
	const int n = s.beginReadArray("scores");
	for (int i = 0; i < n; ++i)
	{
		s.setArrayIndex(i);
		TgScoreEntry e;
		e.m_name = s.value("name").toString();
		e.m_score = s.value("score").toInt();
		m_entries.push_back(e);
	}
	s.endArray();
}

void TgScoreStore::save() const
{
	QSettings s(m_filePath, QSettings::IniFormat);
	s.remove("scores");
	s.beginWriteArray("scores");
	for (int i = 0; i < static_cast<int>(m_entries.size()); ++i)
	{
		s.setArrayIndex(i);
		s.setValue("name", m_entries[static_cast<std::size_t>(i)].m_name);
		s.setValue("score", m_entries[static_cast<std::size_t>(i)].m_score);
	}
	s.endArray();
	s.sync();
}

void TgScoreStore::submitScore(const QString &name, int score)
{
	TgScoreEntry e;
	e.m_name = name.isEmpty() ? QStringLiteral("Pilot") : name;
	e.m_score = score;
	m_entries.push_back(e);
	std::sort(m_entries.begin(), m_entries.end(),
		[](const TgScoreEntry &a, const TgScoreEntry &b) { return a.m_score > b.m_score; });
	if (static_cast<int>(m_entries.size()) > kMaxEntries)
		m_entries.resize(static_cast<std::size_t>(kMaxEntries));
	save();
}

std::vector<TgScoreEntry> TgScoreStore::topTen() const
{
	return m_entries;
}
