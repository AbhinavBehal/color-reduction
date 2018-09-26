#include <array>
#include <iostream>
#include <limits>
#include <random>
#include <utility>
#include <vector>

#include <SFML/Graphics.hpp>

class Cluster {
  private:
    std::array<int, 3> sum;
    int size;

  public:
    sf::Color color;

    Cluster(const sf::Color &c) : color{c}, sum{}, size{0} {}

    void add_color(const sf::Color &c) {
        sum[0] += c.r;
        sum[1] += c.g;
        sum[2] += c.b;
        ++size;
    }

    bool update() {
        if (size > 0) {
            auto prev = color;
            color = {sum[0] / size, sum[1] / size, sum[2] / size};
            sum = {0, 0, 0};
            size = 0;
            return color != prev;
        }
        return false;
    }
};

sf::Color read_color(const sf::Uint8 *pixels, int x, int y, int width) {
    return {pixels[4 * (x + y * width) + 0], pixels[4 * (x + y * width) + 1],
            pixels[4 * (x + y * width) + 2]};
}

int closest_index(const sf::Color &color,
                  const std::vector<Cluster> &clusters) {
    int min_dist = std::numeric_limits<int>::max();
    int index = 0;
    for (auto it = clusters.begin(); it != clusters.end(); ++it) {
        int dr = it->color.r - color.r;
        int dg = it->color.g - color.g;
        int db = it->color.b - color.b;
        int dist = dr * dr + dg * dg + db * db;
        if (dist < min_dist) {
            min_dist = dist;
            index = std::distance(clusters.begin(), it);
        }
    }
    return index;
}

sf::Image reduce_colors(const sf::Image &image, int num_colors) {
    std::mt19937 rng;
    rng.seed(std::random_device()());

    const sf::Uint8 *pixels = image.getPixelsPtr();
    const int width = image.getSize().x;
    const int height = image.getSize().y;
    // in case it takes too long to converge
    const int max_iterations = 100;

    std::uniform_int_distribution<std::mt19937::result_type> width_dist(0,
                                                                        width);
    std::uniform_int_distribution<std::mt19937::result_type> height_dist(
        0, height);

    std::vector<int> indices(width * height, 0);

    std::vector<Cluster> clusters;
    clusters.reserve(num_colors);

    for (int i = 0; i < num_colors; ++i) {
        // generate random points to start the clusters off
        int x = width_dist(rng);
        int y = height_dist(rng);
        clusters.emplace_back(read_color(pixels, x, y, width));
    }

    for (int iter = 0; iter < max_iterations; ++iter) {
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                auto color = read_color(pixels, x, y, width);
                int index = closest_index(color, clusters);
                clusters[index].add_color(color);
                indices[x + y * width] = index;
            }
        }

        bool changed = false;
        for (auto &cluster : clusters) {
            if (cluster.update())
                changed = true;
        }
        if (!changed)
            break;
    }

    sf::Image out;
    out.create(width, height);
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int index = indices[x + y * width];
            out.setPixel(x, y, clusters[index].color);
        }
    }
    return out;
}

int main(int argc, char **argv) {
    if (argc != 4) {
        std::cout << "Usage: <input_file> <number of colors> "
                     "<output_file>"
                  << std::endl;
        return -1;
    }

    sf::Image input_img;
    if (!input_img.loadFromFile(argv[1]))
        return -1;

    sf::Image output_img = reduce_colors(input_img, std::stoi(argv[2]));
    output_img.saveToFile(argv[3]);
    return 0;
}
