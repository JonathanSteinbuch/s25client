// Copyright (c) 2005 - 2020 Settlers Freaks (sf-team at siedler25.org)
//
// This file is part of Return To The Roots.
//
// Return To The Roots is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Return To The Roots is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Return To The Roots. If not, see <http://www.gnu.org/licenses/>.

#pragma once

#include "EconomyModeHandler.h"
#include "IngameWindow.h"

class ctrlText;
class GameWorldViewer;

/// Window for displaying the economic mode progress
class iwEconomicProgress : public IngameWindow
{
public:
    iwEconomicProgress(const GameWorldViewer& gwv);
    ~iwEconomicProgress() override;

private:
    const GameWorldViewer& gwv;
    ctrlText* txtRemainingTime;

    /// Order in which the teams are displayed
    std::vector<const EconomyModeHandler::EconTeam*> teamOrder;

    void Draw_() override;

    void Msg_ButtonClick(unsigned ctrl_id) override;
    void Msg_PaintBefore() override;
};
