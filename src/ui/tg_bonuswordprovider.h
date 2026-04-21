#ifndef _TYPEGAME_TG_BONUSWORDPROVIDER_H_
#define _TYPEGAME_TG_BONUSWORDPROVIDER_H_

#include <QObject>
#include <QString>

class QNetworkAccessManager;

class TgBonusWordProvider : public QObject
{
	Q_OBJECT

public:
	explicit TgBonusWordProvider(QObject *pParent = nullptr);

	TgBonusWordProvider(const TgBonusWordProvider &) = delete;
	TgBonusWordProvider &operator=(const TgBonusWordProvider &) = delete;

	void setEndpoint(const QString &url);
	void setApiKey(const QString &key);

	void requestWordAsync();

signals:
	void wordReady(const QString &word);
	void requestFailed();

private slots:
	void onNetworkFinished();

private:
	QString pickLocalWord() const;

	QString m_endpoint;
	QString m_apiKey;
	QNetworkAccessManager *m_pNetwork = nullptr;
};

#endif
