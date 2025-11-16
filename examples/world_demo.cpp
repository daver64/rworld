#include "rworld/world.h"
#include <iostream>
#include <iomanip>
#include <vector>
#include <cmath>
#include <algorithm>

#ifdef USE_SDL2
#include <SDL2/SDL.h>
#ifdef USE_SDL2_TTF
#include <SDL2/SDL_ttf.h>
#endif
#include "../third_party/FastNoiseLite.h"
#endif

using namespace rworld;

void print_location_info(const World& world, float lon, float lat) {
    float terrain_height = world.get_terrain_height(lon, lat);
    float altitude = std::max(terrain_height, 0.0f); // Use terrain height or 0 for underwater
    
    float temp = world.get_temperature(lon, lat, altitude);
    float precip = world.get_precipitation(lon, lat, altitude);
    float pressure = world.get_air_pressure(lon, lat, altitude);
    float humidity = world.get_humidity(lon, lat, altitude);
    BiomeType biome = world.get_biome(lon, lat, altitude);
    PrecipitationType precip_type = world.get_precipitation_type(lon, lat, altitude);
    
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "\nLocation: (" << lon << "°, " << lat << "°)\n";
    std::cout << "  Terrain Height: " << terrain_height << " m\n";
    std::cout << "  Biome: " << biome_to_string(biome) << "\n";
    std::cout << "  Temperature: " << temp << " °C\n";
    std::cout << "  Precipitation: " << precip << " mm/year";
    
    const char* precip_type_str;
    switch (precip_type) {
        case PrecipitationType::NONE: precip_type_str = "None"; break;
        case PrecipitationType::RAIN: precip_type_str = "Rain"; break;
        case PrecipitationType::SNOW: precip_type_str = "Snow"; break;
        case PrecipitationType::SLEET: precip_type_str = "Sleet"; break;
        default: precip_type_str = "Unknown";
    }
    std::cout << " (" << precip_type_str << ")\n";
    
    std::cout << "  Air Pressure: " << pressure << " hPa\n";
    std::cout << "  Humidity: " << (humidity * 100.0f) << " %\n";
}

void generate_world_map(const World& world, int width, int height) {
    std::cout << "\n=== ASCII World Map ===\n";
    std::cout << "(Showing terrain elevation and basic features)\n\n";
    
    // Map characters for different height ranges
    auto height_to_char = [](float h) -> char {
        if (h < -2000) return '#';      // Deep ocean
        if (h < -200) return '~';       // Ocean
        if (h < 0) return '-';          // Shallow water
        if (h < 100) return '.';        // Low plains
        if (h < 500) return ':';        // Plains
        if (h < 1000) return '=';       // Hills
        if (h < 2000) return '+';       // Low mountains
        if (h < 4000) return '*';       // Mountains
        return '^';                     // High peaks
    };
    
    for (int y = 0; y < height; ++y) {
        float lat = 90.0f - (y * 180.0f / height);
        for (int x = 0; x < width; ++x) {
            float lon = -180.0f + (x * 360.0f / width);
            float h = world.get_terrain_height(lon, lat);
            std::cout << height_to_char(h);
        }
        std::cout << '\n';
    }
}

void sample_transect(const World& world) {
    std::cout << "\n=== Equatorial Transect (Latitude 0°) ===\n";
    std::cout << std::setw(8) << "Long" 
              << std::setw(10) << "Height(m)"
              << std::setw(8) << "Temp(C)"
              << std::setw(25) << "Biome\n";
    std::cout << std::string(51, '-') << "\n";
    
    for (int lon = -180; lon <= 180; lon += 20) {
        float height = world.get_terrain_height(lon, 0);
        float altitude = std::max(height, 0.0f);
        float temp = world.get_temperature(lon, 0, altitude);
        BiomeType biome = world.get_biome(lon, 0, altitude);
        
        std::cout << std::setw(8) << lon
                  << std::setw(10) << std::fixed << std::setprecision(1) << height
                  << std::setw(8) << std::fixed << std::setprecision(1) << temp
                  << "  " << biome_to_string(biome) << "\n";
    }
}

void demonstrate_altitude_effects(const World& world) {
    std::cout << "\n=== Altitude Effects (at 0°, 0°) ===\n";
    std::cout << std::setw(12) << "Altitude(m)"
              << std::setw(10) << "Temp(C)"
              << std::setw(12) << "Pressure"
              << std::setw(10) << "Humidity\n";
    std::cout << std::string(44, '-') << "\n";
    
    for (int alt = 0; alt <= 8000; alt += 1000) {
        float temp = world.get_temperature(0, 0, alt);
        float pressure = world.get_air_pressure(0, 0, alt);
        float humidity = world.get_humidity(0, 0, alt);
        
        std::cout << std::setw(12) << alt
                  << std::setw(10) << std::fixed << std::setprecision(1) << temp
                  << std::setw(12) << std::fixed << std::setprecision(1) << pressure
                  << std::setw(9) << std::fixed << std::setprecision(1) << (humidity * 100) << "%\n";
    }
}

