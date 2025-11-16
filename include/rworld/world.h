#ifndef RWORLD_WORLD_H
#define RWORLD_WORLD_H

#include <cstdint>
#include <memory>
#include <vector>

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
 * Soil type based on particle size and composition
 */
enum class SoilType {
    CLAY,           // Fine particles, poor drainage, high nutrients
    SILT,           // Medium particles, good fertility
    SAND,           // Coarse particles, good drainage, low nutrients
    LOAM,           // Balanced mix, ideal for agriculture
    PEAT,           // Organic-rich, acidic, wetland soils
    ROCKY,          // Minimal soil, mountain/desert regions
    PERMAFROST,     // Frozen soil, arctic regions
    NONE            // Water/ice surfaces
};

/**
 * Data types available for batch queries
 */
enum class DataType {
    TERRAIN_HEIGHT,
    TEMPERATURE,
    TEMPERATURE_AT_TIME,
    BIOME,
    PRECIPITATION,
    CURRENT_PRECIPITATION,
    PRECIPITATION_TYPE,
    AIR_PRESSURE,
    HUMIDITY,
    WIND_SPEED,
    CURRENT_WIND_SPEED,
    WIND_DIRECTION,
    CURRENT_WIND_DIRECTION,
    IS_RIVER,
    RIVER_WIDTH,
    FLOW_ACCUMULATION,
    IS_VOLCANO,
    COAL_DEPOSIT,
    IRON_DEPOSIT,
    OIL_DEPOSIT,
    INSOLATION,
    IS_DAYLIGHT,
    SOLAR_ANGLE,
    VEGETATION_DENSITY,
    SOIL_TYPE,
    SOIL_FERTILITY,
    SOIL_PH,
    ORGANIC_MATTER,
    PRESSURE_AT_LOCATION,
    PRESSURE_GRADIENT,
    IS_STORM_FRONT
};

/**
 * Location for batch queries
 */
struct Location {
    float longitude;
    float latitude;
    float altitude;
    float current_time;  // Used for time-dependent queries
    float detail_level;  // Used for terrain queries
    
    Location(float lon = 0.0f, float lat = 0.0f, float alt = 0.0f, 
             float time = 12.0f, float detail = 1.0f)
        : longitude(lon), latitude(lat), altitude(alt), 
          current_time(time), detail_level(detail) {}
};

/**
 * Results from batch queries
 * Only requested data types will have populated vectors
 */
struct BatchResult {
    std::vector<float> terrain_height;
    std::vector<float> temperature;
    std::vector<float> precipitation;
    std::vector<float> air_pressure;
    std::vector<float> humidity;
    std::vector<float> wind_speed;
    std::vector<float> wind_direction;
    std::vector<float> river_width;
    std::vector<float> flow_accumulation;
    std::vector<float> coal_deposit;
    std::vector<float> iron_deposit;
    std::vector<float> oil_deposit;
    std::vector<float> insolation;
    std::vector<float> solar_angle;
    std::vector<float> vegetation_density;
    std::vector<float> soil_fertility;
    std::vector<float> soil_ph;
    std::vector<float> organic_matter;
    std::vector<float> pressure_at_location;
    std::vector<float> pressure_gradient;
    
    std::vector<BiomeType> biome;
    std::vector<PrecipitationType> precipitation_type;
    std::vector<SoilType> soil_type;
    
    std::vector<bool> is_river;
    std::vector<bool> is_volcano;
    std::vector<bool> is_daylight;
    std::vector<bool> is_storm_front;
    
    size_t count = 0;  // Number of locations queried
};

/**
 * Configuration for world generation
 */
