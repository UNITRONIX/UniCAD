// SPDX-License-Identifier: LGPL-2.1-or-later

/****************************************************************************
 *   Copyright (c) 2025 UniCAD Project                                      *
 *                                                                          *
 *   This file is part of UniCAD.                                           *
 *                                                                          *
 *   UniCAD is free software: you can redistribute it and/or modify it      *
 *   under the terms of the GNU Lesser General Public License as            *
 *   published by the Free Software Foundation, either version 2.1 of the   *
 *   License, or (at your option) any later version.                        *
 *                                                                          *
 *   UniCAD is distributed in the hope that it will be useful, but          *
 *   WITHOUT ANY WARRANTY; without even the implied warranty of             *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU       *
 *   Lesser General Public License for more details.                        *
 *                                                                          *
 *   You should have received a copy of the GNU Lesser General Public       *
 *   License along with UniCAD. If not, see                                 *
 *   <https://www.gnu.org/licenses/>.                                       *
 *                                                                          *
 ****************************************************************************/

#include <QApplication>
#include <QCursor>
#include <QGridLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QScreen>
#include <QScrollArea>
#include <QToolButton>
#include <QVBoxLayout>

#include <App/Application.h>
#include "Application.h"
#include "BitmapFactory.h"
#include "Command.h"
#include "FusionCommandPalette.h"
#include "MainWindow.h"
#include "Action.h"
#include "Workbench.h"
#include "WorkbenchManager.h"

using namespace Gui;

FusionCommandPalette::FusionCommandPalette(QWidget* parent)
    : QDialog(parent, Qt::Popup | Qt::FramelessWindowHint)
{
    setAttribute(Qt::WA_DeleteOnClose, false);
    setAttribute(Qt::WA_TranslucentBackground, false);

    setupUI();
    loadSettings();
}

void FusionCommandPalette::setupUI()
{
    // Dark theme styling
    setStyleSheet(QStringLiteral(
        "FusionCommandPalette {"
        "  background-color: #2D2D2D;"
        "  border: 2px solid #0696D7;"
        "  border-radius: 10px;"
        "}"
        "QLineEdit {"
        "  background-color: #404040;"
        "  color: #E0E0E0;"
        "  border: 1px solid #555555;"
        "  border-radius: 6px;"
        "  padding: 10px 14px;"
        "  font-size: 14px;"
        "  selection-background-color: #0696D7;"
        "}"
        "QLineEdit:focus {"
        "  border: 2px solid #0696D7;"
        "}"
        "QLabel {"
        "  color: #888888;"
        "  font-size: 11px;"
        "  font-weight: bold;"
        "  padding: 4px 0px;"
        "}"
        "QToolButton {"
        "  background-color: transparent;"
        "  border: 1px solid transparent;"
        "  border-radius: 6px;"
        "  padding: 6px;"
        "  color: #E0E0E0;"
        "  font-size: 10px;"
        "}"
        "QToolButton:hover {"
        "  background-color: #3A3A3A;"
        "  border: 1px solid #555555;"
        "}"
        "QToolButton:pressed {"
        "  background-color: #0696D7;"
        "}"
        "QScrollArea {"
        "  background-color: transparent;"
        "  border: none;"
        "}"
        "QScrollBar:vertical {"
        "  background: #2D2D2D;"
        "  width: 8px;"
        "  border-radius: 4px;"
        "}"
        "QScrollBar::handle:vertical {"
        "  background: #555555;"
        "  border-radius: 4px;"
        "  min-height: 20px;"
        "}"
        "QScrollBar::handle:vertical:hover {"
        "  background: #666666;"
        "}"
    ));

    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(12, 12, 12, 12);
    m_mainLayout->setSpacing(10);

    // Search field
    m_searchEdit = new QLineEdit(this);
    m_searchEdit->setPlaceholderText(tr("Search commands..."));
    m_searchEdit->setClearButtonEnabled(true);
    m_searchEdit->installEventFilter(this);
    connect(m_searchEdit, &QLineEdit::textChanged,
            this, &FusionCommandPalette::onSearchTextChanged);
    m_mainLayout->addWidget(m_searchEdit);

    // Pinned commands section
    m_pinnedSection = new QWidget(this);
    auto* pinnedLayout = new QVBoxLayout(m_pinnedSection);
    pinnedLayout->setContentsMargins(0, 0, 0, 0);
    pinnedLayout->setSpacing(6);
    
    m_pinnedLabel = new QLabel(tr("PINNED"), m_pinnedSection);
    pinnedLayout->addWidget(m_pinnedLabel);
    
    auto* pinnedContainer = new QWidget(m_pinnedSection);
    m_pinnedGrid = new QGridLayout(pinnedContainer);
    m_pinnedGrid->setContentsMargins(0, 0, 0, 0);
    m_pinnedGrid->setSpacing(4);
    pinnedLayout->addWidget(pinnedContainer);
    
    m_mainLayout->addWidget(m_pinnedSection);

    // Recent commands section
    m_recentSection = new QWidget(this);
    auto* recentLayout = new QVBoxLayout(m_recentSection);
    recentLayout->setContentsMargins(0, 0, 0, 0);
    recentLayout->setSpacing(6);
    
    m_recentLabel = new QLabel(tr("RECENT"), m_recentSection);
    recentLayout->addWidget(m_recentLabel);
    
    auto* recentContainer = new QWidget(m_recentSection);
    m_recentGrid = new QGridLayout(recentContainer);
    m_recentGrid->setContentsMargins(0, 0, 0, 0);
    m_recentGrid->setSpacing(4);
    recentLayout->addWidget(recentContainer);
    
    m_mainLayout->addWidget(m_recentSection);

    // Search results section (scrollable)
    m_resultsScroll = new QScrollArea(this);
    m_resultsScroll->setWidgetResizable(true);
    m_resultsScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_resultsScroll->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_resultsScroll->setMinimumHeight(150);
    m_resultsScroll->setMaximumHeight(300);
    
    m_resultsWidget = new QWidget();
    m_resultsWidget->setStyleSheet(QStringLiteral("background-color: transparent;"));
    m_resultsGrid = new QGridLayout(m_resultsWidget);
    m_resultsGrid->setContentsMargins(0, 0, 0, 0);
    m_resultsGrid->setSpacing(4);
    m_resultsGrid->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    
    m_resultsScroll->setWidget(m_resultsWidget);
    m_resultsScroll->hide();  // Hidden until search is active
    m_mainLayout->addWidget(m_resultsScroll);

    setFixedWidth(420);
    setLayout(m_mainLayout);
}