void run_text_demo(const World& world) {
    const WorldConfig& config = world.get_config();
    
    std::cout << "\nWorld Configuration:\n";
    std::cout << "  Seed: " << config.seed << "\n";
    std::cout << "  World Scale: " << config.world_scale << "\n";
    std::cout << "  Equator Temperature: " << config.equator_temperature << " °C\n";
    std::cout << "  Pole Temperature: " << config.pole_temperature << " °C\n";
    std::cout << "  Max Terrain Height: " << config.max_terrain_height << " m\n";
    
    // Sample interesting locations
    std::cout << "\n=== Sample Locations ===\n";
    
    struct Location {
        float lon, lat;
        const char* name;
    };
    
    std::vector<Location> locations = {
        {0.0f, 0.0f, "Equator, Prime Meridian"},
        {-74.0f, 40.7f, "New York latitude"},
        {139.7f, 35.7f, "Tokyo latitude"},
        {0.0f, 90.0f, "North Pole"},
        {0.0f, -90.0f, "South Pole"},
        {30.0f, -30.0f, "Southern Hemisphere Mid-latitude"},
        {-120.0f, 45.0f, "Northern Hemisphere Mid-latitude"},
    };
    
    for (const auto& loc : locations) {
        std::cout << "\n--- " << loc.name << " ---";
        print_location_info(world, loc.lon, loc.lat);
    }
    
    // Generate a simple ASCII map
    generate_world_map(world, 80, 40);
    
    // Show equatorial transect
    sample_transect(world);
    
    // Demonstrate altitude effects
    demonstrate_altitude_effects(world);
    
    std::cout << "\n=== Demo Complete ===\n";
    std::cout << "\nNote: Install SDL2 for graphical visualization!\n";
    std::cout << "  Ubuntu/Debian: sudo apt-get install libsdl2-dev\n";
    std::cout << "  Then rebuild with: cd build && cmake .. && cmake --build .\n";
}

#ifdef USE_SDL2

struct RGB {
    uint8_t r, g, b;
};

// Get color for biome type
RGB get_biome_color(BiomeType biome) {
    switch (biome) {
        case BiomeType::DEEP_OCEAN: return {0, 0, 139};
        case BiomeType::OCEAN: return {0, 105, 148};
        case BiomeType::BEACH: return {238, 214, 175};
        case BiomeType::ICE: return {240, 248, 255};
        case BiomeType::SNOW: return {255, 250, 250};
        case BiomeType::TUNDRA: return {150, 180, 150};
        case BiomeType::TAIGA: return {89, 115, 90};
        case BiomeType::MOUNTAIN_PEAK: return {200, 200, 210};
        case BiomeType::MOUNTAIN_TUNDRA: return {170, 180, 170};
        case BiomeType::MOUNTAIN_FOREST: return {100, 130, 100};
        case BiomeType::COLD_DESERT: return {200, 180, 160};
        case BiomeType::GRASSLAND: return {144, 188, 70};
        case BiomeType::TEMPERATE_DECIDUOUS_FOREST: return {80, 150, 80};
        case BiomeType::TEMPERATE_RAINFOREST: return {50, 130, 80};
        case BiomeType::DESERT: return {230, 200, 120};
        case BiomeType::SAVANNA: return {200, 180, 100};
        case BiomeType::TROPICAL_SEASONAL_FOREST: return {100, 160, 80};
        case BiomeType::TROPICAL_RAINFOREST: return {40, 120, 60};
        default: return {128, 128, 128};
    }
}

// Get color based on terrain height (for elevation mode)
RGB get_height_color(float height) {
    if (height < -2000) return {0, 0, 80};      // Deep ocean
    if (height < -500) return {0, 50, 120};     // Ocean
    if (height < 0) return {0, 100, 160};       // Shallow water
    if (height < 100) return {100, 180, 100};   // Low plains
    if (height < 500) return {130, 190, 80};    // Plains
    if (height < 1000) return {160, 160, 100};  // Hills
    if (height < 2000) return {140, 130, 100};  // Low mountains
    if (height < 4000) return {180, 170, 150};  // Mountains
    return {240, 240, 240};                     // High peaks
}

// Get color based on temperature
RGB get_temperature_color(float temp) {
    if (temp < -30) return {0, 0, 139};         // Extreme cold
    if (temp < -10) return {100, 150, 255};     // Very cold
    if (temp < 0) return {150, 200, 255};       // Cold
    if (temp < 10) return {180, 220, 180};      // Cool
    if (temp < 20) return {150, 200, 100};      // Moderate
    if (temp < 30) return {255, 200, 100};      // Warm
    return {255, 100, 50};                      // Hot
}

// Get color based on precipitation
RGB get_precipitation_color(float precip) {
    if (precip < 100) return {230, 200, 120};   // Arid
    if (precip < 500) return {200, 180, 100};   // Dry
    if (precip < 1000) return {150, 200, 100};  // Moderate
    if (precip < 2000) return {100, 180, 150};  // Wet
    return {50, 150, 180};                      // Very wet
}

enum class DisplayMode {
    BIOMES,
    ELEVATION,
    TEMPERATURE,
    PRECIPITATION,
    CLOUDS,
    RIVERS,
    COAL,
    IRON,
    OIL,
    INSOLATION
};

#ifdef USE_SDL2_TTF
struct TextRenderer {
    TTF_Font* font;
    
