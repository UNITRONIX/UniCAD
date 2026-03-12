/***************************************************************************
 *   Copyright (c) 2026 UNITRONIX                                         *
 *   UniCAD - A fork of FreeCAD with Fusion 360-style UI                  *
 *                                                                         *
 *   This file is part of UniCAD.                                         *
 *                                                                         *
 *   UniCAD is free software; you can redistribute it and/or modify       *
 *   it under the terms of the GNU Lesser General Public License (LGPL)   *
 *   as published by the Free Software Foundation; either version 2.1 of  *
 *   the License, or (at your option) any later version.                  *
 ***************************************************************************/

#include "PreCompiled.h"

#include "FusionSketchPalette.h"
#include "Application.h"
#include "Command.h"
#include "BitmapFactory.h"

#include <QPainter>
#include <QPainterPath>
#include <QGraphicsDropShadowEffect>
#include <QApplication>
#include <QScreen>

using namespace Gui;

FusionSketchPalette::FusionSketchPalette(QWidget* parent)
    : QWidget(parent)
    , m_mainLayout(nullptr)
    , m_titleBar(nullptr)
    , m_titleLabel(nullptr)
    , m_pinButton(nullptr)
    , m_closeButton(nullptr)
    , m_dofLabel(nullptr)
    , m_dofWidget(nullptr)
    , m_visible(false)
    , m_pinned(false)
    , m_dragging(false)
    , m_fadeAnimation(nullptr)
{
    // Window flags for floating palette
    setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_ShowWithoutActivating);
    
    setupUI();
    setupStyle();
    
    // Setup fade animation
    m_fadeAnimation = new QPropertyAnimation(this, "windowOpacity", this);
    m_fadeAnimation->setDuration(150);
    
    // Add drop shadow effect
    auto* shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(15);
    shadow->setColor(QColor(0, 0, 0, 100));
    shadow->setOffset(0, 2);
    setGraphicsEffect(shadow);
    
    hide();
}

FusionSketchPalette::~FusionSketchPalette() = default;

