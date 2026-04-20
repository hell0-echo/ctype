#ifndef _TYPEGAME_TG_RESOURCEPROVIDER_H_
#define _TYPEGAME_TG_RESOURCEPROVIDER_H_

#include <QPixmap>
#include <QString>

class TgResourceProvider
{
public:
	explicit TgResourceProvider();

	QPixmap pixmap(const QString &logicalName) const;

private:
	QString m_basePath;
};

#endif