    TextRenderer() : font(nullptr) {}
    
    bool init() {
        if (TTF_Init() < 0) {
            std::cerr << "TTF initialization failed: " << TTF_GetError() << std::endl;
            return false;
        }
        
        // Try to load a system font
        const char* font_paths[] = {
            "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
            "/usr/share/fonts/TTF/DejaVuSans.ttf",
            "/System/Library/Fonts/Helvetica.ttc",
            "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf",
        };
        
        for (const char* path : font_paths) {
            font = TTF_OpenFont(path, 14);
            if (font) {
                std::cout << "Loaded font: " << path << std::endl;
                return true;
            }
        }
        
        std::cerr << "Failed to load any system font" << std::endl;
        return false;
    }
    
    void draw_text(SDL_Renderer* renderer, const char* text, int x, int y, SDL_Color color) {
        if (!font) return;
        
        SDL_Surface* surface = TTF_RenderText_Blended(font, text, color);
        if (!surface) return;
        
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
        if (texture) {
            SDL_Rect dst = {x, y, surface->w, surface->h};
            SDL_RenderCopy(renderer, texture, nullptr, &dst);
            SDL_DestroyTexture(texture);
        }
        SDL_FreeSurface(surface);
    }
    
    void cleanup() {
        if (font) {
            TTF_CloseFont(font);
            font = nullptr;
        }
        TTF_Quit();
    }
    
    ~TextRenderer() {
        cleanup();
    }
};
#endif

// Cloud layer generator
struct CloudLayer {
    FastNoiseLite cloud_noise;
    uint64_t seed;
    const World* world_ptr; // Pointer to world for weather data
    
    CloudLayer(uint64_t world_seed, const World* world) : seed(world_seed), world_ptr(world) {
        cloud_noise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
        cloud_noise.SetFractalType(FastNoiseLite::FractalType_FBm);
        cloud_noise.SetFractalOctaves(3);
        cloud_noise.SetFrequency(0.005f);
        cloud_noise.SetSeed(static_cast<int>(world_seed + 5000));
    }
    
    // Get cloud density at a location (0 = clear, 1 = dense clouds)
    float get_cloud_density(float longitude, float latitude) const {
        // Convert to 3D coordinates on sphere
        float lon_rad = longitude * 3.14159265359f / 180.0f;
        float lat_rad = latitude * 3.14159265359f / 180.0f;
        float r = 1000.0f;
        float x = r * std::cos(lat_rad) * std::cos(lon_rad);
        float y = r * std::cos(lat_rad) * std::sin(lon_rad);
        float z = r * std::sin(lat_rad);
        
        // Get base noise pattern for cloud variation
        float noise = cloud_noise.GetNoise(x, y, z);
        noise = (noise + 1.0f) * 0.5f; // Convert from -1,1 to 0,1
        
        // Get world weather data
        float terrain_height = world_ptr->get_terrain_height(longitude, latitude, 1.0f);
        float altitude = std::max(terrain_height, 0.0f);
        float temperature = world_ptr->get_temperature(longitude, latitude, altitude);
        float humidity = world_ptr->get_humidity(longitude, latitude, altitude);
        float precipitation = world_ptr->get_precipitation(longitude, latitude, altitude);
        
        // Cloud density is heavily influenced by humidity
        float cloud_base = humidity * 0.8f + 0.2f * noise;
        
        // High precipitation areas have more clouds
        float precip_factor = std::clamp(precipitation / 2000.0f, 0.0f, 1.0f);
        cloud_base = cloud_base * 0.6f + precip_factor * 0.4f;
        
        // Temperature affects cloud formation
        // Moderate temperatures (10-25°C) have more clouds
        float temp_factor = 1.0f;
        if (temperature < -10.0f) {
            temp_factor = 0.5f; // Very cold = less clouds
        } else if (temperature > 35.0f) {
            temp_factor = 0.7f; // Very hot/dry = fewer clouds
        } else if (temperature >= 10.0f && temperature <= 25.0f) {
            temp_factor = 1.2f; // Optimal for cloud formation
        }
        cloud_base *= temp_factor;
        
        // Add noise variation for patchiness
        float cloud_density = cloud_base * noise;
        
        // Make clouds more patchy
        cloud_density = std::pow(cloud_density, 1.2f);
        
        return std::clamp(cloud_density, 0.0f, 1.0f);
    }
};

struct ViewState {
    float center_lon = 0.0f;      // Center longitude of view
    float center_lat = 0.0f;      // Center latitude of view
    float zoom = 1.0f;            // Zoom level (1.0 = whole world, higher = more zoomed in)
    float current_time = 12.0f;   // Time of day in hours (0-24)
    bool time_paused = true;      // Whether time advances automatically
    
    // Convert screen coordinates to world coordinates
    void screen_to_world(int screen_x, int screen_y, int width, int height, 
                        float& lon, float& lat) const {
        // Normalized screen coordinates (-0.5 to 0.5)
        float norm_x = (screen_x - width * 0.5f) / width;
        float norm_y = (screen_y - height * 0.5f) / height;
        
        // World span at current zoom
        float lon_span = 360.0f / zoom;
        float lat_span = 180.0f / zoom;
        
        // Calculate world coordinates
        lon = center_lon + norm_x * lon_span;
        lat = center_lat - norm_y * lat_span;
        
        // Clamp to valid ranges
        lat = std::clamp(lat, -90.0f, 90.0f);
        
        // Wrap longitude
        while (lon > 180.0f) lon -= 360.0f;
        while (lon < -180.0f) lon += 360.0f;
    }
};

