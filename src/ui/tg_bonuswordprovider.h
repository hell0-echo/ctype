#ifndef _TYPEGAME_TG_BONUSWORDPROVIDER_H_
#define _TYPEGAME_TG_BONUSWORDPROVIDER_H_

#include <QObject>
#include <QString>

class QNetworkAccessManager;
class QNetworkReply;

class TgBonusWordProvider : public QObject
{
	Q_OBJECT

public:
	explicit TgBonusWordProvider(QObject *pParent = nullptr);

	TgBonusWordProvider(const TgBonusWordProvider &) = delete;
	TgBonusWordProvider &operator=(const TgBonusWordProvider &) = delete;

	void setApiKey(const QString &key);

	void requestWordAsync();

signals:
	void wordReady(const QString &word);
	void requestFailed();

private slots:
	void onNetworkFinished();

private:
	static QString sanitizeWordFromContent(const QString &raw);

	QString m_apiKey;
	QNetworkAccessManager *m_pNetwork = nullptr;
	QNetworkReply *m_pPendingReply = nullptr;
};

#endif
