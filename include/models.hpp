#pragma once

#include <string>
#include <vector>

struct Track {
    std::string name;
    std::string artist;
    int plays;
};

struct SpotifyStats {
    std::vector<Track> top_Tracks;
    std::string userName;
};