void render_cloud_overlay(SDL_Renderer* renderer, const CloudLayer& clouds,
                          int width, int height, const ViewState& view) {
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            float lon, lat;
            view.screen_to_world(x, y, width, height, lon, lat);
            
            float density = clouds.get_cloud_density(lon, lat);
            
            // Only draw clouds where density is significant
            if (density > 0.3f) {
                uint8_t alpha = static_cast<uint8_t>((density - 0.3f) / 0.7f * 180.0f);
                SDL_SetRenderDrawColor(renderer, 255, 255, 255, alpha);
                SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
                SDL_RenderDrawPoint(renderer, x, y);
            }
        }
    }
}

void render_world_map(SDL_Renderer* renderer, const World& world, 
                      int width, int height, DisplayMode mode, const ViewState& view) {
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            float lon, lat;
            view.screen_to_world(x, y, width, height, lon, lat);
            
            RGB color;
            
            switch (mode) {
                case DisplayMode::BIOMES: {
                    // Biomes use base terrain (detail_level = 1.0) so they don't change with zoom
                    float terrain_height = world.get_terrain_height(lon, lat, 1.0f);
                    float altitude = std::max(terrain_height, 0.0f);
                    BiomeType biome = world.get_biome(lon, lat, altitude);
                    color = get_biome_color(biome);
                    break;
                }
                case DisplayMode::ELEVATION: {
                    // Elevation uses detailed terrain that responds to zoom
                    float height = world.get_terrain_height(lon, lat, view.zoom);
                    color = get_height_color(height);
                    break;
                }
                case DisplayMode::TEMPERATURE: {
                    // Temperature uses base terrain so it stays consistent
                    float terrain_height = world.get_terrain_height(lon, lat, 1.0f);
                    float altitude = std::max(terrain_height, 0.0f);
                    float temp = world.get_temperature(lon, lat, altitude);
                    color = get_temperature_color(temp);
                    break;
                }
                case DisplayMode::PRECIPITATION: {
                    // Precipitation uses base terrain so it stays consistent
                    float terrain_height = world.get_terrain_height(lon, lat, 1.0f);
                    float altitude = std::max(terrain_height, 0.0f);
                    float precip = world.get_precipitation(lon, lat, altitude);
                    color = get_precipitation_color(precip);
                    break;
                }
                case DisplayMode::CLOUDS: {
                    // Show base biome map
                    float terrain_height = world.get_terrain_height(lon, lat, 1.0f);
                    float altitude = std::max(terrain_height, 0.0f);
                    BiomeType biome = world.get_biome(lon, lat, altitude);
                    color = get_biome_color(biome);
                    break;
                }
                case DisplayMode::RIVERS: {
                    // Show elevation with rivers highlighted
                    float height = world.get_terrain_height(lon, lat, view.zoom);
                    color = get_height_color(height);
                    
                    // Overlay rivers in blue
                    if (world.is_river(lon, lat)) {
                        float flow = world.get_flow_accumulation(lon, lat);
                        uint8_t blue_intensity = static_cast<uint8_t>(100 + flow * 155);
                        color = {0, 100, blue_intensity};
                    }
                    break;
                }
                case DisplayMode::COAL: {
                    // Show coal deposits on elevation map
                    float height = world.get_terrain_height(lon, lat, view.zoom);
                    color = get_height_color(height);
                    
                    // Overlay coal in black/gray
                    float coal = world.get_coal_deposit(lon, lat);
                    if (coal > 0.3f) {
                        uint8_t intensity = static_cast<uint8_t>(255 * (1.0f - coal * 0.8f));
                        uint8_t dark = static_cast<uint8_t>(intensity / 3);
                        color = {dark, dark, dark};
                    }
                    break;
                }
                case DisplayMode::IRON: {
                    // Show iron deposits on elevation map
                    float height = world.get_terrain_height(lon, lat, view.zoom);
                    color = get_height_color(height);
                    
                    // Overlay iron in rust red/brown
                    float iron = world.get_iron_deposit(lon, lat);
                    if (iron > 0.3f) {
                        uint8_t red = static_cast<uint8_t>(139 + iron * 70);
                        uint8_t brown = static_cast<uint8_t>(69 + iron * 40);
                        color = {red, brown, static_cast<uint8_t>(brown / 2)};
                    }
                    break;
                }
                case DisplayMode::OIL: {
                    // Show oil deposits on elevation map
                    float height = world.get_terrain_height(lon, lat, view.zoom);
                    color = get_height_color(height);
                    
                    // Overlay oil in dark green/black
                    float oil = world.get_oil_deposit(lon, lat);
                    if (oil > 0.3f) {
                        uint8_t darkness = static_cast<uint8_t>(50 * (1.0f - oil));
                        uint8_t green = static_cast<uint8_t>(darkness + oil * 80);
                        color = {darkness, green, darkness};
                    }
                    break;
                }
                case DisplayMode::INSOLATION: {
                    // Show solar radiation based on time of day
                    float insolation = world.get_insolation(lon, lat, view.current_time);
                    
                    // Color based on insolation level (0-1400 W/m²)
                    float normalized = std::clamp(insolation / 1000.0f, 0.0f, 1.4f);
                    
                    if (normalized == 0.0f) {
                        // Night - dark blue/black
                        color = {10, 10, 30};
                    } else {
                        // Day - yellow to white based on intensity
                        uint8_t r = static_cast<uint8_t>(50 + normalized * 205);
                        uint8_t g = static_cast<uint8_t>(50 + normalized * 205);
                        uint8_t b = static_cast<uint8_t>(normalized * 100);
                        color = {r, g, b};
                    }
                    break;
                }
            }
            
            SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 255);
            SDL_RenderDrawPoint(renderer, x, y);
        }
    }
}