void FusionCommandPalette::loadSettings()
{
    auto hGrp = App::GetApplication().GetParameterGroupByPath(
        "User parameter:BaseApp/Preferences/FusionUI/CommandPalette");
    
    // Load pinned commands
    std::string pinned = hGrp->GetASCII("PinnedCommands", "");
    if (!pinned.empty()) {
        m_pinnedCommands = QString::fromStdString(pinned).split(QStringLiteral(";"), 
            Qt::SkipEmptyParts);
    }
    
    // Load recent commands
    std::string recent = hGrp->GetASCII("RecentCommands", "");
    if (!recent.empty()) {
        m_recentCommands = QString::fromStdString(recent).split(QStringLiteral(";"),
            Qt::SkipEmptyParts);
    }
}

void FusionCommandPalette::saveSettings()
{
    auto hGrp = App::GetApplication().GetParameterGroupByPath(
        "User parameter:BaseApp/Preferences/FusionUI/CommandPalette");
    
    hGrp->SetASCII("PinnedCommands", m_pinnedCommands.join(QStringLiteral(";")).toStdString());
    hGrp->SetASCII("RecentCommands", m_recentCommands.join(QStringLiteral(";")).toStdString());
}

void FusionCommandPalette::showAtCursor()
{
    m_searchEdit->clear();
    refreshGrid();
    
    // Position near cursor
    QPoint cursorPos = QCursor::pos();
    QScreen* screen = QGuiApplication::screenAt(cursorPos);
    if (!screen) {
        screen = QGuiApplication::primaryScreen();
    }
    
    QRect screenGeom = screen->availableGeometry();
    
    // Adjust position to ensure dialog stays on screen
    int x = cursorPos.x() - width() / 2;
    int y = cursorPos.y() + 20;  // Slightly below cursor
    
    if (x < screenGeom.left()) {
        x = screenGeom.left() + 10;
    }
    if (x + width() > screenGeom.right()) {
        x = screenGeom.right() - width() - 10;
    }
    if (y + height() > screenGeom.bottom()) {
        y = cursorPos.y() - height() - 10;  // Above cursor if no room below
    }
    
    move(x, y);
    show();
    raise();
    activateWindow();
    m_searchEdit->setFocus();
}

