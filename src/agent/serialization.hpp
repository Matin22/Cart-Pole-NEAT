#pragma once

#include "genome.hpp"

#include <fstream>
#include <string>
#include <iostream>

namespace neat
{
    bool saveGenome(const Genome& genome, const std::string& filename)
    {
        std::ofstream file(filename, std::ios::binary);
        if (!file.is_open())
        {
            std::cerr << "Failed to open file for writing: " << filename << "\n";
            return false;
        }

        // Write fitness
        file.write(reinterpret_cast<const char*>(&genome.fitness), sizeof(genome.fitness));

        // Write number of nodes
        std::size_t nodeCount = genome.nodes.size();
        file.write(reinterpret_cast<const char*>(&nodeCount), sizeof(nodeCount));

        // Write each node
        for (const auto& node : genome.nodes)
        {
            file.write(reinterpret_cast<const char*>(&node.id), sizeof(node.id));
            file.write(reinterpret_cast<const char*>(&node.type), sizeof(node.type));
            file.write(reinterpret_cast<const char*>(&node.activation), sizeof(node.activation));
        }

        // Write number of connections
        std::size_t connCount = genome.connections.size();
        file.write(reinterpret_cast<const char*>(&connCount), sizeof(connCount));

        // Write each connection
        for (const auto& conn : genome.connections)
        {
            file.write(reinterpret_cast<const char*>(&conn.inNode), sizeof(conn.inNode));
            file.write(reinterpret_cast<const char*>(&conn.outNode), sizeof(conn.outNode));
            file.write(reinterpret_cast<const char*>(&conn.weight), sizeof(conn.weight));
            file.write(reinterpret_cast<const char*>(&conn.enabled), sizeof(conn.enabled));
            file.write(reinterpret_cast<const char*>(&conn.innovation), sizeof(conn.innovation));
        }

        file.close();
        std::cout << "Saved genome to: " << filename << "\n";
        return true;
    }

    bool loadGenome(Genome& genome, const std::string& filename)
    {
        std::ifstream file(filename, std::ios::binary);
        if (!file.is_open())
        {
            std::cerr << "Failed to open file for reading: " << filename << "\n";
            return false;
        }

        // Clear existing data
        genome.nodes.clear();
        genome.connections.clear();

        // Read fitness
        file.read(reinterpret_cast<char*>(&genome.fitness), sizeof(genome.fitness));

        // Read number of nodes
        std::size_t nodeCount = 0;
        file.read(reinterpret_cast<char*>(&nodeCount), sizeof(nodeCount));

        // Read each node
        genome.nodes.reserve(nodeCount);
        for (std::size_t i = 0; i < nodeCount; i++)
        {
            Node node;
            file.read(reinterpret_cast<char*>(&node.id), sizeof(node.id));
            file.read(reinterpret_cast<char*>(&node.type), sizeof(node.type));
            file.read(reinterpret_cast<char*>(&node.activation), sizeof(node.activation));
            genome.nodes.push_back(node);
        }

        // Read number of connections
        std::size_t connCount = 0;
        file.read(reinterpret_cast<char*>(&connCount), sizeof(connCount));

        // Read each connection
        genome.connections.reserve(connCount);
        for (std::size_t i = 0; i < connCount; i++)
        {
            Connection conn;
            file.read(reinterpret_cast<char*>(&conn.inNode), sizeof(conn.inNode));
            file.read(reinterpret_cast<char*>(&conn.outNode), sizeof(conn.outNode));
            file.read(reinterpret_cast<char*>(&conn.weight), sizeof(conn.weight));
            file.read(reinterpret_cast<char*>(&conn.enabled), sizeof(conn.enabled));
            file.read(reinterpret_cast<char*>(&conn.innovation), sizeof(conn.innovation));
            genome.connections.push_back(conn);
        }

        file.close();
        std::cout << "Loaded genome from: " << filename << " (fitness: " << genome.fitness << ")\n";
        return true;
    }

    bool genomeFileExists(const std::string& filename)
    {
        std::ifstream file(filename);
        return file.good();
    }
}
