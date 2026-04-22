#ifndef _TYPEGAME_TG_RESOURCEPROVIDER_H_
#define _TYPEGAME_TG_RESOURCEPROVIDER_H_

#include <QPixmap>
#include <QString>

class TgResourceProvider
{
private:
	QPixmap loadPixmap(const QString& subDir, const QString& logicalName) const;

public:
	explicit TgResourceProvider();

	QPixmap commonPixmap(const QString &logicalName) const;
	QPixmap applePixmap(const QString& logicalName) const;
	QPixmap spacePixmap(const QString& logicalName) const;

private:
	QString m_basePath;
};

#endif
