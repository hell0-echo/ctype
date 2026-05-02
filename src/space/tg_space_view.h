#ifndef _TYPEGAME_TG_SPACE_VIEW_H_
#define _TYPEGAME_TG_SPACE_VIEW_H_

#include <QWidget>

class TgResourceProvider;
class TgSpaceModel;
class QLabel;
class TgImageProgressBar;

class TgSpaceView : public QWidget
{
    Q_OBJECT
public:
    explicit TgSpaceView(TgSpaceModel* pModel, TgResourceProvider* pResources, QWidget* pParent = nullptr);
    TgSpaceView(const TgSpaceView&) = delete;
    TgSpaceView& operator=(const TgSpaceView&) = delete;
    void refreshLayout();

signals:
    void escapePressed();

public slots:
    void onModelUpdated();

protected:
    void paintEvent(QPaintEvent* pEvent) override;
    void resizeEvent(QResizeEvent* pEvent) override;
    void keyPressEvent(QKeyEvent* pEvent) override;

private:
    void buildHud();
    void layoutHud();
    void updateHudValues();

    TgSpaceModel* m_pModel = nullptr;
    TgResourceProvider* m_pResources = nullptr;

    QWidget* m_pHud = nullptr;
    QLabel* m_pScoreLabel = nullptr;
    QLabel* m_pScoreValue = nullptr;
    QLabel* m_pLifeLabel = nullptr;
    TgImageProgressBar* m_pLifeBar = nullptr;
    QLabel* m_pTimeLabel = nullptr;
    QLabel* m_pTimeValue = nullptr;
};
#endif