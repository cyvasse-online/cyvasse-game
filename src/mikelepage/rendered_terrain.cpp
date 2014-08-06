/* Copyright 2014 Jonas Platte
 *
 * This file is part of Cyvasse Online.
 *
 * Cyvasse Online is free software: you can redistribute it and/or modify it under the
 * terms of the GNU Affero General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 *
 * Cyvasse Online is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
 * PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License along with this program.
 * If not, see <http://www.gnu.org/licenses/>.
 */

#include "rendered_terrain.hpp"

#include <map>
#include "hexagon_board.hpp"

namespace mikelepage
{
	RenderedTerrain::RenderedTerrain(TerrainType type, Coordinate coord, HexagonBoard<6>& board, TerrainMap& terrainMap)
		: Terrain(type, coord)
		, m_board(board)
		, m_terrainMap(terrainMap)
		, m_quad(board.getTileSize())
	{
		static const std::map<TerrainType, fea::Color> colors {
			{TerrainType::HILL, {255, 191, 0, 127}},
			{TerrainType::FOREST, {0, 95, 0, 127}},
			{TerrainType::GRASSLAND, {0, 255, 0, 127}}
		};

		assert(type != TerrainType::UNDEFINED);

		m_quad.setColor(colors.at(type));
		m_quad.setPosition(board.getTilePosition(coord));
	}

	void RenderedTerrain::setCoord(Coordinate coord)
	{
		auto it = m_terrainMap.find(m_coord);
		assert(it != m_terrainMap.end());

		auto selfSharedPtr = it->second;

		m_coord = coord;
		m_quad.setPosition(m_board.getTilePosition(coord));

		m_terrainMap.erase(it);
		m_terrainMap.emplace(coord, selfSharedPtr);
	}
}
