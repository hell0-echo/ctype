#ifndef _TYPEGAME_TG_SPACE_VIEW_H_
#define _TYPEGAME_TG_SPACE_VIEW_H_

#include <QWidget>

class TgResourceProvider;
class TgSpaceModel;

class TgSpaceView : public QWidget
{
    Q_OBJECT
public:
    explicit TgSpaceView(TgSpaceModel* pModel, TgResourceProvider* pResources, QWidget* pParent = nullptr);
    TgSpaceView(const TgSpaceView&) = delete;
    TgSpaceView& operator=(const TgSpaceView&) = delete;
    void refreshLayout();

protected:
    void paintEvent(QPaintEvent* pEvent) override;
    void resizeEvent(QResizeEvent* pEvent) override;
    void keyPressEvent(QKeyEvent* pEvent) override;

private:
    TgSpaceModel* m_pModel = nullptr;
    TgResourceProvider* m_pResources = nullptr;
};
#endif