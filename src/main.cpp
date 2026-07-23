#include "renderer.hpp"

int main() {
    SpotifyStats stats;
    stats.username = "Angad";

    Track track1{
        "FEIN",
        "Travis Scott",
        67,
        0,
        0
    };

    Track track2{
        "Nights Like This",
        "The Kid LAROI",
        54,
        0,
        0
    };

    Track track3{
        "Timeless",
        "The Weeknd",
        49,
        92,
        245
    };

    stats.top_tracks.push_back(track1);
    stats.top_tracks.push_back(track2);
    stats.top_tracks.push_back(track3);

    stats.current_track = track3;

    render_dashboard(stats);

    return 0;
}