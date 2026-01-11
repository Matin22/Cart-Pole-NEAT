#pragma once

#include <map>
#include <utility>

namespace neat
{
    class InnovationTracker
    {
        public:
            int getInnovation(int inNode, int outNode)
            {
                const Key key{inNode, outNode};
                auto it = innovations.find(key);
                if (it != innovations.end())
                    return it->second;
                
                int id = nextInnovation++;
                innovations[key] = id;
                return id;
            }

            void reset (int startFrom = 0)
            {
                innovations.clear();
                nextInnovation = startFrom;
            }

            int size() { return static_cast<int>(innovations.size()); }

        private:
            using Key = std::pair<int, int>;
            std::map<Key, int> innovations;
            int nextInnovation = 0;
    };
}