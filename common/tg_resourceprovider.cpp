#include "tg_resourceprovider.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QPainter>

namespace
{
QPixmap makePlaceholder(const QSize &size, const QString &label)
{
	QPixmap pm(size);
	pm.fill(Qt::darkGray);
	QPainter p(&pm);
	p.setPen(Qt::white);
	p.drawText(pm.rect(), Qt::AlignCenter, label);
	return pm;
}
}

TgResourceProvider::TgResourceProvider()
{
	m_basePath = QCoreApplication::applicationDirPath() + QDir::separator() + "res";
}

QPixmap TgResourceProvider::pixmap(const QString &logicalName) const
{
	const QString pathPng = m_basePath + QDir::separator() + logicalName + ".png";
	const QString pathJpg = m_basePath + QDir::separator() + logicalName + ".jpg";
	QPixmap pm;
	if (QFile::exists(pathPng))
		pm.load(pathPng);
	else if (QFile::exists(pathJpg))
		pm.load(pathJpg);
	if (pm.isNull())
		return makePlaceholder(QSize(64, 64), logicalName);
	return pm;
}
