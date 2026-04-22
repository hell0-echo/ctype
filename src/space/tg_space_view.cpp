#include "tg_space_view.h"
#include "tg_space_model.h"
#include "common/tg_resourceprovider.h"
#include "common/tg_audiomanager.h"
#include <algorithm>
#include <cmath>
#include <QPainter>
#include <QKeyEvent>
#include <QPaintEvent>
#include <QResizeEvent>

TgSpaceView::TgSpaceView(TgSpaceModel* pModel, TgResourceProvider* pResources, QWidget* pParent)
    : QWidget(pParent), m_pModel(pModel), m_pResources(pResources)
{
    setFocusPolicy(Qt::StrongFocus);
}

void TgSpaceView::refreshLayout()
{
    if (m_pModel) m_pModel->setViewportSize(width(), height());
}

void TgSpaceView::resizeEvent(QResizeEvent* pEvent)
{
    QWidget::resizeEvent(pEvent);
    refreshLayout();
}

void TgSpaceView::paintEvent(QPaintEvent* pEvent)
{
    Q_UNUSED(pEvent);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    if (!m_pModel) return;

    auto spriteSrcRect = [](const QPixmap& sheet, int cols, int rows, int idx, int frameCount) -> QRect {
        if (sheet.isNull() || cols <= 0 || rows <= 0)
            return QRect();
        const int fw = sheet.width() / cols;
        const int fh = sheet.height() / rows;
        if (fw <= 0 || fh <= 0)
            return QRect();
        const int maxCount = cols * rows;
        const int count = (frameCount > 0) ? std::min(frameCount, maxCount) : maxCount;
        const int i = (count > 0) ? (idx % count) : 0;
        const int cx = i % cols;
        const int cy = i / cols;
        return QRect(cx * fw, cy * fh, fw, fh);
    };

    const QPixmap bg = m_pResources->spacePixmap("SPACE_BACKGROUND");
    const qreal scroll = m_pModel->scrollOffset();
    const int tileH = bg.height() > 0 ? bg.height() : 600;
    const int yOff = static_cast<int>(std::fmod(scroll, static_cast<qreal>(tileH)));
    for (int y = -tileH; y < height() + tileH; y += tileH)
        painter.drawPixmap(0, y + yOff, width(), tileH, bg);

    const TgSpaceGameState st = m_pModel->state();
    if (st == TgSpaceGameState::InitialSpaceState) {
        painter.fillRect(rect(), QColor(0, 10, 30, 120));
        painter.setPen(Qt::white);
        painter.setFont(QFont("Arial", 22, QFont::Bold));
        painter.drawText(QRect(0, height() / 4, width(), 40), Qt::AlignCenter, QStringLiteral("Space Typing"));
        return; // 菜单按钮已移至 GameWidget
    }

    const QPixmap planeSheet = m_pResources->spacePixmap("SPACE_ENEMY_0");
    const QPixmap meteorSheet = m_pResources->spacePixmap("SPACE_ENEMY_4");
    const QPixmap shipSheet = m_pResources->spacePixmap("SPACE_SHIP");
    const QPixmap caption = m_pResources->spacePixmap("SPACE_CAPTION_BACK");
    const QPixmap bomb = m_pResources->spacePixmap("SPACE_BOMB");
    const QPixmap boomSheet = m_pResources->spacePixmap("SPACE_EXPLOSION_0");

    for (const TgSpaceEnemy* e : m_pModel->enemies()) {
        const QRectF er = m_pModel->enemyRect(*e);
        if (e->m_explosionFrame >= 0) {
            // Explosion sheet is 3x3 (9 frames). Play once; after frame 8, stop drawing.
            if (e->m_explosionFrame >= 9)
                continue;
            const int f = std::max(0, e->m_explosionFrame);
            painter.setOpacity(0.7);
            if (!boomSheet.isNull()) {
                // 3x3 frames in SPACE_EXPLOSION_0
                const QRect src = spriteSrcRect(boomSheet, 3, 3, f, 9);
                painter.drawPixmap(er.toAlignedRect(), boomSheet, src);
            }
            else {
                painter.setBrush(QColor(255, 140, 40, 200));
                painter.setPen(Qt::NoPen);
                painter.drawEllipse(er);
            }
            painter.setOpacity(1.0);
            continue;
        }

        const bool meteorKind = (e->m_kind == TgSpaceEnemyKind::MeteorEnemyKind);
        const QPixmap& sheet = meteorKind ? meteorSheet : planeSheet;
        // Enemy sheets are 4 rows x 3 cols. Plane uses 11 frames (last cell empty), meteor uses full 12 frames.
        const int frameCount = meteorKind ? 12 : 11;
        const int frameIdx = static_cast<int>(std::abs(e->m_phase * 10.0)) % frameCount;
        const QRect srcRect = spriteSrcRect(sheet, 3, 4, frameIdx, frameCount);
        if (!srcRect.isNull())
            painter.drawPixmap(er.toAlignedRect(), sheet, srcRect);

        // Caption back + letter (reference uses a fixed caption image)
        const QRect erI = er.toAlignedRect();
        // Make caption slightly smaller and lower, overlapping enemy a bit; keep letter on top.
        const int capW = 44;
        const int capH = 16;
        const int capX = erI.center().x() - capW / 2;
        const int capY = erI.top() - 14;
        const QRect capRect(capX, capY, capW, capH);
        if (!caption.isNull())
            painter.drawPixmap(capRect, caption.scaled(capRect.size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
        painter.setPen(Qt::black);
        painter.setFont(QFont("Arial", 11, QFont::Bold));
        painter.drawText(capRect, Qt::AlignCenter, QString(QChar::fromLatin1(e->m_letter)));
    }

    for (const TgSpaceBullet* b : m_pModel->bullets()) {
        const QRect r(static_cast<int>(b->m_x) - 6, static_cast<int>(b->m_y) - 6, 12, 12);
        if (!bomb.isNull())
            painter.drawPixmap(r, bomb.scaled(r.size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
        else {
            painter.setBrush(QColor(255, 240, 120));
            painter.setPen(Qt::NoPen);
            painter.drawEllipse(r);
        }
    }

    const QRectF pr = m_pModel->playerRect();
    if (!shipSheet.isNull()) {
        // Player ship sheet is 4 rows x 3 cols, 11 valid frames (last cell empty).
        const int shipFrameCount = 11;
        const int shipFrame = static_cast<int>(scroll / 6.0) % shipFrameCount;
        const QRect srcRect = spriteSrcRect(shipSheet, 3, 4, shipFrame, shipFrameCount);
        if (!srcRect.isNull())
            painter.drawPixmap(pr.toAlignedRect(), shipSheet, srcRect);
    }
    else {
        painter.setBrush(QColor(80, 200, 255));
        painter.setPen(Qt::NoPen);
        painter.drawEllipse(pr);
    }

    painter.setPen(Qt::white);
    painter.setFont(QFont("Arial", 10));
    const QString hud = QString("Score: %1  HP: %2  Upgrade: %3s")
        .arg(m_pModel->score())
        .arg(m_pModel->hp())
        .arg(static_cast<int>(m_pModel->upgradeCountdownSec() + 0.999));
    painter.drawText(QRect(10, 8, width() - 20, 22), Qt::AlignLeft, hud);

    if (m_pModel->hasBonusWord()) {
        const QRectF wr = m_pModel->bonusWordRect();
        const QString word = m_pModel->bonusWord();
        const int progress = std::min(m_pModel->bonusProgress(), word.size());

        // No solid background; draw letters individually with per-letter color.
        QFont f("Arial", 18, QFont::Bold);
        painter.setFont(f);
        const QFontMetrics fm(f);
        const int totalW = fm.horizontalAdvance(word);
        int x = static_cast<int>(wr.center().x()) - totalW / 2;
        const int baselineY = static_cast<int>(wr.center().y()) + (fm.ascent() - fm.descent()) / 2;

        const QColor kDeepBlue(0, 0, 139);
        const QColor kRed(200, 0, 0);
        for (int i = 0; i < word.size(); ++i)
        {
            const QString ch = word.mid(i, 1);
            painter.setPen(i < progress ? kRed : kDeepBlue);
            painter.drawText(QPoint(x, baselineY), ch);
            x += fm.horizontalAdvance(ch);
        }
    }

    if (st == TgSpaceGameState::PausedSpaceState) {
        painter.fillRect(rect(), QColor(0, 0, 0, 120));
        painter.setPen(Qt::white);
        painter.setFont(QFont("Arial", 18, QFont::Bold));
        painter.drawText(rect(), Qt::AlignCenter, QStringLiteral("Paused"));
    }
    else if (st == TgSpaceGameState::EndSpaceState) {
        painter.fillRect(rect(), QColor(0, 0, 0, 160));
        painter.setPen(Qt::white);
        painter.setFont(QFont("Arial", 18, QFont::Bold));
        painter.drawText(rect(), Qt::AlignCenter, QStringLiteral("Game over"));
    }
}

void TgSpaceView::keyPressEvent(QKeyEvent* pEvent)
{
    if (!m_pModel) { QWidget::keyPressEvent(pEvent); return; }
    if (pEvent->key() == Qt::Key_Escape)
    {
        emit escapePressed();
        return;
    }
    const QString t = pEvent->text();
    if (t.size() == 1 && t.at(0).isLetter()) {
        const char ascii = static_cast<char>(t.at(0).toUpper().toLatin1());
        if (m_pModel->state() == TgSpaceGameState::PlayingSpaceState) {
            TgAudioManager::instance().playCommon(QStringLiteral("TYPE"));
            if (!m_pModel->tryFireAtLetter(ascii))
                m_pModel->applyWrongLetterPenalty();
            return;
        }
    }
    QWidget::keyPressEvent(pEvent);
}