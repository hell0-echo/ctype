#include "tg_bonuswordprovider.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QtGlobal>

namespace
{
const char *const kLocalWords[] = {
	"ORBIT", "NEBULA", "ROCKET", "PHASE", "VECTOR", "MATRIX", "PHOTON", "COSMOS"
};
}

TgBonusWordProvider::TgBonusWordProvider(QObject *pParent)
	: QObject(pParent)
	, m_pNetwork(new QNetworkAccessManager(this))
{
}

void TgBonusWordProvider::setEndpoint(const QString &url)
{
	m_endpoint = url;
}

void TgBonusWordProvider::setApiKey(const QString &key)
{
	m_apiKey = key;
}

void TgBonusWordProvider::requestWordAsync()
{
	if (m_endpoint.trimmed().isEmpty())
	{
		emit wordReady(pickLocalWord());
		return;
	}
	QNetworkRequest req{QUrl(m_endpoint)};
	if (!m_apiKey.isEmpty())
		req.setRawHeader("Authorization", QByteArray("Bearer ") + m_apiKey.toUtf8());
	QNetworkReply *pReply = m_pNetwork->get(req);
	connect(pReply, &QNetworkReply::finished, this, &TgBonusWordProvider::onNetworkFinished);
}

void TgBonusWordProvider::onNetworkFinished()
{
	auto *pReply = qobject_cast<QNetworkReply *>(sender());
	if (nullptr == pReply)
		return;
	pReply->deleteLater();
	if (pReply->error() != QNetworkReply::NoError)
	{
		emit wordReady(pickLocalWord());
		return;
	}
	const QByteArray body = pReply->readAll();
	QString w = QString::fromUtf8(body).trimmed();
	if (w.size() < 3)
		w = pickLocalWord();
	emit wordReady(w);
}

QString TgBonusWordProvider::pickLocalWord() const
{
	const int n = static_cast<int>(sizeof(kLocalWords) / sizeof(kLocalWords[0]));
	const int idx = qrand() % n;
	return QString::fromLatin1(kLocalWords[idx]);
}
