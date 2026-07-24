#include "models.hpp"

#include <vector>
#include <cstddef>
class SpotifyClient {
public:
    explicit SpotifyClient(std::string token);

    UserProfile get_profile();

    std::vector<Track> get_top_tracks(
        std::size_t limit = 5
    );

    Track get_current_track();

    void set_access_token(std::string token);

private:
    std::string access_token_;
};