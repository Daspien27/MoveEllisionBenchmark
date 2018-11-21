#include <iostream>

#define PICOBENCH_IMPLEMENT
#include "include/picobench/picobench.hpp"

#include <random>
#include <vector>
#include <limits>
#undef min
#undef max

using decimal = double;

const decimal MIN = static_cast<decimal> (-10000.0);//std::numeric_limits<decimal>::min ();
const decimal MAX = static_cast<decimal> ( 10000.0);//std::numeric_limits<decimal>::max ();

const size_t ARRAY_SIZE = 200;
const int SAMPLE_SIZE = 400;


template<typename T> 
struct dependent_false : std::false_type {};

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

   void ApplyLambda (const Obj& A, const Obj& B)
   {
      auto lambda = [](auto& O, auto& A, auto& B) { O = A - B; };

      lambda(x, A.x, B.x);
      lambda(y, A.y, B.y);
      lambda(z, A.z, B.z);
      lambda(w, A.w, B.w);
   }
   //Obj () = default;
   //Obj (Obj&&) = default;
   //Obj& operator= (Obj&&) = default;
   //Obj& operator= (const Obj&) = default;
   //Obj (const Obj&) = default;

};

Obj global;
std::vector<Obj> PredeterminedObjects;
decimal record;

std::vector<std::pair<size_t, size_t>> GeneratePairsOfIndices (int NumberOfPairs)
{
   std::random_device rd;
   std::uniform_int_distribution<size_t> u(0, ARRAY_SIZE - 1);

   std::vector<std::pair<size_t,size_t>> Pairs;

   Pairs.resize(NumberOfPairs);

   std::generate(std::begin(Pairs), std::end(Pairs), [&u, &rd](){return std::make_pair(u(rd), u(rd));});

   return Pairs;
}

static void Original (picobench::state& s)
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
PICOBENCH (Original).samples (SAMPLE_SIZE).baseline();


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
PICOBENCH (Normal).samples (SAMPLE_SIZE);


static void NormalMoved (picobench::state& s)
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
PICOBENCH (NormalMoved).samples (SAMPLE_SIZE);

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
PICOBENCH (Slow).samples (SAMPLE_SIZE);

/*
static void SlowMoved (picobench::state& s)
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

      //global = std::move(C);
      record = record/static_cast<decimal>(10) + C.x + C.y + C.z + C.w;
   }
}
PICOBENCH (SlowMoved).samples (SAMPLE_SIZE);
*/

void ObjMinusByRef (Obj& Difference, const Obj& Minuend, const Obj& Subtrahend)
{
      Difference.x = Minuend.x - Subtrahend.x;
      Difference.y = Minuend.y - Subtrahend.y;
      Difference.z = Minuend.z - Subtrahend.z;
      Difference.w = Minuend.w - Subtrahend.w;
}

static void FunctionRef (picobench::state& s)
{
   auto Pairs = GeneratePairsOfIndices(s.iterations());
   auto Iter = Pairs.begin();

   for (auto _ : s)
   {
      const Obj& A = PredeterminedObjects[Iter->first];
      const Obj& B = PredeterminedObjects[Iter->second];
      Iter++;

      Obj C;

      ObjMinusByRef (C, A, B);

      record = record/static_cast<decimal>(10) + C.x + C.y + C.z + C.w;
   }
}
PICOBENCH (FunctionRef).samples (SAMPLE_SIZE);


static void LambdaVersion (picobench::state& s)
{
   auto Pairs = GeneratePairsOfIndices(s.iterations());
   auto Iter = Pairs.begin();

   for (auto _ : s)
   {
      const Obj& A = PredeterminedObjects[Iter->first];
      const Obj& B = PredeterminedObjects[Iter->second];
      Iter++;

      Obj C;

      C.ApplyLambda(A, B);

      record = record/static_cast<decimal>(10) + C.x + C.y + C.z + C.w;
   }
}
PICOBENCH (LambdaVersion).samples (SAMPLE_SIZE);


template <typename T>
void GeneratePredeterminedObjects ()
{
   PredeterminedObjects.resize(ARRAY_SIZE);

   std::random_device rd;

   if constexpr (std::is_floating_point_v<T>)
   {
      std::uniform_real_distribution<decimal> u(MIN, MAX);
      std::generate(std::begin(PredeterminedObjects), std::end (PredeterminedObjects), [u, &rd](){return Obj{u(rd), u(rd), u(rd), u(rd)};});

   }
   else if constexpr (std::is_integral_v<T>)
   {
      std::uniform_int_distribution<T> u(MIN, MAX);
      std::generate(std::begin(PredeterminedObjects), std::end (PredeterminedObjects), [u, &rd](){return Obj{u(rd), u(rd), u(rd), u(rd)};});
   }
   else
   {
    //  static_assert(dependent_false<T>, "Please use an arithmetic type.");
   }
}

int main (int argc, char* argv[])
{

   GeneratePredeterminedObjects<decimal> ();

   picobench::runner runner;
   // Optionally parse command line
   
   runner.parse_cmd_line (argc, argv);
   
   if (runner.should_run ()) // Cmd line may have disabled benchmarks
   {
      runner.run_benchmarks ();
      auto report = runner.generate_report ();
      // Then to output the data in the report use
      report.to_text (std::cout); // Default
      // or
      report.to_text_concise (std::cout); // No iterations breakdown
      // or
      //report.to_csv (std::cout); // Otputs in csv format. Most detailed
   }

   std::cout << record << std::endl;
}