void FusionSketchPalette::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(8, 8, 8, 8);
    m_mainLayout->setSpacing(6);
    
    // Title bar with drag handle, pin and close buttons
    m_titleBar = new QWidget();
    auto* titleLayout = new QHBoxLayout(m_titleBar);
    titleLayout->setContentsMargins(4, 2, 4, 2);
    titleLayout->setSpacing(4);
    
    m_titleLabel = new QLabel(tr("Sketch Tools"));
    m_titleLabel->setStyleSheet(QStringLiteral(
        "QLabel { color: #AAAAAA; font-size: 10px; font-weight: bold; }"
    ));
    titleLayout->addWidget(m_titleLabel);
    titleLayout->addStretch();
    
    // Pin button
    m_pinButton = new QToolButton();
    m_pinButton->setFixedSize(16, 16);
    m_pinButton->setCheckable(true);
    m_pinButton->setToolTip(tr("Pin palette"));
    m_pinButton->setText(QStringLiteral("📌"));
    m_pinButton->setStyleSheet(QStringLiteral(
        "QToolButton { background: transparent; border: none; font-size: 10px; }"
        "QToolButton:checked { background: rgba(6, 150, 215, 0.3); border-radius: 3px; }"
    ));
    connect(m_pinButton, &QToolButton::toggled, this, &FusionSketchPalette::setPinned);
    titleLayout->addWidget(m_pinButton);
    
    // Close button
    m_closeButton = new QToolButton();
    m_closeButton->setFixedSize(16, 16);
    m_closeButton->setText(QStringLiteral("✕"));
    m_closeButton->setToolTip(tr("Close palette"));
    m_closeButton->setStyleSheet(QStringLiteral(
        "QToolButton { background: transparent; border: none; color: #888888; font-size: 10px; }"
        "QToolButton:hover { color: #FF6666; }"
    ));
    connect(m_closeButton, &QToolButton::clicked, this, &FusionSketchPalette::hidePalette);
    titleLayout->addWidget(m_closeButton);
    
    m_mainLayout->addWidget(m_titleBar);
    
    // DOF (Degrees of Freedom) meter widget
    m_dofWidget = new QWidget();
    auto* dofLayout = new QHBoxLayout(m_dofWidget);
    dofLayout->setContentsMargins(4, 4, 4, 4);
    dofLayout->setSpacing(4);
    
    m_dofLabel = new QLabel(tr("Fully Constrained"));
    m_dofLabel->setAlignment(Qt::AlignCenter);
    m_dofLabel->setStyleSheet(QStringLiteral(
        "QLabel {"
        "  color: white;"
        "  font-size: 10px;"
        "  font-weight: bold;"
        "  background-color: #28A745;"
        "  border-radius: 4px;"
        "  padding: 4px 8px;"
        "}"
    ));
    dofLayout->addWidget(m_dofLabel);
    m_mainLayout->addWidget(m_dofWidget);
    
    // Add separator
    addSeparatorLine();
    
    // Create tool sections
    addSection(tr("Draw"), {
        QStringLiteral("Sketcher_CreateLine"),
        QStringLiteral("Sketcher_CreateRectangle"),
        QStringLiteral("Sketcher_CreateCircle"),
        QStringLiteral("Sketcher_CreateArc"),
        QStringLiteral("Sketcher_CreatePolyline"),
        QStringLiteral("Sketcher_CreateSlot")
    });
    
    addSeparatorLine();
    
    addSection(tr("Constrain"), {
        QStringLiteral("Sketcher_ConstrainCoincident"),
        QStringLiteral("Sketcher_ConstrainHorizontal"),
        QStringLiteral("Sketcher_ConstrainVertical"),
        QStringLiteral("Sketcher_ConstrainPerpendicular"),
        QStringLiteral("Sketcher_ConstrainEqual"),
        QStringLiteral("Sketcher_ConstrainDistance")
    });
    
    addSeparatorLine();
    
    addSection(tr("Edit"), {
        QStringLiteral("Sketcher_Trimming"),
        QStringLiteral("Sketcher_Extend"),
        QStringLiteral("Sketcher_Offset"),
        QStringLiteral("Sketcher_Move"),
        QStringLiteral("Sketcher_External")
    });
    
    // Finish sketch button at bottom
    addSeparatorLine();
    
    auto* finishBtn = new QToolButton();
    finishBtn->setText(tr("Finish Sketch"));
    finishBtn->setToolButtonStyle(Qt::ToolButtonTextOnly);
    finishBtn->setMinimumWidth(120);
    finishBtn->setMinimumHeight(24);
    finishBtn->setStyleSheet(QStringLiteral(
        "QToolButton {"
        "  background-color: #0696D7;"
        "  color: white;"
        "  border: none;"
        "  border-radius: 4px;"
        "  padding: 4px 12px;"
        "  font-weight: bold;"
        "  font-size: 11px;"
        "}"
        "QToolButton:hover {"
        "  background-color: #07A8F0;"
        "}"
        "QToolButton:pressed {"
        "  background-color: #0580B5;"
        "}"
    ));
    connect(finishBtn, &QToolButton::clicked, this, [this]() {
        auto& mgr = Application::Instance->commandManager();
        Command* cmd = mgr.getCommandByName("Sketcher_LeaveSketch");
        if (cmd) {
            cmd->invoke(0);
        }
        hidePalette();
    });
    m_mainLayout->addWidget(finishBtn);
    
    // Set fixed width, height will adjust
    setFixedWidth(150);
    adjustSize();
}

void FusionSketchPalette::setupStyle()
{
    setStyleSheet(QStringLiteral(
        "FusionSketchPalette {"
        "  background-color: rgba(45, 45, 45, 240);"
        "  border: 1px solid #444444;"
        "  border-radius: 8px;"
        "}"
    ));
}

void FusionSketchPalette::addSection(const QString& title, const QStringList& commands)
{
    // Section header
    auto* header = new QLabel(title);
    header->setStyleSheet(QStringLiteral(
        "QLabel { color: #888888; font-size: 9px; font-weight: bold; padding-left: 2px; }"
    ));
    m_mainLayout->addWidget(header);
    
    // Tool buttons in a flow layout (2 columns)
    auto* toolsWidget = new QWidget();
    auto* gridLayout = new QGridLayout(toolsWidget);
    gridLayout->setContentsMargins(0, 2, 0, 2);
    gridLayout->setSpacing(2);
    
    int col = 0;
    int row = 0;
    for (const QString& cmdName : commands) {
        QToolButton* btn = createToolButton(cmdName);
        if (btn) {
            gridLayout->addWidget(btn, row, col);
            col++;
            if (col >= 3) {  // 3 columns
                col = 0;
                row++;
            }
        }
    }
    
    m_mainLayout->addWidget(toolsWidget);
}

