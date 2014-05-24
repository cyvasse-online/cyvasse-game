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

#ifndef _HEXAGON_BOARD_HPP_
#define _HEXAGON_BOARD_HPP_

#include <algorithm>
#include <unordered_map>
#include <vector>
#include <cassert>
#include <cstdlib>
#include <fea/rendering/renderer2d.hpp>
#include <fea/rendering/quad.hpp>
#include "hexagon.hpp"

template<int l>
class HexagonBoard
{
	public:
		typedef typename cyvmath::hexagon<l> Hexagon;
		typedef typename Hexagon::Coordinate Coordinate;
		typedef typename std::unordered_map<Coordinate, fea::Quad*> TileMap;
		typedef std::vector<fea::Quad*> TileVec;

	private:
		// non-copyable
		HexagonBoard(const HexagonBoard&) = delete;
		const HexagonBoard& operator= (const HexagonBoard&) = delete;

		fea::Renderer2D& _renderer;

		glm::uvec2 _size;
		glm::uvec2 _position;
		glm::vec2 _tileSize;

		TileMap _tileMap;
		TileVec _tileVec;

	public:
		glm::vec2 getTilePosition(Coordinate c)
		{
			// This should *maybe* be rewritten...
			return {_position.x + _tileSize.x * c.x() + (_tileSize.x / 2) * (c.y() - Hexagon::edgeLength + 1),
			        _position.y - _tileSize.y + (_size.y - (_tileSize.y * c.y()))};
		}

		std::unique_ptr<Coordinate> getCoordinate(glm::ivec2 tilePosition)
		{
			// This is some crap I got from my CAS because that math part is a bit of getting over my head
			// It is very close to be fully functional, but probably no one can understand it.
			// TODO: find someone to tame this beast of a mathematical formula and rewrite this!
			int8_t y = (_size.y - (tilePosition.y - _position.y)) / _tileSize.y;
			std::unique_ptr<Coordinate> c = Coordinate::create(
					(2 * tilePosition.x + Hexagon::edgeLength * _tileSize.x - y * _tileSize.x - 2 * _position.x - _tileSize.x)
					/ (2 * _tileSize.x), y
				);

			return c;
		}

		fea::Quad* getTileAt(Coordinate c)
		{
			typename TileMap::iterator it = _tileMap.find(c);
			if(it == _tileMap.end())
				return nullptr;

			return it->second;
		}

		static fea::Color getTileColor(Coordinate c, bool setup)
		{
			static fea::Color tileColors[3] = {
					{0.8f, 0.8f, 0.8f},
					{0.7f, 0.7f, 0.7f},
					{0.6f, 0.6f, 0.6f}
				};
			static fea::Color tileColorsDark[3] = {
					{0.5f, 0.5f, 0.5f},
					{0.4f, 0.4f, 0.4f},
					{0.3f, 0.3f, 0.3f}
				};

			int8_t index = (((c.x() - c.y()) % 3) + 3) % 3;

			if(setup && c.y() >= (l - 1))
				return tileColorsDark[index];
			else
				return tileColors[index];
		}

		HexagonBoard(fea::Renderer2D& renderer)
			: _renderer(renderer)
		{
			const glm::uvec2 windowSize = renderer.getViewport().getSize();

			unsigned padding = std::min(windowSize.x, windowSize.y) / 20;

			_size = {windowSize.x - padding * 2, windowSize.y - padding * 2};

			if(_size.x / _size.y < 4.0f / 3.0f) // wider than 4:3
			{
				_tileSize.y = _size.y / static_cast<float>(l * 2 - 1);
				_tileSize.x = _tileSize.y / 3 * 4;

				_size.x = _tileSize.x * (l * 2 - 1);

				_position = {(windowSize.x - _size.x) / 2, padding};
			}
			else
			{
				_tileSize.x = _size.x / static_cast<float>(l * 2 - 1);
				_tileSize.y = _tileSize.x / 4 * 3;

				_size.y = _tileSize.x * (l * 2 - 1);

				_position = {padding, (windowSize.y - _size.y) / 2};
			}

			for(Coordinate c : Hexagon::getAllCoordinates())
			{
				fea::Quad* quad = new fea::Quad(_tileSize);

				quad->setPosition(getTilePosition(c));
				quad->setColor(getTileColor(c, true));

				// add the tile to the map and vector
				_tileVec.push_back(quad);
				std::pair<typename TileMap::iterator, bool> res = _tileMap.insert({c, quad});

				assert(res.second); // assert the insertion was successful
			}
		}

		~HexagonBoard()
		{
			for(fea::Quad* it : _tileVec)
				delete it;
		}

		const glm::vec2& getTileSize() const
		{
			return _tileSize;
		}

		void updateTileColors(int8_t fromRow, int8_t toRow, bool setup = false)
		{
			assert(fromRow <= toRow);
			for(auto it : _tileMap)
			{
				if(it.first.y() >= fromRow && it.first.y() <= toRow)
					it.second->setColor(getTileColor(it.first, setup));
			}
		}

		void tick()
		{
			for(const fea::Quad* it : _tileVec)
				_renderer.queue(*it);
		}
};

#endif // _HEXAGON_BOARD_HPP_
