#include <iostream>

#define PICOBENCH_IMPLEMENT
#include "include/picobench/picobench.hpp"

#include <random>
#include <vector>
#include <limits>
#include <memory>
#undef min
#undef max

const size_t ARRAY_SIZE = 200;

template<typename T> 
struct dependent_false : std::false_type {};


template<typename T>
struct BenchmarkHelper
{
   static constexpr T MIN = std::numeric_limits<T>::min ();
   static constexpr T MAX = std::numeric_limits<T>::max ();
};


template<typename T, T const& MIN = BenchmarkHelper<T>::MIN, T const& MAX = BenchmarkHelper<T>::MAX>
class Benchmark
{
public:

   using decimal = T;

private:

   struct Obj
   {
      decimal x = 0;
      decimal y = 0;
      decimal z = 0;
      decimal w = 0;

      Obj operator-(const Obj& RHS) const
      {
         return {x - RHS.x, y - RHS.y, z - RHS.z, w - RHS.w};
      }
   };

   static std::vector<Obj> PredeterminedObjects;


   static std::vector<std::pair<size_t, size_t>> GeneratePairsOfIndices (int NumberOfPairs)
   {
      std::random_device rd;
      std::uniform_int_distribution<size_t> u(0, ARRAY_SIZE - 1);

      std::vector<std::pair<size_t,size_t>> Pairs;

      Pairs.resize(NumberOfPairs);

      std::generate(std::begin(Pairs), std::end(Pairs), [&u, &rd](){return std::make_pair(u(rd), u(rd));});

      return Pairs;
   }

public:


   static decimal record;

   static void GeneratePredeterminedObjects ()
   {
      std::random_device rd;
      PredeterminedObjects.resize(ARRAY_SIZE);

      if constexpr (std::is_floating_point_v<decimal>)
      {
         std::uniform_real_distribution<decimal> u(MIN, MAX);
         std::generate(std::begin(PredeterminedObjects), std::end (PredeterminedObjects), [&u, &rd](){return Obj{u(rd), u(rd), u(rd), u(rd)};});

      }
      else if constexpr (std::is_integral_v<decimal>)
      {
         std::uniform_int_distribution<decimal> u(MIN, MAX);
         std::generate(std::begin(PredeterminedObjects), std::end (PredeterminedObjects), [&u, &rd](){return Obj{u(rd), u(rd), u(rd), u(rd)};});
      }
      else
      {
         static_assert(!std::is_floating_point_v<decimal> && !std::is_integral_v<decimal>, "Please use an arithmetic type.");
      }
   }

   static void Orig (picobench::state& s)
   {
      auto Pairs = GeneratePairsOfIndices(s.iterations());
      auto Iter = Pairs.begin();

      for (auto _ : s)
      {
         const Obj& A = PredeterminedObjects[Iter->first];
         const Obj& B = PredeterminedObjects[Iter->second];
         Iter++;

         Obj C;

         C.x = A.x - B.x;
         C.y = A.y - B.y;
         C.z = A.z - B.z;
         C.w = A.w - B.w;

         record = record/static_cast<decimal>(10) + C.x + C.y + C.z + C.w;
      }
   }

   static void Normal (picobench::state& s)
   {
      auto Pairs = GeneratePairsOfIndices(s.iterations());
      auto Iter = Pairs.begin();

      for (auto _ : s)
      {
         const Obj& A = PredeterminedObjects[Iter->first];
         const Obj& B = PredeterminedObjects[Iter->second];
         Iter++;

         Obj C = A - B;

         record = record/static_cast<decimal>(10) + C.x + C.y + C.z + C.w;
      }
   }

   static void Moved (picobench::state& s)
   {
      auto Pairs = GeneratePairsOfIndices(s.iterations());
      auto Iter = Pairs.begin();

      for (auto _ : s)
      {
         const Obj& A = PredeterminedObjects[Iter->first];
         const Obj& B = PredeterminedObjects[Iter->second];
         Iter++;

         Obj C = std::move(A - B);

         record = record/static_cast<decimal>(10) + C.x + C.y + C.z + C.w;
      }
   }

