#ifndef RWORLD_WORLD_H
#define RWORLD_WORLD_H

#include <cstdint>
#include <memory>

namespace rworld {

/**
 * Biome types based on temperature and moisture levels
 */
enum class BiomeType {
    // Cold biomes
    TUNDRA,
    TAIGA,
    
    // Temperate biomes
    GRASSLAND,
    TEMPERATE_DECIDUOUS_FOREST,
    TEMPERATE_RAINFOREST,
    
    // Warm/Hot biomes
    SAVANNA,
    TROPICAL_SEASONAL_FOREST,
    TROPICAL_RAINFOREST,
    
    // Dry biomes
    COLD_DESERT,
    DESERT,
    
    // Special biomes
    OCEAN,
    DEEP_OCEAN,
    BEACH,
    SNOW,
    ICE,
    
    // Mountain variants
    MOUNTAIN_TUNDRA,
    MOUNTAIN_FOREST,
    MOUNTAIN_PEAK
};

/**
 * Precipitation type based on temperature
 */
enum class PrecipitationType {
    NONE,
    RAIN,
    SNOW,
    SLEET
};

/**
 * Configuration for world generation
 */
struct WorldConfig {
    uint64_t seed = 12345;
    float world_scale = 1.0f;
    
    // Temperature parameters (Celsius)
    float equator_temperature = 30.0f;
    float pole_temperature = -40.0f;
    float temperature_lapse_rate = 6.5f; // °C per 1000m altitude
    
    // Terrain parameters
    float sea_level = 0.0f;
    float max_terrain_height = 8848.0f; // Mt. Everest height in meters
    
    // Noise parameters
    float terrain_frequency = 0.001f;
    int terrain_octaves = 6;
    float terrain_lacunarity = 2.0f;
    float terrain_gain = 0.5f;
    
    float moisture_frequency = 0.002f;
    int moisture_octaves = 4;
};

/**
 * World - A living, active world generator
 * 
 * Provides procedural generation of environmental parameters including
 * terrain, biomes, temperature, precipitation, and atmospheric conditions.
 * 
 * Coordinates:
 * - Longitude: -180 to 180 degrees (West to East)
 * - Latitude: -90 to 90 degrees (South to North)  
 * - Altitude: meters above sea level
 */
class World {
public:
    /**
     * Construct a world with default configuration
     */
    World();
    
    /**
     * Construct a world with custom configuration
     */
    explicit World(const WorldConfig& config);
    
    ~World();
    
    // Prevent copying (noise generators are not copyable)
    World(const World&) = delete;
    World& operator=(const World&) = delete;
    
    // Allow moving
    World(World&&) noexcept;
    World& operator=(World&&) noexcept;
    
    /**
     * Get the biome at a specific location
     * 
     * @param longitude Longitude in degrees (-180 to 180)
     * @param latitude Latitude in degrees (-90 to 90)
     * @param altitude Altitude in meters (use get_terrain_height if unknown)
     * @return BiomeType at the location
     */
    BiomeType get_biome(float longitude, float latitude, float altitude) const;
    
    /**
     * Get temperature at a specific location
     * 
     * Temperature is calculated based on:
     * - Latitude (warmer at equator, colder at poles)
     * - Altitude (decreases with height)
     * 
     * @param longitude Longitude in degrees (-180 to 180)
     * @param latitude Latitude in degrees (-90 to 90)
     * @param altitude Altitude in meters
     * @return Temperature in degrees Celsius
     */
    float get_temperature(float longitude, float latitude, float altitude) const;
    
    /**
     * Get terrain height at a location (ignoring altitude parameter)
     * 
     * @param longitude Longitude in degrees (-180 to 180)
     * @param latitude Latitude in degrees (-90 to 90)
     * @param detail_level Detail multiplier for terrain resolution (1.0 = normal, higher = finer detail)
     * @return Height in meters above sea level (negative for underwater)
     */
    float get_terrain_height(float longitude, float latitude, float detail_level = 1.0f) const;
    