QToolButton* FusionSketchPalette::createToolButton(const QString& cmdName)
{
    auto& mgr = Application::Instance->commandManager();
    Command* cmd = mgr.getCommandByName(cmdName.toLatin1().constData());
    if (!cmd) {
        return nullptr;
    }
    
    auto* btn = new QToolButton();
    btn->setToolButtonStyle(Qt::ToolButtonIconOnly);
    btn->setAutoRaise(true);
    btn->setIconSize(QSize(20, 20));
    btn->setFixedSize(36, 28);
    
    // Get icon
    const char* pixmap = cmd->getPixmap();
    if (pixmap && pixmap[0]) {
        QIcon icon = BitmapFactory().iconFromTheme(pixmap);
        if (!icon.isNull()) {
            btn->setIcon(icon);
        } else {
            QPixmap pm = BitmapFactory().pixmap(pixmap);
            if (!pm.isNull()) {
                btn->setIcon(QIcon(pm));
            }
        }
    }
    
    // Tooltip without '&' accelerator markers
    QString tooltip = QString::fromUtf8(cmd->getMenuText());
    tooltip.remove(QLatin1Char('&'));
    QString accel = QString::fromUtf8(cmd->getAccel());
    if (!accel.isEmpty()) {
        tooltip += QStringLiteral(" (") + accel + QStringLiteral(")");
    }
    btn->setToolTip(tooltip);
    
    btn->setStyleSheet(QStringLiteral(
        "QToolButton {"
        "  background: transparent;"
        "  border: none;"
        "  border-radius: 4px;"
        "}"
        "QToolButton:hover {"
        "  background: rgba(255,255,255,0.1);"
        "}"
        "QToolButton:pressed {"
        "  background: #0696D7;"
        "}"
    ));
    
    // Connect to command
    connect(btn, &QToolButton::clicked, this, [cmdName]() {
        auto& cmdMgr = Application::Instance->commandManager();
        Command* c = cmdMgr.getCommandByName(cmdName.toLatin1().constData());
        if (c) {
            c->invoke(0);
        }
    });
    
    return btn;
}

void FusionSketchPalette::addSeparatorLine()
{
    auto* line = new QFrame();
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Plain);
    line->setStyleSheet(QStringLiteral("background-color: #444444;"));
    line->setFixedHeight(1);
    m_mainLayout->addWidget(line);
}

void FusionSketchPalette::showPalette(const QPoint& position)
{
    if (m_visible) {
        return;
    }
    
    QPoint pos = position;
    if (pos.isNull()) {
        // Default position: right side of primary screen
        QScreen* screen = QApplication::primaryScreen();
        if (screen) {
            QRect screenGeom = screen->availableGeometry();
            pos = QPoint(screenGeom.right() - width() - 20, 
                        screenGeom.top() + 100);
        }
    }
    
    // Ensure palette stays on screen
    QScreen* screen = QApplication::screenAt(pos);
    if (screen) {
        QRect screenGeom = screen->availableGeometry();
        if (pos.x() + width() > screenGeom.right()) {
            pos.setX(screenGeom.right() - width() - 10);
        }
        if (pos.y() + height() > screenGeom.bottom()) {
            pos.setY(screenGeom.bottom() - height() - 10);
        }
        if (pos.x() < screenGeom.left()) {
            pos.setX(screenGeom.left() + 10);
        }
        if (pos.y() < screenGeom.top()) {
            pos.setY(screenGeom.top() + 10);
        }
    }
    
    move(pos);
    
    // Fade in animation
    setWindowOpacity(0.0);
    show();
    m_fadeAnimation->setStartValue(0.0);
    m_fadeAnimation->setEndValue(1.0);
    m_fadeAnimation->start();
    
    m_visible = true;
}

void FusionSketchPalette::hidePalette()
{
    if (!m_visible) {
        return;
    }
    
    // Fade out animation
    m_fadeAnimation->setStartValue(1.0);
    m_fadeAnimation->setEndValue(0.0);
    connect(m_fadeAnimation, &QPropertyAnimation::finished, this, [this]() {
        hide();
        Q_EMIT paletteHidden();
    }, Qt::SingleShotConnection);
    m_fadeAnimation->start();
    
    m_visible = false;
}