void FusionCommandPalette::refreshGrid()
{
    auto& manager = Application::Instance->commandManager();
    
    // Clear existing buttons from pinned grid
    while (QLayoutItem* item = m_pinnedGrid->takeAt(0)) {
        if (item->widget()) {
            item->widget()->deleteLater();
        }
        delete item;
    }
    
    // Clear existing buttons from recent grid
    while (QLayoutItem* item = m_recentGrid->takeAt(0)) {
        if (item->widget()) {
            item->widget()->deleteLater();
        }
        delete item;
    }
    
    // Populate pinned commands
    int row = 0, col = 0;
    for (const QString& cmdName : m_pinnedCommands) {
        Command* cmd = manager.getCommandByName(cmdName.toUtf8().constData());
        if (cmd) {
            QToolButton* btn = createCommandButton(cmd, true);
            m_pinnedGrid->addWidget(btn, row, col);
            col++;
            if (col >= GRID_COLUMNS) {
                col = 0;
                row++;
            }
        }
    }
    m_pinnedSection->setVisible(!m_pinnedCommands.isEmpty());
    
    // Populate recent commands (exclude pinned ones)
    row = 0;
    col = 0;
    for (const QString& cmdName : m_recentCommands) {
        if (m_pinnedCommands.contains(cmdName)) {
            continue;  // Skip if already pinned
        }
        Command* cmd = manager.getCommandByName(cmdName.toUtf8().constData());
        if (cmd) {
            QToolButton* btn = createCommandButton(cmd, false);
            m_recentGrid->addWidget(btn, row, col);
            col++;
            if (col >= GRID_COLUMNS) {
                col = 0;
                row++;
            }
        }
    }
    
    bool hasRecentNonPinned = false;
    for (const QString& cmdName : m_recentCommands) {
        if (!m_pinnedCommands.contains(cmdName)) {
            hasRecentNonPinned = true;
            break;
        }
    }
    m_recentSection->setVisible(hasRecentNonPinned);
    
    // Hide results when not searching
    m_resultsScroll->hide();
    
    adjustSize();
}

QToolButton* FusionCommandPalette::createCommandButton(Command* cmd, bool pinned)
{
    auto* btn = new QToolButton(this);
    btn->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    btn->setIconSize(QSize(ICON_SIZE, ICON_SIZE));
    
    // Get icon
    if (cmd->getPixmap()) {
        QIcon icon = BitmapFactory().iconFromTheme(cmd->getPixmap());
        btn->setIcon(icon);
    }
    
    // Get short name (truncate if too long)
    QString text = Action::commandMenuText(cmd);
    if (text.length() > 10) {
        text = text.left(9) + QStringLiteral("…");
    }
    btn->setText(text);
    btn->setToolTip(Action::commandToolTip(cmd));
    
    // Store command name in property
    btn->setProperty("cmdName", QString::fromUtf8(cmd->getName()));
    btn->setProperty("isPinned", pinned);
    
    // Set fixed size for uniform grid
    btn->setFixedSize(64, 60);
    
    // Connect click
    connect(btn, &QToolButton::clicked, this, &FusionCommandPalette::onCommandClicked);
    
    // Context menu for pin/unpin
    btn->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(btn, &QWidget::customContextMenuRequested, this, [this, btn](const QPoint& pos) {
        QMenu menu;
        bool isPinned = btn->property("isPinned").toBool();
        if (isPinned) {
            QAction* unpinAction = menu.addAction(tr("Unpin"));
            connect(unpinAction, &QAction::triggered, this, [this, btn]() {
                QString cmdName = btn->property("cmdName").toString();
                m_pinnedCommands.removeAll(cmdName);
                saveSettings();
                refreshGrid();
            });
        } else {
            QAction* pinAction = menu.addAction(tr("Pin to favorites"));
            connect(pinAction, &QAction::triggered, this, [this, btn]() {
                QString cmdName = btn->property("cmdName").toString();
                if (!m_pinnedCommands.contains(cmdName)) {
                    m_pinnedCommands.prepend(cmdName);
                    saveSettings();
                    refreshGrid();
                }
            });
        }
        menu.exec(btn->mapToGlobal(pos));
    });
    
    return btn;
}

void FusionCommandPalette::onSearchTextChanged(const QString& text)
{
    // Clear results grid
    while (QLayoutItem* item = m_resultsGrid->takeAt(0)) {
        if (item->widget()) {
            item->widget()->deleteLater();
        }
        delete item;
    }
    
    if (text.length() < 2) {
        m_resultsScroll->hide();
        m_pinnedSection->setVisible(!m_pinnedCommands.isEmpty());
        
        bool hasRecentNonPinned = false;
        for (const QString& cmdName : m_recentCommands) {
            if (!m_pinnedCommands.contains(cmdName)) {
                hasRecentNonPinned = true;
                break;
            }
        }
        m_recentSection->setVisible(hasRecentNonPinned);
        adjustSize();
        return;
    }
    
    // Hide pinned/recent when searching
    m_pinnedSection->hide();
    m_recentSection->hide();
    m_resultsScroll->show();
    
    // Search commands
    QVector<Command*> results = getAvailableCommands(text);
    
    int row = 0, col = 0;
    int count = 0;
    const int MAX_RESULTS = 24;
    
    for (Command* cmd : results) {
        if (count >= MAX_RESULTS) break;
        
        QToolButton* btn = createCommandButton(cmd, m_pinnedCommands.contains(
            QString::fromUtf8(cmd->getName())));
        m_resultsGrid->addWidget(btn, row, col);
        col++;
        if (col >= GRID_COLUMNS) {
            col = 0;
            row++;
        }
        count++;
    }
    
    adjustSize();
}