    /**
     * Get precipitation level at a location
     * 
     * @param longitude Longitude in degrees (-180 to 180)
     * @param latitude Latitude in degrees (-90 to 90)
     * @param altitude Altitude in meters
     * @return Precipitation in mm per year
     */
    float get_precipitation(float longitude, float latitude, float altitude) const;
    
    /**
     * Get the type of precipitation at a location
     * 
     * @param longitude Longitude in degrees (-180 to 180)
     * @param latitude Latitude in degrees (-90 to 90)
     * @param altitude Altitude in meters
     * @return Type of precipitation
     */
    PrecipitationType get_precipitation_type(float longitude, float latitude, float altitude) const;
    
    /**
     * Get atmospheric pressure at a location
     * 
     * Pressure decreases exponentially with altitude
     * 
     * @param longitude Longitude in degrees (-180 to 180)
     * @param latitude Latitude in degrees (-90 to 90)
     * @param altitude Altitude in meters
     * @return Pressure in millibars (hPa)
     */
    float get_air_pressure(float longitude, float latitude, float altitude) const;
    
    /**
     * Get humidity at a location (relative humidity 0-1)
     * 
     * @param longitude Longitude in degrees (-180 to 180)
     * @param latitude Latitude in degrees (-90 to 90)
     * @param altitude Altitude in meters
     * @return Relative humidity (0.0 to 1.0)
     */
    float get_humidity(float longitude, float latitude, float altitude) const;
    
    /**
     * Get wind speed at a location
     * 
     * Wind is influenced by pressure gradients, latitude (Coriolis effect),
     * and terrain features
     * 
     * @param longitude Longitude in degrees (-180 to 180)
     * @param latitude Latitude in degrees (-90 to 90)
     * @param altitude Altitude in meters
     * @return Wind speed in meters per second
     */
    float get_wind_speed(float longitude, float latitude, float altitude) const;
    
    /**
     * Get wind direction at a location
     * 
     * Direction in degrees where 0° is North, 90° is East, 180° is South, 270° is West
     * 
     * @param longitude Longitude in degrees (-180 to 180)
     * @param latitude Latitude in degrees (-90 to 90)
     * @param altitude Altitude in meters
     * @return Wind direction in degrees (0-360)
     */
    float get_wind_direction(float longitude, float latitude, float altitude) const;
    
    /**
     * Check if there is a river at this location
     * 
     * Rivers flow from high elevation to low, following terrain gradients.
     * More common in areas with high precipitation.
     * 
     * @param longitude Longitude in degrees (-180 to 180)
     * @param latitude Latitude in degrees (-90 to 90)
     * @return true if location has a river
     */
    bool is_river(float longitude, float latitude) const;
    
    /**
     * Get river width at a location
     * 
     * Width increases with upstream drainage area and precipitation.
     * Returns 0 if no river present.
     * 
     * @param longitude Longitude in degrees (-180 to 180)
     * @param latitude Latitude in degrees (-90 to 90)
     * @return River width in meters (0 if no river)
     */
    float get_river_width(float longitude, float latitude) const;
    
    /**
     * Get flow accumulation at a location
     * 
     * Represents the upstream drainage area contributing water to this point.
     * Higher values indicate major river channels.
     * 
     * @param longitude Longitude in degrees (-180 to 180)
     * @param latitude Latitude in degrees (-90 to 90)
     * @return Flow accumulation value (0-1, normalized)
     */
    float get_flow_accumulation(float longitude, float latitude) const;
    
    /**
     * Update the world configuration
     * This will reset internal noise generators
     */
    void set_config(const WorldConfig& config);
    
    /**
     * Get the current configuration
     */
    const WorldConfig& get_config() const;
    
private:
    class Impl;
    std::unique_ptr<Impl> pimpl_;
};

/**
 * Convert BiomeType to string name
 */
const char* biome_to_string(BiomeType biome);

} // namespace rworld

#endif // RWORLD_WORLD_H