void render_info_panel(SDL_Renderer* renderer, const World& world, 
                       int mouse_x, int mouse_y, int map_width, int map_height,
                       DisplayMode mode, const ViewState& view
#ifdef USE_SDL2_TTF
                       , TextRenderer* text_renderer = nullptr
#endif
                       ) {
    // Calculate world coordinates from mouse position
    float lon, lat;
    view.screen_to_world(mouse_x, mouse_y, map_width, map_height, lon, lat);
    
    if (lat < -90.0f || lat > 90.0f) {
        return;
    }
    
    // Query world data
    float terrain_height = world.get_terrain_height(lon, lat);
    float altitude = std::max(terrain_height, 0.0f);
    float temp = world.get_temperature(lon, lat, altitude);
    float precip = world.get_precipitation(lon, lat, altitude);
    BiomeType biome = world.get_biome(lon, lat, altitude);
    
    // Draw semi-transparent info panel
    SDL_Rect panel = {10, 10, 320, 250};
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 200);
    SDL_RenderFillRect(renderer, &panel);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderDrawRect(renderer, &panel);
    
#ifdef USE_SDL2_TTF
    if (text_renderer && text_renderer->font) {
        // Draw text information
        SDL_Color white = {255, 255, 255, 255};
        SDL_Color gray = {200, 200, 200, 255};
        
        int text_y = 20;
        char buffer[128];
        
        // Coordinates
        snprintf(buffer, sizeof(buffer), "Location: (%.1f, %.1f)", lon, lat);
        text_renderer->draw_text(renderer, buffer, 20, text_y, white);
        text_y += 25;
        
        // Biome
        snprintf(buffer, sizeof(buffer), "Biome: %s", biome_to_string(biome));
        text_renderer->draw_text(renderer, buffer, 20, text_y, white);
        text_y += 20;
        
        // Height
        snprintf(buffer, sizeof(buffer), "Elevation: %.1f m", terrain_height);
        text_renderer->draw_text(renderer, buffer, 20, text_y, gray);
        text_y += 20;
        
        // Temperature
        snprintf(buffer, sizeof(buffer), "Temperature: %.1f C", temp);
        text_renderer->draw_text(renderer, buffer, 20, text_y, gray);
        text_y += 20;
        
        // Precipitation
        snprintf(buffer, sizeof(buffer), "Precipitation: %.0f mm/yr", precip);
        text_renderer->draw_text(renderer, buffer, 20, text_y, gray);
        text_y += 20;
        
        // Pressure
        float pressure = world.get_air_pressure(lon, lat, altitude);
        snprintf(buffer, sizeof(buffer), "Air Pressure: %.1f hPa", pressure);
        text_renderer->draw_text(renderer, buffer, 20, text_y, gray);
        text_y += 20;
        
        // Humidity
        float humidity = world.get_humidity(lon, lat, altitude);
        snprintf(buffer, sizeof(buffer), "Humidity: %.0f%%", humidity * 100);
        text_renderer->draw_text(renderer, buffer, 20, text_y, gray);
        text_y += 20;
        
        // Wind speed
        float wind_speed = world.get_wind_speed(lon, lat, altitude);
        snprintf(buffer, sizeof(buffer), "Wind Speed: %.1f m/s", wind_speed);
        text_renderer->draw_text(renderer, buffer, 20, text_y, gray);
        text_y += 20;
        
        // Wind direction
        float wind_dir = world.get_wind_direction(lon, lat, altitude);
        const char* wind_compass = "";
        if (wind_dir >= 337.5f || wind_dir < 22.5f) wind_compass = "N";
        else if (wind_dir < 67.5f) wind_compass = "NE";
        else if (wind_dir < 112.5f) wind_compass = "E";
        else if (wind_dir < 157.5f) wind_compass = "SE";
        else if (wind_dir < 202.5f) wind_compass = "S";
        else if (wind_dir < 247.5f) wind_compass = "SW";
        else if (wind_dir < 292.5f) wind_compass = "W";
        else wind_compass = "NW";
        snprintf(buffer, sizeof(buffer), "Wind Dir: %.0f%c (%s)", wind_dir, 176, wind_compass);
        text_renderer->draw_text(renderer, buffer, 20, text_y, gray);
        text_y += 20;
        
        // River info
        if (world.is_river(lon, lat)) {
            float river_width = world.get_river_width(lon, lat);
            snprintf(buffer, sizeof(buffer), "River Width: %.0f m", river_width);
            text_renderer->draw_text(renderer, buffer, 20, text_y, SDL_Color{100, 150, 255, 255});
            text_y += 20;
        }
        
        // Volcano info
        if (world.is_volcano(lon, lat)) {
            snprintf(buffer, sizeof(buffer), "VOLCANO");
            text_renderer->draw_text(renderer, buffer, 20, text_y, SDL_Color{255, 100, 50, 255});
            text_y += 20;
        }
        
        // Mineral resources
        float coal = world.get_coal_deposit(lon, lat);
        float iron = world.get_iron_deposit(lon, lat);
        float oil = world.get_oil_deposit(lon, lat);
        
        if (coal > 0.3f) {
            snprintf(buffer, sizeof(buffer), "Coal: %d%%", static_cast<int>(coal * 100));
            text_renderer->draw_text(renderer, buffer, 20, text_y, SDL_Color{80, 80, 80, 255});
            text_y += 20;
        }
        
        if (iron > 0.3f) {
            snprintf(buffer, sizeof(buffer), "Iron Ore: %d%%", static_cast<int>(iron * 100));
            text_renderer->draw_text(renderer, buffer, 20, text_y, SDL_Color{209, 109, 60, 255});
            text_y += 20;
        }
        
        if (oil > 0.3f) {
            snprintf(buffer, sizeof(buffer), "Oil: %d%%", static_cast<int>(oil * 100));
            text_renderer->draw_text(renderer, buffer, 20, text_y, SDL_Color{50, 130, 50, 255});
            text_y += 20;
        }
        
        // Insolation and time info
        float insolation = world.get_insolation(lon, lat, view.current_time);
        bool is_day = world.is_daylight(lon, lat, view.current_time);
        float solar_angle = world.get_solar_angle(lon, lat, view.current_time);
        
        snprintf(buffer, sizeof(buffer), "Time: %02d:%02d %s", 
                 static_cast<int>(view.current_time), 
                 static_cast<int>((view.current_time - static_cast<int>(view.current_time)) * 60),
                 view.time_paused ? "[PAUSED]" : "");
        text_renderer->draw_text(renderer, buffer, 20, text_y, SDL_Color{200, 200, 255, 255});
        text_y += 20;
        
        snprintf(buffer, sizeof(buffer), "Insolation: %.0f W/m%c", insolation, 178); // ² symbol
        text_renderer->draw_text(renderer, buffer, 20, text_y, 
                                is_day ? SDL_Color{255, 255, 100, 255} : SDL_Color{100, 100, 150, 255});
        text_y += 20;
        
        if (is_day) {
            snprintf(buffer, sizeof(buffer), "Solar Angle: %.1f%c", solar_angle, 176); // ° symbol
            text_renderer->draw_text(renderer, buffer, 20, text_y, SDL_Color{255, 200, 100, 255});
        } else {
            text_renderer->draw_text(renderer, "Night", 20, text_y, SDL_Color{100, 100, 150, 255});
        }
    } else
