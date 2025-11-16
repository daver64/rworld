# RWorld - Living Active World Library

A C++ library for procedural generation of living world environments with environmental parameters including terrain, biomes, temperature, precipitation, and atmospheric conditions.

## Features

- **Procedural Terrain Generation**: Continents, oceans, mountains, and valleys using noise functions
- **Climate Simulation**: Temperature based on latitude and altitude with realistic gradients
- **Biome Classification**: 18+ biome types using Whittaker diagram approach
- **Atmospheric Physics**: Air pressure decreases exponentially with altitude
- **Weather Systems**: Precipitation, humidity, and precipitation type (rain/snow/sleet)
- **Deterministic Generation**: Same seed produces identical worlds
- **Clean API**: Simple coordinate-based queries (longitude, latitude, altitude)
- **Interactive Visualization**: SDL2-based graphical demo with multiple display modes

## SDL2 Visualization Features

The demo application includes an interactive graphical interface when built with SDL2:

- **Multiple Display Modes**:
  - Biomes (default) - Colour-coded biome classification
  - Elevation - Terrain height visualization
  - Temperature - Heat map of world temperatures
  - Precipitation - Rainfall and snowfall distribution

- **Interactive Controls**:
  - `1-4` keys: Switch between display modes
  - `R` key: Regenerate world with random seed
  - `ESC/Q`: Quit application
  - Mouse hover: Shows location details (coordinates, biome, temperature, etc.)

- **Real-time Information Panel**: Displays environmental data for mouse location

## Quick Start

### Building

```bash
# Install SDL2 for graphical visualization (optional but recommended)
sudo apt-get install libsdl2-dev libsdl2-ttf-dev  # Ubuntu/Debian
# or: brew install sdl2 sdl2_ttf                  # macOS

# Build the library
mkdir build && cd build
cmake ..
cmake --build .
```

### Running the Demo

```bash
./world_demo
```

The demo includes:
- **With SDL2 + SDL2_ttf**: Interactive 1200x600 window with colourful visualization and text info
- **With SDL2 only**: Graphical visualisation with colour-coded indicators (no text)
- **Without SDL2**: Text-based output with ASCII map and data tables

## Usage Example

```cpp
#include "rworld/world.h"

int main() {
    // Create a world with a specific seed
    rworld::WorldConfig config;
    config.seed = 12345;
    rworld::World world(config);
    
    // Query environmental parameters
    float lon = 0.0f, lat = 45.0f;  // Location coordinates
    float altitude = world.get_terrain_height(lon, lat);
    
    float temperature = world.get_temperature(lon, lat, altitude);
    float precipitation = world.get_precipitation(lon, lat, altitude);
    rworld::BiomeType biome = world.get_biome(lon, lat, altitude);
    float pressure = world.get_air_pressure(lon, lat, altitude);
    
    return 0;
}
```

## API Overview

### Core Methods

- `float get_terrain_height(longitude, latitude)` - Terrain elevation in meters
- `float get_temperature(longitude, latitude, altitude)` - Temperature in °C
- `BiomeType get_biome(longitude, latitude, altitude)` - Biome classification
- `float get_precipitation(longitude, latitude, altitude)` - Precipitation in mm/year
- `float get_air_pressure(longitude, latitude, altitude)` - Pressure in hPa/millibars
- `float get_humidity(longitude, latitude, altitude)` - Relative humidity (0-1)
- `PrecipitationType get_precipitation_type(...)` - Rain, snow, or sleet

### Coordinate System

- **Longitude**: -180° to 180° (West to East)
- **Latitude**: -90° to 90° (South to North)
- **Altitude**: Meters above sea level

### Biome Types

Cold: Tundra, Taiga, Cold Desert
Temperate: Grassland, Temperate Deciduous Forest, Temperate Rainforest
Tropical: Savanna, Tropical Seasonal Forest, Tropical Rainforest
Dry: Desert, Cold Desert
Special: Ocean, Deep Ocean, Beach, Snow, Ice
Mountain: Mountain Tundra, Mountain Forest, Mountain Peak

## Architecture

The library uses a layered approach:

1. **Noise Layer**: FastNoiseLite generates base terrain and moisture patterns
2. **Environmental Model**: Calculates temperature, pressure based on coordinates
3. **Biome Classification**: Maps environmental parameters to biome types
4. **Facade API**: Simple `World` class coordinates all subsystems

### Design Patterns

- **Facade Pattern**: `World` class provides simple interface to complex subsystems
- **PIMPL Idiom**: Implementation details hidden from public header
- **Strategy Pattern**: Configurable noise parameters and generation rules

## Configuration

Customize world generation through `WorldConfig`:

```cpp
rworld::WorldConfig config;
config.seed = 42;
config.world_scale = 1.0f;
config.equator_temperature = 30.0f;  // °C
config.pole_temperature = -40.0f;     // °C
config.temperature_lapse_rate = 6.5f; // °C per 1000m
config.terrain_frequency = 0.001f;
config.terrain_octaves = 6;

rworld::World world(config);
```

## Dependencies

- **FastNoiseLite**: Single-header noise library (included in `third_party/`)
- **C++17** or later
- **CMake 3.15+**
- **SDL2** (optional, for graphical visualization)
  - Ubuntu/Debian: `sudo apt-get install libsdl2-dev`
  - macOS: `brew install sdl2`
  - The library works without SDL2 (text-mode demo only)

## License

MIT License - See LICENSE file for details

## Future Enhancements

- Chunk-based caching for performance
- Ocean currents and wind patterns
- Seasonal variations
- Rivers and water flow
- Vegetation density
- Mineral/resource distribution
- Dynamic weather simulation
- Batch query API for region generation

## Contributing

Contributions welcome! Please ensure code follows existing style and includes tests.