struct WorldConfig {
    uint64_t seed = 12345;
    float world_scale = 1.0f;
    int day_of_year = 80; // Day of year (0-364), default is spring equinox (March 21)
    
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
     * Get temperature at a specific location and time
     * 
     * This includes dynamic effects from:
     * - Base temperature (latitude, altitude, season)
     * - Solar insolation (time of day)
     * - Cloud cover (cooling effect)
     * 
     * @param longitude Longitude in degrees (-180 to 180)
     * @param latitude Latitude in degrees (-90 to 90)
     * @param altitude Altitude in meters
     * @param current_time Time of day in hours (0-24)
     * @return Temperature in degrees Celsius including dynamic effects
     */
    float get_temperature_at_time(float longitude, float latitude, float altitude, float current_time) const;
    
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
     * Get instantaneous precipitation at a location and time
     * 
     * Includes temporal weather variation. Use this for current rain/snow conditions.
     * 
     * @param longitude Longitude in degrees (-180 to 180)
     * @param latitude Latitude in degrees (-90 to 90)
     * @param altitude Altitude in meters
     * @param current_time Time in hours (used for weather system movement)
     * @return Current precipitation rate (0-1, where 1 = heavy rain/snow)
     */
    float get_current_precipitation(float longitude, float latitude, float altitude, float current_time) const;
    
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
     * Get wind speed at a location and time
     * 
     * Includes temporal weather variation for current wind conditions.
     * 
     * @param longitude Longitude in degrees (-180 to 180)
     * @param latitude Latitude in degrees (-90 to 90)
     * @param altitude Altitude in meters
     * @param current_time Time in hours (used for weather system movement)
     * @return Current wind speed in meters per second
     */
    float get_current_wind_speed(float longitude, float latitude, float altitude, float current_time) const;
    
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
     * Get wind direction at a location and time
     * 
     * Includes temporal weather variation for current wind conditions.
     * 
     * @param longitude Longitude in degrees (-180 to 180)
     * @param latitude Latitude in degrees (-90 to 90)
     * @param altitude Altitude in meters
     * @param current_time Time in hours (used for weather system movement)
     * @return Current wind direction in degrees (0-360)
     */
    float get_current_wind_direction(float longitude, float latitude, float altitude, float current_time) const;
    
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
     * Check if there is a volcano at this location
     * 
     * Volcanoes are cone-shaped features scattered across the terrain,
     * more common at higher elevations.
     * 
     * @param longitude Longitude in degrees (-180 to 180)
     * @param latitude Latitude in degrees (-90 to 90)
     * @return true if location is part of a volcano
     */
    bool is_volcano(float longitude, float latitude) const;
    
    /**
     * Get coal deposit concentration at a location
     * 
     * Coal forms in ancient swamps and forested lowlands.
     * Found in sedimentary rocks at moderate depths.
     * 
     * @param longitude Longitude in degrees (-180 to 180)
     * @param latitude Latitude in degrees (-90 to 90)
     * @return Coal concentration (0-1, 0=none, 1=rich deposit)
     */
    float get_coal_deposit(float longitude, float latitude) const;
    
    /**
     * Get iron ore deposit concentration at a location
     * 
     * Iron ore forms in ancient seabeds and volcanic regions.
     * Often found in banded iron formations.
     * 
     * @param longitude Longitude in degrees (-180 to 180)
     * @param latitude Latitude in degrees (-90 to 90)
     * @return Iron ore concentration (0-1, 0=none, 1=rich deposit)
     */
    float get_iron_deposit(float longitude, float latitude) const;
    
    /**
     * Get oil deposit concentration at a location
     * 
     * Oil forms in ancient ocean sediments under specific conditions.
     * Typically found in sedimentary basins.
     * 
     * @param longitude Longitude in degrees (-180 to 180)
     * @param latitude Latitude in degrees (-90 to 90)
     * @return Oil concentration (0-1, 0=none, 1=rich deposit)
     */
    float get_oil_deposit(float longitude, float latitude) const;
    
    /**
     * Get insolation (solar radiation) at a location
     * 
     * Insolation depends on:
     * - Latitude (maximum at equator, minimum at poles)
     * - Time of day (solar angle based on longitude and current_time)
     * - Cloud cover (reduces insolation)
     * - Season (axial tilt, if implemented)
     * 
     * @param longitude Longitude in degrees (-180 to 180)
     * @param latitude Latitude in degrees (-90 to 90)
     * @param current_time Time of day in hours (0-24, where 12.0 is solar noon at 0° longitude)
     * @return Insolation in W/m² (0-1400, 0=night, ~1000=clear day at equator)
     */
    float get_insolation(float longitude, float latitude, float current_time) const;
    
    /**
     * Check if a location is in daylight
     * 
     * @param longitude Longitude in degrees (-180 to 180)
     * @param latitude Latitude in degrees (-90 to 90)
     * @param current_time Time of day in hours (0-24, where 12.0 is solar noon at 0° longitude)
     * @return true if the sun is above the horizon
     */
    bool is_daylight(float longitude, float latitude, float current_time) const;
    
    /**
     * Get the solar angle above horizon at a location
     * 
     * @param longitude Longitude in degrees (-180 to 180)
     * @param latitude Latitude in degrees (-90 to 90)
     * @param current_time Time of day in hours (0-24, where 12.0 is solar noon at 0° longitude)
     * @return Solar elevation angle in degrees (negative = below horizon)
     */
    float get_solar_angle(float longitude, float latitude, float current_time) const;
    
