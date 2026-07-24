#pragma once

#include "models.hpp"

void render_dashboard(const SpotifyStats& stats);
void render_progress_bar(int progress_seconds, int duration_seconds);