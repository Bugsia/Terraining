#pragma once
#include <memory>
#include <vector>
#include <raylib.h>
#include "Gui.h"
#include "Terrain/TerrainManager.h"
#include "imgui.h"
#include "rlimGui.h"
#include "Noise.h"

namespace DebugGui {
	#define SAMPLE_IMAGE_WIDTH 200
	#define SAMPLE_IMAGE_HEIGHT 200

	class NoiseDebugGui : public Gui {
	public:
		NoiseDebugGui(std::string name, Terrain::TerrainManager& terrain, bool* open);

		bool render();

	private:
		Terrain::TerrainManager& m_terrain;
		std::vector<Color*> m_noiseLayers;
		Noise::noise_settings m_settings;
		Texture2D m_sampleImage;
		int m_selectedLayerIndex = 0;
		bool* m_openPointer;

		void NoiseLayersList();
		static void loadSampleImage(Texture2D& sampleImage, std::vector<Color*>& noiseLayers, int index);
		bool NoiseLayerSettings();
	};
}