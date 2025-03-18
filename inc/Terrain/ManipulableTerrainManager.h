#pragma once
#include "Terrain/TerrainManager.h"
#include "Terrain/ManipulableTerrainElement.h"
#include <unordered_map>

namespace Terrain {
	class ManipulableTerrainManager : public TemplateTerrainManager<ManipulableTerrainManager, ManipulableTerrainElement> {
	public:
		ManipulableTerrainManager(std::string name, terrain_settings terrainSettings, Noise::noise_settings noiseSettings);
		ManipulableTerrainManager(std::string name, const FileAdapter& settings);

		void manipulateTerrain(ManipulableTerrainElement::ManipulateDir dir, ManipulableTerrainElement::ManipulateForm form, ManipulableTerrainElement::ManipulateType type, float strength, float radius, Vector3 position);
		void clearDifference();

		void save(FileAdapter& file) const override;
		bool load(const FileAdapter& file) override;

	private:
		std::unordered_map<PositionIdentifier, float*> m_manipulations;
		
		void initialiseAndAddNewElement(std::unordered_set<ManipulableTerrainElement>& newElements, const PositionIdentifier& posId) override;

		// Settings loading and saving
		bool loadManipulations(const FileAdapter& settings);
		void saveManipulations(FileAdapter& file) const;

		// Helper functions
		void reloadElement(ManipulableTerrainElement* element) override;
		std::string getKeyFromPositionIdentifier(PositionIdentifier posId) const;
		PositionIdentifier getPositionIdentifierFromKey(std::string key) const;
	};
}