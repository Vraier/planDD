#include "order_visualization.h"

#include <stdlib.h>

#include <algorithm>
#include <fstream>
#include <iostream>

#include "logging.h"

namespace visualize {

static const double FIXED_LAYER_DIST = 100.0;
static const double SCALED_LAYER_DIST = 1.0;
static const double RADIUS = 0.3;

void draw_circle(std::ofstream &file, double x, double y, double r) {
    file << "<circle cx=\"" << x << "\" cy=\"" << y << "\" r=\"" << r << "\" fill=\"black\" />\n";
}

void draw_line(std::ofstream &file, double x1, double y1, double x2, double y2, double r, double g, double b,
               double w) {
    file << "<line x1=\"" << x1 << "\" y1=\"" << y1 << "\" x2=\"" << x2 << "\" y2=\"" << y2 << "\" style=\"stroke:rgb("
         << r << "," << g << "," << b << ");stroke-width:" << w << "\" />\n";
}

double randMToN(double M, double N) { return M + (rand() / (RAND_MAX / (N - M))); }

void visualize_var_order(std::vector<int> &var_order, std::vector<planning_logic::logic_primitive> &primitives) {
    srand(1337);

    // calculate ppositions
    std::vector<std::pair<double, double>> var_positions(var_order.size());
    std::vector<std::pair<double, double>> primitive_positions(primitives.size());

    // calculate var positions (on x axis, y=0)
    for (int i = 0; i < var_order.size(); i++) {
        var_positions[i] = std::make_pair(i, 0);
    }

    // calculate inverse permutation
    std::vector<int> var_to_pos(var_order.size());
    for (int i = 0; i < var_order.size(); i++) {
        var_to_pos[var_order[i]] = i;
    }

    double total_span = 0;
    // calculate upper layer (clauses) and lines
    for (int i = 0; i < primitives.size(); i++) {
        double average_pos = 0;
        double min_span = var_order.size() + 2;
        double max_span = 0;
        double span;

        for (int a = 0; a < primitives[i].get_affected_variables().size(); a++) {
            double var_pos = var_to_pos[primitives[i].get_affected_variables()[a]];
            min_span = std::min(min_span, var_pos);
            max_span = std::max(max_span, var_pos);
            average_pos += var_pos;
        }

        // calculate hight/width from span and average_pos
        average_pos /= primitives[i].get_affected_variables().size();
        average_pos += randMToN(-0.2, 0.2);  // random pertubation to avoid overlap
        span = max_span - min_span;

        primitive_positions[i] = std::make_pair(average_pos, -span * SCALED_LAYER_DIST);
        total_span += span;
    }

    LOG_MESSAGE(log_level::info) << "Average span: " << total_span/ primitives.size();

    // callculate max width/height of all circles, shift the image and calculate its dimensions;
    double min_x = var_positions[0].first;
    double max_x = var_positions[0].first;
    double min_y = var_positions[0].second;
    double max_y = var_positions[0].second;

    for (int i = 0; i < var_positions.size(); i++) {
        min_x = std::min(min_x, var_positions[i].first);
        max_x = std::max(max_x, var_positions[i].first);
        min_y = std::min(min_y, var_positions[i].second);
        max_y = std::max(max_y, var_positions[i].second);
    }

    for (int i = 0; i < primitive_positions.size(); i++) {
        min_x = std::min(min_x, primitive_positions[i].first);
        max_x = std::max(max_x, primitive_positions[i].first);
        min_y = std::min(min_y, primitive_positions[i].second);
        max_y = std::max(max_y, primitive_positions[i].second);
    }

    // shift image to upper left corner
    for (int i = 0; i < var_positions.size(); i++) {
        var_positions[i] = std::make_pair(var_positions[i].first - min_x + 1, var_positions[i].second - min_y + 1);
    }

    for (int i = 0; i < primitive_positions.size(); i++) {
        //std::cout << "p: " << primitive_positions[i].first << " " << primitive_positions[i].second << std::endl;
        primitive_positions[i] =
            std::make_pair(primitive_positions[i].first - min_x + 1, primitive_positions[i].second - min_y + 1);
        std::cout << "p: " << primitive_positions[i].first << " " << primitive_positions[i].second << std::endl;

    }

    // draw the image
    std::ofstream file;
    file.open("var_order.svg");

    file << "<?xml version=\"1.0\" standalone=\"no\"?>\n<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG "
            "1.1//EN\"\n\"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd\">\n<html>\n<body>\n";
    file << "<svg width=\"" << max_x - min_x + 2 << "\" height=\"" << max_y - min_y + 2
         << "\" version=\"1.1\" xmlns=\"http://www.w3.org/2000/svg\">\n";

    // draw variables
    for (int i = 0; i < var_positions.size(); i++) {
        std::cout << "v: " << var_positions[i].first << " " << var_positions[i].second << std::endl;
        draw_circle(file, var_positions[i].first, var_positions[i].second, RADIUS);
    }

    // draw clauses with lines
    for (int i = 0; i < primitives.size(); i++) {
        std::cout << "p: " << primitive_positions[i].first << " " << primitive_positions[i].second << std::endl;
        draw_circle(file, primitive_positions[i].first, primitive_positions[i].second, RADIUS);

        for (int a = 0; a < primitives[i].get_affected_variables().size(); a++) {
            int neighbour = primitives[i].get_affected_variables()[a];
            // determine colour of line (colour is dependent on the span of a clause, capped at 100)
            double diff = (std::min(primitive_positions[i].second/SCALED_LAYER_DIST, 100.0) / 100) * 255;

            draw_line(file, primitive_positions[i].first, primitive_positions[i].second, 
                            var_positions[neighbour].first, var_positions[neighbour].second, 
                            diff, 255.0 - diff, 0, 0.1);
        }
    }

    file << "</svg>\n</body>\n</html>";
    file.close();
}
}  // namespace visualize