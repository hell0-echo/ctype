#include "tg_bonuswordprovider.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>

namespace
{
constexpr char kDeepSeekChatUrl[] = "https://api.deepseek.com/chat/completions";
constexpr char kModelId[] = "deepseek-v4-flash";

QString buildRequestJson()
{
	QJsonObject thinking;
	thinking.insert(QStringLiteral("type"), QStringLiteral("disabled"));

	QJsonArray messages;
	{
		QJsonObject systemMsg;
		systemMsg.insert(QStringLiteral("role"), QStringLiteral("system"));
		systemMsg.insert(QStringLiteral("content"),
			QStringLiteral("You help a typing game. Reply with exactly one English word: only letters A–Z, "
				"length 5–12, space or astronomy themed. No punctuation, quotes, numbers, spaces, or explanation."));
		messages.append(systemMsg);
	}
	{
		QJsonObject userMsg;
		userMsg.insert(QStringLiteral("role"), QStringLiteral("user"));
		userMsg.insert(QStringLiteral("content"), QStringLiteral("Reply with one word only."));
		messages.append(userMsg);
	}

	QJsonObject root;
	root.insert(QStringLiteral("model"), QString::fromLatin1(kModelId));
	root.insert(QStringLiteral("messages"), messages);
	root.insert(QStringLiteral("thinking"), thinking);
	root.insert(QStringLiteral("temperature"), 0.9);
	root.insert(QStringLiteral("max_tokens"), 32);

	return QString::fromUtf8(QJsonDocument(root).toJson(QJsonDocument::Compact));
}
} // namespace

TgBonusWordProvider::TgBonusWordProvider(QObject *pParent)
	: QObject(pParent)
	, m_pNetwork(new QNetworkAccessManager(this))
{
}

void TgBonusWordProvider::setApiKey(const QString &key)
{
	m_apiKey = key;
}

void TgBonusWordProvider::requestWordAsync()
{
	if (m_apiKey.trimmed().isEmpty())
	{
		emit requestFailed();
		emit wordReady(QString());
		return;
	}

	if (m_pPendingReply)
	{
		disconnect(m_pPendingReply, &QNetworkReply::finished, this, &TgBonusWordProvider::onNetworkFinished);
		m_pPendingReply->abort();
		m_pPendingReply->deleteLater();
		m_pPendingReply = nullptr;
	}

	QNetworkRequest req{QUrl(QString::fromLatin1(kDeepSeekChatUrl))};
	req.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));
	req.setRawHeader("Authorization", QByteArray("Bearer ") + m_apiKey.trimmed().toUtf8());

	const QByteArray payload = buildRequestJson().toUtf8();
	m_pPendingReply = m_pNetwork->post(req, payload);
	connect(m_pPendingReply, &QNetworkReply::finished, this, &TgBonusWordProvider::onNetworkFinished);
}

QString TgBonusWordProvider::sanitizeWordFromContent(const QString &raw)
{
	QString best;
	QString cur;
	auto flushToken = [&]() {
		if (cur.size() < 4 || cur.size() > 64)
		{
			cur.clear();
			return;
		}
		if (cur.size() > best.size())
			best = cur;
		cur.clear();
	};

	for (const QChar &ch : raw)
	{
		const ushort u = ch.unicode();
		if ((u >= static_cast<ushort>('A') && u <= static_cast<ushort>('Z'))
			|| (u >= static_cast<ushort>('a') && u <= static_cast<ushort>('z')))
		{
			if (u >= static_cast<ushort>('a'))
				cur.append(QChar(static_cast<ushort>(u - static_cast<ushort>('a') + static_cast<ushort>('A'))));
			else
				cur.append(ch);
		}
		else
		{
			flushToken();
		}
	}
	flushToken();
	return best;
}

void TgBonusWordProvider::onNetworkFinished()
{
	auto *pReply = qobject_cast<QNetworkReply *>(sender());
	if (nullptr == pReply)
		return;
	if (pReply == m_pPendingReply)
		m_pPendingReply = nullptr;
	pReply->deleteLater();

	if (pReply->error() != QNetworkReply::NoError)
	{
		emit requestFailed();
		emit wordReady(QString());
		return;
	}

	const QByteArray body = pReply->readAll();
	QJsonParseError parseErr{};
	const QJsonDocument doc = QJsonDocument::fromJson(body, &parseErr);
	if (parseErr.error != QJsonParseError::NoError || !doc.isObject())
	{
		emit requestFailed();
		emit wordReady(QString());
		return;
	}

	const QJsonObject root = doc.object();
	const QJsonArray choices = root.value(QStringLiteral("choices")).toArray();
	if (choices.isEmpty())
	{
		emit requestFailed();
		emit wordReady(QString());
		return;
	}

	const QJsonObject choice0 = choices.at(0).toObject();
	const QJsonObject message = choice0.value(QStringLiteral("message")).toObject();
	const QString content = message.value(QStringLiteral("content")).toString();
	const QString w = sanitizeWordFromContent(content);
	if (w.isEmpty())
	{
		emit requestFailed();
		emit wordReady(QString());
		return;
	}

	emit wordReady(w);
}
