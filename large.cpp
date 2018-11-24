#define PICOBENCH_IMPLEMENT
#include "include/picobench/picobench.hpp"

#include <algorithm>
#include <random>
#include <numeric>

const size_t ARRAY_SIZE = 200;
const int SAMPLE_SIZE = 400;

struct Obj
{
    double a[50];

    Obj operator-(const Obj& B) const
    {
        Obj Ret;

        std::transform (std::begin(a), std::end(a), std::begin(B.a), std::begin(Ret.a), [](auto a, auto b) {return a - b;});

        return Ret;
    }

    static void ObjMinus (Obj& Ret, const Obj& A, const Obj& B)
    {
        std::transform (std::begin(A.a), std::end(A.a), std::begin(B.a), std::begin(Ret.a), [](auto a, auto b) {return a - b;});
    }

    double Sum() const
    {
        double Sum = std::accumulate(std::begin(a), std::end(a), 0.0);

        return Sum;
    }

    template <class _Fn>
    static Obj RandObj (_Fn _Func)
    {
        Obj Ret;

        std::generate(std::begin(Ret.a), std::end(Ret.a), _Func);

        return Ret;
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

        std::transform (std::begin(A.a), std::end(A.a), std::begin(B.a), std::begin(C.a), [](auto a, auto b) {return a - b;});

        record = record/static_cast<double>(10) + C.Sum ();
    }
}
PICOBENCH (Orig).samples (SAMPLE_SIZE).baseline();

static void Func (picobench::state& s)
{
    auto Pairs = GeneratePairsOfIndices(s.iterations());
    auto Iter = Pairs.begin();

    for (auto _ : s)
    {
        const Obj& A = global[Iter->first];
        const Obj& B = global[Iter->second];
        Iter++;

        Obj C;

        Obj::ObjMinus(C, A, B);

        record = record/static_cast<double>(10) + C.Sum ();
    }
}
PICOBENCH (Func).samples (SAMPLE_SIZE);


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

        record = record/static_cast<double>(10) + C.Sum ();
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

        record = record/static_cast<double>(10) + C.Sum ();
    }
}
PICOBENCH (Slow).samples (SAMPLE_SIZE);

Obj GlobalC;

static void GlobalSlow (picobench::state& s)
{
    auto Pairs = GeneratePairsOfIndices(s.iterations());
    auto Iter = Pairs.begin();

    for (auto _ : s)
    {
        const Obj& A = global[Iter->first];
        const Obj& B = global[Iter->second];
        Iter++;

        GlobalC = A - B;

        record = record/static_cast<double>(10) + GlobalC.Sum ();
    }
}
PICOBENCH (GlobalSlow).samples (SAMPLE_SIZE);


int main (int argc, char* argv[])
{
   //Pregenerate some objects per type tested

   std::random_device rd;
   std::uniform_real_distribution<double> u (-100.0, 100.0);
   std::generate(std::begin(global), std::end(global), [&]()
   {
       return Obj::RandObj ([&](){return u(rd);}); 
   });

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