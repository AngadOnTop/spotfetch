#include "mock_data.hpp"

SpotifyStats create_mock_stats() {
    SpotifyStats stats;
    stats.username = "Angad";

    Track track1{
        "FEIN",
        "Travis Scott",
        67,
        0,
        0,
        false
    };

    Track track2{
        "Nights Like This",
        "The Kid LAROI",
        54,
        0,
        0,
        false
    };

    Track track3{
        "Timeless",
        "The Weeknd",
        49,
        92,
        245,
        true
    };

    stats.top_tracks = {
        track1,
        track2,
        track3
    };

    stats.current_track = track3;

    return stats;
}