   static void Slow (picobench::state& s)
   {
      auto Pairs = GeneratePairsOfIndices(s.iterations());
      auto Iter = Pairs.begin();

      for (auto _ : s)
      {
         const Obj& A = PredeterminedObjects[Iter->first];
         const Obj& B = PredeterminedObjects[Iter->second];
         Iter++;

         Obj C;

         C = A - B;

         record = record/static_cast<decimal>(10) + C.x + C.y + C.z + C.w;
      }
   }
};

template<typename T, const T& MIN, const T& MAX>
std::vector<typename Benchmark<T, MIN, MAX>::Obj> Benchmark<T, MIN, MAX>::PredeterminedObjects = std::vector<typename Benchmark<T, MIN, MAX>::Obj> (ARRAY_SIZE);

template<typename T, const T& MIN, const T& MAX>
typename Benchmark<T, MIN, MAX>::decimal Benchmark<T, MIN, MAX>::record = static_cast<typename Benchmark<T, MIN, MAX>::decimal> (0.0);


const int SAMPLE_SIZE = 400;

/////////////// Double Test/////////////////////
using DoubleBenchmark = Benchmark<double>;

PICOBENCH_SUITE("Double benchmark");
PICOBENCH (DoubleBenchmark::Orig).samples (SAMPLE_SIZE).baseline();
PICOBENCH (DoubleBenchmark::Normal).samples (SAMPLE_SIZE);
PICOBENCH (DoubleBenchmark::Moved).samples (SAMPLE_SIZE);
PICOBENCH (DoubleBenchmark::Slow).samples (SAMPLE_SIZE);


/////////////// Float Test/////////////////////
using FloatBenchmark = Benchmark<float>;

PICOBENCH_SUITE("Float benchmark");
PICOBENCH (FloatBenchmark::Orig).samples (SAMPLE_SIZE).baseline();
PICOBENCH (FloatBenchmark::Normal).samples (SAMPLE_SIZE);
PICOBENCH (FloatBenchmark::Moved).samples (SAMPLE_SIZE);
PICOBENCH (FloatBenchmark::Slow).samples (SAMPLE_SIZE);


/////////////// Int Test/////////////////////
using IntBenchmark = Benchmark<int>;

PICOBENCH_SUITE("Int benchmark");
PICOBENCH (IntBenchmark::Orig).samples (SAMPLE_SIZE).baseline();
PICOBENCH (IntBenchmark::Normal).samples (SAMPLE_SIZE);
PICOBENCH (IntBenchmark::Moved).samples (SAMPLE_SIZE);
PICOBENCH (IntBenchmark::Slow).samples (SAMPLE_SIZE);

/////////////// Int Test/////////////////////
using LongBenchmark = Benchmark<long>;

PICOBENCH_SUITE("Long benchmark");
PICOBENCH (LongBenchmark::Orig).samples (SAMPLE_SIZE).baseline();
PICOBENCH (LongBenchmark::Normal).samples (SAMPLE_SIZE);
PICOBENCH (LongBenchmark::Moved).samples (SAMPLE_SIZE);
PICOBENCH (LongBenchmark::Slow).samples (SAMPLE_SIZE);

int main (int argc, char* argv[])
{
   DoubleBenchmark::GeneratePredeterminedObjects();
   FloatBenchmark::GeneratePredeterminedObjects();
   IntBenchmark::GeneratePredeterminedObjects();
   LongBenchmark::GeneratePredeterminedObjects();

   picobench::runner runner;
   
   runner.parse_cmd_line (argc, argv);
   
   if (runner.should_run ())
   {
      runner.run_benchmarks ();
      auto report = runner.generate_report ();

      report.to_text (std::cout); // Default
      report.to_text_concise (std::cout);
   }

   std::cout << DoubleBenchmark::record << std::endl;
   std::cout << FloatBenchmark::record << std::endl;
   std::cout << IntBenchmark::record << std::endl;
   std::cout << LongBenchmark::record << std::endl;
}