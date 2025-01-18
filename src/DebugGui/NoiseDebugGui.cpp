#include "DebugGui/NoiseDebugGui.h"

namespace DebugGui {
	NoiseDebugGui::NoiseDebugGui(std::string name, Terrain::TerrainManager& terrain) : Gui(name), m_terrain(terrain) {}

	bool NoiseDebugGui::render() {
		bool reloadSampleImage = false;

		ImGui::Begin(m_name.c_str(), &m_open);
		defaultStyle();

		// Set Seed
		ImGui::Text("Seed: ");
		ImGui::SameLine();
		if (ImGui::InputScalar("##Seed", ImGuiDataType_S32, &m_settings.seed)) reloadSampleImage = true;
		ImGui::SameLine();
		if (ImGui::Button("Randomize")) {
			m_settings.seed = GetRandomValue(-999999, 999999);
			reloadSampleImage = true;
		}

		// Noise Layers
		ImGui::SeparatorText("Noise Layers");
		NoiseLayersList(m_settings, m_noiseLayers, m_sampleImage, m_selectedLayerIndex);

		// Noise layer settings
		ImGui::SeparatorText("Noise Layer Settings");
		if (NoiseLayerSettings(m_settings, m_sampleImage, m_selectedLayerIndex)) reloadSampleImage = true;

		// Apply changes
		ImGui::SeparatorText("");
		if (ImGui::Button("Apply")) {
			// Update terrain
			(*m_terrain.refNoiseSettings()) = m_settings;
			m_terrain.updateTerrainNoise();

			// Update preview images
			m_noiseLayers = Noise::generateNoiseLayers(std::make_shared<Noise::noise_settings>(m_settings), { 0, 0, 0 }, SAMPLE_IMAGE_WIDTH, SAMPLE_IMAGE_HEIGHT, 1.0f, m_settings.seed);
			m_sampleImage = LoadTextureFromImage({ m_noiseLayers[m_selectedLayerIndex], SAMPLE_IMAGE_WIDTH, SAMPLE_IMAGE_HEIGHT, 1, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8 });
		}
		ImGui::SameLine();
		if (ImGui::Button("Revert")) {
			m_settings = *m_terrain.refNoiseSettings();
			m_noiseLayers = Noise::generateNoiseLayers(std::make_shared<Noise::noise_settings>(m_settings), { 0, 0, 0 }, SAMPLE_IMAGE_WIDTH, SAMPLE_IMAGE_HEIGHT, 1.0f, m_settings.seed);
			m_sampleImage = LoadTextureFromImage({ m_noiseLayers[m_selectedLayerIndex], SAMPLE_IMAGE_WIDTH, SAMPLE_IMAGE_HEIGHT, 1, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8 });
		}

		if (reloadSampleImage) {
			std::vector<Color*> layer = Noise::generateNoiseLayers(std::make_shared<Noise::noise_settings>(m_settings), { 0, 0, 0 }, SAMPLE_IMAGE_WIDTH, SAMPLE_IMAGE_HEIGHT, 1.0f, m_settings.seed);
			loadSampleImage(m_sampleImage, layer, m_selectedLayerIndex);
		}

		ImGui::End();

		return m_open;
	}

