#include "DebugGui/NoiseDebugGui.h"

namespace DebugGui {
	NoiseDebugGui::NoiseDebugGui(std::string name, Terrain::TerrainManager& terrain) : Gui(name), m_terrain(terrain), m_settings(*terrain.refNoiseSettings()), m_selectedLayerIndex(0) {
		m_noiseLayers = Noise::generateNoiseLayers(std::make_shared<Noise::noise_settings>(m_settings), { 0, 0, 0 }, SAMPLE_IMAGE_WIDTH, SAMPLE_IMAGE_HEIGHT, 1.0f, m_settings.seed);
		m_sampleImage = LoadTextureFromImage({ m_noiseLayers[m_selectedLayerIndex], SAMPLE_IMAGE_WIDTH, SAMPLE_IMAGE_HEIGHT, 1, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8 });
	}

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
		NoiseLayersList();

		// Noise layer settings
		ImGui::SeparatorText("Noise Layer Settings");
		if (NoiseLayerSettings()) reloadSampleImage = true;

		// Apply changes
		ImGui::SeparatorText("");
		if (ImGui::Button("Apply")) {
			// Update terrain
			(*m_terrain.refNoiseSettings()) = m_settings;
			m_terrain.updateTerrainNoise();

			// Update preview images
			m_noiseLayers = Noise::generateNoiseLayers(std::make_shared<Noise::noise_settings>(m_settings), { 0, 0, 0 }, SAMPLE_IMAGE_WIDTH, SAMPLE_IMAGE_HEIGHT, 1.0f, m_settings.seed);
			loadSampleImage(m_sampleImage, m_noiseLayers, m_selectedLayerIndex);
		}
		ImGui::SameLine();
		if (ImGui::Button("Revert")) {
			m_settings = *m_terrain.refNoiseSettings();
			m_noiseLayers = Noise::generateNoiseLayers(std::make_shared<Noise::noise_settings>(m_settings), { 0, 0, 0 }, SAMPLE_IMAGE_WIDTH, SAMPLE_IMAGE_HEIGHT, 1.0f, m_settings.seed);
			loadSampleImage(m_sampleImage, m_noiseLayers, m_selectedLayerIndex);
		}

		if (reloadSampleImage) {
			std::vector<Color*> layer = Noise::generateNoiseLayers(std::make_shared<Noise::noise_settings>(m_settings), { 0, 0, 0 }, SAMPLE_IMAGE_WIDTH, SAMPLE_IMAGE_HEIGHT, 1.0f, m_settings.seed);
			loadSampleImage(m_sampleImage, layer, m_selectedLayerIndex);
		}

		ImGui::End();

		return m_open;
	}

	void NoiseDebugGui::NoiseLayersList() {
		if (ImGui::BeginListBox("##Noise layers", ImVec2(-FLT_MIN, 3 * ImGui::GetTextLineHeightWithSpacing())))
		{
			for (int n = 0; n < m_settings.noiseLayerSettings.size(); n++)
			{
				const bool is_selected = (m_selectedLayerIndex == n);
				std::string label = "Noise Layer " + std::to_string(n);
				if (ImGui::Selectable(label.c_str(), is_selected)) {
					m_selectedLayerIndex = n;
					loadSampleImage(m_sampleImage, m_noiseLayers, m_selectedLayerIndex);
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
			m_settings.noiseLayerSettings.push_back(Noise::newNoiseLayerSettings());
			m_settings.noiseLayerSettings = m_settings.noiseLayerSettings;
			m_selectedLayerIndex = m_settings.noiseLayerSettings.size() - 1;
			m_noiseLayers = Noise::generateNoiseLayers(std::make_shared<Noise::noise_settings>(m_settings), { 0, 0, 0 }, SAMPLE_IMAGE_WIDTH, SAMPLE_IMAGE_HEIGHT, 1.0f, m_settings.seed);
			loadSampleImage(m_sampleImage, m_noiseLayers, m_selectedLayerIndex);
		}
		ImGui::SameLine();
		if (ImGui::Button("-", buttonSize) && m_settings.noiseLayerSettings.size() >= 1) {
			m_settings.noiseLayerSettings.erase(m_settings.noiseLayerSettings.begin() + m_selectedLayerIndex);
			m_settings.noiseLayerSettings = m_settings.noiseLayerSettings;
			if (m_selectedLayerIndex = m_settings.noiseLayerSettings.size()) m_selectedLayerIndex--;
			m_noiseLayers = Noise::generateNoiseLayers(std::make_shared<Noise::noise_settings>(m_settings), { 0, 0, 0 }, SAMPLE_IMAGE_WIDTH, SAMPLE_IMAGE_HEIGHT, 1.0f, m_settings.seed);
			loadSampleImage(m_sampleImage, m_noiseLayers, m_selectedLayerIndex);
		}
	}

	void NoiseDebugGui::loadSampleImage(Texture2D& sampleImage, std::vector<Color*>& noiseLayers, int index) {
		UnloadTexture(sampleImage);
		sampleImage = LoadTextureFromImage({ noiseLayers[index], SAMPLE_IMAGE_WIDTH, SAMPLE_IMAGE_HEIGHT, 1, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8 });
	}

	bool NoiseDebugGui::NoiseLayerSettings() {
		bool reloadSampleImage = false;

		rlImGuiImageSizeV(&m_sampleImage, Vector2({ 200, 200 }));

		if (ImGui::SliderFloat("Scale", &m_settings.noiseLayerSettings[m_selectedLayerIndex].horizontalScale, 0.1f, 5.0f, "%.3f")) reloadSampleImage = true;
		if (ImGui::SliderFloat("Vertical scale", &m_settings.noiseLayerSettings[m_selectedLayerIndex].verticalScale, 0.1f, 250.0f, "%.1f")) reloadSampleImage = true;
		if (ImGui::SliderInt("OffsetX", &m_settings.noiseLayerSettings[m_selectedLayerIndex].offsetX, -999999, 999999)) reloadSampleImage = true;
		if (ImGui::SliderInt("OffsetZ", &m_settings.noiseLayerSettings[m_selectedLayerIndex].offsetZ, -999999, 999999)) reloadSampleImage = true;

		if (ImGui::Button("Randomize Offsets")) {
			m_settings.noiseLayerSettings[m_selectedLayerIndex].offsetX = GetRandomValue(-999999, 999999);
			m_settings.noiseLayerSettings[m_selectedLayerIndex].offsetZ = GetRandomValue(-999999, 999999);
			reloadSampleImage = true;
		}

		if (ImGui::SliderFloat("Lacunarity", &m_settings.noiseLayerSettings[m_selectedLayerIndex].lacunarity, 0.0f, 5.0f, "%.1f")) reloadSampleImage = true;
		if (ImGui::SliderFloat("Gain", &m_settings.noiseLayerSettings[m_selectedLayerIndex].gain, 0.0f, 2.0f, "%.1f")) reloadSampleImage = true;
		if (ImGui::SliderInt("Octaves", &m_settings.noiseLayerSettings[m_selectedLayerIndex].octaves, 1, 10)) reloadSampleImage = true;
		if (ImGui::Checkbox("Around Zero", &m_settings.noiseLayerSettings[m_selectedLayerIndex].aroundZero)) reloadSampleImage = true;

		return reloadSampleImage;
	}
}