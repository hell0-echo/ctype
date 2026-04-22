#include "tg_mainwindow.h"

#include <QApplication>
#include <QCoreApplication>
#include <QTime>

int main(int argc, char *argv[])
{
	qsrand(static_cast<uint>(QTime::currentTime().msec()));
	QApplication app(argc, argv);
	QCoreApplication::setApplicationName(QStringLiteral("typegame"));
	QCoreApplication::setOrganizationName(QStringLiteral("typegame"));

	TgMainWindow w;
	w.resize(960, 600);
	w.show();
	return app.exec();
}
