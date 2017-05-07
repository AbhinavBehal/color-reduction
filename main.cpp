#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>

int indexOfMinDist(int r, int g, int b, int rm[], int bm[], int gm[], int k)
{
    int lowestDist = INT_MAX;
    int lowestIndex = 0;
    for(int i = 0; i < k; ++i)
    {
        int dr = rm[i] - r;
        int dg = gm[i] - g;
        int db = bm[i] - b;
        int currentDist = dr*dr + dg*dg + db*db;
        if(currentDist < lowestDist)
        {
            lowestDist = currentDist;
            lowestIndex = i;
        }
    }
    return lowestIndex;
}

void segmentImage(sf::Image &im, int k, int iter)
{
    int rm[k];
    int gm[k];
    int bm[k];

    int width = im.getSize().x;
    int height = im.getSize().y;
    int maxIterations = iter;
    int currentIteration = 0;

    std::vector<int> indices(width*height, 0);
    const sf::Uint8 *colors = im.getPixelsPtr();

    for(int i = 0; i < k; ++i)
    {
        int px = rand() % width;///Generate random points to start the means off
        int py = rand() % height;

        rm[i] = colors[4*(px + py*width) + 0];
        gm[i] = colors[4*(px + py*width) + 1];
        bm[i] = colors[4*(px + py*width) + 2];
    }
    while(currentIteration++ < maxIterations)
    {
        std::vector<std::vector<int>> sums(k, std::vector<int>(3, 0));
        std::vector<int> clusterSize(k, 0);
        for(int y = 0; y < height; ++y)
        {
            for(int x = 0; x < width; ++x)
            {
                int r = colors[4*(x + y*width) + 0];
                int g = colors[4*(x + y*width) + 1];
                int b = colors[4*(x + y*width) + 2];

                int index = indexOfMinDist(r, g, b, rm, bm, gm, k);///Loop through the means and find the index
                                                                   ///which gives the lowest distance value
                sums[index][0] += r;
                sums[index][1] += g;
                sums[index][2] += b;
                clusterSize[index]++;

                if(currentIteration == maxIterations - 1)///If it's the final iteration
                {
                    indices[x + y*width] = index; ///Add to the indices vector to save having to calculate indices again
                }
            }
        }
        for(int i = 0; i < k; ++i)///Recalculate cluster means by averaging corresponding pixel values
        {
            int currentSize = clusterSize.at(i);
            if(currentSize > 0)
            {
                rm[i] = sums[i][0] / currentSize;
                gm[i] = sums[i][1] / currentSize;
                bm[i] = sums[i][2] / currentSize;
            }
        }
    }

    sf::Image out;
    out.create(width, height);
    for(int y = 0; y < height; ++y)
    {
        for(int x = 0; x < width; ++x)
        {
            int index = indices[x + y*width];
            out.setPixel(x, y, sf::Color(rm[index], gm[index], bm[index]));
        }
    }
    im = out;
}

int main(int argc, char** argv)
{
    srand(time(0));
    sf::err().rdbuf(0);
    if(argc != 5) {
        std::cout << "Usage: <input filename> <iterations> <number of colors> <output filename>" << std::endl;
        return -1;
    }
    std::string filename = argv[1];
    sf::Image im;
    if(!im.loadFromFile(filename)) {
        std::cout << filename << " not found" << std::endl;
        return -1;
    }

    segmentImage(im, atoi(argv[3]), atoi(argv[2]));

    im.saveToFile(argv[4]);

    return 0;
}
