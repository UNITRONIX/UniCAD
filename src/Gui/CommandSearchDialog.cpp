// SPDX-License-Identifier: LGPL-2.1-or-later

/****************************************************************************
 *   Copyright (c) 2025 FusionCAD Project                                  *
 *                                                                          *
 *   This file is part of FusionCAD.                                        *
 *                                                                          *
 *   FusionCAD is free software: you can redistribute it and/or modify it   *
 *   under the terms of the GNU Lesser General Public License as            *
 *   published by the Free Software Foundation, either version 2.1 of the   *
 *   License, or (at your option) any later version.                        *
 *                                                                          *
 *   FusionCAD is distributed in the hope that it will be useful, but       *
 *   WITHOUT ANY WARRANTY; without even the implied warranty of             *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU       *
 *   Lesser General Public License for more details.                        *
 *                                                                          *
 *   You should have received a copy of the GNU Lesser General Public       *
 *   License along with FusionCAD. If not, see                              *
 *   <https://www.gnu.org/licenses/>.                                       *
 *                                                                          *
 ****************************************************************************/

#include <QKeyEvent>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QWidget>
#include <QApplication>
#include <QScreen>

#include "Application.h"
#include "Command.h"
#include "CommandCompleter.h"
#include "CommandSearchDialog.h"
#include "MainWindow.h"

using namespace Gui;

CommandSearchDialog::CommandSearchDialog(QWidget* parent)
    : QDialog(parent, Qt::Popup | Qt::FramelessWindowHint)
{
    setAttribute(Qt::WA_DeleteOnClose, false);
    setAttribute(Qt::WA_TranslucentBackground, false);

    // Apply dark styling consistent with FusionCAD dark theme
    setStyleSheet(
        QStringLiteral(
            "CommandSearchDialog {"
            "  background-color: #333333;"
            "  border: 2px solid #0696D7;"
            "  border-radius: 8px;"
            "}"
            "QLineEdit {"
            "  background-color: #404040;"
            "  color: #E0E0E0;"
            "  border: 1px solid #555555;"
            "  border-radius: 4px;"
            "  padding: 8px 12px;"
            "  font-size: 14px;"
            "  selection-background-color: #0696D7;"
            "}"
            "QLineEdit:focus {"
            "  border: 1px solid #0696D7;"
            "}"
        )
    );

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(8, 8, 8, 8);

    m_searchEdit = new QLineEdit(this);
    m_searchEdit->setPlaceholderText(tr("Search commands... (type at least 3 characters)"));
    m_searchEdit->setMinimumWidth(450);
    m_searchEdit->setClearButtonEnabled(true);
    layout->addWidget(m_searchEdit);

    // Use the existing CommandCompleter infrastructure
    m_completer = new CommandCompleter(m_searchEdit, this);
    connect(m_completer, &CommandCompleter::commandActivated,
            this, &CommandSearchDialog::onCommandActivated);

    m_searchEdit->installEventFilter(this);

    setLayout(layout);
    setFixedWidth(500);
}

void CommandSearchDialog::showCentered()
{
    // Clear any previous search text
    m_searchEdit->clear();

    // Position at the top-center of the main window
    QWidget* mainWin = getMainWindow();
    if (mainWin) {
        QPoint topCenter = mainWin->mapToGlobal(
            QPoint(mainWin->width() / 2 - width() / 2, 80)
        );
        move(topCenter);
    }

    show();
    raise();
    activateWindow();
    m_searchEdit->setFocus();
}

bool CommandSearchDialog::eventFilter(QObject* obj, QEvent* event)
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

void CommandSearchDialog::onCommandActivated(const QByteArray& name)
{
    // Close the dialog first
    close();

    // Execute the selected command
    auto& manager = Application::Instance->commandManager();
    Command* cmd = manager.getCommandByName(name.constData());
    if (cmd) {
        cmd->invoke(0);
    }
}

#include "moc_CommandSearchDialog.cpp"