QVector<Command*> FusionCommandPalette::getAvailableCommands(const QString& filter)
{
    QVector<Command*> results;
    auto& manager = Application::Instance->commandManager();
    
    QString filterLower = filter.toLower();
    
    for (auto& pair : manager.getCommands()) {
        Command* cmd = pair.second;
        
        // Get searchable text
        QString name = QString::fromUtf8(cmd->getName()).toLower();
        QString menuText = Action::commandMenuText(cmd).toLower();
        QString tooltip = Action::commandToolTip(cmd).toLower();
        
        // Check if filter matches
        if (name.contains(filterLower) || 
            menuText.contains(filterLower) || 
            tooltip.contains(filterLower)) {
            results.append(cmd);
        }
    }
    
    // Sort by relevance (exact matches first, then by name)
    std::sort(results.begin(), results.end(), [&filterLower](Command* a, Command* b) {
        QString nameA = QString::fromUtf8(a->getName()).toLower();
        QString nameB = QString::fromUtf8(b->getName()).toLower();
        QString menuA = Action::commandMenuText(a).toLower();
        QString menuB = Action::commandMenuText(b).toLower();
        
        // Exact prefix match in menu text gets priority
        bool aStartsWith = menuA.startsWith(filterLower);
        bool bStartsWith = menuB.startsWith(filterLower);
        if (aStartsWith != bStartsWith) {
            return aStartsWith;
        }
        
        // Then by menu text alphabetically
        return menuA < menuB;
    });
    
    return results;
}

QVector<Command*> FusionCommandPalette::getContextualCommands()
{
    // Get commands relevant to current workbench
    QVector<Command*> results;
    auto& manager = Application::Instance->commandManager();
    
    Workbench* wb = WorkbenchManager::instance()->active();
    if (!wb) {
        return results;
    }
    
    QString wbName = QString::fromUtf8(wb->name().c_str());
    
    for (auto& pair : manager.getCommands()) {
        Command* cmd = pair.second;
        QString group = QString::fromUtf8(cmd->getGroupName());
        
        // Include if command belongs to current workbench or is standard
        if (group.startsWith(wbName) || group == QStringLiteral("Standard") ||
            group == QStringLiteral("Edit") || group == QStringLiteral("View")) {
            results.append(cmd);
        }
    }
    
    return results;
}

void FusionCommandPalette::onCommandClicked()
{
    auto* btn = qobject_cast<QToolButton*>(sender());
    if (!btn) return;
    
    QString cmdName = btn->property("cmdName").toString();
    executeCommand(cmdName);
}

void FusionCommandPalette::executeCommand(const QString& cmdName)
{
    // Add to recent
    addToRecent(cmdName);
    saveSettings();
    
    // Close palette
    close();
    
    // Execute command
    auto& manager = Application::Instance->commandManager();
    Command* cmd = manager.getCommandByName(cmdName.toUtf8().constData());
    if (cmd) {
        cmd->invoke(0);
    }
}

void FusionCommandPalette::addToRecent(const QString& cmdName)
{
    // Remove if already exists (to move to front)
    m_recentCommands.removeAll(cmdName);
    
    // Add to front
    m_recentCommands.prepend(cmdName);
    
    // Trim to max size
    while (m_recentCommands.size() > MAX_RECENT) {
        m_recentCommands.removeLast();
    }
}

void FusionCommandPalette::onPinCommand()
{
    auto* action = qobject_cast<QAction*>(sender());
    if (!action) return;
    
    QString cmdName = action->data().toString();
    if (!m_pinnedCommands.contains(cmdName)) {
        m_pinnedCommands.prepend(cmdName);
        saveSettings();
        refreshGrid();
    }
}

void FusionCommandPalette::onUnpinCommand()
{
    auto* action = qobject_cast<QAction*>(sender());
    if (!action) return;
    
    QString cmdName = action->data().toString();
    m_pinnedCommands.removeAll(cmdName);
    saveSettings();
    refreshGrid();
}

bool FusionCommandPalette::eventFilter(QObject* obj, QEvent* event)
{
    if (obj == m_searchEdit && event->type() == QEvent::KeyPress) {
        auto* keyEvent = static_cast<QKeyEvent*>(event);
        if (keyEvent->key() == Qt::Key_Escape) {
            close();
            return true;
        }
    }
    return QDialog::eventFilter(obj, event);
}

#include "moc_FusionCommandPalette.cpp"
