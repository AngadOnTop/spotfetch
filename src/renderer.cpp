#include <iostream>

#include "renderer.hpp"

void render_dashboard(const SpotifyStats& stats) {
    std::cout << "SPOTFETCH\n";
    std::cout << "User: " << stats.username << '\n';

    if (stats.current_track.is_playing) {
        std::cout << "\n▶ Now Playing:\n";
    } else {
        std::cout << "\n⏸ Paused:\n";
    }

    std::cout << stats.current_track.name
              << " by "
              << stats.current_track.artist
              << '\n';

    render_progress_bar(
        stats.current_track.progress_seconds,
        stats.current_track.duration_seconds
    );

    std::cout << "\nTop Tracks:\n";

    for (std::size_t i = 0; i < stats.top_tracks.size(); i++) {
        const Track& track = stats.top_tracks[i];

        std::cout << i + 1 << ". "
                  << track.name << " by "
                  << track.artist << " - "
                  << track.plays << " plays\n";
    }
}

void render_time(int total_seconds) {
    int minutes = total_seconds / 60;
    int seconds = total_seconds % 60;

    std::cout << minutes << ':';

    if (seconds < 10) {
        std::cout << '0';
    }

    std::cout << seconds;
}

void render_progress_bar(int progress_seconds, int duration_seconds) {
    if (duration_seconds <= 0) {
        std::cout << "[--------------------]\n";
        return;
    }

    double ratio =
        static_cast<double>(progress_seconds) / duration_seconds;

    int bar_width = 20;
    int filled = static_cast<int>(ratio * bar_width);

    if (filled < 0) {
        filled = 0;
    }

    if (filled > bar_width) {
        filled = bar_width;
    }

    std::cout << '[';

    for (int i = 0; i < filled; i++) {
        std::cout << '#';
    }

    for (int i = filled; i < bar_width; i++) {
        std::cout << '-';
    }

    std::cout << "] ";

    render_time(progress_seconds);

    std::cout << " / ";

    render_time(duration_seconds);

    std::cout << '\n';
}