#endif
    {
        // Draw color indicators for each property (fallback without SDL_ttf)
        int info_y = 25;
        
        // Biome indicator
        RGB biome_color = get_biome_color(biome);
        SDL_Rect biome_rect = {20, info_y, 30, 15};
        SDL_SetRenderDrawColor(renderer, biome_color.r, biome_color.g, biome_color.b, 255);
        SDL_RenderFillRect(renderer, &biome_rect);
        info_y += 25;
        
        // Temperature indicator
        RGB temp_color = get_temperature_color(temp);
        SDL_Rect temp_rect = {20, info_y, 30, 15};
        SDL_SetRenderDrawColor(renderer, temp_color.r, temp_color.g, temp_color.b, 255);
        SDL_RenderFillRect(renderer, &temp_rect);
        info_y += 25;
        
        // Precipitation indicator
        RGB precip_color = get_precipitation_color(precip);
        SDL_Rect precip_rect = {20, info_y, 30, 15};
        SDL_SetRenderDrawColor(renderer, precip_color.r, precip_color.g, precip_color.b, 255);
        SDL_RenderFillRect(renderer, &precip_rect);
        info_y += 25;
        
        // Height indicator
        RGB height_color = get_height_color(terrain_height);
        SDL_Rect height_rect = {20, info_y, 30, 15};
        SDL_SetRenderDrawColor(renderer, height_color.r, height_color.g, height_color.b, 255);
        SDL_RenderFillRect(renderer, &height_rect);
    }
    
    // Mode indicator at bottom
    SDL_Rect mode_panel = {10, map_height - 40, 250, 30};
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 200);
    SDL_RenderFillRect(renderer, &mode_panel);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderDrawRect(renderer, &mode_panel);
    