    /**
     * Get vegetation density at a location
     * 
     * Vegetation density is based on temperature, precipitation, and biome type.
     * Useful for determining tree cover, foliage rendering, or resource availability.
     * 
     * @param longitude Longitude in degrees (-180 to 180)
     * @param latitude Latitude in degrees (-90 to 90)
     * @param altitude Altitude in meters
     * @return Vegetation density (0-1, where 0=barren, 1=dense forest/jungle)
     */
    float get_vegetation_density(float longitude, float latitude, float altitude) const;
    
    /**
     * Get soil type at a location
     * 
     * Soil type is determined by climate, terrain, biome, and drainage.
     * 
     * @param longitude Longitude in degrees (-180 to 180)
     * @param latitude Latitude in degrees (-90 to 90)
     * @param altitude Altitude in meters
     * @return SoilType at the location
     */
    SoilType get_soil_type(float longitude, float latitude, float altitude) const;
    
    /**
     * Get soil fertility at a location
     * 
     * Fertility depends on soil type, organic matter, precipitation, and temperature.
     * Higher values indicate better agricultural potential.
     * 
     * @param longitude Longitude in degrees (-180 to 180)
     * @param latitude Latitude in degrees (-90 to 90)
     * @param altitude Altitude in meters
     * @return Fertility rating (0-1, where 0=barren, 1=highly fertile)
     */
    float get_soil_fertility(float longitude, float latitude, float altitude) const;
    
    /**
     * Get soil pH at a location
     * 
     * pH affects nutrient availability and what plants can grow.
     * 
     * @param longitude Longitude in degrees (-180 to 180)
     * @param latitude Latitude in degrees (-90 to 90)
     * @param altitude Altitude in meters
     * @return pH value (typical range 4.0-9.0, where 7.0=neutral)
     */
    float get_soil_ph(float longitude, float latitude, float altitude) const;
    
    /**
     * Get organic matter content in soil
     * 
     * Organic matter improves soil structure, water retention, and fertility.
     * 
     * @param longitude Longitude in degrees (-180 to 180)
     * @param latitude Latitude in degrees (-90 to 90)
     * @param altitude Altitude in meters
     * @return Organic matter content (0-1, where 0=none, 1=peat/very high)
     */
    float get_organic_matter(float longitude, float latitude, float altitude) const;
    
    /**
     * Get atmospheric pressure at a location including weather systems
     * 
     * Pressure varies with altitude and weather patterns (highs/lows).
     * High pressure = clear weather, low pressure = storms.
     * 
     * @param longitude Longitude in degrees (-180 to 180)
     * @param latitude Latitude in degrees (-90 to 90)
     * @param altitude Altitude in meters
     * @param current_time Time in hours (used for moving pressure systems)
     * @return Pressure in millibars/hPa (typical range 950-1050)
     */
    float get_pressure_at_location(float longitude, float latitude, float altitude, float current_time) const;
    
    /**
     * Get pressure gradient magnitude at a location
     * 
     * Large gradients indicate strong winds and potential storm activity.
     * 
     * @param longitude Longitude in degrees (-180 to 180)
     * @param latitude Latitude in degrees (-90 to 90)
     * @param current_time Time in hours
     * @return Pressure gradient in mb per degree (typical range 0-5)
     */
    float get_pressure_gradient(float longitude, float latitude, float current_time) const;
    
    /**
     * Check if location is near a storm front
     * 
     * Fronts occur where pressure gradients are steep, bringing
     * rapid weather changes, precipitation, and strong winds.
     * 
     * @param longitude Longitude in degrees (-180 to 180)
     * @param latitude Latitude in degrees (-90 to 90)
     * @param current_time Time in hours
     * @return true if near a storm front (steep pressure gradient)
     */
    bool is_storm_front(float longitude, float latitude, float current_time) const;
    
    /**
     * Batch query multiple locations efficiently
     * 
     * This method is optimized for querying many locations at once, providing
     * 10-100x better performance than individual queries for bulk operations.
     * 
     * @param locations Vector of Location structs with coordinates and parameters
     * @param data_types Vector of DataType enum values specifying what data to retrieve
     * @return BatchResult containing only the requested data types
     * 
     * Example:
     * ```cpp
     * std::vector<Location> locs = {{0, 45, 0}, {10, 45, 100}, {20, 45, 500}};
     * std::vector<DataType> types = {DataType::TEMPERATURE, DataType::BIOME};
     * BatchResult result = world.batch_query(locs, types);
     * // result.temperature and result.biome are populated
     * ```
     */
    BatchResult batch_query(const std::vector<Location>& locations,
                           const std::vector<DataType>& data_types) const;
    
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

/**
 * Convert SoilType to string name
 */
const char* soil_to_string(SoilType soil);

} // namespace rworld

#endif // RWORLD_WORLD_H
