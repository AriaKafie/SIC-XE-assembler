
#include <chrono>
#include <iostream>
#include <sstream>

#include "bitboard.h"
#include "movegen.h"
#include "position.h"
#include "types.h"
#include "uci.h"

uint64_t curr_time_millis() {
    return std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now()).time_since_epoch().count();
}

static uint64_t nodes;

template<Color SideToMove>
void search(int depth)
{
    if (depth == 0)
    {
        nodes++;
        return;
    }

    MoveList<SideToMove> moves;

    if (depth == 1)
    {
        nodes += moves.size();
        return;
    }

    for (Move m : moves)
    {
        do_move<SideToMove>(m);
        search<!SideToMove>(depth -1);
        undo_move<SideToMove>(m);
    }
}

template<Color SideToMove>
void PerfT(int depth)
{
    uint64_t total_ms = 0, total_nodes = 0;

    MoveList<SideToMove> moves;

    for (Move m : moves)
    {
        nodes = 0;

        auto start = curr_time_millis();

        do_move<SideToMove>(m);
        search<!SideToMove>(depth - 1);
        undo_move<SideToMove>(m);

        auto end = curr_time_millis();

        total_ms += end - start;

        std::cout << move_to_uci(m) << ": " << nodes << std::endl;

        total_nodes += nodes;
    }

    std::cout << "\nnodes searched: " << total_nodes << "\nin " << total_ms << " ms\n" << std::endl;
}

void perft(std::istringstream& is)
{
    int depth;
    is >> depth;
    
    if (Position::white_to_move()) PerfT<WHITE>(depth);
    else                           PerfT<BLACK>(depth);
}

void position(std::istringstream& is)
{
    std::string fen = "", token;

    is >> token;

    while (is >> token)
        fen += token + " ";

    Position::set(fen);
}

int main() {

    Bitboards::init();
    Position::set("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

    std::string cmd, token;

    do
    {
        std::getline(std::cin, cmd);

        std::istringstream is(cmd);
        is >> token;

        if (token == "perft")    perft(is);
        if (token == "position") position(is);
        if (token == "d")        std::cout << Position::to_string() << std::endl;
        
    } while (cmd != "quit");
}