#ifdef USE_SDL2_TTF
    if (text_renderer && text_renderer->font) {
        SDL_Color white = {255, 255, 255, 255};
        const char* mode_text = "Unknown";
        switch (mode) {
            case DisplayMode::BIOMES: mode_text = "Mode: Biomes (1)"; break;
            case DisplayMode::ELEVATION: mode_text = "Mode: Elevation (2)"; break;
            case DisplayMode::TEMPERATURE: mode_text = "Mode: Temperature (3)"; break;
            case DisplayMode::PRECIPITATION: mode_text = "Mode: Precipitation (4)"; break;
            case DisplayMode::CLOUDS: mode_text = "Mode: Clouds (5)"; break;
            case DisplayMode::RIVERS: mode_text = "Mode: Rivers (6)"; break;
            case DisplayMode::COAL: mode_text = "Mode: Coal (7)"; break;
            case DisplayMode::IRON: mode_text = "Mode: Iron (8)"; break;
            case DisplayMode::OIL: mode_text = "Mode: Oil (9)"; break;
            case DisplayMode::INSOLATION: mode_text = "Mode: Insolation (0)"; break;
        }
        text_renderer->draw_text(renderer, mode_text, 20, map_height - 33, white);
    }
#endif
}

void run_sdl_demo(World& world) {
    const int WINDOW_WIDTH = 1200;
    const int WINDOW_HEIGHT = 600;
    
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL initialization failed: " << SDL_GetError() << std::endl;
        return;
    }
    
#ifdef USE_SDL2_TTF
    TextRenderer text_renderer;
    bool has_text = text_renderer.init();
    if (!has_text) {
        std::cout << "Running without text rendering (SDL_ttf not available)\n";
    }
