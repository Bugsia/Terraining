#include "Terrain/TerrainManager.h"

namespace Terrain {
	TerrainManager::TerrainManager(std::string name, terrain_settings terrainSettings, Noise::noise_settings noiseSettings) : TemplateTerrainManager<TerrainElement>(name, terrainSettings, noiseSettings) {
	}

	TerrainManager::TerrainManager(std::string name, const FileAdapter& settings) : TemplateTerrainManager<TerrainElement>(name, settings) {
	}
}