void FusionSketchPalette::setPinned(bool pinned)
{
    m_pinned = pinned;
    m_pinButton->setChecked(pinned);
    Q_EMIT palettePinned(pinned);
}

void FusionSketchPalette::updatePosition(const QPoint& cursorPos)
{
    if (m_pinned || m_dragging) {
        return;
    }
    
    // Keep palette near cursor but offset to the right
    QPoint newPos = cursorPos + QPoint(50, -height() / 2);
    
    // Keep on screen
    QScreen* screen = QApplication::screenAt(cursorPos);
    if (screen) {
        QRect screenGeom = screen->availableGeometry();
        if (newPos.x() + width() > screenGeom.right()) {
            // Put on left side of cursor instead
            newPos.setX(cursorPos.x() - width() - 50);
        }
        newPos.setY(qBound(screenGeom.top() + 10, 
                          newPos.y(), 
                          screenGeom.bottom() - height() - 10));
    }
    
    move(newPos);
}

void FusionSketchPalette::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // Draw rounded rectangle background
    QPainterPath path;
    path.addRoundedRect(rect().adjusted(1, 1, -1, -1), 8, 8);
    
    painter.fillPath(path, QColor(45, 45, 45, 240));
    painter.setPen(QPen(QColor(68, 68, 68), 1));
    painter.drawPath(path);
}

void FusionSketchPalette::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton && m_titleBar->geometry().contains(event->pos())) {
        m_dragging = true;
        m_dragOffset = event->pos();
        setCursor(Qt::ClosedHandCursor);
    }
    QWidget::mousePressEvent(event);
}

void FusionSketchPalette::mouseMoveEvent(QMouseEvent* event)
{
    if (m_dragging) {
        move(event->globalPosition().toPoint() - m_dragOffset);
        // Auto-pin when dragged
        if (!m_pinned) {
            setPinned(true);
        }
    }
    QWidget::mouseMoveEvent(event);
}

void FusionSketchPalette::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        m_dragging = false;
        setCursor(Qt::ArrowCursor);
    }
    QWidget::mouseReleaseEvent(event);
}

void FusionSketchPalette::enterEvent(QEnterEvent* event)
{
    Q_UNUSED(event)
    // Could add hover effects here
}

void FusionSketchPalette::leaveEvent(QEvent* event)
{
    Q_UNUSED(event)
    // Could add hover effects here
}

void FusionSketchPalette::updateDOF(const QString& state, const QString& msg, 
                                     const QString& link, const QString& linkMsg)
{
    Q_UNUSED(link)
    
    if (!m_dofLabel) {
        return;
    }
    
    QString displayText = msg;
    if (!linkMsg.isEmpty()) {
        displayText += linkMsg;
    }
    
    // Remove trailing spaces and colons
    displayText = displayText.trimmed();
    if (displayText.endsWith(QLatin1Char(':'))) {
        displayText.chop(1);
    }
    
    m_dofLabel->setText(displayText);
    
    // Color based on state
    QString bgColor;
    if (state == QStringLiteral("fully_constrained")) {
        bgColor = QStringLiteral("#28A745");  // Green
    }
    else if (state == QStringLiteral("under_constrained")) {
        bgColor = QStringLiteral("#FFC107");  // Yellow/amber
    }
    else if (state == QStringLiteral("conflicting_constraints") 
             || state == QStringLiteral("redundant_constraints")
             || state == QStringLiteral("malformed_constraints")) {
        bgColor = QStringLiteral("#DC3545");  // Red
    }
    else if (state == QStringLiteral("partially_redundant_constraints")) {
        bgColor = QStringLiteral("#FF8C00");  // Orange
    }
    else if (state == QStringLiteral("solver_failed")) {
        bgColor = QStringLiteral("#DC3545");  // Red
    }
    else if (state == QStringLiteral("empty")) {
        bgColor = QStringLiteral("#6C757D");  // Gray
    }
    else {
        bgColor = QStringLiteral("#6C757D");  // Default gray
    }
    
    m_dofLabel->setStyleSheet(QStringLiteral(
        "QLabel {"
        "  color: white;"
        "  font-size: 10px;"
        "  font-weight: bold;"
        "  background-color: %1;"
        "  border-radius: 4px;"
        "  padding: 4px 8px;"
        "}"
    ).arg(bgColor));
}

#include "moc_FusionSketchPalette.cpp"
