#define PICOBENCH_IMPLEMENT
#include "include/picobench/picobench.hpp"

#include <algorithm>
#include <random>

const size_t ARRAY_SIZE = 200;
const int SAMPLE_SIZE = 400;

struct Obj
{
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;
    double w = 0.0;

    Obj operator-(const Obj& RHS) const
    {
        return {x - RHS.x, y - RHS.y, z - RHS.z, w - RHS.w};
    }
};

Obj global[ARRAY_SIZE];
double record = 0.0;

std::vector<std::pair<size_t, size_t>> GeneratePairsOfIndices(int NumberOfIterations)
{
    std::vector<std::pair<size_t, size_t>> Pairs (NumberOfIterations);

    std::random_device rd;
    std::uniform_int_distribution<size_t> u (0, ARRAY_SIZE - 1);

    std::generate(std::begin(Pairs), std::end(Pairs), [&u, &rd](){return std::make_pair(u(rd), u(rd));});

    return Pairs;
}


static void Orig (picobench::state& s)
{
    auto Pairs = GeneratePairsOfIndices(s.iterations());
    auto Iter = Pairs.begin();

    for (auto _ : s)
    {
        const Obj& A = global[Iter->first];
        const Obj& B = global[Iter->second];
        Iter++;

        Obj C;

        C.x = A.x - B.x;
        C.y = A.y - B.y;
        C.z = A.z - B.z;
        C.w = A.w - B.w;

        record = record/static_cast<double>(10) + C.x + C.y + C.z + C.w;
    }
}
PICOBENCH (Orig).samples (SAMPLE_SIZE).baseline();


static void Normal (picobench::state& s)
{
    auto Pairs = GeneratePairsOfIndices(s.iterations());
    auto Iter = Pairs.begin();

    for (auto _ : s)
    {
        const Obj& A = global[Iter->first];
        const Obj& B = global[Iter->second];
        Iter++;

        Obj C = A - B;

        record = record/static_cast<double>(10) + C.x + C.y + C.z + C.w;
    }
}
PICOBENCH (Normal).samples (SAMPLE_SIZE);


static void Slow (picobench::state& s)
{
    auto Pairs = GeneratePairsOfIndices(s.iterations());
    auto Iter = Pairs.begin();

    for (auto _ : s)
    {
        const Obj& A = global[Iter->first];
        const Obj& B = global[Iter->second];
        Iter++;

        Obj C;

        C = A - B;

        record = record/static_cast<double>(10) + C.x + C.y + C.z + C.w;
    }
}
PICOBENCH (Slow).samples (SAMPLE_SIZE);


int main (int argc, char* argv[])
{
   //Pregenerate some objects per type tested

   std::random_device rd;
   std::uniform_real_distribution<double> u (-100.0, 100.0);
   std::generate(std::begin(global), std::end(global), [&](){return Obj{u(rd),u(rd),u(rd),u(rd)}; });

   picobench::runner runner;
   
   runner.parse_cmd_line (argc, argv);
   
   if (runner.should_run ())
   {
      runner.run_benchmarks ();
      auto report = runner.generate_report ();

      //report.to_text (std::cout); // Default
      report.to_text_concise (std::cout);
   }
    
   std::cout << record << std::endl;
}