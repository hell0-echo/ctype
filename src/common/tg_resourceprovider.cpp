#include "tg_resourceprovider.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QPainter>



TgResourceProvider::TgResourceProvider()
{
	m_basePath = QCoreApplication::applicationDirPath() + QDir::separator() + "res";
}

QPixmap TgResourceProvider::loadPixmap(const QString& subDir, const QString& logicalName) const
{
    const QString pathPng = m_basePath + QDir::separator() + subDir +
        QDir::separator() + "images" + QDir::separator() +
        logicalName + ".png";
    QPixmap pm;
    pm.load(pathPng);
    return pm;
}

QPixmap TgResourceProvider::commonPixmap(const QString& logicalName) const
{
    return loadPixmap("Common", logicalName);
}

QPixmap TgResourceProvider::applePixmap(const QString& logicalName) const
{
    return loadPixmap("Apple", logicalName);
}

QPixmap TgResourceProvider::spacePixmap(const QString& logicalName) const
{
    return loadPixmap("Space", logicalName);
}