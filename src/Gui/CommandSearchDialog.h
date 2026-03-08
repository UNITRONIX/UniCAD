// SPDX-License-Identifier: LGPL-2.1-or-later

/****************************************************************************
 *   Copyright (c) 2025 UniCAD Project                                  *
 *                                                                          *
 *   This file is part of UniCAD.                                        *
 *                                                                          *
 *   UniCAD is free software: you can redistribute it and/or modify it   *
 *   under the terms of the GNU Lesser General Public License as            *
 *   published by the Free Software Foundation, either version 2.1 of the   *
 *   License, or (at your option) any later version.                        *
 *                                                                          *
 *   UniCAD is distributed in the hope that it will be useful, but       *
 *   WITHOUT ANY WARRANTY; without even the implied warranty of             *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU       *
 *   Lesser General Public License for more details.                        *
 *                                                                          *
 *   You should have received a copy of the GNU Lesser General Public       *
 *   License along with UniCAD. If not, see                              *
 *   <https://www.gnu.org/licenses/>.                                       *
 *                                                                          *
 ****************************************************************************/

#ifndef GUI_COMMAND_SEARCH_DIALOG_H
#define GUI_COMMAND_SEARCH_DIALOG_H

#include <FCGlobal.h>
#include <QDialog>

class QLineEdit;

namespace Gui
{

class CommandCompleter;

/**
 * A Fusion 360-style command search dialog.
 *
 * Provides a floating search bar (Ctrl+/) that allows the user to quickly
 * find and execute any command by name, keyword, or shortcut.
 * Uses the existing CommandCompleter infrastructure.
 */
class GuiExport CommandSearchDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CommandSearchDialog(QWidget* parent = nullptr);

    /// Show the dialog centered at the top of the main window
    void showCentered();

protected:
    bool eventFilter(QObject* obj, QEvent* event) override;

private Q_SLOTS:
    void onCommandActivated(const QByteArray& name);

private:
    QLineEdit* m_searchEdit;
    CommandCompleter* m_completer;
};

}  // namespace Gui

#endif  // GUI_COMMAND_SEARCH_DIALOG_H
