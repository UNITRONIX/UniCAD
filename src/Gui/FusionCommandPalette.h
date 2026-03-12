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

#ifndef GUI_FUSION_COMMAND_PALETTE_H
#define GUI_FUSION_COMMAND_PALETTE_H

#include <FCGlobal.h>
#include <QDialog>
#include <QVector>
#include <QStringList>

class QLineEdit;
class QGridLayout;
class QVBoxLayout;
class QLabel;
class QToolButton;
class QScrollArea;
class QWidget;

namespace Gui
{

class Command;

/**
 * Fusion 360-style command palette triggered by 'S' key.
 *
 * Features:
 * - Appears near the cursor position
 * - Search field for filtering commands
 * - Grid of pinned (favorite) commands
 * - Recent commands section
 * - Context-aware filtering based on active workbench
 */
class GuiExport FusionCommandPalette : public QDialog
{
    Q_OBJECT

public:
    explicit FusionCommandPalette(QWidget* parent = nullptr);

    /// Show the palette near the cursor position
    void showAtCursor();

protected:
    bool eventFilter(QObject* obj, QEvent* event) override;

private Q_SLOTS:
    void onSearchTextChanged(const QString& text);
    void onCommandClicked();
    void onPinCommand();
    void onUnpinCommand();

private:
    /// Initialize the UI layout
    void setupUI();
    
    /// Load pinned and recent commands from settings
    void loadSettings();
    
    /// Save pinned and recent commands to settings
    void saveSettings();
    
    /// Refresh the command grid based on current state
    void refreshGrid();
    
    /// Create a command button with icon and label
    QToolButton* createCommandButton(Command* cmd, bool pinned = false);
    
    /// Add command to recent list
    void addToRecent(const QString& cmdName);
    
    /// Get all available commands, optionally filtered by workbench
    QVector<Command*> getAvailableCommands(const QString& filter = QString());
    
    /// Get commands for the current workbench context
    QVector<Command*> getContextualCommands();
    
    /// Execute a command by name
    void executeCommand(const QString& cmdName);

    QLineEdit* m_searchEdit;
    QVBoxLayout* m_mainLayout;
    
    // Pinned commands section
    QWidget* m_pinnedSection;
    QLabel* m_pinnedLabel;
    QGridLayout* m_pinnedGrid;
    
    // Recent commands section
    QWidget* m_recentSection;
    QLabel* m_recentLabel;
    QGridLayout* m_recentGrid;
    
    // Search results section
    QScrollArea* m_resultsScroll;
    QWidget* m_resultsWidget;
    QGridLayout* m_resultsGrid;
    
    // Data
    QStringList m_pinnedCommands;
    QStringList m_recentCommands;
    
    static constexpr int MAX_RECENT = 12;
    static constexpr int GRID_COLUMNS = 6;
    static constexpr int ICON_SIZE = 32;
};

}  // namespace Gui

#endif  // GUI_FUSION_COMMAND_PALETTE_H