	void NoiseDebugGui::NoiseLayersList(Noise::noise_settings& settings, std::vector<Color*>& noiseLayers, Texture2D& sampleImage, int& selectedLayerIndex) {
		if (ImGui::BeginListBox("##Noise layers", ImVec2(-FLT_MIN, 3 * ImGui::GetTextLineHeightWithSpacing())))
		{
			for (int n = 0; n < settings.noiseLayerSettings.size(); n++)
			{
				const bool is_selected = (selectedLayerIndex == n);
				std::string label = "Noise Layer " + std::to_string(n);
				if (ImGui::Selectable(label.c_str(), is_selected)) {
					selectedLayerIndex = n;
					loadSampleImage(sampleImage, noiseLayers, selectedLayerIndex);
				}

				if (is_selected) ImGui::SetItemDefaultFocus();
			}
			ImGui::EndListBox();
		}

		// Plus and minus buttons
		ImVec2 buttonSize(50.f, 0.f);
		float widthNeeded = buttonSize.x + ImGui::GetStyle().ItemSpacing.x + buttonSize.x;
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x - widthNeeded);
		if (ImGui::Button("+", buttonSize)) {
			settings.noiseLayerSettings.push_back(Noise::newNoiseLayerSettings());
			settings.noiseLayerSettings = settings.noiseLayerSettings;
			selectedLayerIndex = settings.noiseLayerSettings.size() - 1;
			noiseLayers = Noise::generateNoiseLayers(std::make_shared<Noise::noise_settings>(settings), { 0, 0, 0 }, SAMPLE_IMAGE_WIDTH, SAMPLE_IMAGE_HEIGHT, 1.0f, settings.seed);
			loadSampleImage(sampleImage, noiseLayers, selectedLayerIndex);
		}
		ImGui::SameLine();
		if (ImGui::Button("-", buttonSize) && settings.noiseLayerSettings.size() >= 1) {
			settings.noiseLayerSettings.erase(settings.noiseLayerSettings.begin() + selectedLayerIndex);
			settings.noiseLayerSettings = settings.noiseLayerSettings;
			if (selectedLayerIndex = settings.noiseLayerSettings.size()) selectedLayerIndex--;
			noiseLayers = Noise::generateNoiseLayers(std::make_shared<Noise::noise_settings>(settings), { 0, 0, 0 }, SAMPLE_IMAGE_WIDTH, SAMPLE_IMAGE_HEIGHT, 1.0f, settings.seed);
			loadSampleImage(sampleImage, noiseLayers, selectedLayerIndex);
		}
	}

	void NoiseDebugGui::loadSampleImage(Texture2D& sampleImage, std::vector<Color*>& noiseLayers, int index) {
		UnloadTexture(sampleImage);
		sampleImage = LoadTextureFromImage({ noiseLayers[index], SAMPLE_IMAGE_WIDTH, SAMPLE_IMAGE_HEIGHT, 1, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8 });
	}

	bool NoiseDebugGui::NoiseLayerSettings(Noise::noise_settings& settings, Texture2D& sampleImage, int& selectedLayerIndex) {
		bool reloadSampleImage = false;

		rlImGuiImageSizeV(&sampleImage, Vector2({ 200, 200 }));

		if (ImGui::SliderFloat("Scale", &settings.noiseLayerSettings[selectedLayerIndex].horizontalScale, 0.1f, 5.0f, "%.3f")) reloadSampleImage = true;
		if (ImGui::SliderFloat("Vertical scale", &settings.noiseLayerSettings[selectedLayerIndex].verticalScale, 0.1f, 250.0f, "%.1f")) reloadSampleImage = true;
		if (ImGui::SliderInt("OffsetX", &settings.noiseLayerSettings[selectedLayerIndex].offsetX, -999999, 999999)) reloadSampleImage = true;
		if (ImGui::SliderInt("OffsetZ", &settings.noiseLayerSettings[selectedLayerIndex].offsetZ, -999999, 999999)) reloadSampleImage = true;

		if (ImGui::Button("Randomize Offsets")) {
			settings.noiseLayerSettings[selectedLayerIndex].offsetX = GetRandomValue(-999999, 999999);
			settings.noiseLayerSettings[selectedLayerIndex].offsetZ = GetRandomValue(-999999, 999999);
			reloadSampleImage = true;
		}

		if (ImGui::SliderFloat("Lacunarity", &settings.noiseLayerSettings[selectedLayerIndex].lacunarity, 0.0f, 5.0f, "%.1f")) reloadSampleImage = true;
		if (ImGui::SliderFloat("Gain", &settings.noiseLayerSettings[selectedLayerIndex].gain, 0.0f, 2.0f, "%.1f")) reloadSampleImage = true;
		if (ImGui::SliderInt("Octaves", &settings.noiseLayerSettings[selectedLayerIndex].octaves, 1, 10)) reloadSampleImage = true;
		if (ImGui::Checkbox("Around Zero", &settings.noiseLayerSettings[selectedLayerIndex].aroundZero)) reloadSampleImage = true;

		return reloadSampleImage;
	}
}