#endif
    
    SDL_Window* window = SDL_CreateWindow(
        "RWorld - Living Active World Visualization",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH, WINDOW_HEIGHT,
        SDL_WINDOW_SHOWN
    );
    
    if (!window) {
        std::cerr << "Window creation failed: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return;
    }
    
    SDL_Renderer* renderer = SDL_CreateRenderer(
        window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );
    
    if (!renderer) {
        std::cerr << "Renderer creation failed: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return;
    }
    
    std::cout << "\n=== SDL2 Interactive World Visualization ===\n";
    std::cout << "Controls:\n";
    std::cout << "  1 - Show Biomes\n";
    std::cout << "  2 - Show Elevation\n";
    std::cout << "  3 - Show Temperature\n";
    std::cout << "  4 - Show Precipitation\n";
    std::cout << "  5 - Show Clouds\n";
    std::cout << "  6 - Show Rivers\n";
    std::cout << "  7 - Show Coal Deposits\n";
    std::cout << "  8 - Show Iron Deposits\n";
    std::cout << "  9 - Show Oil Deposits\n";
    std::cout << "  0 - Show Insolation (Day/Night)\n";
    std::cout << "  SPACE - Pause/Resume time\n";
    std::cout << "  [ / ] - Decrease/Increase time\n";
    std::cout << "  R - Regenerate world (new seed)\n";
    std::cout << "  Mouse Wheel - Zoom in/out at cursor position\n";
    std::cout << "  ESC/Q - Quit\n";
    std::cout << "  Mouse - Hover to see location details\n";
    std::cout << "\nGenerating world map...\n";
    
    WorldConfig config = world.get_config();
    
    DisplayMode current_mode = DisplayMode::BIOMES;
    ViewState view_state;
    CloudLayer clouds(config.seed, &world);
    bool need_redraw = true;
    bool running = true;
    int mouse_x = 0, mouse_y = 0;
    
    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            } else if (event.type == SDL_KEYDOWN) {
                switch (event.key.keysym.sym) {
                    case SDLK_ESCAPE:
                    case SDLK_q:
                        running = false;
                        break;
                    case SDLK_1:
                        current_mode = DisplayMode::BIOMES;
                        need_redraw = true;
                        std::cout << "Display mode: Biomes\n";
                        break;
                    case SDLK_2:
                        current_mode = DisplayMode::ELEVATION;
                        need_redraw = true;
                        std::cout << "Display mode: Elevation\n";
                        break;
                    case SDLK_3:
                        current_mode = DisplayMode::TEMPERATURE;
                        need_redraw = true;
                        std::cout << "Display mode: Temperature\n";
                        break;
                    case SDLK_4:
                        current_mode = DisplayMode::PRECIPITATION;
                        need_redraw = true;
                        std::cout << "Display mode: Precipitation\n";
                        break;
                    case SDLK_5:
                        current_mode = DisplayMode::CLOUDS;
                        need_redraw = true;
                        std::cout << "Display mode: Clouds\n";
                        break;
                    case SDLK_6:
                        current_mode = DisplayMode::RIVERS;
                        need_redraw = true;
                        std::cout << "Display mode: Rivers\n";
                        break;
                    case SDLK_7:
                        current_mode = DisplayMode::COAL;
                        need_redraw = true;
                        std::cout << "Display mode: Coal Deposits\n";
                        break;
                    case SDLK_8:
                        current_mode = DisplayMode::IRON;
                        need_redraw = true;
                        std::cout << "Display mode: Iron Deposits\n";
                        break;
                    case SDLK_9:
                        current_mode = DisplayMode::OIL;
                        need_redraw = true;
                        std::cout << "Display mode: Oil Deposits\n";
                        break;
                    case SDLK_0:
                        current_mode = DisplayMode::INSOLATION;
                        need_redraw = true;
                        std::cout << "Display mode: Insolation (Time: " << view_state.current_time << "h)\n";
                        break;
                    case SDLK_SPACE:
                        view_state.time_paused = !view_state.time_paused;
                        std::cout << "Time " << (view_state.time_paused ? "paused" : "running") << "\n";
                        break;
                    case SDLK_LEFTBRACKET: // [ key - decrease time
                        view_state.current_time -= 0.5f;
                        if (view_state.current_time < 0.0f) view_state.current_time += 24.0f;
                        need_redraw = true;
                        std::cout << "Time: " << view_state.current_time << "h\n";
                        break;
                    case SDLK_RIGHTBRACKET: // ] key - increase time
                        view_state.current_time += 0.5f;
                        if (view_state.current_time >= 24.0f) view_state.current_time -= 24.0f;
                        need_redraw = true;
                        std::cout << "Time: " << view_state.current_time << "h\n";
                        break;
                    case SDLK_r:
                        config.seed = static_cast<uint64_t>(SDL_GetTicks64());
                        world.set_config(config);
                        clouds = CloudLayer(config.seed, &world); // Regenerate clouds too
                        view_state = ViewState(); // Reset view
                        need_redraw = true;
                        std::cout << "Regenerating world with seed " << config.seed << "\n";
                        break;
                }
            } else if (event.type == SDL_MOUSEMOTION) {
                mouse_x = event.motion.x;
                mouse_y = event.motion.y;
            } else if (event.type == SDL_MOUSEWHEEL) {
                // Get mouse position for zoom center
                SDL_GetMouseState(&mouse_x, &mouse_y);
                
                // Calculate world coordinates at mouse position before zoom
                float target_lon, target_lat;
                view_state.screen_to_world(mouse_x, mouse_y, WINDOW_WIDTH, WINDOW_HEIGHT, 
                                          target_lon, target_lat);
                
                // Adjust zoom level
                float zoom_factor = (event.wheel.y > 0) ? 1.2f : 0.833f;
                float new_zoom = view_state.zoom * zoom_factor;
                
                // Clamp zoom level (0.5 = zoomed out to see more, 50 = very zoomed in)
                new_zoom = std::clamp(new_zoom, 0.5f, 50.0f);
                
                // Adjust center to keep mouse position stationary
                // The point under the mouse should remain under the mouse after zoom
                float lon_before, lat_before;
                view_state.screen_to_world(mouse_x, mouse_y, WINDOW_WIDTH, WINDOW_HEIGHT,
                                          lon_before, lat_before);
                
                view_state.zoom = new_zoom;
                
                float lon_after, lat_after;
                view_state.screen_to_world(mouse_x, mouse_y, WINDOW_WIDTH, WINDOW_HEIGHT,
                                          lon_after, lat_after);
                
                // Adjust center to compensate for the shift
                view_state.center_lon += (lon_before - lon_after);
                view_state.center_lat += (lat_before - lat_after);
                
                // Clamp center position
                view_state.center_lat = std::clamp(view_state.center_lat, -90.0f, 90.0f);
                while (view_state.center_lon > 180.0f) view_state.center_lon -= 360.0f;
                while (view_state.center_lon < -180.0f) view_state.center_lon += 360.0f;
                
                need_redraw = true;
                std::cout << "Zoom: " << std::fixed << std::setprecision(2) 
                         << view_state.zoom << "x at (" << target_lon << ", " 
                         << target_lat << ")\n";
            }
        }
        
        if (need_redraw) {
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderClear(renderer);
            
            render_world_map(renderer, world, WINDOW_WIDTH, WINDOW_HEIGHT, current_mode, view_state);
            
            // Add cloud overlay if in clouds mode
            if (current_mode == DisplayMode::CLOUDS) {
                render_cloud_overlay(renderer, clouds, WINDOW_WIDTH, WINDOW_HEIGHT, view_state);
            }
            
            need_redraw = false;
            
            std::cout << "World map rendered.\n";
        }
        
        // Always redraw UI overlay
        render_info_panel(renderer, world, mouse_x, mouse_y, 
                         WINDOW_WIDTH, WINDOW_HEIGHT, current_mode, view_state
#ifdef USE_SDL2_TTF
                         , has_text ? &text_renderer : nullptr
#endif
                         );
        
        SDL_RenderPresent(renderer);
        
        // Advance time if not paused
        if (!view_state.time_paused) {
            view_state.current_time += 0.01f; // Advance by ~36 seconds per frame
            if (view_state.current_time >= 24.0f) {
                view_state.current_time -= 24.0f;
            }
            // Redraw if showing insolation (day/night changes)
            if (current_mode == DisplayMode::INSOLATION) {
                need_redraw = true;
            }
        }
        
        SDL_Delay(16); // ~60 FPS
    }
    
    std::cout << "Shutting down...\n" << std::flush;
    
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    
    std::cout << "Shutdown complete.\n" << std::flush;
}

#endif // USE_SDL2

int main() {
    std::cout << "=== RWorld - Living Active World Demo ===\n";
    
    // Create a world with default settings
    WorldConfig config;
    config.seed = 42;
    World world(config);
    
#ifdef USE_SDL2
    std::cout << "\nSDL2 visualization enabled!\n";
    run_sdl_demo(world);
#else
    run_text_demo(world);
#endif
    
    return 0;
}
