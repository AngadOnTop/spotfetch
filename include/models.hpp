#pragma once

#include <string>
#include <vector>

struct Track {
    std::string name;
    std::string artist;
    int plays;

    int progress_seconds;
    int duration_seconds;
};

struct SpotifyStats {
    std::vector<Track> top_tracks;
    Track current_track;
    std::string username;
};

